#include <hx711.h>
#include <io.h>
#include <s5pv210/reg-gpio.h>
#include <s5pv210-tick-delay.h>

#define HX711_GPIO_CON		S5PV210_GPBCON
#define HX711_GPIO_DAT		S5PV210_GPBDAT
#define HX711_GPIO_PUD		S5PV210_GPBPUD

static float g_counts_per_gram = 430.0f;

static void gpio_cfg_input(u32_t pin)
{
	u32_t v;

	v = readl(HX711_GPIO_CON);
	v &= ~(0xf << (pin * 4));
	writel(HX711_GPIO_CON, v);
}

static void gpio_cfg_output(u32_t pin)
{
	u32_t v;

	v = readl(HX711_GPIO_CON);
	v &= ~(0xf << (pin * 4));
	v |= (0x1 << (pin * 4));
	writel(HX711_GPIO_CON, v);
}

static void gpio_set(u32_t pin, bool_t high)
{
	u32_t v;

	v = readl(HX711_GPIO_DAT);
	if(high)
		v |= (0x1 << pin);
	else
		v &= ~(0x1 << pin);
	writel(HX711_GPIO_DAT, v);
}

static bool_t gpio_get(u32_t pin)
{
	return (readl(HX711_GPIO_DAT) & (0x1 << pin)) ? 1 : 0;
}

void hx711_init(void)
{
	u32_t v;

	gpio_cfg_input(HX711_DOUT_PIN);
	gpio_cfg_output(HX711_SCK_PIN);
	gpio_set(HX711_SCK_PIN, 0);

	/* Pull-up DOUT for cleaner idle level. */
	v = readl(HX711_GPIO_PUD);
	v &= ~(0x3 << (HX711_DOUT_PIN * 2));
	v |= (0x2 << (HX711_DOUT_PIN * 2));
	writel(HX711_GPIO_PUD, v);
}

bool_t hx711_read_raw(s32_t * raw, u32_t timeout_ms)
{
	u32_t i;
	u32_t data;

	if(!raw)
		return 0;

	for(i = 0; i < timeout_ms; i++)
	{
		if(!gpio_get(HX711_DOUT_PIN))
			break;
		mdelay(1);
	}
	if(i >= timeout_ms)
		return 0;

	data = 0;
	for(i = 0; i < 24; i++)
	{
		gpio_set(HX711_SCK_PIN, 1);
		udelay(1);
		data = (data << 1) | (gpio_get(HX711_DOUT_PIN) ? 1 : 0);
		gpio_set(HX711_SCK_PIN, 0);
		udelay(1);
	}

	/* Select channel A, gain 128. */
	gpio_set(HX711_SCK_PIN, 1);
	udelay(1);
	gpio_set(HX711_SCK_PIN, 0);
	udelay(1);

	if(data & 0x800000)
		data |= 0xff000000;

	*raw = (s32_t)data;
	return 1;
}

void hx711_set_scale(float counts_per_gram)
{
	if(counts_per_gram > 1e-6f)
		g_counts_per_gram = counts_per_gram;
}

float hx711_get_scale(void)
{
	return g_counts_per_gram;
}

float hx711_raw_to_grams(s32_t raw)
{
	return (float)raw / g_counts_per_gram;
}
