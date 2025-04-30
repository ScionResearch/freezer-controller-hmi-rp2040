#include "screenMain.h"

// Global assets
lv_obj_t * screenMain;

lv_obj_t * labelTempSP;
lv_obj_t * labelTempPV;
lv_obj_t * arcSP;
lv_obj_t * arcPV;

lv_obj_t * labelHumidPV;
lv_obj_t * arcHumidPV;
lv_obj_t * labelPressPV;

lv_obj_t * imgLink;
lv_obj_t * imgEth;
lv_obj_t * imgCycle;
lv_obj_t * imgBell;
lv_obj_t * imgLock;
lv_obj_t * imgUnlock;   

lv_obj_t * alarmBox;
lv_obj_t * alarmText;
lv_obj_t * btnAlarmAck;

lv_color_t colourLink;
lv_color_t colourEth;
lv_color_t colourCycle;
lv_color_t colourBell;
lv_color_t colourLock;
lv_color_t colourUnlock;

lv_color_t colourOffline;

lv_obj_t * btnLockUnlock;

bool locked = true;

static void temperatureSPchanged_cb(lv_event_t * e);
static void lockUnlockEvent_cb(lv_event_t * e);
static void alarmAckEvent_cb(lv_event_t * e);

void screenMainInit(void) {
    screenMain = lv_obj_create(NULL);
    lv_scr_load(screenMain);

    colourOffline = lv_palette_darken(LV_PALETTE_GREY, 3);

    // Sensor modbus link icon
    colourLink = lv_palette_lighten(LV_PALETTE_GREEN, 2);
    imgLink = lv_img_create(lv_scr_act());
    lv_img_set_src(imgLink, &link);
    lv_obj_set_style_img_recolor_opa(imgLink, LV_OPA_COVER, LV_PART_MAIN);
    lv_obj_set_style_img_recolor(imgLink, colourOffline, LV_PART_MAIN);
    lv_obj_align(imgLink, LV_ALIGN_TOP_RIGHT, -4, 10);

    // Ethernet interface icon
    colourEth = lv_palette_lighten(LV_PALETTE_BLUE, 2);
    imgEth = lv_img_create(lv_scr_act());
    lv_img_set_src(imgEth, &network_wired);
    lv_obj_set_style_img_recolor_opa(imgEth, LV_OPA_COVER, LV_PART_MAIN);
    lv_obj_set_style_img_recolor(imgEth, colourOffline, LV_PART_MAIN);
    lv_obj_align(imgEth, LV_ALIGN_TOP_RIGHT, -42, 10);

    // Compressor icon
    colourCycle = lv_palette_lighten(LV_PALETTE_ORANGE, 2);
    imgCycle = lv_img_create(lv_scr_act());
    lv_img_set_src(imgCycle, &arrows_spin);
    lv_obj_set_style_img_recolor_opa(imgCycle, LV_OPA_COVER, LV_PART_MAIN);
    lv_obj_set_style_img_recolor(imgCycle, colourOffline, LV_PART_MAIN);
    lv_obj_align(imgCycle, LV_ALIGN_TOP_LEFT, 4, 10);

    // Alarm bell icon
    colourBell = lv_palette_lighten(LV_PALETTE_RED, 2);
    imgBell = lv_img_create(lv_scr_act());
    lv_img_set_src(imgBell, &bell);
    lv_obj_set_style_img_recolor_opa(imgBell, LV_OPA_COVER, LV_PART_MAIN);
    lv_obj_set_style_img_recolor(imgBell, colourOffline, LV_PART_MAIN);
    lv_obj_align(imgBell, LV_ALIGN_TOP_LEFT, 42, 10);

    // Lock icon
    colourLock = lv_palette_lighten(LV_PALETTE_RED, 2);
    imgLock = lv_img_create(lv_scr_act());
    lv_img_set_src(imgLock, &lock);
    lv_obj_set_style_img_recolor_opa(imgLock, LV_OPA_COVER, LV_PART_MAIN);
    lv_obj_set_style_img_recolor(imgLock, colourLock, LV_PART_MAIN);
    lv_obj_align(imgLock, LV_ALIGN_LEFT_MID, 30, -60);
    if (!locked) lv_obj_add_flag(imgLock, LV_OBJ_FLAG_HIDDEN);

    // Unlock icon
    colourUnlock = lv_palette_lighten(LV_PALETTE_GREEN, 2);
    imgUnlock = lv_img_create(lv_scr_act());
    lv_img_set_src(imgUnlock, &lock_open);
    lv_obj_set_style_img_recolor_opa(imgUnlock, LV_OPA_COVER, LV_PART_MAIN);
    lv_obj_set_style_img_recolor(imgUnlock, colourUnlock, LV_PART_MAIN);
    lv_obj_align(imgUnlock, LV_ALIGN_LEFT_MID, 30, -60);
    if (locked) lv_obj_add_flag(imgUnlock, LV_OBJ_FLAG_HIDDEN);

    static lv_style_t btn_style;
    lv_style_init(&btn_style);
    lv_style_set_bg_opa(&btn_style, LV_OPA_0);
    lv_style_set_border_width(&btn_style, 0);

    btnLockUnlock = lv_btn_create(lv_scr_act());
    lv_obj_add_style(btnLockUnlock, &btn_style, LV_PART_MAIN);
    lv_obj_set_size(btnLockUnlock, 60, 60);
    lv_obj_align(btnLockUnlock, LV_ALIGN_LEFT_MID, 0, -60);
    lv_obj_add_event_cb(btnLockUnlock, lockUnlockEvent_cb, LV_EVENT_LONG_PRESSED, NULL);

    temperatureScaleInit();
    humidityScaleInit();
    pressureValInit();
    alarmModalInit();
}

