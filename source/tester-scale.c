#include <main.h>
#include <scale.h>

#define SCALE_SAMPLE_PERIOD_MS	(50)
#define SCALE_KEY_PERIOD_MS		(10)
#define SCALE_RENDER_PERIOD_MS	(80)

static u32_t ms_to_ticks(u32_t ms)
{
	u32_t hz;
	u32_t ticks;

	hz = get_system_hz();
	if(hz == 0)
		return 1;

	ticks = (ms * hz + 999) / 1000;
	if(ticks == 0)
		ticks = 1;

	return ticks;
}

static void scale_log_state(struct scale_state_t * state)
{
	if(!state)
		return;

	if(state->sensor_fault || state->sensor_fail_count > 0)
		serial_printf(2, "[SCALE] raw=%ld grams=%.1f fault=%u fail=%u\r\n",
			(long)state->raw,
			state->grams,
			(unsigned)state->sensor_fault,
			(unsigned)state->sensor_fail_count);
}

int tester_scale(int argc, char * argv[])
{
	struct scale_state_t state;
	u32_t keyup, keydown;
	struct surface_t * screen;
	u32_t now;
	u32_t sample_period;
	u32_t key_period;
	u32_t render_period;
	u32_t next_sample;
	u32_t next_key;
	u32_t next_render;

	(void)argc;
	(void)argv;

	hx711_init();
	hx711_set_scale(SCALE_COUNTS_PER_GRAM);

	screen = s5pv210_screen_surface();
	scale_ui_init(screen);
	scale_state_init(&state);
	scale_state_bootstrap(&state);
	scale_print_banner();

	sample_period = ms_to_ticks(SCALE_SAMPLE_PERIOD_MS);
	key_period = ms_to_ticks(SCALE_KEY_PERIOD_MS);
	render_period = ms_to_ticks(SCALE_RENDER_PERIOD_MS);

	next_sample = jiffies;
	next_key = jiffies;
	next_render = jiffies;

	while(1)
	{
		now = jiffies;

		if(time_after_eq(now, next_sample))
		{
			scale_update_raw(&state);
			if(!state.sensor_fault)
				scale_update_grams(&state);
			scale_log_state(&state);
			next_sample = now + sample_period;
		}

		if(time_after_eq(now, next_key))
		{
			if(get_key_event(&keyup, &keydown))
			{
				(void)keyup;
				scale_handle_keydown(&state, keydown);
			}
			next_key = now + key_period;
		}

		if(time_after_eq(now, next_render))
		{
			/* Render screen (screen is 1024x600) */
			s5pv210_screen_swap();
			screen = s5pv210_screen_surface();
			scale_render(screen, &state);

			s5pv210_screen_flush();

			state.frame++;
			next_render = now + render_period;
		}
	}

	return 0;
}
