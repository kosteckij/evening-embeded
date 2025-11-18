#ifndef SCPI_TRANSPORT_H
#define SCPI_TRANSPORT_H

#include <stdint.h>

void SCPI_OnUsbData(uint8_t *buf, uint32_t len);

#endif