void temperatureScaleInit(void) {
    // Create circular scale
    lv_obj_t * scale = lv_scale_create(lv_screen_active());
    lv_obj_set_size(scale, 200, 200);
    lv_scale_set_label_show(scale, true);
    lv_scale_set_mode(scale, LV_SCALE_MODE_ROUND_OUTER);
    lv_obj_align(scale, LV_ALIGN_CENTER, 0, -110);

    lv_scale_set_total_tick_count(scale, 26);
    lv_scale_set_major_tick_every(scale, 5);

    lv_obj_set_style_length(scale, 5, LV_PART_ITEMS);
    lv_obj_set_style_length(scale, 10, LV_PART_INDICATOR);
    lv_scale_set_range(scale, 0, 100);

    static const char * custom_labels[] = {"-20 °C", "-15 °C", "-10 °C", "-5 °C", "0 °C", "5 °C", NULL};
    lv_scale_set_text_src(scale, custom_labels);

    static lv_style_t indicator_style;
    lv_style_init(&indicator_style);

    // Scale label style properties
    lv_style_set_text_font(&indicator_style, LV_FONT_DEFAULT);
    lv_style_set_text_color(&indicator_style, lv_palette_lighten(LV_PALETTE_BLUE, 3));

    // Scale major tick properties
    lv_style_set_line_color(&indicator_style, lv_palette_lighten(LV_PALETTE_BLUE, 3));
    lv_style_set_width(&indicator_style, 10U);      /*Tick length*/
    lv_style_set_line_width(&indicator_style, 4U);  /*Tick width*/
    lv_obj_add_style(scale, &indicator_style, LV_PART_INDICATOR);

    static lv_style_t minor_ticks_style;
    lv_style_init(&minor_ticks_style);
    lv_style_set_line_color(&minor_ticks_style, lv_palette_lighten(LV_PALETTE_BLUE, 2));
    lv_style_set_width(&minor_ticks_style, 5U);         /*Tick length*/
    lv_style_set_line_width(&minor_ticks_style, 2U);    /*Tick width*/
    lv_obj_add_style(scale, &minor_ticks_style, LV_PART_ITEMS);

    static lv_style_t main_line_style;
    lv_style_init(&main_line_style);
    // Main line properties
    lv_style_set_arc_color(&main_line_style, lv_palette_lighten(LV_PALETTE_BLUE, 3));
    lv_style_set_arc_width(&main_line_style, 4U); /*Tick width*/
    lv_obj_add_style(scale, &main_line_style, LV_PART_MAIN);

    // Setpoint label and value
    static lv_style_t labelTempSP_style;
    lv_style_init(&labelTempSP_style);
    lv_style_set_text_font(&labelTempSP_style, &lv_font_montserrat_28);
    lv_style_set_text_color(&labelTempSP_style, lv_palette_lighten(LV_PALETTE_BLUE, 4));

    labelTempSP = lv_label_create(lv_screen_active());
    lv_obj_add_style(labelTempSP, &labelTempSP_style, LV_PART_MAIN);
    lv_obj_align(labelTempSP, LV_ALIGN_CENTER, 0, -30);

    lv_obj_t * labelTempSPname = lv_label_create(lv_screen_active());
    lv_label_set_text(labelTempSPname, "Setpoint");
    lv_obj_align(labelTempSPname, LV_ALIGN_CENTER, 0, -55);

    // Process value label and value
    static lv_style_t labelTempPV_style;
    lv_style_init(&labelTempPV_style);
    lv_style_set_text_font(&labelTempPV_style, &lv_font_montserrat_24);
    lv_style_set_text_color(&labelTempPV_style, lv_palette_lighten(LV_PALETTE_GREEN, 4));

    labelTempPV = lv_label_create(lv_screen_active());
    lv_obj_add_style(labelTempPV, &labelTempPV_style, LV_PART_MAIN);
    lv_obj_align(labelTempPV, LV_ALIGN_CENTER, 0, -110);
    lv_label_set_text(labelTempPV, "--.- °C");

    lv_obj_t * labelTempPVname = lv_label_create(lv_screen_active());
    lv_label_set_text(labelTempPVname, "Temperature");
    lv_obj_align(labelTempPVname, LV_ALIGN_CENTER, 0, -135);

    // Process value arc (non-interractive)
    arcPV = lv_arc_create(lv_screen_active());

    static lv_style_t arc_bg_style;
    lv_style_init(&arc_bg_style);
    lv_style_set_arc_width(&arc_bg_style, 10U);
    lv_style_set_arc_rounded(&arc_bg_style, false);
    lv_obj_add_style(arcPV, &arc_bg_style, LV_PART_MAIN);

    static lv_style_t arcPV_style;
    lv_style_init(&arcPV_style);
    lv_style_set_arc_width(&arcPV_style, 20U);
    lv_style_set_arc_rounded(&arcPV_style, false);
    lv_style_set_arc_color(&arcPV_style, lv_palette_main(LV_PALETTE_GREEN));
    lv_obj_add_style(arcPV, &arcPV_style, LV_PART_INDICATOR);

    lv_obj_remove_style(arcPV, NULL, LV_PART_KNOB);
    lv_obj_remove_flag(arcPV, LV_OBJ_FLAG_CLICKABLE);

    lv_obj_set_size(arcPV, 194, 194);
    lv_arc_set_mode(arcPV, LV_ARC_MODE_REVERSE);
    lv_arc_set_rotation(arcPV, 135);
    lv_arc_set_bg_angles(arcPV, 0, 270);
    lv_arc_set_range(arcPV, 0, 250);
    lv_arc_set_value(arcPV, 0);
    lv_obj_align(arcPV, LV_ALIGN_CENTER, 0, -110);

    // Setpoint arc (interractive)
    arcSP = lv_arc_create(lv_screen_active());

    lv_obj_add_style(arcSP, &arc_bg_style, LV_PART_MAIN);
    
    static lv_style_t arcSP_style;
    lv_style_init(&arcSP_style);
    lv_style_set_arc_width(&arcSP_style, 20U);
    lv_style_set_arc_rounded(&arcSP_style, false);
    lv_obj_add_style(arcSP, &arcSP_style, LV_PART_INDICATOR);

    static lv_style_t knobSP_style;
    lv_style_init(&knobSP_style);
    lv_style_set_pad_all(&knobSP_style, 0);
    lv_style_set_border_width(&knobSP_style, 2);
    lv_style_set_border_color(&knobSP_style, lv_palette_lighten(LV_PALETTE_BLUE, 5));
    lv_obj_add_style(arcSP, &knobSP_style, LV_PART_KNOB);
    if (locked) lv_obj_remove_flag(arcSP, LV_OBJ_FLAG_CLICKABLE);

    lv_obj_set_size(arcSP, 164, 164);
    lv_arc_set_mode(arcSP, LV_ARC_MODE_REVERSE);
    lv_arc_set_rotation(arcSP, 135);
    lv_arc_set_bg_angles(arcSP, 0, 270);
    lv_arc_set_range(arcSP, 0, 50);
    float arcSPVal = (controlTempSP - 5) / -0.5;
    lv_arc_set_value(arcSP, arcSPVal);
    lv_obj_align(arcSP, LV_ALIGN_CENTER, 0, -110);
    lv_obj_add_event_cb(arcSP, temperatureSPchanged_cb, LV_EVENT_VALUE_CHANGED, NULL);

    /*Manually update the label for the first time*/
    lv_obj_send_event(arcSP, LV_EVENT_VALUE_CHANGED, NULL);
}

