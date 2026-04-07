#include <main.h>
#include <scale.h>
#include <graphic/color.h>
#include <graphic/text.h>
#include <readline.h>
#include <stdio.h>

struct scale_ui_colors_t {
	u32_t bg;
	u32_t fg;
	u32_t accent;
	u32_t dim;
	u32_t warn;
};

static struct scale_ui_colors_t g_colors;
static u32_t g_cal_confirm_deadline = 0;

static const char * scale_fsm_state_name(enum scale_fsm_state_t state)
{
	switch(state)
	{
	case SCALE_FSM_BOOT:
		return "BOOT";
	case SCALE_FSM_WEIGHING:
		return "WEIGHING";
	case SCALE_FSM_TARE_DONE:
		return "TARE_DONE";
	case SCALE_FSM_CAL_PENDING:
		return "CAL_PENDING";
	case SCALE_FSM_CAL_DONE:
		return "CAL_DONE";
	case SCALE_FSM_FAULT:
		return "FAULT";
	default:
		return "UNKNOWN";
	}
}

static void scale_fsm_set_state(struct scale_state_t * state, enum scale_fsm_state_t next, const char * reason)
{
	if(!state)
		return;

	if(state->fsm_state == next)
		return;

	serial_printf(2, "[FSM] %s -> %s (%s)\r\n",
		scale_fsm_state_name(state->fsm_state),
		scale_fsm_state_name(next),
		reason ? reason : "none");
	state->fsm_state = next;
}

static u32_t map_rgb(struct surface_t * screen, u8_t r, u8_t g, u8_t b)
{
	struct color_t c;

	color_init_rgb(&c, r, g, b);
	return surface_map_color(screen, &c);
}

static void fill_rect(struct surface_t * screen, s32_t x, s32_t y, s32_t w, s32_t h, u32_t color)
{
	struct rect_t rect;

	if(w <= 0 || h <= 0)
		return;

	rect.x = x;
	rect.y = y;
	rect.w = w;
	rect.h = h;
	surface_fill(screen, &rect, color, BLEND_MODE_REPLACE);
}

static void init_colors(struct surface_t * screen)
{
	g_colors.bg = map_rgb(screen, 10, 18, 28);
	g_colors.fg = map_rgb(screen, 92, 239, 160);
	g_colors.accent = map_rgb(screen, 255, 168, 65);
	g_colors.dim = map_rgb(screen, 22, 45, 35);
	g_colors.warn = map_rgb(screen, 231, 86, 86);
}

static void scale_reset_filter(struct scale_state_t * state)
{
	u32_t i;

	if(!state)
		return;

	state->sample_sum = 0;
	state->sample_idx = 0;
	state->sample_count = 0;

	for(i = 0; i < SCALE_RAW_FILTER_SIZE; i++)
		state->samples[i] = 0;
}

static float scale_get_counts_per_gram(struct scale_state_t * state)
{
	if(!state)
		return SCALE_COUNTS_PER_GRAM;

	if(state->counts_per_gram > 1e-6f)
		return state->counts_per_gram;

	return SCALE_COUNTS_PER_GRAM;
}

static s32_t scale_get_trimmed_average(struct scale_state_t * state)
{
	u32_t i;
	u32_t count;
	s32_t min_value;
	s32_t max_value;
	s64_t total;

	if(!state)
		return 0;

	count = state->sample_count;
	if(count == 0)
		return 0;

	min_value = state->samples[0];
	max_value = state->samples[0];
	total = 0;

	for(i = 0; i < count; i++)
	{
		s32_t value = state->samples[i];

		total += value;
		if(value < min_value)
			min_value = value;
		if(value > max_value)
			max_value = value;
	}

	if(count >= 4)
	{
		total -= (s64_t)min_value + (s64_t)max_value;
		count -= 2;
	}

	return (s32_t)(total / (s64_t)count);
}

