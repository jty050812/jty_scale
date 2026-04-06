#include <malloc.h>
#include <stdio.h>
#include <sizes.h>
#include <s5pv210-serial.h>
#include <s5pv210-serial-stdio.h>

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
	free(buf);

	return rv;
}
