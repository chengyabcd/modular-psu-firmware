/*
* EEZ Generic Firmware
* Copyright (C) 2018-present, Envox d.o.o.
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

#include <py/obj.h>

mp_obj_t modeez_scpi(mp_obj_t commandOrQueryText);
mp_obj_t modeez_getU(mp_obj_t channelIndexObj);
mp_obj_t modeez_setU(mp_obj_t channelIndexObj, mp_obj_t value);
mp_obj_t modeez_getI(mp_obj_t channelIndexObj);
mp_obj_t modeez_setI(mp_obj_t channelIndexObj, mp_obj_t value);
mp_obj_t modeez_getOutputMode(mp_obj_t channelIndexObj);
mp_obj_t modeez_dlogTraceData(size_t n_args, const mp_obj_t *args);
