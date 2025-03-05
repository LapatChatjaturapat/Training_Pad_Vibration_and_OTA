#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include "sdkconfig.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "driver/gpio.h"
#include "driver/ledc.h"
#include "esp_err.h"
#include "esp_log.h"
#include "driver/rmt.h"
#include "led_strip.h"
#include "driver/adc.h"
#include "esp_adc_cal.h"
#include "freertos/semphr.h"
#include <stdlib.h>
#include <sys/time.h>
#include "esp_sleep.h"
#include "driver/rtc_io.h"


#define VIBRATION_PIN      21   // GPIO pin for vibration sensor
#define VIBRATION_EN_PIN   32   // GPIO pin for vibration sensor enable
#define GPIO_INPUT_VIBRATION_PIN_SEL 1ULL<<VIBRATION_PIN
#define ESP_INTR_FLAG_DEFAULT 0

static const char *Vibration_tag = "Vibration_data";

#define VIBRATION_TIMEOUT 300 // vibration timeout time
#define VIB_THRESHOLD_MULTIPLIER 1.5 // threshold multiplier
static uint8_t g_vibration_count = 0; // จำนวนค่าที่อ่านได้จาก vibration (จำนวน Negedge)
static uint8_t g_vibration_reading = 0; // แสดงสถานะการอ่าน vibration 1=กำลังอ่านค่า 
static uint8_t g_vib_threshold = 1;  // ค่าเริ่มต้นสำหรับ vibration threshold
TaskHandle_t Vibration_Handle = NULL;
TaskHandle_t Vibration_Timer_Handle =NULL;


static void IRAM_ATTR gpio_isr_handler(void* arg)
{
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;
    uint32_t gpio_num = (uint32_t) arg;
    //uint8_t  level = gpio_get_level(gpio_num);
    if (gpio_num == VIBRATION_PIN) {
        if (g_vibration_reading == 0) {
            g_vibration_reading=1;
            g_vibration_count=1;
            vTaskNotifyGiveFromISR(Vibration_Timer_Handle,&xHigherPriorityTaskWoken); 
        }
        else if (g_vibration_reading == 1) {
            g_vibration_count++;
        }
    } 
}

void vibration_timer(void* pvParameters){
    uint32_t ulNotificationValue;
    while(1){
        ulNotificationValue = ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
        if (ulNotificationValue > 0) {
            ESP_LOGI(Vibration_tag, "Start timer");
            vTaskDelay(VIBRATION_TIMEOUT/ portTICK_PERIOD_MS);
            ESP_LOGI(Vibration_tag, "Stop timer");
            xTaskNotifyGive(Vibration_Handle);
        }
    }
}

void vibration_task(void *pvParameters)
{
    esp_err_t ret;
    uint8_t on = 255;
    uint8_t off = 0;
    uint32_t ulNotificationValue;
    while(1){
        ulNotificationValue = ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
        if (ulNotificationValue > 0) {
            gpio_set_intr_type(VIBRATION_PIN, GPIO_INTR_DISABLE);
            if(g_vibration_count>=g_vib_threshold*VIB_THRESHOLD_MULTIPLIER){
                ret=esp_ble_gatts_set_attr_value(iwing_training_table[IDX_CHAR_VIBRATION_VAL], sizeof(on), &on);
                ble_send_notify(iwing_training_table[IDX_CHAR_VIBRATION_VAL], &on, sizeof(on));
                if (ret != ESP_OK) {
                    ESP_LOGE(ble_tag, "Failed to set vibration value : %d", ret);
                }
                else {
                    ESP_LOGI(ble_tag, "vibration value set to : 255");
                }
            }
            ESP_LOGI(Vibration_tag, "count = %u", g_vibration_count);
            vTaskDelay(1000 / portTICK_PERIOD_MS);
            ret=esp_ble_gatts_set_attr_value(iwing_training_table[IDX_CHAR_VIBRATION_VAL], sizeof(off), &off);
            ble_send_notify(iwing_training_table[IDX_CHAR_VIBRATION_VAL], &off, sizeof(off));
            if (ret != ESP_OK) {
                ESP_LOGE(ble_tag, "Failed to set vibration value : %d", ret);
            }
            else {
                ESP_LOGI(ble_tag, "vibration value set to : 0");
            }
            ESP_LOGI(ble_tag, "threshold = %u", g_vib_threshold);
            g_vibration_reading=0;
            gpio_set_intr_type(VIBRATION_PIN, GPIO_INTR_NEGEDGE);
        }
        vTaskDelay(1/ portTICK_PERIOD_MS);
    }
}  

void gpio_init(void){
    gpio_config_t io_conf = {};
    io_conf.intr_type = GPIO_INTR_NEGEDGE;
    io_conf.pin_bit_mask = GPIO_INPUT_VIBRATION_PIN_SEL ;
    io_conf.mode = GPIO_MODE_INPUT;
    io_conf.pull_up_en = GPIO_PULLUP_ENABLE;
    gpio_config(&io_conf);

    gpio_install_isr_service(ESP_INTR_FLAG_DEFAULT);
    gpio_isr_handler_add(VIBRATION_PIN, gpio_isr_handler, (void*) VIBRATION_PIN);
}

void app_main(void)
{
    ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK( ret );
    
    gpio_init();

    xTaskCreate(vibration_task, "vibration_task", 2048, NULL, 16, &Vibration_Handle);
    xTaskCreate(vibration_timer, "vibration_timer", 2048, NULL, 16, &Vibration_Timer_Handle);

    while(1) {
        vTaskDelay(1000 / portTICK_PERIOD_MS);
    }
}