static s32_t scale_get_sample_span(struct scale_state_t * state)
{
	u32_t i;
	u32_t count;
	s32_t min_value;
	s32_t max_value;

	if(!state)
		return 0;

	count = state->sample_count;
	if(count == 0)
		return 0;

	min_value = state->samples[0];
	max_value = state->samples[0];
	for(i = 1; i < count; i++)
	{
		if(state->samples[i] < min_value)
			min_value = state->samples[i];
		if(state->samples[i] > max_value)
			max_value = state->samples[i];
	}

	return max_value - min_value;
}

static void scale_mark_sensor_fault(struct scale_state_t * state)
{
	if(!state)
		return;

	state->sensor_fault = 1;
	state->grams = 0.0f;

	if(!state->sensor_fault_reported)
	{
		serial_printf(2, "[SCALE] HX711 read timeout or sensor disconnected.\r\n");
		state->sensor_fault_reported = 1;
	}

	scale_fsm_set_state(state, SCALE_FSM_FAULT, "sensor fault");
}

static void scale_clear_sensor_fault(struct scale_state_t * state)
{
	if(!state)
		return;

	if(state->sensor_fault)
		serial_printf(2, "[SCALE] HX711 recovered.\r\n");

	state->sensor_fault = 0;
	state->sensor_fault_reported = 0;
	state->sensor_fail_count = 0;
	scale_fsm_set_state(state, SCALE_FSM_WEIGHING, "sensor recovered");
}

void scale_ui_init(struct surface_t * screen)
{
	if(!screen)
		return;

	init_colors(screen);
}

void scale_state_init(struct scale_state_t * state)
{
	u32_t i;

	if(!state)
		return;

	state->raw = 0;
	state->tare_raw = 0;
	state->sample_sum = 0;
	state->sample_idx = 0;
	state->sample_count = 0;
	state->grams = 0.0f;
	state->counts_per_gram = SCALE_COUNTS_PER_GRAM;
	state->last_reference_grams = SCALE_CAL_REFERENCE_GRAMS;
	state->frame = 0;
	state->sensor_fail_count = 0;
	state->sensor_fault = 0;
	state->sensor_fault_reported = 0;
	state->cal_ready = 0;
	state->unstable = 1;
	state->overload = 0;
	state->invalid_scale = 0;
	state->fsm_state = SCALE_FSM_BOOT;

	for(i = 0; i < SCALE_RAW_FILTER_SIZE; i++)
		state->samples[i] = 0;
}

void scale_state_bootstrap(struct scale_state_t * state)
{
	if(!state)
		return;

	scale_reset_filter(state);

	if(hx711_read_raw(&state->raw, 1000))
	{
		state->tare_raw = state->raw;
		state->sensor_fail_count = 0;
		state->sensor_fault = 0;
		state->sensor_fault_reported = 0;
		state->cal_ready = 0;
		state->last_reference_grams = SCALE_CAL_REFERENCE_GRAMS;
		state->unstable = 1;
		state->overload = 0;
		state->invalid_scale = 0;
		scale_fsm_set_state(state, SCALE_FSM_WEIGHING, "bootstrap ok");
	}
	else
	{
		scale_mark_sensor_fault(state);
	}
}

