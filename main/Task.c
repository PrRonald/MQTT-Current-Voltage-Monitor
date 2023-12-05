#include <stdio.h>

#include "esp_adc_cal.h"
#include "esp_system.h"
#include "driver/gpio.h"
#include "hal/gpio_types.h"
#include "esp_wifi.h"
#include "nvs_flash.h"
#include "esp_event.h"
#include "esp_netif.h"
#include "protocol_examples_common.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"

#include "driver/adc.h"
#include "driver/gpio.h"

#include "lwip/sockets.h"
#include "lwip/dns.h"
#include "lwip/netdb.h"

#include "esp_log.h"
#include "mqtt_client.h"

#define LED_GREEN       GPIO_NUM_19
#define LED_RED         GPIO_NUM_18
#define MOSFET_1        GPIO_NUM_17
#define MOSFET_2        GPIO_NUM_16
#define RELAY           GPIO_NUM_4
#define CAR_ON          GPIO_NUM_0
#define BATTERY         ADC_CHANNEL_7
#define VOLT_OUT        ADC_CHANNEL_6
#define CURRENT_OUT     ADC_CHANNEL_4




//adc config star here

static esp_adc_cal_characteristics_t adc1_chars;


//adc config end here

static const char *TAG = "MQTT_EXAMPLE";

static void log_error_if_nonzero(const char *message, int error_code)
{
    if (error_code != 0) {
        ESP_LOGE(TAG, "Last error %s: 0x%x", message, error_code);
    }
}

static void mqtt_event_handler(void *handler_args, esp_event_base_t base, int32_t event_id, void *event_data)
{
    ESP_LOGD(TAG, "Event dispatched from event loop base=%s, event_id=%" PRIi32 "", base, event_id);
    esp_mqtt_event_handle_t event = event_data;
    esp_mqtt_client_handle_t client = event->client;
    int msg_id;
    switch ((esp_mqtt_event_id_t)event_id) {
    case MQTT_EVENT_CONNECTED:
        ESP_LOGI(TAG, "MQTT_EVENT_CONNECTED");
        msg_id = esp_mqtt_client_subscribe(client, "/topic/qos1", 0);
        ESP_LOGI(TAG, "sent subscribe successful, msg_id=%d", msg_id);

        //msg_id = esp_mqtt_client_publish(client, "/topic/qos1", "data_3", 0, 1, 0);
        //ESP_LOGI(TAG, "sent publish successful, msg_id=%d", msg_id);
    case MQTT_EVENT_DISCONNECTED:
        ESP_LOGI(TAG, "MQTT_EVENT_DISCONNECTED");
        break;

    case MQTT_EVENT_SUBSCRIBED:
        ESP_LOGI(TAG, "MQTT_EVENT_SUBSCRIBED, msg_id=%d", event->msg_id);
        //msg_id = esp_mqtt_client_publish(client, "/topic/qos0", "data", 0, 0, 0);
        //ESP_LOGI(TAG, "sent publish successful, msg_id=%d", msg_id);
        break;
    case MQTT_EVENT_UNSUBSCRIBED:
        ESP_LOGI(TAG, "MQTT_EVENT_UNSUBSCRIBED, msg_id=%d", event->msg_id);
        break;
    case MQTT_EVENT_PUBLISHED:
        ESP_LOGI(TAG, "MQTT_EVENT_PUBLISHED, msg_id=%d", event->msg_id);
        break;
    case MQTT_EVENT_DATA:
        ESP_LOGI(TAG, "MQTT_EVENT_DATA");
        printf("TOPIC=%.*s\r\n", event->topic_len, event->topic);
        printf("DATA=%.*s\r\n", event->data_len, event->data);
        break;
    case MQTT_EVENT_ERROR:
        ESP_LOGI(TAG, "MQTT_EVENT_ERROR");
        if (event->error_handle->error_type == MQTT_ERROR_TYPE_TCP_TRANSPORT) {
            log_error_if_nonzero("reported from esp-tls", event->error_handle->esp_tls_last_esp_err);
            log_error_if_nonzero("reported from tls stack", event->error_handle->esp_tls_stack_err);
            log_error_if_nonzero("captured as transport's socket errno",  event->error_handle->esp_transport_sock_errno);
            ESP_LOGI(TAG, "Last errno string (%s)", strerror(event->error_handle->esp_transport_sock_errno));

        }
        break;
    default:
        ESP_LOGI(TAG, "Other event id:%d", event->event_id);
        break;
    }
}