void humidityScaleInit(void) {
    // Create circular scale
    lv_obj_t * scale = lv_scale_create(lv_screen_active());
    lv_obj_set_size(scale, 200, 200);
    lv_scale_set_label_show(scale, true);
    lv_scale_set_mode(scale, LV_SCALE_MODE_ROUND_OUTER);
    lv_obj_align(scale, LV_ALIGN_CENTER, 0, 130);

    lv_scale_set_total_tick_count(scale, 51);
    lv_scale_set_major_tick_every(scale, 5);

    lv_obj_set_style_length(scale, 5, LV_PART_ITEMS);
    lv_obj_set_style_length(scale, 10, LV_PART_INDICATOR);
    lv_scale_set_range(scale, 0, 100);

    static const char * custom_labels[] = {"0 %", "10 %", "20 %", "30 %", "40 %", "50 %", "60 %", "70 %", "80 %", "90 %", "100 %", NULL};
    lv_scale_set_text_src(scale, custom_labels);

    static lv_style_t indicator_style;
    lv_style_init(&indicator_style);

    // Scale label style properties
    lv_style_set_text_font(&indicator_style, LV_FONT_DEFAULT);
    lv_style_set_text_color(&indicator_style, lv_palette_lighten(LV_PALETTE_BLUE, 3));

    // Scale major tick properties
    lv_style_set_line_color(&indicator_style, lv_palette_lighten(LV_PALETTE_BLUE, 3));
    lv_style_set_width(&indicator_style, 10U);      /*Tick length*/
    lv_style_set_line_width(&indicator_style, 4U);  /*Tick width*/
    lv_obj_add_style(scale, &indicator_style, LV_PART_INDICATOR);

    static lv_style_t minor_ticks_style;
    lv_style_init(&minor_ticks_style);
    lv_style_set_line_color(&minor_ticks_style, lv_palette_lighten(LV_PALETTE_BLUE, 2));
    lv_style_set_width(&minor_ticks_style, 5U);         /*Tick length*/
    lv_style_set_line_width(&minor_ticks_style, 2U);    /*Tick width*/
    lv_obj_add_style(scale, &minor_ticks_style, LV_PART_ITEMS);

    static lv_style_t main_line_style;
    lv_style_init(&main_line_style);
    // Main line properties
    lv_style_set_arc_color(&main_line_style, lv_palette_lighten(LV_PALETTE_BLUE, 3));
    lv_style_set_arc_width(&main_line_style, 4U); /*Tick width*/
    lv_obj_add_style(scale, &main_line_style, LV_PART_MAIN);

    // Process value label and value
    static lv_style_t labelHumidPV_style;
    lv_style_init(&labelHumidPV_style);
    lv_style_set_text_font(&labelHumidPV_style, &lv_font_montserrat_24);
    lv_style_set_text_color(&labelHumidPV_style, lv_palette_lighten(LV_PALETTE_PURPLE, 4));

    labelHumidPV = lv_label_create(lv_screen_active());
    lv_obj_add_style(labelHumidPV, &labelHumidPV_style, LV_PART_MAIN);
    lv_obj_align(labelHumidPV, LV_ALIGN_CENTER, 0, 130);
    lv_label_set_text(labelHumidPV, "--.- %");

    lv_obj_t * labelHumidPVname = lv_label_create(lv_screen_active());
    lv_label_set_text(labelHumidPVname, "Rel. Humidity");
    lv_obj_align(labelHumidPVname, LV_ALIGN_CENTER, 0, 105);

    // Process value arc (non-interractive)
    arcHumidPV = lv_arc_create(lv_screen_active());

    static lv_style_t arc_bg_style;
    lv_style_init(&arc_bg_style);
    lv_style_set_arc_width(&arc_bg_style, 10U);
    lv_style_set_arc_rounded(&arc_bg_style, false);
    lv_obj_add_style(arcHumidPV, &arc_bg_style, LV_PART_MAIN);

    static lv_style_t arcPV_style;
    lv_style_init(&arcPV_style);
    lv_style_set_arc_width(&arcPV_style, 20U);
    lv_style_set_arc_rounded(&arcPV_style, false);
    lv_style_set_arc_color(&arcPV_style, lv_palette_main(LV_PALETTE_PURPLE));
    lv_obj_add_style(arcHumidPV, &arcPV_style, LV_PART_INDICATOR);

    lv_obj_remove_style(arcHumidPV, NULL, LV_PART_KNOB);
    lv_obj_remove_flag(arcHumidPV, LV_OBJ_FLAG_CLICKABLE);

    lv_obj_set_size(arcHumidPV, 194, 194);
    lv_arc_set_mode(arcHumidPV, LV_ARC_MODE_NORMAL);
    lv_arc_set_rotation(arcHumidPV, 135);
    lv_arc_set_bg_angles(arcHumidPV, 0, 270);
    lv_arc_set_range(arcHumidPV, 0, 100);
    lv_arc_set_value(arcHumidPV, 0);
    lv_obj_align(arcHumidPV, LV_ALIGN_CENTER, 0, 130);
}

