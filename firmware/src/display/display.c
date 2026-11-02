#include <stdio.h>
#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/drivers/display.h>
#include <zephyr/logging/log.h>
#include <lvgl.h>

#include "display/display.h"
#include "core/app_state.h"
#include "core/format.h"

LOG_MODULE_REGISTER(display, LOG_LEVEL_INF);

static const struct device *disp = DEVICE_DT_GET(DT_CHOSEN(zephyr_display));
static bool ready;

static lv_obj_t *lbl_soil;
static lv_obj_t *lbl_air;
static lv_obj_t *lbl_water;
static lv_obj_t *lbl_status;

static lv_obj_t *make_label(lv_obj_t *parent, int y)
{
	lv_obj_t *label = lv_label_create(parent);

	lv_obj_set_style_text_color(label, lv_color_white(), 0);
	lv_obj_align(label, LV_ALIGN_TOP_LEFT, 8, y);
	lv_label_set_text(label, "");
	return label;
}

int display_init(void)
{
	if (!device_is_ready(disp)) {
		LOG_ERR("Skjermen er ikke klar");
		return -ENODEV;
	}

	lv_obj_t *scr = lv_screen_active();
	lv_obj_set_style_bg_color(scr, lv_color_black(), 0);

	lv_obj_t *title = make_label(scr, 4);
	lv_obj_set_style_text_color(title, lv_palette_main(LV_PALETTE_GREEN), 0);
	lv_label_set_text(title, "Vanningssystem");

	lbl_soil   = make_label(scr, 30);
	lbl_air    = make_label(scr, 55);
	lbl_water  = make_label(scr, 80);
	lbl_status = make_label(scr, 105);

	display_blanking_off(disp);
	ready = true;
	lv_timer_handler();
	return 0;
}

void display_update(void)
{
	struct app_state s;
	char buf[64], t[16], h[16];

	if (!ready) {
		return;
	}
	app_state_snapshot(&s);

	if (s.soil_ok) {
		snprintf(buf, sizeof(buf), "Jord: %d %%  (grense %d %%)", s.soil_pct, s.threshold_pct);
	} else {
		snprintf(buf, sizeof(buf), "Jord: SENSORFEIL");
	}
	lv_label_set_text(lbl_soil, buf);

	if (s.aht_ok) {
		fmt_tenths(t, sizeof(t), s.temp_c10);
		fmt_tenths(h, sizeof(h), s.hum_pct10);
		snprintf(buf, sizeof(buf), "Luft: %s C   %s %%", t, h);
	} else {
		snprintf(buf, sizeof(buf), "Luft: ingen AHT20");
	}
	lv_label_set_text(lbl_air, buf);

	bool water_alarm = !s.water_ok || !s.water_sensor_ok;
	lv_label_set_text(lbl_water, !s.water_sensor_ok ? "Vann: SENSORFEIL" :
				     s.water_ok ? "Vann: OK" : "Vann: LAVT - fyll pa!");
	lv_obj_set_style_text_color(lbl_water,
		water_alarm ? lv_palette_main(LV_PALETTE_RED) : lv_color_white(), 0);

	snprintf(buf, sizeof(buf), "Pumpe: %s  WiFi:%s MQTT:%s",
		 s.pump_on ? "PA" : "AV", s.wifi_ok ? "ok" : "-", s.mqtt_ok ? "ok" : "-");
	lv_label_set_text(lbl_status, buf);

	lv_timer_handler();
}
