#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "esp_system.h"
#include "driver/gpio.h"
#include "hal/gpio_types.h"

#include "driver/adc.h"
#include "driver/gpio.h"
#include "esp_adc_cal.h"

static esp_adc_cal_characteristics_t adc1_chars;
QueueHandle_t xQueue;

//Function that send the data
void Sendvalue(void *pvParameter){
    int32_t voltage1;
    int32_t voltage2;
    int32_t voltage3;
    

    BaseType_t xStatus;

    const TickType_t xTickToWaite= pdMS_TO_TICKS(100);
    int caseVol[3];
    int VarHandle;

    //int i = 1;
       
    VarHandle = (int32_t)pvParameter;
	while(1)
	{
	    voltage1 = esp_adc_cal_raw_to_voltage(adc1_get_raw(ADC1_CHANNEL_6), &adc1_chars);
        voltage2 = esp_adc_cal_raw_to_voltage(adc1_get_raw(ADC1_CHANNEL_7), &adc1_chars);
        voltage3 = esp_adc_cal_raw_to_voltage(adc1_get_raw(ADC1_CHANNEL_4), &adc1_chars);
        printf("Volt 1: %lu \n", voltage1);
        printf("vol 2: %lu \n", voltage2);
        printf("Vol 3: %lu \n", voltage3);
             
       // i = (i > 3) ? 1 : i;
        caseVol[1] = (voltage1 > 2500) ? 1 : 0;
        caseVol[2] = (voltage2 > 2500) ? 2 : 0;
        caseVol[3] = (voltage3 > 2500) ? 3 : 0;
       // VarHandle = caseVol[i];
        //i++;
        for(int i = 1; i < 4; i++){
            if(caseVol[i] > 0){
                VarHandle = caseVol[i];
            }
        }
        VarHandle = (VarHandle > 0) ? VarHandle : 0;
        xStatus = xQueueSendToBack(xQueue, &VarHandle, xTickToWaite);
        if(xStatus != pdTRUE){
           printf("No se pudo enviar");
        }
        
    }
}
//Function that receive the data
void RecieValue(void *pvParameter){
    int32_t ReceiV;
    BaseType_t xStatus;
    const TickType_t xTickToWaite= pdMS_TO_TICKS(100);

    while(1) {
        xStatus = xQueueReceive(xQueue, &ReceiV, xTickToWaite);
        if(xStatus != errQUEUE_FULL){
        printf("status: %lu \n", ReceiV);
        if(ReceiV > 0 ){
            gpio_set_level(GPIO_NUM_19, 1);
            gpio_set_level(GPIO_NUM_18, 0);
        }
        else{
            gpio_set_level(GPIO_NUM_19, 0);
            gpio_set_level(GPIO_NUM_18, 1);
        }
            vTaskDelay(pdMS_TO_TICKS(100));
        } 
        else{
            printf("no se pudo recivir el queue");
        } 
    }
}


void app_main(){
    esp_adc_cal_characterize(ADC_UNIT_1, ADC_ATTEN_DB_11, ADC_WIDTH_BIT_DEFAULT, 0, &adc1_chars);

    ESP_ERROR_CHECK(adc1_config_width(ADC_WIDTH_BIT_DEFAULT));
    ESP_ERROR_CHECK(adc1_config_channel_atten(ADC1_CHANNEL_6, ADC_ATTEN_DB_11));
    ESP_ERROR_CHECK(adc1_config_channel_atten(ADC1_CHANNEL_7, ADC_ATTEN_DB_11));
    ESP_ERROR_CHECK(adc1_config_channel_atten(ADC1_CHANNEL_4, ADC_ATTEN_DB_11));

    gpio_set_direction(GPIO_NUM_19, GPIO_MODE_OUTPUT);
    gpio_set_direction(GPIO_NUM_18, GPIO_MODE_OUTPUT);

    xQueue = xQueueCreate(3, sizeof(int32_t)); //Creat the Queue
    if(xQueue != NULL){
        xTaskCreate(&Sendvalue, "Sendvalue", 2048, NULL, 1, NULL);
        xTaskCreate(&RecieValue, "RecieValue", 2048,NULL, 2,NULL );
    }

    

    //vTaskStartScheduler();
}