void pressureValInit() {
    // Setpoint label and value
    static lv_style_t labelPressPV_style;
    lv_style_init(&labelPressPV_style);
    lv_style_set_text_font(&labelPressPV_style, &lv_font_montserrat_22);
    lv_style_set_text_color(&labelPressPV_style, lv_palette_lighten(LV_PALETTE_YELLOW, 3));

    labelPressPV = lv_label_create(lv_screen_active());
    lv_obj_add_style(labelPressPV, &labelPressPV_style, LV_PART_MAIN);
    lv_obj_align(labelPressPV, LV_ALIGN_CENTER, 0, 220);
    lv_label_set_text(labelPressPV, "---.- hPa");

    lv_obj_t * labelPressPVname = lv_label_create(lv_screen_active());
    lv_label_set_text(labelPressPVname, "Pressure");
    lv_obj_align(labelPressPVname, LV_ALIGN_CENTER, 0, 200);
}

void alarmModalInit(void) {
    // Alarm modal window style
    static lv_style_t alarmBoxStyle;
    lv_style_init(&alarmBoxStyle);
    lv_style_set_radius(&alarmBoxStyle, 10);
    lv_style_set_width(&alarmBoxStyle, 280);
    lv_style_set_height(&alarmBoxStyle, 360);

    // Alarm modal window
    alarmBox = lv_obj_create(lv_screen_active());
    lv_obj_add_style(alarmBox, &alarmBoxStyle, 0);
    lv_obj_remove_flag(alarmBox, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_scrollbar_mode(alarmBox, LV_SCROLLBAR_MODE_OFF);
    lv_obj_align(alarmBox, LV_ALIGN_CENTER, 0, 0);
    lv_style_set_text_font(&alarmBoxStyle, &lv_font_montserrat_22);

    // Alarm header
    lv_obj_t * alarmHeader = lv_label_create(alarmBox);
    lv_label_set_recolor(alarmHeader, true);
    lv_label_set_text(alarmHeader, LV_SYMBOL_WARNING " #ff5555 Alarm#");
    lv_obj_align(alarmHeader, LV_ALIGN_TOP_MID, 0, 0);

    // Alarm text
    alarmText = lv_label_create(alarmBox);
    lv_obj_set_width(alarmText, 240);
    lv_obj_align(alarmText, LV_ALIGN_TOP_LEFT, 0, 50);
    lv_label_set_long_mode(alarmText, LV_LABEL_LONG_WRAP);
    lv_label_set_recolor(alarmText, true);

    // Button style
    static lv_style_t btnAckStyle;
    lv_style_init(&btnAckStyle);

    lv_style_set_radius(&btnAckStyle, 5);

    lv_style_set_bg_opa(&btnAckStyle, LV_OPA_100);
    lv_style_set_bg_color(&btnAckStyle, lv_palette_main(LV_PALETTE_RED));
    lv_style_set_bg_grad_color(&btnAckStyle, lv_palette_darken(LV_PALETTE_RED, 2));
    lv_style_set_bg_grad_dir(&btnAckStyle, LV_GRAD_DIR_VER);

    lv_style_set_border_opa(&btnAckStyle, LV_OPA_40);
    lv_style_set_border_width(&btnAckStyle, 2);
    lv_style_set_border_color(&btnAckStyle, lv_palette_darken(LV_PALETTE_RED, 4));

    lv_style_set_outline_opa(&btnAckStyle, LV_OPA_COVER);
    lv_style_set_outline_color(&btnAckStyle, lv_palette_darken(LV_PALETTE_RED, 4));
    lv_style_set_pad_all(&btnAckStyle, 10);

    btnAlarmAck = lv_button_create(alarmBox);
    lv_obj_add_style(btnAlarmAck, &btnAckStyle, LV_PART_MAIN);
    lv_obj_align(btnAlarmAck, LV_ALIGN_BOTTOM_MID, 0, 0);

    lv_obj_t * labelAlarmAck = lv_label_create(btnAlarmAck);
    lv_label_set_text(labelAlarmAck, "Acknowledge  " LV_SYMBOL_OK);
    lv_obj_align(labelAlarmAck, LV_ALIGN_CENTER, 0, 0);

    lv_obj_add_event_cb(btnAlarmAck, alarmAckEvent_cb, LV_EVENT_CLICKED, NULL);

    lv_obj_add_flag(alarmBox, LV_OBJ_FLAG_HIDDEN);
}

static void temperatureSPchanged_cb(lv_event_t * e)
{
    float val = (lv_arc_get_value(arcSP)*-0.5) + 5;
    char buf[10];
    snprintf(buf, sizeof(buf), "%.1f °C", val);
    lv_label_set_text(labelTempSP, buf);
    static bool init = false;
    if (!init) {
        init = true;
        return;
    }
    setTemperatureSetpoint(val);
}

static void lockUnlockEvent_cb(lv_event_t * e)
{
    if (locked) {
        lv_obj_remove_flag(imgUnlock, LV_OBJ_FLAG_HIDDEN);
        lv_obj_add_flag(imgLock, LV_OBJ_FLAG_HIDDEN);
        lv_obj_add_flag(arcSP, LV_OBJ_FLAG_CLICKABLE);
        locked = false;
    } else {
        lv_obj_remove_flag(imgLock, LV_OBJ_FLAG_HIDDEN);
        lv_obj_add_flag(imgUnlock, LV_OBJ_FLAG_HIDDEN);
        lv_obj_remove_flag(arcSP, LV_OBJ_FLAG_CLICKABLE);
        locked = true;
    }
}

static void alarmAckEvent_cb(lv_event_t * e) {
    lv_obj_add_flag(alarmBox, LV_OBJ_FLAG_HIDDEN);
    if (controlAlarm) {
        controlAlarm = false;
        setAlarmState();
    }
}

void setIconLink(bool state) {
    if (state) {
        lv_obj_set_style_img_recolor(imgLink, colourLink, LV_PART_MAIN);
    } else {
        lv_obj_set_style_img_recolor(imgLink, colourOffline, LV_PART_MAIN);
    }
}

void setIconEth(bool state) {
    if (state) {
        lv_obj_set_style_img_recolor(imgEth, colourEth, LV_PART_MAIN);
    } else {
        lv_obj_set_style_img_recolor(imgEth, colourOffline, LV_PART_MAIN);
    }
}

void setIconCycle(bool state) {
    if (state) {
        lv_obj_set_style_img_recolor(imgCycle, colourCycle, LV_PART_MAIN);
    } else {
        lv_obj_set_style_img_recolor(imgCycle, colourOffline, LV_PART_MAIN);
    }
}

void setIconBell(bool state) {
    if (state) {
        lv_obj_set_style_img_recolor(imgBell, colourBell, LV_PART_MAIN);
    } else {
        lv_obj_set_style_img_recolor(imgBell, colourOffline, LV_PART_MAIN);
    }
}

void setAlarmState(void) {
    if (sensorAlarm || controlAlarm) {
        setIconBell(true);

        char buf[200] = "";
        
        if (sensorAlarm) strcat(buf, "Lost connection to temperature and humidity sensor probe.\n\n");
        if (controlAlarm) strcat(buf, "Unable to reach temperature setpoint, check seals for leaks.\n\n");

        lv_label_set_text(alarmText, buf);
        lv_obj_remove_flag(alarmBox, LV_OBJ_FLAG_HIDDEN);
    }
    else setIconBell(false);
}

void updateProcessValues(float temperature, float humidity, float pressure) {
    char buf[10];
    snprintf(buf, sizeof(buf), "%.1f °C", temperature);
    lv_label_set_text(labelTempPV, buf);
    snprintf(buf, sizeof(buf), "%.1f %%RH", humidity);
    lv_label_set_text(labelHumidPV, buf);
    snprintf(buf, sizeof(buf), "%.1f hPa", pressure);
    lv_label_set_text(labelPressPV, buf);
    int32_t arcTempVal = lround(((temperature - 5) / -0.5) * 5);
    int32_t arcHumidVal = lround(humidity);
    if (arcTempVal < 0) arcTempVal = 0;
    if (arcHumidVal < 0) arcHumidVal = 0;
    if (arcTempVal > 250) arcTempVal = 250;
    if (arcHumidVal > 100) arcHumidVal = 100;
    lv_arc_set_value(arcPV, arcTempVal);
    lv_arc_set_value(arcHumidPV, arcHumidVal);
}