/* 
 * Author(s):         Lucas Moiseyev
 * Date Created:      12/5/20
 * Date Last Updated: 12/5/20
 *
 */

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/timers.h"
#include "esp_compiler.h"
#include "esp_log.h"
#include "driver/dedic_gpio.h"
#include "driver/gpio.h"
#include "driver/ledc.h"
#include "fan_pwm.h"
#include "esp_rom_sys.h"

esp_err_t fan_config(fan_pwm_t *fan, char *TAG, int gpio, uint32_t freq, ledc_timer_t timer_sel, ledc_channel_t channel_sel){
    fan->freq = freq;
    fan->max_speed = FAN_MAX_FREQUENCY;
    fan->duty = 1024;
    fan->max_duty = (1ULL<<FAN_DUTY_RES)-1;
    fan->fan_pwm_timer = (ledc_timer_config_t){
        .duty_resolution = FAN_DUTY_RES,  // resolution of PWM duty
        .freq_hz = freq,                  // frequency of PWM signal
        .speed_mode = FAN_PWM_LS_MODE,    // timer mode
        .timer_num = timer_sel,           // timer index
        .clk_cfg = LEDC_AUTO_CLK,         // Auto select the source clock
    };
    ledc_timer_config(&(fan->fan_pwm_timer));

    fan->fan_pwm_channel = (ledc_channel_config_t){
        .channel    = channel_sel,
        .duty       = fan->duty,
        .gpio_num   = gpio,
        .speed_mode = FAN_PWM_LS_MODE,
        .hpoint     = 0,
        .timer_sel  = timer_sel,
    };

    ledc_channel_config(&(fan->fan_pwm_channel));


    fan->TAG = "FAN";
//    strcpy(fan->TAG, TAG);

     return ESP_OK;
}

void set_fan_speed(fan_pwm_t *fan, uint32_t freq){
    fan->freq = freq;
    ledc_set_duty_and_update((fan->fan_pwm_channel).speed_mode, (fan->fan_pwm_channel).channel, fan->duty, (fan->fan_pwm_channel).hpoint);
    ledc_set_freq((fan->fan_pwm_channel).speed_mode, (fan->fan_pwm_channel).timer_sel, freq);
    ESP_LOGI(fan->TAG, "FAN FREQUENCY SET TO %d", fan->freq);
}

uint32_t get_fan_speed(fan_pwm_t *fan){
    return fan->freq;
}

void set_fan_duty(fan_pwm_t *fan, uint32_t duty){
    if(duty <= fan->max_duty){
        fan->duty = duty;
        ledc_set_duty_and_update((fan->fan_pwm_channel).speed_mode, (fan->fan_pwm_channel).channel, fan->duty, (fan->fan_pwm_channel).hpoint);
        ESP_LOGI(fan->TAG, "FAN FREQUENCY SET TO %d", fan->freq);
    } else{
        ESP_LOGW(fan->TAG, "ERROR: FAN DUTY INPUT OVER MAX DUTY OF %d", fan->max_duty);
    }
}

uint32_t get_fan_duty(fan_pwm_t *fan){
    return fan->duty;
}



