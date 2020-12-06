/* 
 * Author(s):         Lucas Moiseyev
 * Date Created:      12/5/20
 * Date Last Updated: 12/5/20
 *
 */
#include <esp_log.h>
#include <esp_system.h>
#include <nvs_flash.h>
#include <sys/param.h>
#include "nvs_flash.h"
#include "esp_netif.h"
#include "esp_eth.h"
#include <esp_http_server.h>
#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "driver/gpio.h"
#include "sdkconfig.h"
#include "driver/ledc.h"
#include "esp_err.h"

#include "stepper.h"

/* MiGroBox ESP32s2 stepper motor driver code - ECE Capstone Fall 2020 
    - Controls stepper drivers
*/
#define PWM_LS_TIMER            LEDC_TIMER_0
#define PWM_LS_MODE             LEDC_LOW_SPEED_MODE

#define Z_PWM_LS_CH0_GPIO       (11)
#define Z_PWM_LS_CH0_CHANNEL    LEDC_CHANNEL_0
#define Y_PWM_LS_CH1_GPIO       (14)
#define Y_PWM_LS_CH1_CHANNEL    LEDC_CHANNEL_1

#define PWM_CH_NUM             (1)
#define PWM_TIMER_FREQ         (500)
#define PWM_TEST_DUTY          (1024)
#define PWM_TEST_FADE_TIME     (250)

#define ESP_INTR_FLAG_DEFAULT  0


static const char *TAG = "stepper";

typedef struct stepper_t stepper_t;

struct stepper_t stepper_t{
    uint32_t dir_gpio;
    uint32_t step_gpio;
    uint32_t en_gpio;
    
    int direction;
    int position;
    int max_position;
    int min_position;  
    int travel_distance_command;
    

    ledc_channel_config_t pwm_channel; 
    TaskHandle_t stepper_command_xHandle;

};


void IRAM_ATTR stepper_isr_handler(void* arg){
    uint32_t gpio_num = (uint32_t) arg;

    //xQueueGenericSendFromISR(gpio_evt_quque, &gpio_num, NULL)
    //xQueueSendFromISR(gpio_evt_queue, &gpio_num, NULL);
}
/*
static void IRAM_ATTR y_endstop_isr_handler(void* arg){
    uint32_t gpio_num = (uint32_t) arg;
    //xQueueGenericSendFromISR(gpio_evt_quque, &gpio_num, NULL)
    //xQueueSendFromISR(gpio_evt_queue, &gpio_num, NULL);
    xTaskResumeFromISR(y_endstop_xHandle);
}
*/

static void z_endstop_task(void* arg){
    uint32_t io_num;
    //Loop forever, same as while(1)
    for(;;){
        vTaskSuspend(NULL);
        //if(xQueueReceive(gpio_evt_queue, &io_num, portMAX_DELAY)){
            ledc_stop(z_pwm_channel.speed_mode, z_pwm_channel.channel, 0);
            ESP_LOGI(TAG, "Z ENDSTOP HIT, Z MOTOR STOPPED!");
        //}
    }
}

static void y_endstop_task(void* arg){
    uint32_t io_num;
    //Loop forever, same as while(1)
    for(;;){
        //vTaskSuspend(NULL);
       // if(xQueueReceive(gpio_evt_queue, &io_num, portMAX_DELAY)){
            ledc_stop(y_pwm_channel.speed_mode, y_pwm_channel.channel, 0);
            ESP_LOGI(TAG, "Y ENDSTOP HIT, Y MOTOR STOPPED!");
       // }
    }
}

