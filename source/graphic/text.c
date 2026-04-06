#include <graphic/text.h>
#include <stdio.h>
#include <stdarg.h>

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

static const u8_t * glyph5x7(char ch)
{
	static const u8_t g0[7] = {0x0e, 0x11, 0x13, 0x15, 0x19, 0x11, 0x0e};
	static const u8_t g1[7] = {0x04, 0x0c, 0x14, 0x04, 0x04, 0x04, 0x1f};
	static const u8_t g2[7] = {0x0e, 0x11, 0x01, 0x06, 0x08, 0x10, 0x1f};
	static const u8_t g3[7] = {0x1e, 0x01, 0x01, 0x06, 0x01, 0x01, 0x1e};
	static const u8_t g4[7] = {0x02, 0x06, 0x0a, 0x12, 0x1f, 0x02, 0x02};
	static const u8_t g5[7] = {0x1f, 0x10, 0x10, 0x1e, 0x01, 0x01, 0x1e};
	static const u8_t g6[7] = {0x0e, 0x10, 0x10, 0x1e, 0x11, 0x11, 0x0e};
	static const u8_t g7[7] = {0x1f, 0x01, 0x02, 0x04, 0x08, 0x08, 0x08};
	static const u8_t g8[7] = {0x0e, 0x11, 0x11, 0x0e, 0x11, 0x11, 0x0e};
	static const u8_t g9[7] = {0x0e, 0x11, 0x11, 0x0f, 0x01, 0x01, 0x0e};
	static const u8_t gA[7] = {0x04, 0x0a, 0x11, 0x11, 0x1f, 0x11, 0x11};
	static const u8_t gE[7] = {0x1f, 0x10, 0x10, 0x1e, 0x10, 0x10, 0x1f};
	static const u8_t gG[7] = {0x0e, 0x11, 0x10, 0x17, 0x11, 0x11, 0x0e};
	static const u8_t gH[7] = {0x11, 0x11, 0x11, 0x1f, 0x11, 0x11, 0x11};
	static const u8_t gI[7] = {0x1f, 0x04, 0x04, 0x04, 0x04, 0x04, 0x1f};
	static const u8_t gO[7] = {0x0e, 0x11, 0x11, 0x11, 0x11, 0x11, 0x0e};
	static const u8_t gP[7] = {0x1e, 0x11, 0x11, 0x1e, 0x10, 0x10, 0x10};
	static const u8_t gR[7] = {0x1e, 0x11, 0x11, 0x1e, 0x14, 0x12, 0x11};
	static const u8_t gS[7] = {0x0f, 0x10, 0x10, 0x0e, 0x01, 0x01, 0x1e};
	static const u8_t gT[7] = {0x1f, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04};
	static const u8_t gW[7] = {0x11, 0x11, 0x11, 0x15, 0x15, 0x15, 0x0a};
	static const u8_t gColon[7] = {0x00, 0x04, 0x04, 0x00, 0x04, 0x04, 0x00};
	static const u8_t gDot[7] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x06, 0x06};
	static const u8_t gMinus[7] = {0x00, 0x00, 0x00, 0x1f, 0x00, 0x00, 0x00};
	static const u8_t gSpace[7] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};

	if(ch >= 'a' && ch <= 'z')
		ch = ch - 'a' + 'A';

	switch(ch)
	{
	case '0': return g0;
	case '1': return g1;
	case '2': return g2;
	case '3': return g3;
	case '4': return g4;
	case '5': return g5;
	case '6': return g6;
	case '7': return g7;
	case '8': return g8;
	case '9': return g9;
	case 'A': return gA;
	case 'E': return gE;
	case 'G': return gG;
	case 'H': return gH;
	case 'I': return gI;
	case 'O': return gO;
	case 'P': return gP;
	case 'R': return gR;
	case 'S': return gS;
	case 'T': return gT;
	case 'W': return gW;
	case ':': return gColon;
	case '.': return gDot;
	case '-': return gMinus;
	case ' ': return gSpace;
	default:
		return gSpace;
	}
}

void screen_putc(struct surface_t * screen, s32_t x, s32_t y, s32_t scale, char ch, u32_t color)
{
	s32_t row, col;
	const u8_t * g = glyph5x7(ch);

	for(row = 0; row < 7; row++)
	{
		for(col = 0; col < 5; col++)
		{
			if(g[row] & (1U << (4 - col)))
				fill_rect(screen, x + col * scale, y + row * scale, scale, scale, color);
		}
	}
}

void screen_print(struct surface_t * screen, s32_t x, s32_t y, s32_t scale, u32_t color, const char * text)
{
	s32_t i;

	for(i = 0; text[i] != '\0'; i++)
		screen_putc(screen, x + i * (6 * scale), y, scale, text[i], color);
}

void screen_printf(struct surface_t * screen, s32_t x, s32_t y, s32_t scale, u32_t color, const char * fmt, ...)
{
	char out[96];
	va_list ap;

	va_start(ap, fmt);
	vsnprintf(out, sizeof(out), fmt, ap);
	va_end(ap);

	screen_print(screen, x, y, scale, color, out);
}
