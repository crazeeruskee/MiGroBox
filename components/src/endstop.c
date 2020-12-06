/* 
 * Author(s):         Lucas Moiseyev
 * Date Created:      12/4/20
 * Date Last Updated: 12/6/20
 *
 */
#pragma once

#include <stdlib.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/timers.h"
#include "esp_compiler.h"
#include "esp_log.h"
#include "driver/dedic_gpio.h"
#include "driver/gpio.h"
#include "driver/ledc.h"
#include "endstop.h"
#include "esp_rom_sys.h"

/* MiGroBox endstop code - ECE Capstone Fall 2020 

*/
#define Z_ENDSTOP_GPIO         21
#define Y_ENDSTOP_GPIO         22
#define ENDSTOP_GPIO_PIN_SEL   ((1ULL<<Z_ENDSTOP_GPIO) | (1ULL<<Y_ENDSTOP_GPIO))

#define ESP_INTR_FLAG_DEFAULT  0

static const char *TAG = "Endstop";

struct endstop_t {
    uint32_t gpio;
    char pressed_status; //0 = released, 1 = pressed
    char min_max_axis;   //Endstop at min axis or max axis position, 0 = min, 1 = max
    stepper_t* stepper;  //address to stepper driver this endstop is responsible for    
} endstop_t;

void IRAM_ATTR endstop_isr_handler(void* arg){
    TaskHandle_t *endstop_xHandle = arg;
    xTaskResumeFromISR(*endstop_xHandle);
    //xQueueGenericSendFromISR(gpio_evt_quque, &gpio_num, NULL)
    //xQueueSendFromISR(gpio_evt_queue, &gpio_num, NULL);
}

void endstop_task(void* arg){
    endstop_t *endstop = arg;
    //Loop forever, same as while(1)
    for(;;){
        vTaskSuspend(NULL);
        ledc_stop((endstop_t->stepper).stepper_pwm_channel.speed_mode, z_pwm_channel.channel, 0);
        ESP_LOGI(TAG, "ENDSTOP HIT, MOTOR STOPPED!");
    }
}
esp_err_t config_endstop(endstop_t *endstop, stepper_t *stepper, uint32_t gpio, char min_max_axis){
    endstop->gpio = gpio;
    gpio_config_t endstop_gpio_conf;
    endstop_gpio_conf.intr_type = GPIO_INTR_NEGEDGE;       //interrupt of rising edge
    endstop_gpio_conf.pin_bit_mask = (1ULL<<gpio);                 //bit mask of the pins, use GPIO4/5 here
    endstop_gpio_conf.mode = GPIO_MODE_INPUT;              //set as input mode
    endstop_gpio_conf.pull_up_en = 1;                      //enable pull-up mode
    gpio_config(&endstop_gpio_conf);

    endstop->pressed_status = 0;
    endstop->min_max_axis = min_max_axis;
    endstop->stepper = stepper;

    //create endstop task
    TaskHandle_t xHandle_z_endstop = xTaskCreate(endstop_task, "endstop_task", 2048, (void *) endstop, 1, NULL);
    gpio_install_isr_service(ESP_INTR_FLAG_DEFAULT); //install gpio isr service
    gpio_isr_handler_add(gpio, endstop_isr_handler, (void*) endstop); //hook isr handler for specific gpio pin

    return ESP_OK;
}

char get_endstop_status(endstop_t *endstop){
    return endstop->pressed_status;
}


