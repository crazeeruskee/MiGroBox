/* MiGroBox ESP32s2 code - ECE Capstone Fall 2020 
   
   Based off of: 
   - Simple HTTP Server Example
   - Blink Example

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
#include "driver/gpio.h"
#include "sdkconfig.h"


/* A simple example that demonstrates how to create GET and POST
 * handlers for the web server.
 */
#define DECIMAL 10

#define PUMP_GPIO CONFIG_PUMP_RELAY_GPIO
#define LIGHT_GPIO CONFIG_LIGHT_RELAY_GPIO

static const char *TAG = "example";
int lights_value;
int pump_value;
int fan_1_value;
int fan_2_value;
int temperature_value;
int humidity_value;

void setup_gpio(){
    gpio_reset_pin(PUMP_GPIO);
    gpio_reset_pin(LIGHT_GPIO);
    gpio_set_direction(PUMP_GPIO, GPIO_MODE_OUTPUT);
    gpio_set_direction(LIGHT_GPIO, GPIO_MODE_OUTPUT);
}

/* An HTTP GET handler */
static esp_err_t get_handler(httpd_req_t *req)
{
    char* buf;
    size_t buf_len;

    bool device_selected = false;
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


    device_selected = true;

    if(strcmp(device, "not_selected") == 0){
        device_selected = false;
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
            gpio_set_level(PUMP_GPIO, device_value);
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
    } else{
        device_selected = false;
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

    ESP_ERROR_CHECK(nvs_flash_init());
    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());

    /* This helper function configures Wi-Fi or Ethernet, as selected in menuconfig.
     * Read "Establishing Wi-Fi or Ethernet Connection" section in
     * examples/protocols/README.md for more information about this function.
     */
    ESP_ERROR_CHECK(example_connect());

    /* Register event handlers to stop the server when Wi-Fi or Ethernet is disconnected,
     * and re-start it upon connection.
     */
#ifdef CONFIG_EXAMPLE_CONNECT_WIFI
    ESP_ERROR_CHECK(esp_event_handler_register(IP_EVENT, IP_EVENT_STA_GOT_IP, &connect_handler, &server));
    ESP_ERROR_CHECK(esp_event_handler_register(WIFI_EVENT, WIFI_EVENT_STA_DISCONNECTED, &disconnect_handler, &server));
#endif // CONFIG_EXAMPLE_CONNECT_WIFI
#ifdef CONFIG_EXAMPLE_CONNECT_ETHERNET
    ESP_ERROR_CHECK(esp_event_handler_register(IP_EVENT, IP_EVENT_ETH_GOT_IP, &connect_handler, &server));
    ESP_ERROR_CHECK(esp_event_handler_register(ETH_EVENT, ETHERNET_EVENT_DISCONNECTED, &disconnect_handler, &server));
#endif // CONFIG_EXAMPLE_CONNECT_ETHERNET

    /* Start the server for the first time */
    server = start_webserver();
}
