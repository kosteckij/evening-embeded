#include "scpi_parser.h"
#include "usbd_cdc_if.h"


extern TIM_HandleTypeDef htim3;



// *IDN?
void SCPI_IDN(bool is_query, const char *args)
{
    (void)args;

    if (is_query) {
        SCPI_Send("Eltesta,STM3240G,1234,1.0");
    } else {
        SCPI_Send("ERR:READ_ONLY_COMMAND");
    }
}


// :LED ON / :LED OFF / :LED?
static void SCPI_LED_OnOff(bool is_query, const char *args)
{
    if (is_query) {
        // :LED?
        GPIO_PinState st = HAL_GPIO_ReadPin(LED_GPIO_Port, LED_Pin);
        SCPI_Send(st == GPIO_PIN_SET ? "On" : "Off");
    } else {
        // :LED <ARG>
        if (args == NULL) {
            SCPI_Send("ERR:ARG_MISSING");
            return;
        }

        if (strcmp(args, "ON") == 0 || strcmp(args, "1") == 0) {
            //HAL_GPIO_WritePin(LED_GPIO_Port, LED_Pin, GPIO_PIN_SET);
            HAL_TIM_PWM_Start(&htim3, TIM_CHANNEL_2);
            SCPI_Send("OK");
        } else if (strcmp(args, "OFF") == 0 || strcmp(args, "0") == 0) {
            //HAL_GPIO_WritePin(LED_GPIO_Port, LED_Pin, GPIO_PIN_RESET);
            HAL_TIM_PWM_Stop(&htim3, TIM_CHANNEL_2);
            SCPI_Send("OK");
        } else {
            SCPI_Send("ERR:BAD_ARG");
        }
    }
}

// 0..100 (%)
static void LED_SetBrightnessPercent(float percent)
{
    if (percent > 100) percent = 100;

    uint32_t arr = __HAL_TIM_GET_AUTORELOAD(&htim3); // Period
    uint32_t cmp = (arr * percent) / 100u;

    __HAL_TIM_SET_COMPARE(&htim3, TIM_CHANNEL_2, cmp);
}


static void SCPI_LED_BRIGHT(bool is_query, const char *args)
{
    static int8_t s_led_bright = 0;   // запоминаем, чтобы отвечать на запрос

    if (is_query) {
        char buf[16];
        snprintf(buf, sizeof(buf), "%u", s_led_bright);
        SCPI_Send(buf);
        return;
    }

    // --- установка яркости ---

    if (args == NULL || *args == '\0') {
        SCPI_Send("ERR:ARG_MISSING");
        return;
    }

    char *end;
    long val = strtol(args, &end, 10);  // converting string to decimal

    // пропустим пробелы после числа
    while (*end == ' ' || *end == '\t') end++;

//    if (*end != '\0' || val < 0 || val > 100) {
    if (*end != '\0') {
        SCPI_Send("ERR:");    // некорректный аргумент
        return;
    }
    else if (val < 0)
        val = 0;
        else if (val > 100) 
            val = 100;


    s_led_bright = (int8_t)val;
    LED_SetBrightnessPercent(s_led_bright);
    SCPI_Send("OK");
}


const TScpiCommand ScpiCommand[] = {
    { "*IDN",           SCPI_IDN             }, // Instead Hello Word
    { "LED",            SCPI_LED_OnOff       }, // On Off
    { "LED:BRIGHT",     SCPI_LED_BRIGHT      }, // 0..100
    { NULL,             NULL                 }  
};