bool_t scale_update_raw(struct scale_state_t * state)
{
	if(!state)
		return 0;

	if(hx711_read_raw(&state->raw, SCALE_HX711_READ_TIMEOUT_MS))
	{
		s32_t previous_raw;
		s32_t filtered;

		previous_raw = state->raw;
		state->sample_sum -= state->samples[state->sample_idx];
		state->samples[state->sample_idx] = state->raw;
		state->sample_sum += state->samples[state->sample_idx];
		state->sample_idx = (state->sample_idx + 1) % SCALE_RAW_FILTER_SIZE;
		if(state->sample_count < SCALE_RAW_FILTER_SIZE)
			state->sample_count++;
		filtered = scale_get_trimmed_average(state);
		if(state->sample_count >= 4)
			state->raw = (previous_raw * 3 + filtered) / 4;
		else
			state->raw = filtered;
		if(state->sample_count >= 4)
			state->unstable = (scale_get_sample_span(state) > SCALE_STABLE_RAW_SPAN_MAX) ? 1 : 0;
		else
			state->unstable = 1;
		state->sensor_fail_count = 0;

		if(state->sensor_fault)
			scale_clear_sensor_fault(state);

		if(state->fsm_state == SCALE_FSM_BOOT ||
			state->fsm_state == SCALE_FSM_TARE_DONE ||
			state->fsm_state == SCALE_FSM_CAL_DONE)
		{
			scale_fsm_set_state(state, SCALE_FSM_WEIGHING, "stable update");
		}

		if(state->fsm_state == SCALE_FSM_CAL_PENDING &&
			g_cal_confirm_deadline != 0 &&
			time_after(jiffies, g_cal_confirm_deadline))
		{
			g_cal_confirm_deadline = 0;
			scale_fsm_set_state(state, SCALE_FSM_WEIGHING, "cal confirm timeout");
		}

		return 1;
	}

	if(state->sensor_fail_count < 0xffffffffu)
		state->sensor_fail_count++;

	if(state->sensor_fail_count >= SCALE_SENSOR_FAIL_LIMIT)
		scale_mark_sensor_fault(state);

	return 0;
}

static bool_t scale_commit_calibration(struct scale_state_t * state, float reference_grams)
{
	s32_t delta;
	float counts_per_gram;

	if(!state)
		return 0;

	if(reference_grams < SCALE_CAL_REFERENCE_MIN_GRAMS || reference_grams > SCALE_CAL_REFERENCE_MAX_GRAMS)
	{
		serial_printf(2, "[CAL] Reference out of range: %.2f g\r\n", reference_grams);
		return 0;
	}

	if(state->sample_count < 4)
	{
		serial_printf(2, "[CAL] Not enough samples yet. Please wait.\r\n");
		return 0;
	}

	if(state->unstable)
	{
		serial_printf(2, "[CAL] Scale is unstable, keep weight still and retry.\r\n");
		return 0;
	}

	delta = state->raw - state->tare_raw;
	if(delta <= 0)
	{
		serial_printf(2, "[CAL] Invalid calibration: raw delta is not positive.\r\n");
		return 0;
	}

	counts_per_gram = (float)delta / reference_grams;
	if(counts_per_gram < SCALE_COUNTS_PER_GRAM_MIN || counts_per_gram > SCALE_COUNTS_PER_GRAM_MAX)
	{
		serial_printf(2, "[CAL] Invalid factor %.2f counts/gram\r\n", counts_per_gram);
		return 0;
	}

	state->counts_per_gram = counts_per_gram;
	state->last_reference_grams = reference_grams;
	hx711_set_scale(counts_per_gram);
	state->cal_ready = 1;
	state->invalid_scale = 0;
	scale_fsm_set_state(state, SCALE_FSM_CAL_DONE, "calibration success");
	serial_printf(2, "[CAL] Reference: %.2f g, factor: %.2f counts/gram\r\n", reference_grams, counts_per_gram);
	return 1;
}

