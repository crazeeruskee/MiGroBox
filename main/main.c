/* 
 * Author(s):         Lucas Moiseyev
 * Date Created:      11/30/20
 * Date Last Updated: 1/13/21
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

#include <string.h>
#include "esp_rom_sys.h"

#include "fan_pwm.h"
#include "endstop.h"
#include "stepper.h"
#include "real_time.h"

/* MiGroBox ESP32s2 code - ECE Capstone Fall 2020 
  
    - Runs web server to handle http GET requests
    - Controls stepper drivers
    - Controls power relays
*/
#define DECIMAL                 10

#define RELAY_1_GPIO            (21)
#define RELAY_8_GPIO            (38)
#define LIGHT_GPIO              RELAY_1_GPIO//CONFIG_LIGHT_RELAY_GPIO
#define PUMP_GPIO               RELAY_8_GPIO//CONFIG_PUMP_RELAY_GPIO

#define Z_DIR_GPIO              (10)
#define Z_EN_GPIO               (12)

#define Y_DIR_GPIO              (13)
#define Y_EN_GPIO               (15)

#define STEPPER_PWM_LS_TIMER    LEDC_TIMER_0
#define Z_PWM_LS_GPIO           (11)
#define Z_PWM_LS_CHANNEL        LEDC_CHANNEL_0
#define Y_PWM_LS_GPIO           (14)
#define Y_PWM_LS_CHANNEL        LEDC_CHANNEL_1

#define Z_ENDSTOP_MIN_GPIO      18
#define Z_ENDSTOP_MAX_GPIO      19
#define Y_ENDSTOP_MIN_GPIO      20
#define Y_ENDSTOP_MAX_GPIO      21

#define ESP_INTR_FLAG_DEFAULT   0

#define FAN_PWM_LS_TIMER        LEDC_TIMER_1

#define FAN_1_PWM_LS_GPIO       (16)
#define FAN_1_PWM_LS_CHANNEL    LEDC_CHANNEL_2
#define FAN_2_PWM_LS_GPIO       (17)
#define FAN_2_PWM_LS_CHANNEL    LEDC_CHANNEL_3


//#define ENDSTOP_MIN_Z
//#define ENDSTOP_MAX_Z
//#define ENDSTOP_MIN_Y
//#define ENDSTOP_MAX_Y

static const char *TAG = "MiGroBox Main";

time_t* now_p;

fan_pwm_t* fan_1_p = NULL;
fan_pwm_t* fan_2_p = NULL;

stepper_t* z_stepper_p = NULL;
stepper_t* y_stepper_p = NULL;

int travel_distance = 100;
int lights_value;
int pump_value;
int temperature_value;
int humidity_value;

//TaskHandle_t z_endstop_xHandle = NULL;
//TaskHandle_t y_endstop_xHandle = NULL; 
static xQueueHandle evt_queue = NULL;

static void IRAM_ATTR gpio_isr_handler(void* arg){
    endstop_t* endstop_p = (endstop_t *) arg;
    xQueueSendFromISR(evt_queue, (void *) endstop_p, NULL);
}

static void endstop_task(void* arg)
{
    endstop_t endstop_p;
    for(;;) {
        if(xQueueReceive(evt_queue, (void *) &endstop_p, portMAX_DELAY)) {
            if (gpio_get_level(endstop_p.gpio) == 0){
                    endstop_p.pressed_status = 1;
                    ESP_LOGW(TAG, "%s HIT!", endstop_p.TAG); 
                    if(endstop_p.min_max_axis == 0){
                        (endstop_p.stepper)->position = (endstop_p.stepper)->min_position;
                    } else if(endstop_p.min_max_axis == 1){
                        (endstop_p.stepper)->position = (endstop_p.stepper)->max_position;
                    } else{
                        ESP_LOGE(TAG, "%s.min_max_axis set incorrectly! Must be 0 or 1", endstop_p.TAG);
                    }
                    ledc_stop(((endstop_p.stepper)->stepper_pwm_channel).speed_mode, ((endstop_p.stepper)->stepper_pwm_channel).channel, 0);
                    travel_distance = -travel_distance;
                    tick_stepper(endstop_p.stepper, travel_distance);
                } else if(gpio_get_level(endstop_p.gpio) == 1){
                    endstop_p.pressed_status = 0;
                } else{
                    printf("ERROR: NON-ZERO GPIO LEVEL FOR ENDSTOP MIN PIN!");
                }
        }
    }
}
   
