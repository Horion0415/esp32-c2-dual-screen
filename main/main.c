/*
 * SPDX-FileCopyrightText: 2021-2022 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: CC0-1.0
 */

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"

#include "esp_log.h"
#include "nvs_flash.h"
#include "bsp/esp-bsp.h"
#include "bsp/display.h"

static const char *TAG = "main";

static button_handle_t *g_btn_handle = NULL;

static void btn_select_sw_cb(void *handle, void *arg)
{
    ESP_LOGI("BTN", "select switch");
    // vTaskDelay(pdMS_TO_TICKS(2*1000));
    esp_restart();
}

void printf_stack()
{
    static char buffer[128];
    sprintf(buffer, "   Biggest /     Free /    Min/    Total\n"
            "\t  SRAM : [%8d / %8d / %8d / %8d]",
            heap_caps_get_largest_free_block(MALLOC_CAP_INTERNAL),
            heap_caps_get_free_size(MALLOC_CAP_INTERNAL),
            heap_caps_get_minimum_free_size(MALLOC_CAP_INTERNAL),
            heap_caps_get_total_size(MALLOC_CAP_INTERNAL));
    ESP_LOGI("MEM", "%s", buffer);
}

esp_err_t app_btn_init(void)
{
    ESP_ERROR_CHECK((NULL != g_btn_handle));

    int btn_num = 0;
    g_btn_handle = calloc(sizeof(button_handle_t), (BSP_BUTTON_NUM + BSP_ADC_BUTTON_NUM));
    assert((g_btn_handle) && "memory is insufficient for button");
    return bsp_iot_button_create(g_btn_handle, &btn_num, (BSP_BUTTON_NUM + BSP_ADC_BUTTON_NUM));
}

esp_err_t app_btn_register_callback(bsp_button_t btn, button_event_t event, button_cb_t callback, void *user_data)
{
    assert((g_btn_handle) && "button not initialized");
    assert((btn < (BSP_BUTTON_NUM + BSP_ADC_BUTTON_NUM)) && "button id incorrect");

    if (NULL == callback) {
        return iot_button_unregister_cb(g_btn_handle[btn], event);
    }
    return iot_button_register_cb(g_btn_handle[btn], event, callback, user_data);
}

esp_err_t app_btn_register_event_callback(bsp_button_t btn, button_event_config_t cfg, button_cb_t callback, void *user_data)
{
    assert((g_btn_handle) && "button not initialized");
    esp_err_t err = iot_button_register_event_cb(g_btn_handle[btn], cfg, callback, (void *)(cfg.event));
    ESP_ERROR_CHECK(err);
    return ESP_OK;
}

esp_err_t app_btn_rm_all_callback(bsp_button_t btn)
{
    assert((g_btn_handle) && "button not initialized");
    assert((btn < (BSP_BUTTON_NUM + BSP_ADC_BUTTON_NUM)) && "button id incorrect");

    for (size_t event = 0; event < BUTTON_EVENT_MAX; event++) {
        iot_button_unregister_cb(g_btn_handle[btn], event);
    }
    return ESP_OK;
}

esp_err_t app_btn_rm_event_callback(bsp_button_t btn, size_t event)
{
    assert((g_btn_handle) && "button not initialized");
    assert((btn < (BSP_BUTTON_NUM + BSP_ADC_BUTTON_NUM)) && "button id incorrect");

    iot_button_unregister_cb(g_btn_handle[btn], event);
    return ESP_OK;
}

void ui_1_28_start(lv_disp_t *disp)
{
    // 设置为当前活动显示设备
    lv_disp_set_default(disp);

    // 创建主屏幕
    lv_obj_t *scr = lv_scr_act();
    lv_obj_set_style_bg_color(scr, lv_color_black(), 0);
    
    // 创建一个简单的标签
    lv_obj_t *label = lv_label_create(scr);
    lv_obj_set_style_text_font(label, &lv_font_montserrat_24, 0);
    lv_obj_set_style_text_color(label, lv_color_white(), 0);
    lv_label_set_text(label, "1.28\" LCD");
    lv_obj_center(label);
}

void ui_096_start(lv_disp_t *disp)
{
    // 设置为当前活动显示设备
    lv_disp_set_default(disp);

    // 创建主屏幕
    lv_obj_t *scr = lv_scr_act();
    lv_obj_set_style_bg_color(scr, lv_color_black(), 0);

    // 创建标题标签
    lv_obj_t *title = lv_label_create(scr);
    lv_obj_set_style_text_font(title, &lv_font_montserrat_16, 0);
    lv_obj_set_style_text_color(title, lv_color_white(), 0);
    lv_label_set_text(title, "0.96\" LCD");
    lv_obj_align(title, LV_ALIGN_TOP_MID, 0, 5);
}

void app_main(void)
{
    esp_err_t err = nvs_flash_init();
    if (err == ESP_ERR_NVS_NO_FREE_PAGES || err == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        err = nvs_flash_init();
    }
    ESP_ERROR_CHECK(err);

    lv_disp_t* disp_096;
    lv_disp_t* disp_128;

    printf_stack();
    bsp_display_start(&disp_096, &disp_128);
    bsp_display_backlight_on();

    // app_btn_init();
    // app_btn_register_callback(BSP_BUTTON_CONFIG, BUTTON_PRESS_UP, btn_select_sw_cb, NULL);
    // app_btn_register_callback(BSP_BUTTON_CONFIG, BUTTON_PRESS_DOWN, btn_select_sw_cb, NULL);

// #if USE_SCREEN_096
    // ui_1_28_start(0);
// #else
    // ui_1_28_start(1);
// #endif
    ui_1_28_start(disp_128);  // 使用1.28寸作为默认显示设备
    ui_096_start(disp_096);  // 获取第二个显示设备用于0.96寸屏幕
}
