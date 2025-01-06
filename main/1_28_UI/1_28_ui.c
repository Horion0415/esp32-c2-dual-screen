/*
 * SPDX-FileCopyrightText: 2021-2022 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: CC0-1.0
 */

#include <dirent.h>
#include <stdbool.h>
#include <stdint.h>
#include <time.h>

#include "esp_log.h"
#include "bsp/esp-bsp.h"
#include "bsp/display.h"

#include "esp_lv_fs.h"
#include "esp_lv_decoder.h"

#include "mmap_generate_out1.h"
#include "mmap_generate_out2.h"

static const char *TAG = "1_28_ui";

static mmap_assets_handle_t mmap_handle_A;
static mmap_assets_handle_t mmap_handle_B;

static void btn_press_left_cb(void *handle, void *arg);

static void btn_press_OK_cb(void *handle, void *arg);

static void btn_press_right_cb(void *handle, void *arg);

static void image_mmap_init();

#define MAX_COUNTERS        3

typedef enum {
    THEME_SELECT_CHILD,
    THEME_SELECT_CLEAN,
    THEME_SELECT_QUICK,
    THEME_MAX_NUM,
} theme_select_t;

typedef struct {
    int64_t start;
    int64_t acc;
    char str1[15];
    char str2[15];
} PerfCounter;

static theme_select_t theme_select = THEME_SELECT_CHILD;
static bool anmi_do_run = true;

static mmap_assets_handle_t asset_DriverA_handle;
static esp_lv_fs_handle_t fs_DriverA_handle;

static mmap_assets_handle_t asset_DriverB_handle;
static esp_lv_fs_handle_t fs_DriverB_handle;

static esp_lv_decoder_handle_t decoder_handle = NULL;

static PerfCounter perf_counters[MAX_COUNTERS] = {0};

static void perfmon_start(int ctr, const char* fmt1, const char* fmt2, ...)
{
    va_list args;
    va_start(args, fmt2);
    vsnprintf(perf_counters[ctr].str1, sizeof(perf_counters[ctr].str1), fmt1, args);
    vsnprintf(perf_counters[ctr].str2, sizeof(perf_counters[ctr].str2), fmt2, args);
    va_end(args);

    perf_counters[ctr].start = esp_timer_get_time();
}

static void perfmon_end(int ctr, int count)
{
    int64_t time_diff = esp_timer_get_time() - perf_counters[ctr].start;
    float time_in_sec = (float)time_diff / 1000000;
    float frequency = count / time_in_sec;

    printf("Perf ctr[%d], [%8s][%8s]: %.2f FPS (%.2f ms)\n",
           ctr, perf_counters[ctr].str1, perf_counters[ctr].str2, frequency, time_in_sec * 1000 / count);
}

void ui_1_28_init(void)
{
    // app_btn_register_callback(BSP_BUTTON_NUM + BSP_ADC_BUTTON_PREV, BUTTON_PRESS_UP, btn_press_left_cb, NULL);
    // app_btn_register_callback(BSP_BUTTON_NUM + BSP_ADC_BUTTON_ENTER, BUTTON_PRESS_UP, btn_press_OK_cb, NULL);
    // app_btn_register_callback(BSP_BUTTON_NUM + BSP_ADC_BUTTON_NEXT, BUTTON_PRESS_UP, btn_press_right_cb, NULL);

    image_mmap_init();
}


