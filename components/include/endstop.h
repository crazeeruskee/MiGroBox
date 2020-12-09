/* 
 * Author(s):         Lucas Moiseyev
 * Date Created:      12/4/20
 * Date Last Updated: 12/6/20
 *
 */
#pragma once

#include <stdint.h>
#include "esp_err.h"
#include "driver/gpio.h"
#include "driver/ledc.h"
#include "stepper.h"

/* MiGroBox endstop code - ECE Capstone Fall 2020 

*/
#define ESP_INTR_FLAG_DEFAULT    0
#define ENDSTOP_INTR_FLAG_LEVEL  ESP_INTR_FLAG_LEVEL3
#define ENDSTOP_INTR_FLAG        (ENDSTOP_INTR_FLAG_LEVEL | ESP_INTR_FLAG_EDGE)

#define TAG_SIZE 32

typedef struct {
    uint32_t gpio;
    char TAG[TAG_SIZE];              //Custom endstop esp log tag
    short pressed_status;             //0 = released, 1 = pressed
    short min_max_axis;               //Endstop at min axis or max axis position, 0 = min, 1 = max
    stepper_t* stepper;              //Address to stepper driver this endstop is responsible for
    xQueueHandle* evt_queue;         //Address to main static event queue
    TaskHandle_t endstop_taskHandle; //Endstop interrupt task handle 
} endstop_t;

/**
 * @brief Default configuration for endstop
 *
 */
#define ENDSTOP_DEFAULT_CONFIG(){ \
    .gpio = 0,                    \
    .pressed_status = 0,          \
    .min_max_angle = 0,           \
    .stepper = NULL,              \
    .evt_queue = NULL,            \
    .TAG = "defaul endstop",      \
}

//void IRAM_ATTR endstop_isr_handler(void* arg);

//void endstop_task(void* arg);

endstop_t* init_endstop();

esp_err_t config_endstop(char *TAG, endstop_t *endstop, stepper_t *stepper, uint32_t gpio, short min_max_axis, xQueueHandle *evt_queue);

char get_endstop_status(endstop_t *endstop);


