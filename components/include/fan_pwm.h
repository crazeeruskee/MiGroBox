/* 
 * Author(s):         Lucas Moiseyev
 * Date Created:      12/5/20
 * Date Last Updated: 12/5/20
 *
 */

#pragma once

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "esp_err.h"
#include "driver/ledc.h"

#define FAN_DUTY_RES              LEDC_TIMER_11_BIT
#define FAN_PWM_LS_MODE           LEDC_LOW_SPEED_MODE
#define FAN_PWM_DEFAULT_FREQUENCY (250)
#define FAN_MAX_FREQUENCY         (4000)

typedef struct{
    uint32_t              freq;
    uint32_t              max_speed;
    uint32_t              duty;
    uint32_t              max_duty;
    ledc_timer_bit_t      duty_res;
    ledc_timer_config_t   fan_pwm_timer;
    ledc_channel_config_t fan_pwm_channel;
    char*                 TAG;
} fan_pwm_t;


/**
 * @brief Default configuration for pwm fan
 *
 */
#define FAN_DEFAULT_CONFIG(){ \
    .freq = 0,                \
    .duty = 0,                \
    .max_speed = 4000,        \
    .fan_pwm_timer = NULL,    \
    .fan_pwm_channel = NULL   \
}

esp_err_t fan_config(fan_pwm_t *fan, char *TAG, int gpio, uint32_t freq, ledc_timer_t timer_sel, ledc_channel_t channel_sel);

void set_fan_speed(fan_pwm_t *fan, uint32_t freq);

uint32_t get_fan_speed(fan_pwm_t *fan);

void set_fan_duty(fan_pwm_t *fan, uint32_t duty);

uint32_t get_fan_duty(fan_pwm_t *fan);


