#ifndef __SCALE_H__
#define __SCALE_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <types.h>
#include <hw-key.h>

struct surface_t;

#define SCALE_COUNTS_PER_GRAM	(430.0f)
#define SCALE_RAW_FILTER_SIZE	(8)
#define SCALE_SENSOR_FAIL_LIMIT	(100)
#define SCALE_HX711_READ_TIMEOUT_MS	(100)
#define SCALE_TARE_KEY		KEY_NAME_POWER
#define SCALE_CAL_KEY		KEY_NAME_MENU
#define SCALE_CAL_REFERENCE_GRAMS	(500.0f)
#define SCALE_CAL_REFERENCE_MIN_GRAMS	(1.0f)
#define SCALE_CAL_REFERENCE_MAX_GRAMS	(50000.0f)
#define SCALE_COUNTS_PER_GRAM_MIN	(10.0f)
#define SCALE_COUNTS_PER_GRAM_MAX	(50000.0f)
#define SCALE_MAX_DISPLAY_GRAMS	(5000.0f)
#define SCALE_MIN_DISPLAY_GRAMS	(-500.0f)
#define SCALE_STABLE_RAW_SPAN_MAX	(1200)

struct scale_state_t {
	s32_t raw;
	s32_t tare_raw;
	s32_t samples[SCALE_RAW_FILTER_SIZE];
	s64_t sample_sum;
	u32_t sample_idx;
	u32_t sample_count;
	float grams;
	float counts_per_gram;
	float last_reference_grams;
	u32_t frame;
	u32_t sensor_fail_count;
	bool_t sensor_fault;
	bool_t sensor_fault_reported;
	bool_t cal_ready;
	bool_t unstable;
	bool_t overload;
	bool_t invalid_scale;
};

void scale_ui_init(struct surface_t * screen);
void scale_state_init(struct scale_state_t * state);
void scale_state_bootstrap(struct scale_state_t * state);
bool_t scale_update_raw(struct scale_state_t * state);
bool_t scale_handle_keydown(struct scale_state_t * state, u32_t keydown);
bool_t scale_calibrate(struct scale_state_t * state);
void scale_update_grams(struct scale_state_t * state);
void scale_render(struct surface_t * screen, struct scale_state_t * state);
void scale_print_banner(void);

#ifdef __cplusplus
}
#endif

#endif /* __SCALE_H__ */