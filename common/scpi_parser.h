#ifndef SCPI_PARSER_H
#define SCPI_PARSER_H

#include <stdbool.h>



typedef void (*ScpiHandler)(bool is_query, const char *args);

typedef struct {
    const char  *name;    // etc "*IDN", "LED", "GEN:WAVE"
    ScpiHandler  handler;
} TScpiCommand;


#define LED_Pin GPIO_PIN_7
#define LED_GPIO_Port GPIOC

// Вызывается из CDC, когда пришла целая строка (без \r\n)
void SCPI_ProcessLine(char *line);

// посыл с авто строки \r\n
void SCPI_Send(const char *s);
    
#endif
