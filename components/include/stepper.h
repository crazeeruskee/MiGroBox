/* 
 * Author(s):         Lucas Moiseyev
 * Date Created:      12/5/20
 * Date Last Updated: 12/5/20
 *
 */

#pragma once

#include <stdlib.h>
#include "esp_err.h"
#include "driver/gpio.h"
#include "driver/ledc.h"
#include "driver/pcnt.h"
#include "esp_rom_sys.h"

/* MiGroBox ESP32s2 stepper motor driver code - ECE Capstone Fall 2020 
    - Controls stepper drivers
*/

#define STEPPER_PWM_DUTY_RES           LEDC_TIMER_11_BIT
#define STEPPER_PWM_LS_MODE            LEDC_LOW_SPEED_MODE
#define STEPPER_PWM_DUTY_DEFAULT       (1024)
#define STEPPER_PWM_TIMER_FREQ_DEFAULT (500)

//#define 


#define ESP_INTR_FLAG_DEFAULT  0

#define TAG_SIZE 32

/* A sample structure to pass events from the PCNT
 * interrupt handler to the main program.
 *//*
typedef struct {
    int unit;  // the PCNT unit that originated an interrupt
    uint32_t status; // information on the event type that caused the interrupt
} pcnt_evt_t;
*/
typedef struct {
    uint32_t               dir_gpio;
    uint32_t               step_gpio;
    uint32_t               en_gpio;

    int                    max_position;
    int                    min_position;  
 
    int                    direction;
    int                    position;
    int                    travel_distance_command;

    ledc_timer_config_t    stepper_pwm_timer;
    ledc_channel_config_t  stepper_pwm_channel;

    //xQueueHandle           step_count_evt_queue;
    //pcnt_evt_t             step_count_evt;
    
    //TaskHandle_t stepper_command_xHandle;
    char                 TAG[TAG_SIZE];
} stepper_t;


/**
 * @brief Default configuration for pwm stepper
 *
 */
#define STEPPER_DEFAULT_CONFIG(){ \
    .dir_gpio = 0,                \
    .step_gpio = 0,               \
    .en_gpio = 0,                 \
    .max_position = 0,            \
    .min_position = 0,            \
    .direction = 0,               \
    .position = 0,                \
    .travel_distance_command = 0, \
    .fan_pwm_timer = NULL,        \
    .fan_pwm_channel = NULL       \
    .TAG = "Stepper",             \
}

//void IRAM_ATTR stepper_isr_handler(void* arg);

//static void IRAM_ATTR y_endstop_isr_handler(void* arg);

stepper_t* init_stepper();

esp_err_t config_stepper(stepper_t *stepper, char *TAG, ledc_timer_t timer_sel, ledc_channel_t channel_sel, uint32_t dir_gpio, uint32_t step_gpio, uint32_t en_gpio, int max_position, int min_position);

void enable_stepper_driver(stepper_t *stepper);

void disable_stepper_driver(stepper_t *stepper);

void stop_stepper_driver(stepper_t *stepper);

void tick_stepper(stepper_t *stepper, int travel_distance_command);



