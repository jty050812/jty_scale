#ifndef __GRAPHIC_TEXT_H__
#define __GRAPHIC_TEXT_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <types.h>
#include <graphic/surface.h>

void screen_putc(struct surface_t * screen, s32_t x, s32_t y, s32_t scale, char ch, u32_t color);
void screen_print(struct surface_t * screen, s32_t x, s32_t y, s32_t scale, u32_t color, const char * text);
void screen_printf(struct surface_t * screen, s32_t x, s32_t y, s32_t scale, u32_t color, const char * fmt, ...);

#ifdef __cplusplus
}
#endif

#endif /* __GRAPHIC_TEXT_H__ */