stepper_t stepper_init(){   
    
    //CNC Stepper Motors
    gpio_reset_pin(Z_DIR_GPIO);
    gpio_set_direction(Z_DIR_GPIO, GPIO_MODE_OUTPUT);
    //gpio_reset_pin(Z_STEP_GPIO); 
    //gpio_set_direction(Z_STEP_GPIO, GPIO_MODE_OUTPUT);
    gpio_reset_pin(Z_EN_GPIO); 
    gpio_set_direction(Z_EN_GPIO, GPIO_MODE_OUTPUT);
    gpio_reset_pin(Y_DIR_GPIO); 
    gpio_set_direction(Y_DIR_GPIO, GPIO_MODE_OUTPUT);
    gpio_reset_pin(Y_STEP_GPIO); 
    gpio_set_direction(Y_STEP_GPIO, GPIO_MODE_OUTPUT);
    gpio_reset_pin(Y_EN_GPIO); 
    gpio_set_direction(Y_EN_GPIO, GPIO_MODE_OUTPUT);




  
    gpio_config_t io_conf;
    //interrupt of rising edge
    io_conf.intr_type = GPIO_INTR_POSEDGE;
    //bit mask of the pins, use GPIO4/5 here
    io_conf.pin_bit_mask = BIT64(Z_ENDSTOP_GPIO);
    //set as input mode
    io_conf.mode = GPIO_MODE_INPUT;
    //enable pull-up mode
    io_conf.pull_up_en = 1;
    gpio_config(&io_conf);

    //interrupt of rising edge
    io_conf.intr_type = GPIO_INTR_NEGEDGE;
    //bit mask of the pins, use GPIO4/5 here
    io_conf.pin_bit_mask = BIT64(Y_ENDSTOP_GPIO);
    //set as input mode
    io_conf.mode = GPIO_MODE_INPUT;
    //enable pull-up mode
    io_conf.pull_up_en = 1;
    gpio_config(&io_conf);


    //create a queue to handle gpio event from isr
   // gpio_evt_queue = xQueueCreate(10, sizeof(uint32_t));
    
    //create endstop task
    //BaseType_t xReturned_z_endstop;
    z_endstop_xHandle = xTaskCreate(z_endstop_task, "z_endstop_task", 2048, NULL, 5, NULL);

    //create endstop task
    //BaseType_t xReturned_y_endstop;
   // y_endstop_xHandle = xTaskCreate(y_endstop_task, "y_endstop_task", 2048, NULL, 5, NULL);


    //install gpio isr service
    gpio_install_isr_service(ESP_INTR_FLAG_DEFAULT);
    //hook isr handler for specific gpio pin
    gpio_isr_handler_add(Z_ENDSTOP_GPIO, z_endstop_isr_handler, (void*) Z_ENDSTOP_GPIO);
    //hook isr handler for specific gpio pin
 //   gpio_isr_handler_add(Y_ENDSTOP_GPIO, gpio_isr_handler, (void*) Y_ENDSTOP_GPIO);



/*
    gpio_reset_pin(Z_ENDSTOP_GPIO); 
    gpio_set_direction(Z_ENDSTOP_GPIO, GPIO_MODE_OUTPUT);
    gpio_reset_pin(Y_ENDSTOP_GPIO); 
    gpio_set_direction(Y_ENDSTOP_GPIO, GPIO_MODE_OUTPUT);
*/


}

void pwm_config(){
    /*
     * Prepare and set configuration of timers
     * that will be used by LED Controller
     */
    ledc_timer_config_t pwm_timer = {
        .duty_resolution = LEDC_TIMER_11_BIT, // resolution of PWM duty
        .freq_hz = PWM_TIMER_FREQ,            // frequency of PWM signal
        .speed_mode = PWM_LS_MODE,            // timer mode
        .timer_num = PWM_LS_TIMER,            // timer index
        .clk_cfg = LEDC_AUTO_CLK,             // Auto select the source clock
    };
    // Set configuration of timer0 for high speed channels
    ledc_timer_config(&pwm_timer);
    
    /*
     * Prepare individual configuration
     * for each channel of LED Controller
     * by selecting:
     * - controller's channel number
     * - output duty cycle, set initially to 0
     * - GPIO number where LED is connected to
     * - speed mode, either high or low
     * - timer servicing selected channel
     *   Note: if different channels use one timer,
     *         then frequency and bit_num of these channels
     *         will be the same
     */
    z_pwm_channel = (ledc_channel_config_t){
        .channel    = Z_PWM_LS_CH0_CHANNEL,
        .duty       = 0,
        .gpio_num   = Z_PWM_LS_CH0_GPIO,
        .speed_mode = PWM_LS_MODE,
        .hpoint     = 0,
        .timer_sel  = PWM_LS_TIMER
    };

    y_pwm_channel = (ledc_channel_config_t){
        .channel    = Y_PWM_LS_CH1_CHANNEL,
        .duty       = 0,
        .gpio_num   = Y_PWM_LS_CH1_GPIO,
        .speed_mode = PWM_LS_MODE,
        .hpoint     = 0,
        .timer_sel  = PWM_LS_TIMER
    };

    // Set LED Controller with previously prepared configuration
    ledc_channel_config(&z_pwm_channel);
    ledc_channel_config(&y_pwm_channel);

    // Initialize fade service.
     ledc_fade_func_install(0);
}

