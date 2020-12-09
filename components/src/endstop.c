/* 
 * Author(s):         Lucas Moiseyev
 * Date Created:      12/4/20
 * Date Last Updated: 12/6/20
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
#include "endstop.h"
#include "esp_rom_sys.h"

#include <string.h>
#include "esp_rom_sys.h"

/* MiGroBox endstop code - ECE Capstone Fall 2020 
*/

///*static*/ xQueueHandle endstop_evt_queue = NULL;
/*
static void IRAM_ATTR endstop_isr_handler(void* arg){
    endstop_t *endstop = arg;
    //xTaskResumeFromISR(*endstop_xHandle);
    xQueueSendFromISR(*(endstop->evt_queue), endstop, NULL);
}

static void endstop_task(void* arg){
    endstop_t *endstop = arg;
    //Loop forever, same as while(1)
    for(;;){
        if(xQueueReceive(*(endstop->evt_queue), endstop, portMAX_DELAY)) {
        //  vTaskSuspend(NULL);
            endstop->pressed_status = 1;
            ledc_stop(((endstop->stepper)->stepper_pwm_channel).speed_mode, ((endstop->stepper)->stepper_pwm_channel).channel, 0);
            ESP_LOGW(endstop->TAG, "ENDSTOP HIT, MOTOR STOPPED!");
        }
    }
}
*/


endstop_t* init_endstop(){
    endstop_t* endstop_p = malloc(sizeof(endstop_t));
    if(endstop_p == NULL){
        printf("ENDSTOP INIT: MALLOC FAILED!");
    }   
    return endstop_p;
}


esp_err_t config_endstop(char *TAG, endstop_t* endstop, stepper_t *stepper, uint32_t gpio, short min_max_axis, xQueueHandle *evt_queue/*, TaskFunction_t *endstop_isr_handler*/){
   
    ESP_LOGW(TAG, "ENTERING ENDSTOP CONFIG");

    endstop->evt_queue = evt_queue;
    
    endstop->gpio = gpio;
    gpio_config_t endstop_gpio_conf;
    endstop_gpio_conf.intr_type = GPIO_INTR_NEGEDGE;       //interrupt of falling edge
    endstop_gpio_conf.pin_bit_mask = (1ULL<<gpio);         //bit mask of the pins
    endstop_gpio_conf.mode = GPIO_MODE_INPUT;              //set as input mode
    endstop_gpio_conf.pull_up_en = 0;
    endstop_gpio_conf.pull_down_en = 1;                    //enable pull-down mode
    gpio_config(&endstop_gpio_conf);

    endstop->pressed_status = 0;
    endstop->min_max_axis = min_max_axis;
    endstop->stepper = stepper;

    //char *task_handle
/*
    endstop->endstop_taskHandle = (TaskHandle_t) xTaskCreate(endstop_task, "endstop_task", 2048, (void *) endstop, 5, NULL);  //create endstop task
    //gpio_install_isr_service(ENDSTOP_INTR_FLAG);                                                                  //install gpio isr service
    gpio_isr_handler_add(gpio, endstop_isr_handler, endstop);                                             //hook isr handler for specific gpio pin
*/

    strncpy(endstop->TAG, TAG, TAG_SIZE-1);
    endstop->TAG[TAG_SIZE-1] = '\0';

    ESP_LOGW(TAG, "EXITING ENDSTOP CONFIG");

    //return ESP_OK;
    
    return endstop;
}

char get_endstop_status(endstop_t *endstop){
    return endstop->pressed_status;
}




////===========================

/*
extern portMUX_TYPE rtc_spinlock; //TODO: Will be placed in the appropriate position after the rtc module is finished.
#define TOUCH_ENTER_CRITICAL_SAFE()  portENTER_CRITICAL_SAFE(&rtc_spinlock) // Can be called in isr and task.
#define TOUCH_EXIT_CRITICAL_SAFE()  portEXIT_CRITICAL_SAFE(&rtc_spinlock)
#define TOUCH_ENTER_CRITICAL()  portENTER_CRITICAL(&rtc_spinlock)
#define TOUCH_EXIT_CRITICAL()  portEXIT_CRITICAL(&rtc_spinlock)

static SemaphoreHandle_t rtc_touch_mux = NULL;




esp_err_t touch_pad_isr_register(intr_handler_t fn, void *arg, touch_pad_intr_mask_t intr_mask)
{
    static bool reg_flag = false;
    TOUCH_CHECK(fn != NULL, TOUCH_PARAM_CHECK_STR("intr_mask"), ESP_ERR_INVALID_ARG);
    TOUCH_INTR_MASK_CHECK(intr_mask);

    uint32_t en_msk = 0;
    if (intr_mask & TOUCH_PAD_INTR_MASK_DONE) {
        en_msk |= RTC_CNTL_TOUCH_DONE_INT_ST_M;
    }
    if (intr_mask & TOUCH_PAD_INTR_MASK_ACTIVE) {
        en_msk |= RTC_CNTL_TOUCH_ACTIVE_INT_ST_M;
    }
    if (intr_mask & TOUCH_PAD_INTR_MASK_INACTIVE) {
        en_msk |= RTC_CNTL_TOUCH_INACTIVE_INT_ST_M;
    }
    if (intr_mask & TOUCH_PAD_INTR_MASK_SCAN_DONE) {
        en_msk |= RTC_CNTL_TOUCH_SCAN_DONE_INT_ST_M;
    }
    if (intr_mask & TOUCH_PAD_INTR_MASK_TIMEOUT) {
        en_msk |= RTC_CNTL_TOUCH_TIMEOUT_INT_ST_M;
    }
    esp_err_t ret = rtc_isr_register(fn, arg, en_msk);
//    / Must ensure: After being registered, it is executed first. /
    if ( (ret == ESP_OK) && (reg_flag == false) && (intr_mask & (TOUCH_PAD_INTR_MASK_SCAN_DONE | TOUCH_PAD_INTR_MASK_TIMEOUT)) ) {
        rtc_isr_register(touch_pad_workaround_isr_internal, NULL, RTC_CNTL_TOUCH_SCAN_DONE_INT_ST_M | RTC_CNTL_TOUCH_TIMEOUT_INT_ST_M);
        reg_flag = true;
    }

    return ret;
}






esp_err_t touch_pad_intr_disable(touch_pad_intr_mask_t int_mask)
{
    TOUCH_INTR_MASK_CHECK(int_mask);
    TOUCH_ENTER_CRITICAL();
    touch_hal_intr_disable(int_mask);
    TOUCH_EXIT_CRITICAL();
    return ESP_OK;
}

esp_err_t touch_pad_intr_clear(touch_pad_intr_mask_t int_mask)
{
    TOUCH_INTR_MASK_CHECK(int_mask);
    TOUCH_ENTER_CRITICAL();
    touch_hal_intr_clear(int_mask);
    TOUCH_EXIT_CRITICAL();
    return ESP_OK;
}


*/