void Task1(void *pvParameter){
//Protocolo mqtt start here
int VoltageBatteryIn;
int VoltageBatteryOut;
int VoltageBatteryMediaIn = 0;
int VoltageBatteryMediaOut = 0;
int CurrentOut;
int CurrentOutMedia = 0;
int counter = 0;
char buff_volt[20];
char buff_current[20];


esp_mqtt_client_config_t mqtt_cfg = {
        .broker.address.uri = "mqtt://test.mosquitto.org",
        .broker.address.port = 1883
    };

    esp_mqtt_client_handle_t client = esp_mqtt_client_init(&mqtt_cfg);
    /* The last argument may be used to pass data to the event handler, in this example mqtt_event_handler */
    esp_mqtt_client_register_event(client, ESP_EVENT_ANY_ID, mqtt_event_handler, NULL);
    esp_mqtt_client_start(client);
    //protocolo mqtt end here
    while(1){
	    //Here it will take place the calculation of voltage mean and current mean 
        //We take 100 evidences.
        if(counter == 1000){
            VoltageBatteryMediaIn = VoltageBatteryMediaIn / 1000;
            VoltageBatteryMediaOut = VoltageBatteryMediaOut / 1000;
            CurrentOutMedia = CurrentOutMedia / 1000;
            sprintf(buff_volt, "%d", VoltageBatteryMediaIn);
            sprintf(buff_volt, "%d", VoltageBatteryMediaOut);
            sprintf(buff_current, "%d", CurrentOutMedia);

            esp_mqtt_client_publish(client, "/topic/qos1", buff_volt, 0, 1, 0);
            esp_mqtt_client_publish(client, "/topic/qos2", buff_current, 0, 1, 0);
            esp_mqtt_client_publish(client, "/topic/qos3", buff_current, 0, 1, 0);
            vTaskDelay(pdMS_TO_TICKS(800));
            counter = 0;
        }
        else{
            VoltageBatteryIn = esp_adc_cal_raw_to_voltage(adc1_get_raw(BATTERY), &adc1_chars);
            VoltageBatteryOut = esp_adc_cal_raw_to_voltage(adc1_get_raw(VOLT_OUT), &adc1_chars);
	        CurrentOut = esp_adc_cal_raw_to_voltage(adc1_get_raw(CURRENT_OUT), &adc1_chars);
            VoltageBatteryMediaIn = VoltageBatteryMediaIn + VoltageBatteryIn;
            VoltageBatteryMediaOut = VoltageBatteryMediaOut + VoltageBatteryOut;
            CurrentOutMedia = CurrentOutMedia + CurrentOut;
            counter++;
        }
    } 
}
//Function that send the data
void Sendvalue(void *pvParameter){

//control potencia 
    void ControlPotencia(int mosfet1, int mosfet2, int relay, int LedGreen, int LedRed){
        gpio_set_level(MOSFET_1, mosfet1);
        gpio_set_level(MOSFET_2, mosfet2);
        gpio_set_level(RELAY, relay);
        //Leds 
        gpio_set_level(LED_GREEN, LedGreen);
        gpio_set_level(LED_RED, LedRed);
        
    }

    int coditional = 1; //this value will be 0 in overcurrent
    int caseVol[3];
    int VarHandle;
    int voltage1;
    int voltage2;
    int voltage3;

	while(1){

	    voltage1 = esp_adc_cal_raw_to_voltage(adc1_get_raw(BATTERY), &adc1_chars);
        voltage2 = esp_adc_cal_raw_to_voltage(adc1_get_raw(VOLT_OUT), &adc1_chars);
        voltage3 = esp_adc_cal_raw_to_voltage(adc1_get_raw(CURRENT_OUT), &adc1_chars);
             
        caseVol[1] = (voltage1 > 2500) ? 1 : 0;
        caseVol[2] = (voltage2 > 2500) ? 2 : 0;
        caseVol[3] = (voltage3 > 2500) ? 3 : 0; 

        for(int i = 1; i < 4; i++){
            if(caseVol[i] > 0){
                VarHandle = caseVol[i];
            }
        }

        VarHandle = (VarHandle > 0) ? VarHandle : 0;
        printf("var handle : %d and conditionn : %d \n", VarHandle, coditional);

            if((VarHandle == 0) && coditional){    
                ControlPotencia(1, 1, 1, 0, 1);
            }
            else{
                coditional = 0;
                ControlPotencia(0, 0, 0, 1, 0);
            }
    }      
}