static void ui_1_28_task(void *arg)
{
    lv_disp_t *disp = (lv_disp_t *)arg;
    lv_img_dsc_t img_dsc_motive;

    // bsp_display_lock(0);

    ESP_LOGI(TAG, "ui_0_96_task screen size:[%d,%d]", LV_HOR_RES, LV_VER_RES);

    bsp_display_lock(0);

    lv_disp_set_default(disp);
    lv_obj_t *obj_bg = lv_obj_create(lv_scr_act());
    lv_obj_set_size(obj_bg, LV_HOR_RES, LV_VER_RES);
    lv_obj_set_align(obj_bg, LV_ALIGN_CENTER);
    lv_obj_clear_flag(obj_bg, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_style_bg_color(obj_bg, lv_color_hex(0x000000), 0);
    lv_obj_set_style_border_width(obj_bg, 0, 0);
    lv_obj_set_style_radius(obj_bg, 0, 0);

    lv_obj_t *obj_img_run_particles = lv_img_create(obj_bg);
    lv_obj_set_align(obj_img_run_particles, LV_ALIGN_CENTER);

    bsp_display_unlock();

    theme_select_t theme_last = THEME_MAX_NUM;
    uint8_t list = 100;
    // int fps_count = 0;

    uint32_t max_size = 0;
    // if (select == 1) {
        mmap_handle_A  = asset_DriverA_handle;
        max_size = MMAP_OUT1_FILES;
    // }
    // else {
    //     mmap_handle  = asset_DriverB_handle;
    //     max_size = MMAP_OUT2_FILES;
    // }

    while (1) {
        bsp_display_lock(0);

        if (theme_last ^ theme_select) {
            theme_last = theme_select;
        }
        if (true == anmi_do_run) {
            list++;
            lv_obj_clear_flag(obj_img_run_particles, LV_OBJ_FLAG_HIDDEN);

            img_dsc_motive.data_size = mmap_assets_get_size(mmap_handle_A, (list) % max_size);
            img_dsc_motive.data = mmap_assets_get_mem(mmap_handle_A, (list) % max_size);
            lv_img_set_src(obj_img_run_particles, &img_dsc_motive);

            // if (fps_count % 10 == 0) {
            //     perfmon_start(0, "PFS", "png");
            //     // printf_stack();
            // } else if (fps_count % 10 == 9) {
            //     perfmon_end(0, 10);
            // }
            // fps_count++;
        } else {
            list = 0;
            lv_obj_add_flag(obj_img_run_particles, LV_OBJ_FLAG_HIDDEN);
        }
        lv_refr_now(NULL);
        bsp_display_unlock();

        // if (select == 1) {
            vTaskDelay(pdMS_TO_TICKS(5));
        // } 
        // else {
        //     vTaskDelay(pdMS_TO_TICKS(1));
        // }
    }

    mmap_assets_del(asset_DriverA_handle);
    mmap_assets_del(asset_DriverB_handle);
}

static void ui_0_96_task(void *arg)
{
    lv_disp_t *disp = (lv_disp_t *)arg;
    lv_img_dsc_t img_dsc_motive;

    // bsp_display_lock(0);

    ESP_LOGI(TAG, "ui_0_96_task screen size:[%d,%d]", LV_HOR_RES, LV_VER_RES);

    bsp_display_lock(0);

    lv_disp_set_default(disp);
    lv_obj_t *obj_bg = lv_obj_create(lv_scr_act());
    lv_obj_set_size(obj_bg, LV_HOR_RES, LV_VER_RES);
    lv_obj_set_align(obj_bg, LV_ALIGN_CENTER);
    lv_obj_clear_flag(obj_bg, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_style_bg_color(obj_bg, lv_color_hex(0x000000), 0);
    lv_obj_set_style_border_width(obj_bg, 0, 0);
    lv_obj_set_style_radius(obj_bg, 0, 0);

    lv_obj_t *obj_img_run_particles = lv_img_create(obj_bg);
    lv_obj_set_align(obj_img_run_particles, LV_ALIGN_CENTER);

    bsp_display_unlock();

    theme_select_t theme_last = THEME_MAX_NUM;
    uint8_t list = 100;
    // int fps_count = 0;

    static mmap_assets_handle_t mmap_handle_B;
    uint32_t max_size = 0;
    // if (select == 1) {
    //     mmap_handle  = asset_DriverA_handle;
    //     max_size = MMAP_OUT1_FILES;
    // }
    // else {
        mmap_handle_B  = asset_DriverB_handle;
        max_size = MMAP_OUT2_FILES;
    // }

    while (1) {
        bsp_display_lock(0);

        if (theme_last ^ theme_select) {
            theme_last = theme_select;
        }
        if (true == anmi_do_run) {
            list++;
            lv_obj_clear_flag(obj_img_run_particles, LV_OBJ_FLAG_HIDDEN);

            img_dsc_motive.data_size = mmap_assets_get_size(mmap_handle_B, (list) % max_size);
            img_dsc_motive.data = mmap_assets_get_mem(mmap_handle_B, (list) % max_size);
            lv_img_set_src(obj_img_run_particles, &img_dsc_motive);

            // if (fps_count % 10 == 0) {
            //     perfmon_start(0, "PFS", "png");
            //     // printf_stack();
            // } else if (fps_count % 10 == 9) {
            //     perfmon_end(0, 10);
            // }
            // fps_count++;
        } else {
            list = 0;
            lv_obj_add_flag(obj_img_run_particles, LV_OBJ_FLAG_HIDDEN);
        }
        lv_refr_now(NULL);
        bsp_display_unlock();

        // if (select == 1) {
        //     vTaskDelay(pdMS_TO_TICKS(5));
        // } 
        // else {
            vTaskDelay(pdMS_TO_TICKS(1));
        // }
    }

    mmap_assets_del(asset_DriverA_handle);
    mmap_assets_del(asset_DriverB_handle);
}

void ui_start(lv_disp_t *disp_096, lv_disp_t *disp_128)
{
    // xTaskCreate(ui_1_28_task, "ui_1_28_task", 4096, (void *)disp_128, 5, NULL);
    // xTaskCreate(ui_0_96_task, "ui_0_96_task", 4096, (void *)disp_096, 5, NULL);
    // static lv_img_dsc_t img_dsc_motive;
    image_mmap_init();
    static lv_img_dsc_t img_dsc_motive_A;
    static lv_img_dsc_t img_dsc_motive_B;

    // bsp_display_lock(0);

    // ESP_LOGI(TAG, "screen size:[%d,%d]", LV_HOR_RES, LV_VER_RES);

    bsp_display_lock(0);

    // lv_disp_set_default(disp_096);
    lv_obj_t *obj_bg_096 = lv_obj_create(lv_disp_get_scr_act(disp_096));
    // lv_obj_t *obj_bg_096 = lv_obj_create(lv_scr_act());
    lv_obj_set_size(obj_bg_096, BSP_LCD_0_9_6_H_RES, BSP_LCD_0_9_6_H_RES);
    lv_obj_set_align(obj_bg_096, LV_ALIGN_CENTER);
    lv_obj_clear_flag(obj_bg_096, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_style_bg_color(obj_bg_096, lv_color_hex(0x000000), 0);
    lv_obj_set_style_border_width(obj_bg_096, 0, 0);
    lv_obj_set_style_radius(obj_bg_096, 0, 0);

    lv_obj_t *obj_img_run_particles_096 = lv_img_create(obj_bg_096);
    lv_obj_set_align(obj_img_run_particles_096, LV_ALIGN_CENTER);

    // lv_disp_set_default(disp_128);
    lv_obj_t *obj_bg_128 = lv_obj_create(lv_disp_get_scr_act(disp_128));
    lv_obj_set_size(obj_bg_128, BSP_LCD_1_2_8_H_RES, BSP_LCD_1_2_8_H_RES);
    lv_obj_set_align(obj_bg_128, LV_ALIGN_CENTER);
    lv_obj_clear_flag(obj_bg_128, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_style_bg_color(obj_bg_128, lv_color_hex(0x000000), 0);
    lv_obj_set_style_border_width(obj_bg_128, 0, 0);
    lv_obj_set_style_radius(obj_bg_128, 0, 0);

    lv_obj_t *obj_img_run_particles_128 = lv_img_create(obj_bg_128);
    lv_obj_set_align(obj_img_run_particles_128, LV_ALIGN_CENTER);

    bsp_display_unlock();

    theme_select_t theme_last = THEME_MAX_NUM;
    uint8_t list = 100;
    int fps_count = 0;

    // static mmap_assets_handle_t handle;
    uint32_t max_size_A = 0;
    uint32_t max_size_B = 0;
    // if (select == 1) {
        mmap_handle_A  = asset_DriverA_handle;
        max_size_A = MMAP_OUT1_FILES;
    // } else {
        mmap_handle_B  = asset_DriverB_handle;
        max_size_B = MMAP_OUT2_FILES;
    // }

    while (1) {
        bsp_display_lock(0);

        if (theme_last ^ theme_select) {
            theme_last = theme_select;
        }
        if (true == anmi_do_run) {
            list++;
            lv_obj_clear_flag(obj_img_run_particles_096, LV_OBJ_FLAG_HIDDEN);

            img_dsc_motive_A.data_size = mmap_assets_get_size(mmap_handle_A, (list) % max_size_A);
            img_dsc_motive_A.data = mmap_assets_get_mem(mmap_handle_A, (list) % max_size_A);
            lv_img_set_src(obj_img_run_particles_096, &img_dsc_motive_A);

            lv_obj_clear_flag(obj_img_run_particles_128, LV_OBJ_FLAG_HIDDEN);

            img_dsc_motive_B.data_size = mmap_assets_get_size(mmap_handle_B, (list) % max_size_B);
            img_dsc_motive_B.data = mmap_assets_get_mem(mmap_handle_B, (list) % max_size_B);
            lv_img_set_src(obj_img_run_particles_128, &img_dsc_motive_B);

            if (fps_count % 10 == 0) {
                perfmon_start(0, "PFS", "png");
                // printf_stack();
            } else if (fps_count % 10 == 9) {
                perfmon_end(0, 10);
            }
            fps_count++;
        } else {
            list = 0;
            lv_obj_add_flag(obj_img_run_particles_096, LV_OBJ_FLAG_HIDDEN);

            lv_obj_add_flag(obj_img_run_particles_128, LV_OBJ_FLAG_HIDDEN);
        }
        lv_refr_now(NULL);

        // if (theme_last ^ theme_select) {
        //     theme_last = theme_select;
        // }
        // if (true == anmi_do_run) {
        //     list++;
        //     lv_obj_clear_flag(obj_img_run_particles_128, LV_OBJ_FLAG_HIDDEN);

        //     img_dsc_motive.data_size = mmap_assets_get_size(mmap_handle_B, (list) % max_size_B);
        //     img_dsc_motive.data = mmap_assets_get_mem(mmap_handle_B, (list) % max_size_B);
        //     lv_img_set_src(obj_img_run_particles_128, &img_dsc_motive);

        //     // if (fps_count % 10 == 0) {
        //     //     perfmon_start(0, "PFS", "png");
        //     //     // printf_stack();
        //     // } else if (fps_count % 10 == 9) {
        //     //     perfmon_end(0, 10);
        //     // }
        //     // fps_count++;
        // } else {
        //     list = 0;
        //     lv_obj_add_flag(obj_img_run_particles_128, LV_OBJ_FLAG_HIDDEN);
        // }
        // lv_refr_now(NULL);

        bsp_display_unlock();

        // if (select == 1) {
        //     vTaskDelay(pdMS_TO_TICKS(5));
        // } else {
            vTaskDelay(pdMS_TO_TICKS(1));
        // }
    }

    mmap_assets_del(asset_DriverA_handle);
    mmap_assets_del(asset_DriverB_handle);
}

static void btn_press_left_cb(void *handle, void *arg)
{
    theme_select = (theme_select + 2) % THEME_MAX_NUM;
    anmi_do_run = true;
    ESP_LOGI("BTN", "left:%d", theme_select);
}

static void btn_press_OK_cb(void *handle, void *arg)
{
    anmi_do_run = !anmi_do_run;
    ESP_LOGI("BTN", "OK:%d", anmi_do_run);
}

static void btn_press_right_cb(void *handle, void *arg)
{
    anmi_do_run = true;
    theme_select = (theme_select + 1) % THEME_MAX_NUM;
    ESP_LOGI("BTN", "right:%d", theme_select);
}

static void image_mmap_init()
{
    const mmap_assets_config_t config_DriveA = {
        .partition_label = "assets_A",
        .max_files = MMAP_OUT1_FILES,
        .checksum = MMAP_OUT1_CHECKSUM,
        .flags = {
            .app_bin_check = true,
            .mmap_enable = true,
        }
    };
    ESP_ERROR_CHECK(mmap_assets_new(&config_DriveA, &asset_DriverA_handle));

    const fs_cfg_t fs_cfg_a = {
        .fs_letter = 'A',
        .fs_assets = asset_DriverA_handle,
        .fs_nums = MMAP_OUT1_FILES
    };
    esp_lv_fs_desc_init(&fs_cfg_a, &fs_DriverA_handle);

    const mmap_assets_config_t config_DriveB = {
        .partition_label = "assets_B",
        .max_files = MMAP_OUT2_FILES,
        .checksum = MMAP_OUT2_CHECKSUM,
        .flags = {
            .app_bin_check = true,
            .mmap_enable = true,
        }
    };
    ESP_ERROR_CHECK(mmap_assets_new(&config_DriveB, &asset_DriverB_handle));

    const fs_cfg_t fs_cfg_b = {
        .fs_letter = 'B',
        .fs_assets = asset_DriverB_handle,
        .fs_nums = MMAP_OUT2_FILES
    };
    esp_lv_fs_desc_init(&fs_cfg_b, &fs_DriverB_handle);

    esp_lv_decoder_init(&decoder_handle); //Initialize this after lvgl starts
}