void setup_gpio(){   
    //Relay Peripherals
    gpio_reset_pin(PUMP_GPIO);
    gpio_set_direction(PUMP_GPIO, GPIO_MODE_OUTPUT);
    gpio_reset_pin(LIGHT_GPIO); 
    gpio_set_direction(LIGHT_GPIO, GPIO_MODE_OUTPUT);
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
            device_value = fan_1_p->duty;
            itoa(device_value, device_value_str, DECIMAL);
        } else if (device_value >= 0 && device_value <= fan_1_p->max_duty){
            set_fan_duty(fan_1_p, (uint32_t)device_value);
            if (device_value == 0) ledc_stop((fan_1_p->fan_pwm_channel).speed_mode, (fan_1_p->fan_pwm_channel).channel, 0);
        } else {
            ESP_LOGW(TAG, "INVALID FAN_1 INPUT, CHOOSE VALUE BETWEEN -1 and %d", fan_1_p->max_duty); 
        }
    } else if(strcmp(device, "fan_2") == 0){ 
        ESP_LOGI(TAG, "Fan_2 command found...");
        if (device_value == -1) {
            device_value = fan_2_p->duty;
            itoa(device_value, device_value_str, DECIMAL);
        } else if (device_value >= 0 && device_value <= fan_2_p->max_duty){
            set_fan_duty(fan_2_p, (uint32_t)device_value);
            if (device_value == 0) ledc_stop((fan_2_p->fan_pwm_channel).speed_mode, (fan_2_p->fan_pwm_channel).channel, 0);
        } else {
            ESP_LOGW(TAG, "INVALID FAN_2 INPUT, CHOOSE VALUE BETWEEN -1 and %d", fan_2_p->max_duty); 
        }
    } else if(strcmp(device, "z_stepper") == 0){
        ESP_LOGI(TAG, "Z stepper motor command found...");
        if ((z_stepper_p->position + device_value) >= z_stepper_p->min_position && (z_stepper_p->position + device_value) <= z_stepper_p->max_position){
            z_stepper_p->travel_distance_command = device_value;
            tick_stepper(z_stepper_p, z_stepper_p->travel_distance_command);    
        } else {
            ESP_LOGI(TAG, "INVALID Z STEPPER INPUT, CHOOSE VALUE BETWEEN %d and  %d", z_stepper_p->min_position, z_stepper_p->max_position); 
        } 
    } else if(strcmp(device, "y_stepper") == 0){
        ESP_LOGI(TAG, "Y stepper motor command found...");
        if ((y_stepper_p->position + device_value) >= y_stepper_p->min_position && (y_stepper_p->position + device_value) <= y_stepper_p->max_position){
            y_stepper_p->travel_distance_command = device_value;
            tick_stepper(y_stepper_p, y_stepper_p->travel_distance_command);    
        } else {
            ESP_LOGI(TAG, "INVALID Y STEPPER INPUT, CHOOSE VALUE BETWEEN %d and  %d", y_stepper_p->min_position, y_stepper_p->max_position); 
        } 
    } else{
        ESP_LOGI(TAG, "INVALID DEVICE INPUT");
    }
    
    if (read_device){
        ESP_LOGI(TAG, "RESPONDING WITH CURRENT VALUE OF DEVICE: %s is %d", device, device_value);
    } else if (device_value == -2){
        ESP_LOGI(TAG, "NO INPUT VALUE IN GET REQUEST!");
    }

    struct tm* current_time = get_current_time(now_p);
    char resp_str[32];
    sprintf(resp_str, "%d", current_time->tm_year);
    free(current_time);

        //device_value_str; //strcat(strcat("DEVICE: ", device), strcat("VALUE: ", device_value_str));//(const char*) req->user_ctx;
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

    evt_queue = xQueueCreate(20, sizeof(endstop_t));
    if(evt_queue == NULL){
        ESP_LOGE(TAG, "FAILED TO CREATE INTERRUPT QUEUE!");
        exit(1);
    }

    now_p = real_time_init();

    setup_gpio();

    z_stepper_p = init_stepper();
    config_stepper(z_stepper_p, "Z Stepper", STEPPER_PWM_LS_TIMER, Z_PWM_LS_CHANNEL, Z_DIR_GPIO, Z_PWM_LS_GPIO, Z_EN_GPIO, 2000, 0);
    y_stepper_p = init_stepper();
    config_stepper(y_stepper_p, "Y Stepper", STEPPER_PWM_LS_TIMER, Y_PWM_LS_CHANNEL, Y_DIR_GPIO, Y_PWM_LS_GPIO, Y_EN_GPIO, 4000, 0);

    
    #ifdef ENDSTOP_MIN_Z
    endstop_t* z_endstop_min_p = init_endstop();
    if(z_endstop_min_p == NULL){
        printf("INIT_ENDSTOP RETURNED NULL!");
        exit(1);
    } 
    config_endstop("Z MIN ENDSTOP", z_endstop_min_p, z_stepper_p, Z_ENDSTOP_MIN_GPIO, 0, &evt_queue);
    #endif
 
    #ifdef ENDSTOP_MAX_Z
    endstop_t* z_endstop_max_p = init_endstop();
    if(z_endstop_max_p == NULL){
        printf("INIT_ENDSTOP RETURNED NULL!");
        exit(1);
    } 
    config_endstop("Z MAX ENDSTOP", z_endstop_max_p, z_stepper_p, Z_ENDSTOP_MAX_GPIO, 0, &evt_queue);
    #endif
   
    #ifdef ENDSTOP_MIN_Y
    endstop_t* y_endstop_min_p = init_endstop();
    if(y_endstop_min_p == NULL){
        printf("INIT_ENDSTOP RETURNED NULL!");
        exit(1);
    } 
    config_endstop("Y MIN ENDSTOP", y_endstop_min_p, y_stepper_p, Y_ENDSTOP_MIN_GPIO, 0, &evt_queue);
    #endif
   
    #ifdef ENDSTOP_MAX_Y
    endstop_t* y_endstop_max_p = init_endstop();
    if(y_endstop_max_p == NULL){
        printf("INIT_ENDSTOP RETURNED NULL!");
        exit(1);
    } 
    config_endstop("Y MAX ENDSTOP", y_endstop_max_p, y_stepper_p, Y_ENDSTOP_MAX_GPIO, 0, &evt_queue);
    #endif
    
    //create endstop task
    TaskHandle_t endstop_taskHandle = (TaskHandle_t) xTaskCreate(endstop_task, "es_task", 4096, NULL, 10, NULL);
    
    #ifdef ENDSTOP_MIN_Z
        z_endstop_min_p->endstop_taskHandle = endstop_taskHandle;
    #endif
    #ifdef ENDSTOP_MAX_Z
        z_endstop_max_p->endstop_taskHandle = endstop_taskHandle;
    #endif
    #ifdef ENDSTOP_MIN_Y
        y_endstop_min_p->endstop_taskHandle = endstop_taskHandle;
    #endif
    #ifdef ENDSTOP_MAX_Y
        y_endstop_max_p->endstop_taskHandle = endstop_taskHandle;
    #endif

    gpio_install_isr_service(/*ENDSTOP_INTR_FLAG*/ESP_INTR_FLAG_DEFAULT); //install gpio isr service

    #ifdef ENDSTOP_MIN_Z
        gpio_isr_handler_add(z_endstop_min_p->gpio, gpio_isr_handler, (void *) z_endstop_min_p);
    #endif
    #ifdef ENDSTOP_MAX_Z
        gpio_isr_handler_add(z_endstop_max_p->gpio, gpio_isr_handler, (void *) z_endstop_max_p);
    #endif
    #ifdef ENDSTOP_MIN_Y
        gpio_isr_handler_add(y_endstop_min_p->gpio, gpio_isr_handler, (void *) y_endstop_min_p);
    #endif
    #ifdef ENDSTOP_MAX_Y
        gpio_isr_handler_add(y_endstop_max_p->gpio, gpio_isr_handler, (void *) y_endstop_max_p);
    #endif

    fan_1_p = init_fan();
    fan_2_p = init_fan();
    config_fan(fan_1_p, "Fan_1", FAN_1_PWM_LS_GPIO, FAN_PWM_DEFAULT_FREQUENCY, FAN_PWM_LS_TIMER, FAN_1_PWM_LS_CHANNEL);
    config_fan(fan_2_p, "Fan_2", FAN_2_PWM_LS_GPIO, FAN_PWM_DEFAULT_FREQUENCY, FAN_PWM_LS_TIMER, FAN_2_PWM_LS_CHANNEL);

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
