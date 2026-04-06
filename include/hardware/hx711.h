#ifndef __HX711_H__
#define __HX711_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <types.h>

/*
 * Default wiring (adjust if your board uses different pins):
 * HX711 DOUT -> GPB_2 (SPI0_MISO)
 * HX711 SCK  -> GPB_0 (SPI0_CLK)
 * Note: Configure as GPIO mode (0x0) to override SPI functionality
 */
#define HX711_DOUT_PIN		(2)
#define HX711_SCK_PIN		(0)

void hx711_init(void);
bool_t hx711_read_raw(s32_t * raw, u32_t timeout_ms);
void hx711_set_scale(float counts_per_gram);
float hx711_get_scale(void);
float hx711_raw_to_grams(s32_t raw);

#ifdef __cplusplus
}
#endif

#endif /* __HX711_H__ */
