/* 
 * Author(s):         Lucas Moiseyev
 * Date Created:      11/30/20
 * Date Last Updated: 12/4/20
 *
 */
#include <esp_wifi.h>
#include <esp_event.h>
#include <esp_log.h>
#include <esp_system.h>
#include <nvs_flash.h>
#include <sys/param.h>
#include "nvs_flash.h"
#include "esp_netif.h"
#include "esp_eth.h"
#include "protocol_examples_common.h"
#include <esp_http_server.h>
#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "driver/gpio.h"
#include "sdkconfig.h"
#include "driver/ledc.h"
#include "esp_err.h"

#include "fan_pwm.h"

/* MiGroBox ESP32s2 code - ECE Capstone Fall 2020 
  
    - Runs web server to handle http GET requests
    - Controls stepper drivers
    - Controls power relays
*/
#define DECIMAL       10

#define PUMP_GPIO CONFIG_PUMP_RELAY_GPIO
#define LIGHT_GPIO CONFIG_LIGHT_RELAY_GPIO

#define Z_DIR_GPIO   10
#define Z_STEP_GPIO  11
#define Z_EN_GPIO    12

#define Y_DIR_GPIO   13
#define Y_STEP_GPIO  14
#define Y_EN_GPIO    15

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

#define Z_ENDSTOP_GPIO         20
#define Y_ENDSTOP_GPIO         21
#define ENDSTOP_GPIO_PIN_SEL   ((1ULL<<Z_ENDSTOP_GPIO) | (1ULL<<Y_ENDSTOP_GPIO))

#define ESP_INTR_FLAG_DEFAULT  0

#define FAN_PWM_LS_TIMER            LEDC_TIMER_0
#define FAN_PWM_LS_MODE             LEDC_LOW_SPEED_MODE

#define FAN_1_PWM_LS_GPIO           (16)
#define FAN_1_PWM_LS_CHANNEL        LEDC_CHANNEL_2

static const char *TAG = "MiGroBox Main";

//typedef struct fan_pwm_t fan_pwm_t;

fan_pwm_t fan_1;

int lights_value;
int pump_value;
int fan_1_value;
int fan_2_value;
int temperature_value;
int humidity_value;

int z_distance;
const int z_speed = 10;
const int z_lower_bound = -2000;
const int z_upper_bound = 2000;

int y_distance;
const int y_speed = 10;
const int y_lower_bound = -2000;
const int y_upper_bound = 2000;

ledc_channel_config_t z_pwm_channel;
ledc_channel_config_t y_pwm_channel;

TaskHandle_t z_endstop_xHandle = NULL;
TaskHandle_t y_endstop_xHandle = NULL; 

//static xQueueHandle gpio_evt_queue = NULL;

void IRAM_ATTR endstop_isr_handler(void* arg){

    TaskHandle_t *endstop_xHandle = arg;
    //xQueueGenericSendFromISR(gpio_evt_quque, &gpio_num, NULL)
    //xQueueSendFromISR(gpio_evt_queue, &gpio_num, NULL);
    xTaskResumeFromISR(*endstop_xHandle);
}
/*
static void IRAM_ATTR y_endstop_isr_handler(void* arg){
    uint32_t gpio_num = (uint32_t) arg;
    //xQueueGenericSendFromISR(gpio_evt_quque, &gpio_num, NULL)
    //xQueueSendFromISR(gpio_evt_queue, &gpio_num, NULL);
    xTaskResumeFromISR(y_endstop_xHandle);
}
*/

void endstop_task(void* arg){
    ledc_channel_config_t *pwm_channel = arg;
    //Loop forever, same as while(1)
    for(;;){
        //if(xQueueReceive(gpio_evt_queue, &io_num, portMAX_DELAY)){
            ledc_stop(pwm_channel->speed_mode, pwm_channel->channel, 0);
        //    ESP_LOGI(TAG, "%d ENDSTOP HIT, %d MOTOR STOPPED!", (int)(pwm_channel->gpio_num));
        ESP_LOGI(TAG, "ENDSTOP HIT, MOTOR STOPPED!");
        //}
        vTaskSuspend(NULL);
    }
}

