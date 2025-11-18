#include "scpi_transport.h"
#include "scpi_parser.h"      // там будет SCPI_ProcessLine
#include <string.h>

#define SCPI_BUF_SIZE 64

static char     scpi_line_buf[SCPI_BUF_SIZE];
static uint16_t scpi_line_pos = 0;

void SCPI_OnUsbData(uint8_t *buf, uint32_t len)
{
    for (uint32_t i = 0; i < len; ++i)
    {
        char c = (char)buf[i];

        if (c == '\r' || c == '\n')
        {
            if (scpi_line_pos > 0)
            {
                scpi_line_buf[scpi_line_pos] = '\0';
                SCPI_ProcessLine(scpi_line_buf);   // тут уже разбираем команду
                scpi_line_pos = 0;
            }
        }
        else
        {
            if (scpi_line_pos < SCPI_BUF_SIZE - 1)
            {
                scpi_line_buf[scpi_line_pos++] = c;
            }
            else
            {
                // переполнение – просто сброс
                scpi_line_pos = 0;
            }
        }
    }
}