void enable_z_stepper_driver(){
    gpio_set_level(Z_EN_GPIO, 0);
}

void disable_z_stepper_driver(){
    gpio_set_level(Z_EN_GPIO, 1);
}

void enable_y_stepper_driver(){
    gpio_set_level(Y_EN_GPIO, 0);
}

void disable_y_stepper_driver(){
    gpio_set_level(Y_EN_GPIO, 1);
}

void tick_z_stepper(int dist, int speed_ms){
    //Enable Z stepper driver
    enable_z_stepper_driver();

    //Set z stepper direction
    if(dist < 0) gpio_set_level(Z_DIR_GPIO, 1);
    else gpio_set_level(Z_DIR_GPIO, 0);


    //fade up
    /*
    ledc_set_fade_with_time(ledc_channel[ch].speed_mode, ledc_channel[ch].channel, LEDC_TEST_DUTY, LEDC_TEST_FADE_TIME);
    ledc_fade_start(ledc_channel[ch].speed_mode, ledc_channel[ch].channel, LEDC_FADE_NO_WAIT);
    */
    
    ledc_set_duty(z_pwm_channel.speed_mode, z_pwm_channel.channel, PWM_TEST_DUTY);
    ledc_update_duty(z_pwm_channel.speed_mode, z_pwm_channel.channel);

   // ledc_set_duty_and_update(z_pwm_channel.speed_mode, z_pwm_channel.channel, PWM_TEST_DUTY, z_pwm_channel.hpoint);
/*
    for(int i = 0; i < abs(dist); i++){
        gpio_set_level(Z_STEP_GPIO, 1);
        vTaskDelay(speed_ms / portTICK_PERIOD_MS);
        gpio_set_level(Z_STEP_GPIO, 0);
        vTaskDelay(speed_ms / portTICK_PERIOD_MS); 
    }

    disable_z_stepper_driver();
}
*/
}

void tick_y_stepper(int dist, int speed_ms){
    //Enable Y stepper driver
    enable_y_stepper_driver();

    //Set Y stepper direction
    if(dist < 0) gpio_set_level(Y_DIR_GPIO, 1);
    else gpio_set_level(Y_DIR_GPIO, 0);
/* 
    for(int i = 0; i < abs(dist); i++){
        gpio_set_level(Y_STEP_GPIO, 1);
        vTaskDelay(speed_ms / portTICK_PERIOD_MS);
        gpio_set_level(Y_STEP_GPIO, 0);
        vTaskDelay(speed_ms / portTICK_PERIOD_MS); 
    }

*/ 

 
    ledc_set_duty(y_pwm_channel.speed_mode, y_pwm_channel.channel, PWM_TEST_DUTY);
    ledc_update_duty(y_pwm_channel.speed_mode, y_pwm_channel.channel);


   // ledc_set_duty_and_update(y_pwm_channel.speed_mode, y_pwm_channel.channel, PWM_TEST_DUTY, y_pwm_channel.hpoint);

    //disable_y_stepper_driver();
}


