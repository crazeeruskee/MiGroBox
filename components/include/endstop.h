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

/* MiGroBox endstop code - ECE Capstone Fall 2020 

*/
#define Z_ENDSTOP_GPIO         21
#define Y_ENDSTOP_GPIO         22

#define ESP_INTR_FLAG_DEFAULT  0

typdef struct {
    uint32_t gpio;
    char pressed_status; //0 = released, 1 = pressed
    char min_max_axis;   //Endstop at min axis or max axis position, 0 = min, 1 = max
    stepper_t* stepper;  //address to stepper driver this endstop is responsible for    
} endstop_t;

/**
 * @brief Default configuration for endstop
 *
 */
#define ENDSTOP_DEFAULT_CONFIG(){ \
    .gpio = 0,                \
    .pressed_status = 0,      \
    .min_max_angle = 0,       \
    .stepper = NULL,          \
}

void IRAM_ATTR endstop_isr_handler(void* arg);

void endstop_task(void* arg);

esp_err_t config_endstop(endstop_t *endstop, stepper_t *stepper, uint32_t gpio, char min_max_axis);

char get_endstop_status(endstop_t *endstop);


