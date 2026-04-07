#include <malloc.h>
#include <stdio.h>
#include <stdarg.h>
#include <sizes.h>
#include <s5pv210-serial.h>
#include <s5pv210-serial-stdio.h>

extern s32_t cx;
extern s32_t cy;

static void serial_track_cursor_for_readline(const char * s)
{
	unsigned char ch;

	if(!s)
		return;

	while(*s)
	{
		ch = (unsigned char)(*s++);

		switch(ch)
		{
		case '\r':
			cx = 0;
			break;

		case '\n':
			if(cy + 1 < 24)
				cy = cy + 1;
			break;

		case '\t':
		{
			s32_t n = 8 - (cx % 8);
			if(n + cx >= 80)
				n = 80 - cx - 1;
			if(n > 0)
				cx = cx + n;
			break;
		}

		default:
			if(ch >= 0x20)
			{
				if(cx + 1 < 80)
					cx = cx + 1;
				else
				{
					if(cy + 1 < 24)
						cy = cy + 1;
					cx = 0;
				}
			}
			break;
		}
	}
}

int serial_printf(int ch, const char * fmt, ...)
{
	va_list ap;
	va_list ap_len;
	va_list ap_fmt;
	char * buf;
	int len;
	int rv;

	va_start(ap, fmt);
	va_copy(ap_len, ap);
	len = vsnprintf(NULL, 0, fmt, ap_len);
	va_end(ap_len);
	if(len < 0)
	{
		va_end(ap);
		return 0;
	}
	buf = malloc(len + 1);
	if(!buf)
	{
		va_end(ap);
		return 0;
	}
	va_copy(ap_fmt, ap);
	rv = vsnprintf(buf, len + 1, fmt, ap_fmt);
	va_end(ap_fmt);
	va_end(ap);

	rv = (s5pv210_serial_write_string(ch, buf) < 0) ? 0 : rv;
	if(rv > 0 && ch == 2)
		serial_track_cursor_for_readline(buf);
	free(buf);

	return rv;
}
