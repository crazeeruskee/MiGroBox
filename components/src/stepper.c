/* 
 * Author(s):         Lucas Moiseyev
 * Date Created:      12/5/20
 * Date Last Updated: 12/5/20
 *
 */
#include <stdlib.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "freertos/timers.h"
#include "esp_compiler.h"
#include "esp_log.h"
#include "driver/dedic_gpio.h"
#include "driver/gpio.h"
#include "driver/ledc.h"
#include "stepper.h"
#include "esp_rom_sys.h"

/* MiGroBox ESP32s2 stepper motor driver code - ECE Capstone Fall 2020 
    - Controls stepper drivers
*/

/*
void IRAM_ATTR stepper_isr_handler(void* arg){
    uint32_t gpio_num = (uint32_t) arg;

    //xQueueGenericSendFromISR(gpio_evt_quque, &gpio_num, NULL)
    //xQueueSendFromISR(gpio_evt_queue, &gpio_num, NULL);
}

static void IRAM_ATTR y_endstop_isr_handler(void* arg){
    uint32_t gpio_num = (uint32_t) arg;
    //xQueueGenericSendFromISR(gpio_evt_quque, &gpio_num, NULL)
    //xQueueSendFromISR(gpio_evt_queue, &gpio_num, NULL);
    xTaskResumeFromISR(y_endstop_xHandle);
}

*/

esp_err_t config_stepper(stepper_t *stepper, char *TAG, ledc_timer_t timer_sel, ledc_channel_t channel_sel, uint32_t dir_gpio, uint32_t step_gpio, uint32_t en_gpio, int max_position, int min_position){
    stepper->dir_gpio = dir_gpio;
    stepper->step_gpio = step_gpio;
    stepper->en_gpio = en_gpio;
    gpio_config_t io_conf;
    io_conf.intr_type = GPIO_INTR_DISABLE;
    io_conf.pin_bit_mask = ((1ULL<<dir_gpio) | (1ULL<<step_gpio) | (1ULL<<en_gpio));
    io_conf.mode = GPIO_MODE_OUTPUT;
    io_conf.pull_down_en = 0;
    io_conf.pull_up_en = 0;
    gpio_config(&io_conf);

    stepper->max_position = max_position;
    stepper->min_position = min_position;

    stepper->direction = 0;
    stepper->position = 0; //set to -1 for unknown position
    stepper->travel_distance_command = 0;

    stepper->stepper_pwm_timer = (ledc_timer_config_t) {
        .duty_resolution = STEPPER_PWM_DUTY_RES ,  // resolution of PWM duty
        .freq_hz = STEPPER_PWM_TIMER_FREQ_DEFAULT, // frequency of PWM signal
        .speed_mode = STEPPER_PWM_LS_MODE,         // timer mode
        .timer_num = timer_sel,                    // timer index
        .clk_cfg = LEDC_AUTO_CLK,                  // Auto select the source clock
    };
    ledc_timer_config(&(stepper->stepper_pwm_timer));
    
    stepper->stepper_pwm_channel = (ledc_channel_config_t) {
        .channel    = channel_sel,
        .duty       = 0,
        .gpio_num   = step_gpio,
        .speed_mode = STEPPER_PWM_LS_MODE,
        .hpoint     = 0,
        .timer_sel  = timer_sel,
    };
    ledc_channel_config(&(stepper->stepper_pwm_channel));

    stepper->TAG = "Stepper";
   // strcpy(stepper->TAG, TAG);

    // Initialize fade service.
     ledc_fade_func_install(0);

     return ESP_OK;
}

void enable_stepper_driver(stepper_t *stepper){
    gpio_set_level(stepper->en_gpio, 0);
}

void disable_stepper_driver(stepper_t *stepper){
    gpio_set_level(stepper->en_gpio, 1);
}

void tick_stepper(stepper_t *stepper, int travel_distance_command){
    stepper->travel_distance_command = travel_distance_command;

    //Temporary position edit
    if(stepper->position + travel_distance_command > stepper->max_position || stepper->position + travel_distance_command < stepper->min_position){
        ESP_LOGW(stepper->TAG, "INVALID MOVE COMMAND, THIS WOULD PUT THE STEPPER BEYOND ITS POSITION BOUNDS!");
        return;
    }
    
    stepper->position += travel_distance_command;
    

    //Enable stepper driver
    enable_stepper_driver(stepper);

    //Set stepper direction
    if(travel_distance_command < 0) gpio_set_level(stepper->dir_gpio, 1);
    else gpio_set_level(stepper->dir_gpio, 0);
 
//    ledc_set_duty(y_pwm_channel.speed_mode, y_pwm_channel.channel, PWM_TEST_DUTY);
//    ledc_update_duty(y_pwm_channel.speed_mode, y_pwm_channel.channel);
    ledc_set_duty_and_update((stepper->stepper_pwm_channel).speed_mode, (stepper->stepper_pwm_channel).channel, STEPPER_PWM_DUTY_DEFAULT, (stepper->stepper_pwm_channel).hpoint);
}