/* An HTTP GET handler */
static esp_err_t get_handler(httpd_req_t *req)
{
    char* buf;
    size_t buf_len;

    bool read_device = false;
    char device[32] = "not_selected";
    char device_value_str[32] = "-2";
    int device_value = -2;

    /* Read URL query string length and allocate memory for length + 1,
     * extra byte for null termination */
    buf_len = httpd_req_get_url_query_len(req) + 1;
    if (buf_len > 1) {
        buf = malloc(buf_len);
        if (httpd_req_get_url_query_str(req, buf, buf_len) == ESP_OK) {
            ESP_LOGI(TAG, "Found URL query => %s", buf);
            char param[32];
            /* Get value of expected key from query string */
            if (httpd_query_key_value(buf, "dev", param, sizeof(param)) == ESP_OK) {
                ESP_LOGI(TAG, "Found URL query parameter => dev=%s", param);
                strcpy(device, param);
            }
            if (httpd_query_key_value(buf, "val", param, sizeof(param)) == ESP_OK) {
                ESP_LOGI(TAG, "Found URL query parameter => val=%s", param); 
                device_value = atoi(param);
                if(device_value == -1) read_device = true;
                strcpy(device_value_str, param);
            }
        }
        free(buf);
    }

    if(strcmp(device, "not_selected") == 0){
        ESP_LOGI(TAG, "NO DEVICE SELECTED");
    } else if(strcmp(device, "lights") == 0){ 
        ESP_LOGI(TAG, "Lights command found...");
        if (device_value == -1) {
            device_value = lights_value;
            itoa(device_value, device_value_str, DECIMAL);
        } else if (device_value == 0 || device_value == 1){
            lights_value = device_value;
            gpio_set_level(LIGHT_GPIO, device_value);
        } else {
            ESP_LOGI(TAG, "INVALID LIGHT INPUT, CHOOSE 0 OR 1");
        }
    } else if(strcmp(device, "pump") == 0){
        ESP_LOGI(TAG, "Pump command found...");
        if (device_value == -1){
            device_value = pump_value;
            itoa(device_value, device_value_str, DECIMAL);
        } else if (device_value == 0 || device_value == 1){
            pump_value = device_value;
            gpio_set_level(PUMP_GPIO, 0x1 ^ device_value);
        } else {
            ESP_LOGI(TAG, "INVALID PUMP INPUT, CHOOSE -1, 0, OR 1"); 
        }
    } else if(strcmp(device, "fan_1") == 0){ 
        ESP_LOGI(TAG, "Fan_1 command found...");
        if (device_value == -1) {
            device_value = fan_1_value;
            itoa(device_value, device_value_str, DECIMAL);
        } else if (device_value >= 0 && device_value <= 255){
            fan_1_value = device_value;
        } else {
            ESP_LOGI(TAG, "INVALID FAN_1 INPUT, CHOOSE VALUE BETWEEN -1 and 255"); 
        }
    } else if(strcmp(device, "fan_2") == 0){ 
        ESP_LOGI(TAG, "Fan_2 command found...");
        if (device_value == -1) {
            device_value = fan_2_value;
            itoa(device_value, device_value_str, DECIMAL);
        } else if (device_value >= 0 && device_value <= 255){
            fan_2_value = device_value;
        } else {
            ESP_LOGI(TAG, "INVALID FAN_2 INPUT, CHOOSE VALUE BETWEEN -1 and 255"); 
        }
    } else if(strcmp(device, "z_stepper") == 0){
        ESP_LOGI(TAG, "Z stepper motor command found...");
        if (device_value >= z_lower_bound && device_value <= z_upper_bound){
            z_distance = device_value;
            tick_z_stepper(z_distance, z_speed);    
        } else {
            ESP_LOGI(TAG, "INVALID Z STEPPER INPUT, CHOOSE VALUE BETWEEN %d and  %d", z_lower_bound, z_upper_bound); 
        } 
    } else if(strcmp(device, "y_stepper") == 0){
        ESP_LOGI(TAG, "Y stepper motor command found...");
        /*if (device_value == -1){
            device_value = y_distance;
            itoa(device_value, device_value_str, DECIMAL);
        } else*/
        if (device_value >= y_lower_bound && device_value <= y_upper_bound){
            y_distance = device_value;
            tick_y_stepper(y_distance, y_speed);    
        } else {
            ESP_LOGI(TAG, "INVALID Y STEPPER INPUT, CHOOSE VALUE BETWEEN %d and %d", y_lower_bound, y_upper_bound); 
        } 
    } else{
        ESP_LOGI(TAG, "INVALID DEVICE INPUT");
    }
    
    if (read_device){
        ESP_LOGI(TAG, "RESPONDING WITH CURRENT VALUE OF DEVICE: %s is %d", device, device_value);
    } else if (device_value == -2){
        ESP_LOGI(TAG, "NO INPUT VALUE IN GET REQUEST!");
    }

    const char* resp_str = device_value_str; //strcat(strcat("DEVICE: ", device), strcat("VALUE: ", device_value_str));//(const char*) req->user_ctx;
    /* Set some custom headers */
    if(strcmp(device, "not_selected") != 0){
        httpd_resp_set_hdr(req, "DEVICE: ", device);
    }
   
    httpd_resp_send(req, resp_str, HTTPD_RESP_USE_STRLEN);

    /* After sending the HTTP response the old HTTP request
     * headers are lost. Check if HTTP request headers can be read now. */
    if (httpd_req_get_hdr_value_len(req, "Host") == 0) {
        ESP_LOGI(TAG, "Request headers lost");
    }
    return ESP_OK;
}