static bool_t scale_prompt_reference_grams(float * reference_grams)
{
	char line[32];
	char * end;
	double value;
	size_t len;
	u8_t ch;

	if(!reference_grams)
		return 0;

	serial_printf(2, "[CAL] Put known weight on scale, then type grams in serial.\r\n");
	serial_printf(2, "CAL REF (g)> ");

	len = 0;
	for(;;)
	{
		if(s5pv210_serial_read(2, &ch, 1) != 1)
			continue;

		if(ch == '\r' || ch == '\n')
		{
			s5pv210_serial_write_string(2, "\r\n");
			line[len] = '\0';
			break;
		}

		if(ch == 0x08 || ch == 0x7f)
		{
			if(len > 0)
			{
				len--;
				s5pv210_serial_write_string(2, "\b \b");
			}
			continue;
		}

		if(ch < 0x20 || ch > 0x7e)
			continue;

		if(len + 1 < sizeof(line))
		{
			line[len++] = (char)ch;
			s5pv210_serial_write(2, &ch, 1);
		}
	}

	value = strtod(line, &end);
	while(*end == ' ' || *end == '\t')
		end++;

	if(end == line || *end != '\0' || value <= 0.0)
	{
		serial_printf(2, "[CAL] Invalid input. Example: 200 or 500.0\r\n");
		return 0;
	}

	if(value < SCALE_CAL_REFERENCE_MIN_GRAMS || value > SCALE_CAL_REFERENCE_MAX_GRAMS)
	{
		serial_printf(2, "[CAL] Input out of range: %.2f g\r\n", (float)value);
		return 0;
	}

	*reference_grams = (float)value;
	return 1;
}

bool_t scale_handle_keydown(struct scale_state_t * state, u32_t keydown)
{
	float reference_grams;
	u32_t now;
	u32_t confirm_ticks;

	if(!state)
		return 0;

	if((keydown & SCALE_TARE_KEY) != 0)
	{
		g_cal_confirm_deadline = 0;

		if(state->sensor_fault)
		{
			serial_printf(2, "[TARE] Ignored while HX711 is in fault state.\r\n");
			return 1;
		}

		state->tare_raw = state->raw;
		beep_set_status(BEEP_STATUS_ON);
		mdelay(40);
		beep_set_status(BEEP_STATUS_OFF);
		mdelay(40);
		beep_set_status(BEEP_STATUS_ON);
		mdelay(40);
		beep_set_status(BEEP_STATUS_OFF);
		state->grams = 0.0f;
		state->cal_ready = 0;
		state->overload = 0;
		scale_fsm_set_state(state, SCALE_FSM_TARE_DONE, "tare key");
		serial_printf(2, "[TARE] Zero point set.\r\n");
		return 1;
	}

	if((keydown & SCALE_CAL_KEY) != 0)
	{
		if(state->sensor_fault)
		{
			serial_printf(2, "[CAL] Ignored while HX711 is in fault state.\r\n");
			return 1;
		}

		now = jiffies;
		if(g_cal_confirm_deadline != 0 && time_before_eq(now, g_cal_confirm_deadline))
		{
			g_cal_confirm_deadline = 0;
		}
		else
		{
			confirm_ticks = get_system_hz();
			if(confirm_ticks == 0)
				confirm_ticks = 100;
			g_cal_confirm_deadline = now + confirm_ticks;
			scale_fsm_set_state(state, SCALE_FSM_CAL_PENDING, "wait confirm");
			serial_printf(2, "[CAL] Press MENU again within 1s to confirm.\r\n");
			return 1;
		}

		if(scale_prompt_reference_grams(&reference_grams))
			return scale_commit_calibration(state, reference_grams);

		scale_fsm_set_state(state, SCALE_FSM_WEIGHING, "calibration canceled");

		return 1;
	}

	return 0;
}

bool_t scale_calibrate(struct scale_state_t * state)
{
	if(!state)
		return 0;

	return scale_commit_calibration(state, state->last_reference_grams);
}

void scale_update_grams(struct scale_state_t * state)
{
	float cpg;

	if(!state)
		return;

	cpg = scale_get_counts_per_gram(state);
	if(cpg < SCALE_COUNTS_PER_GRAM_MIN || cpg > SCALE_COUNTS_PER_GRAM_MAX)
	{
		state->invalid_scale = 1;
		state->grams = 0.0f;
		return;
	}

	state->invalid_scale = 0;
	state->grams = (state->raw - state->tare_raw) / cpg;
	if(state->grams < 0.3f && state->grams > -0.3f)
		state->grams = 0.0f;

	if(state->grams > SCALE_MAX_DISPLAY_GRAMS || state->grams < SCALE_MIN_DISPLAY_GRAMS)
	{
		state->overload = 1;
		if(state->grams > SCALE_MAX_DISPLAY_GRAMS)
			state->grams = SCALE_MAX_DISPLAY_GRAMS;
		else
			state->grams = SCALE_MIN_DISPLAY_GRAMS;
	}
	else
	{
		state->overload = 0;
	}
}