int VoltageBattery;

void app_main(){
    //protocolo installetion start herer
    ESP_LOGI(TAG, "[APP] Startup..");
    ESP_LOGI(TAG, "[APP] Free memory: %" PRIu32 " bytes", esp_get_free_heap_size());
    ESP_LOGI(TAG, "[APP] IDF version: %s", esp_get_idf_version());

    esp_log_level_set("*", ESP_LOG_INFO);
    esp_log_level_set("mqtt_client", ESP_LOG_VERBOSE);
    esp_log_level_set("MQTT_EXAMPLE", ESP_LOG_VERBOSE);
    esp_log_level_set("TRANSPORT_BASE", ESP_LOG_VERBOSE);
    esp_log_level_set("esp-tls", ESP_LOG_VERBOSE);
    esp_log_level_set("TRANSPORT", ESP_LOG_VERBOSE);
    esp_log_level_set("outbox", ESP_LOG_VERBOSE);

    ESP_ERROR_CHECK(nvs_flash_init());
    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());

    /* This helper function configures Wi-Fi or Ethernet, as selected in menuconfig.*/
    ESP_ERROR_CHECK(example_connect());

    //adc config star here
    esp_adc_cal_characterize(ADC_UNIT_1, ADC_ATTEN_DB_11, ADC_WIDTH_BIT_DEFAULT, 0, &adc1_chars);

    ESP_ERROR_CHECK(adc1_config_width(ADC_WIDTH_BIT_DEFAULT));
    ESP_ERROR_CHECK(adc1_config_channel_atten(ADC1_CHANNEL_6, ADC_ATTEN_DB_11));
    ESP_ERROR_CHECK(adc1_config_channel_atten(ADC1_CHANNEL_7, ADC_ATTEN_DB_11));
    ESP_ERROR_CHECK(adc1_config_channel_atten(ADC1_CHANNEL_4, ADC_ATTEN_DB_11));

    //adc config end here

    //gpio config star here
    gpio_set_direction(LED_GREEN, GPIO_MODE_OUTPUT);
    gpio_set_direction(LED_RED, GPIO_MODE_OUTPUT);
    gpio_set_direction(MOSFET_1, GPIO_MODE_OUTPUT);
    gpio_set_direction(MOSFET_2, GPIO_MODE_OUTPUT);
    gpio_set_direction(RELAY, GPIO_MODE_OUTPUT);
    gpio_set_direction(CAR_ON, GPIO_MODE_INPUT);

	VoltageBattery = esp_adc_cal_raw_to_voltage(adc1_get_raw(BATTERY), &adc1_chars);

    //gpio config end here
    if((VoltageBattery > 1000) && (gpio_get_level(CAR_ON))){
        xTaskCreate(&Task1, "Task1", 2048, NULL, 1, NULL);
        xTaskCreate(&Sendvalue, "Sendvalue", 2048, NULL, 1, NULL);
    }else{
        //if the battery is down than 1800 then led red will turn on
        gpio_set_level(LED_RED, 1);
        VoltageBattery = esp_adc_cal_raw_to_voltage(adc1_get_raw(BATTERY), &adc1_chars);
        printf("El carro esta apagado, el voltaje de la bateria es %d", VoltageBattery);
 
    }
}
