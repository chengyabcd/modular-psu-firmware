/*
 * EEZ Modular Firmware
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

#include <scpi/scpi.h>

#include <eez/modules/psu/scpi/params.h>
#include <eez/scpi/regs.h>

namespace eez {
namespace psu {
/// SCPI commands.
namespace scpi {

/// EEZ PSU specific SCPI parser context data.
struct scpi_psu_t {
    scpi_reg_val_t *registers;
    uint32_t selectedChannels;
    char currentDirectory[MAX_PATH_LENGTH + 1];
    bool isBufferOverrun;
    uint32_t bufferOverrunTime;
};

void init(scpi_t &scpi_context, scpi_psu_t &scpi_psu_context, scpi_interface_t *interface,
          char *input_buffer, size_t input_buffer_length, scpi_error_t *error_queue_data,
          int16_t error_queue_size);

void input(scpi_t &scpi_context, const char *str, size_t size);

void emptyBuffer(scpi_t &context);
void onBufferOverrun(scpi_t &context);

void printError(int_fast16_t err);

void resultChoiceName(scpi_t *context, scpi_choice_def_t *choice, int tag);

void abortDownloading();

bool mmemUpload(const char *filePath, scpi_t *context, int *err);

} // namespace scpi
} // namespace psu
} // namespace eez
