/*
 * Author(s):         Lucas Moiseyev
 * Date Created:      1/12/21
 * Date Last Updated: 1/13/21
 *
 */
#include "esp_log.h"
#include "real_time.h"

time_t* real_time_init(){
    time_t* now_p = malloc(sizeof(time_t));
    if(now_p == NULL){
        printf("REAL TIME INIT: MALLOC FAILED!");
    }   
    
    *now_p = time(NULL);
    setenv("TZ", "UTC-5", 1);
    tzset();

    return now_p;
}

struct tm* get_current_time(time_t* now_p){
    size_t buf_size = 64;
    char strftime_buf[buf_size];
    struct tm* timeinfo_p = malloc(sizeof(struct tm));

    time(now_p);
    localtime_r(now_p, timeinfo_p);
    strftime(strftime_buf, buf_size, "%c", timeinfo_p);
    ESP_LOGI("REAL TIME CLOCK", "The current date/time (EST) is: %s", strftime_buf);

    return timeinfo_p;
}