void scale_render(struct surface_t * screen, struct scale_state_t * state)
{
	if(!screen || !state)
		return;

	fill_rect(screen, 0, 0, screen->w, screen->h, g_colors.bg);

	screen_printf(screen, 420, 24, 4, g_colors.dim, "Simple Scale");

	if(state->sensor_fault)
	{
		screen_printf(screen, 310, 90, 7, g_colors.warn, "HX711 ERROR");
		screen_printf(screen, 190, 190, 4, g_colors.dim, "CHECK SENSOR WIRING / POWER");
		screen_printf(screen, 245, 255, 4, g_colors.warn, "NO VALID DATA");
		screen_printf(screen, 280, 345, 4, g_colors.warn, "PRESS POWER HAS NO EFFECT");
	}
	else
	{
		screen_printf(screen, 380, 90, 10, g_colors.fg, "%7.2f", state->grams);
		screen_printf(screen, 770, 120, 5, g_colors.accent, "g");
		screen_printf(screen, 260, 190, 4, g_colors.dim, "POWER:TARE  MENU:CAL(SERIAL)");
		screen_printf(screen, 280, 250, 4, g_colors.dim, "LAST REF: %.1fg", state->last_reference_grams);
		screen_printf(screen, 250, 220, 4, g_colors.dim, "STATE: %s", scale_fsm_state_name(state->fsm_state));
		if(state->cal_ready)
			screen_printf(screen, 330, 300, 4, g_colors.accent, "CAL OK");
		else
			screen_printf(screen, 300, 300, 4, g_colors.warn, "CAL PENDING");

		if(state->unstable)
			screen_printf(screen, 300, 345, 4, g_colors.warn, "UNSTABLE");
		else if(state->overload)
			screen_printf(screen, 300, 345, 4, g_colors.warn, "OVERLOAD");
		else if(state->invalid_scale)
			screen_printf(screen, 260, 345, 4, g_colors.warn, "SCALE FACTOR ERROR");

		if((state->frame & 0x08) && state->grams < 0.5f)
			screen_printf(screen, 520, 345, 4, g_colors.warn, "TARE");
		else if((state->frame & 0x10))
			screen_printf(screen, 460, 345, 4, g_colors.dim, "PRESS POWER");
	}
}

void scale_print_banner(void)
{
	serial_printf(2, "========================================\r\n");
	serial_printf(2, "  Electronic Scale v1.0\r\n");
	serial_printf(2, "========================================\r\n");
	serial_printf(2, "Scale factor: %.2f counts/gram\r\n", hx711_get_scale());
	serial_printf(2, "POWER:TARE\r\n");
	serial_printf(2, "MENU:CALIBRATE (INPUT REFERENCE GRAMS IN SERIAL)\r\n");
	serial_printf(2, "Default ref hint: %.2fg\r\n", SCALE_CAL_REFERENCE_GRAMS);
	serial_printf(2, "Ref range: %.1f g to %.1f g\r\n", SCALE_CAL_REFERENCE_MIN_GRAMS, SCALE_CAL_REFERENCE_MAX_GRAMS);
	serial_printf(2, "Display range: %.1f g to %.1f g\r\n", SCALE_MIN_DISPLAY_GRAMS, SCALE_MAX_DISPLAY_GRAMS);
	serial_printf(2, "HX711 DOUT: GPB_2, SCK: GPB_0\r\n");
	serial_printf(2, "========================================\r\n");
}