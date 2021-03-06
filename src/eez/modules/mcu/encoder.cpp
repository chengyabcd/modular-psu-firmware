/*
* EEZ Middleware
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

#if OPTION_DISPLAY && OPTION_ENCODER

#include <math.h>

#include <eez/system.h>
#include <eez/debug.h>
#include <eez/util.h>

#include <eez/modules/mcu/encoder.h>

#if defined(EEZ_PLATFORM_STM32)	
#include <eez/modules/mcu/button.h>
#endif

#include <eez/modules/psu/psu.h>
#include <eez/modules/psu/channel_dispatcher.h>
#include <eez/modules/psu/persist_conf.h>

namespace eez {
namespace mcu {
namespace encoder {

#if defined(EEZ_PLATFORM_STM32)
#define CONF_ENCODER_SPEED_FACTOR 0.25f
#else
#define CONF_ENCODER_SPEED_FACTOR 0.5f
#endif

#if defined(EEZ_PLATFORM_STM32)
static Button g_encoderSwitch(ENC_SW_GPIO_Port, ENC_SW_Pin, true, false);
#endif	

static uint16_t g_totalCounter;
static int16_t g_diffCounter;
#if defined(EEZ_PLATFORM_SIMULATOR)
bool g_simulatorClicked;
#endif	
static float g_accumulatedCounter;
EncoderMode g_encoderMode = ENCODER_MODE_AUTO;

////////////////////////////////////////////////////////////////////////////////

struct CalcAutoModeStepLevel {
    static const int CONF_NUM_STEP_LEVELS = 4;
    static const int CONF_NUM_ACCELERATION_STEP_COUNTERS = 10;
    static const int CONF_ACCELERATION_STEP_COUNTER_DURATION_MS = 50;
    static const int CONF_COUNTER_TO_STEP_DIVISOR = 5;

    uint32_t m_lastTick;
    int m_accumulatedCounter;
    int m_counters[CONF_NUM_ACCELERATION_STEP_COUNTERS];
    unsigned int m_counterIndex;
    int m_stepLevel;

    void reset() {
        m_lastTick = millis();
        m_accumulatedCounter = 0;
        for (int i = 0; i < CONF_NUM_ACCELERATION_STEP_COUNTERS; i++) {
            m_counters[i] = 0;
        }
        m_counterIndex = 0;
        m_stepLevel = 0;
    }

    void update(int16_t diffCounter) {
        m_accumulatedCounter += diffCounter;

        uint32_t tick = millis();
        int32_t diffTick = tick - m_lastTick;
        if (diffTick >= CONF_ACCELERATION_STEP_COUNTER_DURATION_MS) {
            m_counters[m_counterIndex % CONF_NUM_ACCELERATION_STEP_COUNTERS] = m_accumulatedCounter;

            int stepCountersSum = 0;
            for (int i = 0; i < CONF_NUM_ACCELERATION_STEP_COUNTERS; i++) {
                stepCountersSum += m_counters[(m_counterIndex - i) % CONF_NUM_ACCELERATION_STEP_COUNTERS];
            }

            stepCountersSum = abs(stepCountersSum);

            m_stepLevel = MIN(stepCountersSum / CONF_COUNTER_TO_STEP_DIVISOR, CONF_NUM_STEP_LEVELS - 1);

            //if (stepCountersSum != 0) {
            //    DebugTrace("%d, %d\n", stepCountersSum, m_stepLevel);
            //}

            m_lastTick = tick;
            m_accumulatedCounter = 0;
            m_counterIndex++;
        }
    }

    int stepLevel() {
        return m_stepLevel;
    }
};

CalcAutoModeStepLevel g_calcAutoModeStepLevel;

////////////////////////////////////////////////////////////////////////////////

int getAutoModeStepLevel() {
    return g_calcAutoModeStepLevel.stepLevel();
}

////////////////////////////////////////////////////////////////////////////////

void init() {
    g_calcAutoModeStepLevel.reset();
}

#if defined(EEZ_PLATFORM_STM32)

void onPinInterrupt() {
    static const uint8_t g_ttable[6][4] = {
        { 0x3 , 0x2, 0x1,  0x0 },
        { 0x23, 0x0, 0x1,  0x0 },
        { 0x13, 0x2, 0x0,  0x0 },
        { 0x3 , 0x5, 0x4,  0x0 },
        { 0x3 , 0x3, 0x4, 0x10 },
        { 0x3 , 0x5, 0x3, 0x20 }
    };
    static volatile uint8_t g_rotationState;
    
    int offset = 0;

    uint8_t pinState = (HAL_GPIO_ReadPin(ENC_B_GPIO_Port, ENC_B_Pin) << 1) | HAL_GPIO_ReadPin(ENC_A_GPIO_Port, ENC_A_Pin);
    g_rotationState = g_ttable[g_rotationState & 0xf][pinState];
    if (g_rotationState == 0x20) {
        offset = 1;
    } else if (g_rotationState == 0x10) {
        offset = -1;
    }

    if (offset) {
        g_diffCounter += offset;
    }
}
#endif

int getCounter() {
    int16_t diffCounter = g_diffCounter;
    g_diffCounter -= diffCounter;

    g_totalCounter += diffCounter;
    psu::debug::g_encoderCounter.set(g_totalCounter);

    // update acceleration
    g_calcAutoModeStepLevel.update(diffCounter);

    //
    float speedFactor = remap(g_accumulatedCounter > 0 ? psu::persist_conf::devConf.encoderMovingSpeedUp : psu::persist_conf::devConf.encoderMovingSpeedDown, MIN_MOVING_SPEED, 4.0f, MAX_MOVING_SPEED, 1.0f);
    g_accumulatedCounter += diffCounter / speedFactor;
    int result = (int)g_accumulatedCounter;
    g_accumulatedCounter -= result;

    return result;
}

void resetEncoder() {
    g_calcAutoModeStepLevel.reset();
}

bool isButtonClicked() {
#if defined(EEZ_PLATFORM_SIMULATOR)
    return g_simulatorClicked;
#endif

#if defined(EEZ_PLATFORM_STM32)
    return g_encoderSwitch.isClicked();
#endif
}

void read(int &counter, bool &clicked) {
    clicked = isButtonClicked();
    if (clicked) {
        resetEncoder();
    }
    counter = getCounter();
}

#if defined(EEZ_PLATFORM_SIMULATOR)
void write(int counter, bool clicked) {
	g_diffCounter = counter;
	g_simulatorClicked = clicked;
}
#endif

void switchEncoderMode() {
    if (g_encoderMode == ENCODER_MODE_STEP4) {
        g_encoderMode = ENCODER_MODE_AUTO;
    } else {
        g_encoderMode = EncoderMode(g_encoderMode + 1);
    }
}

} // namespace encoder
} // namespace mcu
} // namespace eez

#endif