void setup_gpio(){   
    //Relay Peripherals
    gpio_reset_pin(PUMP_GPIO);
    gpio_set_direction(PUMP_GPIO, GPIO_MODE_OUTPUT);
    gpio_reset_pin(LIGHT_GPIO); 
    gpio_set_direction(LIGHT_GPIO, GPIO_MODE_OUTPUT);
    
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

    //CNC Endstops
  
   // gpio_reset_pin(Z_ENDSTOP_GPIO); 
  //  gpio_set_direction(Z_ENDSTOP_GPIO, GPIO_INTR_NEGEDGE);



  
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
//    z_endstop_xHandle = (TaskHandle_t) xTaskCreate(endstop_task, "z_endstop_task", 2048, (void *)(&z_pwm_channel), 5, NULL);
//    vTaskSuspend(z_endstop_xHandle);
    //create endstop task
    //BaseType_t xReturned_y_endstop;
   // y_endstop_xHandle = xTaskCreate(y_endstop_task, "y_endstop_task", 2048, NULL, 5, NULL);
   // vTaskSuspend(z_endstop_xHandle);

    //install gpio isr service
 //   gpio_install_isr_service(ESP_INTR_FLAG_DEFAULT);
    //hook isr handler for specific gpio pin
//    gpio_isr_handler_add(Z_ENDSTOP_GPIO, endstop_isr_handler, (void*) z_endstop_xHandle);
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

    //ledc_channel_config_t 
    z_pwm_channel = (ledc_channel_config_t){
        .channel    = Z_PWM_LS_CH0_CHANNEL,
        .duty       = 0,
        .gpio_num   = Z_PWM_LS_CH0_GPIO,
        .speed_mode = PWM_LS_MODE,
        .hpoint     = 0,
        .timer_sel  = PWM_LS_TIMER
    };

    //ledc_channel_config_t
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

void tick_z_stepper(int dist, int freq){
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

    //ledc_set_duty(z_pwm_channel.speed_mode, z_pwm_channel.channel, PWM_TEST_DUTY);
    //ledc_update_duty(z_pwm_channel.speed_mode, z_pwm_channel.channel);

    ledc_set_duty_and_update(z_pwm_channel.speed_mode, z_pwm_channel.channel, PWM_TEST_DUTY, z_pwm_channel.hpoint);
    ledc_set_freq(z_pwm_channel.speed_mode, z_pwm_channel.timer_sel, freq);
}

void tick_y_stepper(int dist, int freq){
    //Enable Y stepper driver
    enable_y_stepper_driver();

    //Set Y stepper direction
    if(dist < 0) gpio_set_level(Y_DIR_GPIO, 1);
    else gpio_set_level(Y_DIR_GPIO, 0);

    ledc_set_duty(y_pwm_channel.speed_mode, y_pwm_channel.channel, PWM_TEST_DUTY);
    ledc_update_duty(y_pwm_channel.speed_mode, y_pwm_channel.channel);
    ledc_set_freq(y_pwm_channel.speed_mode, y_pwm_channel.timer_sel, freq);
   // ledc_set_duty_and_update(y_pwm_channel.speed_mode, y_pwm_channel.channel, PWM_TEST_DUTY, y_pwm_channel.hpoint);
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
            device_value = fan_1.duty;
            itoa(device_value, device_value_str, DECIMAL);
        } else if (device_value >= 0 && device_value <= fan_1.max_duty){
            set_fan_duty(&fan_1, (uint32_t)device_value);
            if (device_value == 0) ledc_stop((fan_1.fan_pwm_channel).speed_mode, (fan_1.fan_pwm_channel).channel, 0);
        } else {
            ESP_LOGI(TAG, "INVALID FAN_1 INPUT, CHOOSE VALUE BETWEEN -1 and %d", fan_1.max_duty); 
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

static const httpd_uri_t hello = {
    .uri       = "/hello",
    .method    = HTTP_GET,
    .handler   = get_handler,
    /* Let's pass response string in user
     * context to demonstrate its usage */
    .user_ctx  = "Hello World!"
};

static const httpd_uri_t get = {
    .uri       = "/get",
    .method    = HTTP_GET,
    .handler   = get_handler,
    .user_ctx  = "GET request served"
};

/* An HTTP POST handler */
static esp_err_t echo_post_handler(httpd_req_t *req)
{
    char buf[100];
    int ret, remaining = req->content_len;

    while (remaining > 0) {
        /* Read the data for the request */
        if ((ret = httpd_req_recv(req, buf,
                        MIN(remaining, sizeof(buf)))) <= 0) {
            if (ret == HTTPD_SOCK_ERR_TIMEOUT) {
                /* Retry receiving if timeout occurred */
                continue;
            }
            return ESP_FAIL;
        }

        /* Send back the same data */
        httpd_resp_send_chunk(req, buf, ret);
        remaining -= ret;

        /* Log data received */
        ESP_LOGI(TAG, "=========== RECEIVED DATA ==========");
        ESP_LOGI(TAG, "%.*s", ret, buf);
        ESP_LOGI(TAG, "====================================");
    }

    // End response
    httpd_resp_send_chunk(req, NULL, 0);
    return ESP_OK;
}

static const httpd_uri_t echo = {
    .uri       = "/echo",
    .method    = HTTP_POST,
    .handler   = echo_post_handler,
    .user_ctx  = NULL
};

/* This handler allows the custom error handling functionality to be
 * tested from client side. For that, when a PUT request 0 is sent to
 * URI /ctrl, the /hello and /echo URIs are unregistered and following
 * custom error handler http_404_error_handler() is registered.
 * Afterwards, when /hello or /echo is requested, this custom error
 * handler is invoked which, after sending an error message to client,
 * either closes the underlying socket (when requested URI is /echo)
 * or keeps it open (when requested URI is /hello). This allows the
 * client to infer if the custom error handler is functioning as expected
 * by observing the socket state.
 */
esp_err_t http_404_error_handler(httpd_req_t *req, httpd_err_code_t err)
{
    if (strcmp("/hello", req->uri) == 0) {
        httpd_resp_send_err(req, HTTPD_404_NOT_FOUND, "/hello uri is not available");
        /* return esp_ok to keep underlying socket open */
        return ESP_OK;
    } else if (strcmp("/get", req->uri) == 0) {
        httpd_resp_send_err(req, HTTPD_404_NOT_FOUND, "/get uri is not available");
        /* return esp_ok to keep underlying socket open */
        return ESP_OK;
    } else if (strcmp("/echo", req->uri) == 0) {
        httpd_resp_send_err(req, HTTPD_404_NOT_FOUND, "/echo URI is not available");
        /* Return ESP_FAIL to close underlying socket */
        return ESP_FAIL;
    }
    /* For any other URI send 404 and close socket */
    httpd_resp_send_err(req, HTTPD_404_NOT_FOUND, "Some 404 error message");
    return ESP_FAIL;
}

/* An HTTP PUT handler. This demonstrates realtime
 * registration and deregistration of URI handlers
 */
static esp_err_t ctrl_put_handler(httpd_req_t *req)
{
    char buf;
    int ret;

    if ((ret = httpd_req_recv(req, &buf, 1)) <= 0) {
        if (ret == HTTPD_SOCK_ERR_TIMEOUT) {
            httpd_resp_send_408(req);
        }
        return ESP_FAIL;
    }

    if (buf == '0') {
        /* URI handlers can be unregistered using the uri string */
        ESP_LOGI(TAG, "Unregistering /hello, /get, and /echo URIs");
        httpd_unregister_uri(req->handle, "/hello");
        httpd_unregister_uri(req->handle, "/get");
        httpd_unregister_uri(req->handle, "/echo");
        /* Register the custom error handler */
        httpd_register_err_handler(req->handle, HTTPD_404_NOT_FOUND, http_404_error_handler);
    }
    else {
        ESP_LOGI(TAG, "Registering /hello, /get, and /echo URIs");
        httpd_register_uri_handler(req->handle, &hello);
        httpd_register_uri_handler(req->handle, &get);
        httpd_register_uri_handler(req->handle, &echo);
        /* Unregister custom error handler */
        httpd_register_err_handler(req->handle, HTTPD_404_NOT_FOUND, NULL);
    }

    /* Respond with empty body */
    httpd_resp_send(req, NULL, 0);
    return ESP_OK;
}

static const httpd_uri_t ctrl = {
    .uri       = "/ctrl",
    .method    = HTTP_PUT,
    .handler   = ctrl_put_handler,
    .user_ctx  = NULL
};

static httpd_handle_t start_webserver(void)
{
    httpd_handle_t server = NULL;
    httpd_config_t config = HTTPD_DEFAULT_CONFIG();

    // Start the httpd server
    ESP_LOGI(TAG, "Starting server on port: '%d'", config.server_port);
    if (httpd_start(&server, &config) == ESP_OK) {
        // Set URI handlers
        ESP_LOGI(TAG, "Registering URI handlers");
        httpd_register_uri_handler(server, &hello);
        httpd_register_uri_handler(server, &get);
        httpd_register_uri_handler(server, &echo);
        httpd_register_uri_handler(server, &ctrl);
        return server;
    }

    ESP_LOGI(TAG, "Error starting server!");
    return NULL;
}

static void stop_webserver(httpd_handle_t server)
{
    // Stop the httpd server
    httpd_stop(server);
}

static void disconnect_handler(void* arg, esp_event_base_t event_base,
                               int32_t event_id, void* event_data)
{
    httpd_handle_t* server = (httpd_handle_t*) arg;
    if (*server) {
        ESP_LOGI(TAG, "Stopping webserver");
        stop_webserver(*server);
        *server = NULL;
    }
}

static void connect_handler(void* arg, esp_event_base_t event_base,
                            int32_t event_id, void* event_data)
{
    httpd_handle_t* server = (httpd_handle_t*) arg;
    if (*server == NULL) {
        ESP_LOGI(TAG, "Starting webserver");
        *server = start_webserver();
    }
}




void app_main(void)
{
    static httpd_handle_t server = NULL;

    setup_gpio();
    pwm_config();

    fan_config(&fan_1, FAN_1_PWM_LS_GPIO, FAN_PWM_DEFAULT_FREQUENCY, FAN_PWM_LS_TIMER, FAN_1_PWM_LS_CHANNEL);

    ESP_ERROR_CHECK(nvs_flash_init());
    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());

    /* This helper function configures Wi-Fi or Ethernet, as selected in menuconfig.
     * Read "Establishing Wi-Fi or Ethernet Connection" section in
     * examples/protocols/README.md for more information about this function.
     */
    ESP_ERROR_CHECK(example_connect());

    /* Register event handlers to stop the server when Wi-Fi is disconnected,
     * and re-start it upon connection.
     */
    ESP_ERROR_CHECK(esp_event_handler_register(IP_EVENT, IP_EVENT_STA_GOT_IP, &connect_handler, &server));
    ESP_ERROR_CHECK(esp_event_handler_register(WIFI_EVENT, WIFI_EVENT_STA_DISCONNECTED, &disconnect_handler, &server));

    /* Start the server for the first time */
    server = start_webserver();
}
