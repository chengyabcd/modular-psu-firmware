/*
 * EEZ PSU Firmware
 * Copyright (C) 2015-present, Envox d.o.o.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#pragma once

#ifdef STM32F429xx
#include "stm32f4xx_hal.h"
#endif

#ifdef STM32F769xx
#include "stm32f7xx_hal.h"
#endif

#define HIGH 1
#define LOW 0

#define ISOLATOR_DISABLE LOW
#define ISOLATOR_ENABLE  HIGH

namespace eez {
namespace psu {

extern SPI_HandleTypeDef MCP23S08_SPI;
extern SPI_HandleTypeDef DAC8552_SPI;
extern SPI_HandleTypeDef ADS1120_SPI;

void SPI_beginTransaction(SPI_HandleTypeDef& spiHandle);
void SPI_endTransaction();

void digitalWrite(int pin, int state);

class SPIClass {
public:
	uint8_t transfer(uint8_t value);
};

extern SPIClass SPI;

}
} // namespace eez::psu