/*
    ------------------------------------------------------------------------------------
    LICENSE:
    ------------------------------------------------------------------------------------
    This file is part of EVEmu: EVE Online Server Emulator
    Copyright 2006 - 2021 The EVEmu Team
    For the latest information visit https://evemu.dev
    ------------------------------------------------------------------------------------
    This program is free software; you can redistribute it and/or modify it under
    the terms of the GNU Lesser General Public License as published by the Free Software
    Foundation; either version 2 of the License, or (at your option) any later
    version.

    This program is distributed in the hope that it will be useful, but WITHOUT
    ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
    FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public License along with
    this program; if not, write to the Free Software Foundation, Inc., 59 Temple
    Place - Suite 330, Boston, MA 02111-1307, USA, or go to
    http://www.gnu.org/copyleft/lesser.txt.
    ------------------------------------------------------------------------------------
    Author:        Zhur
    Rewrite:    Allan
*/

#include <ctime>
#include "eve-core.h"

#include "utils/timer.h"
#include "utils/utils_time.h"

static uint32 currentTime = 0;
//static float currentSeconds = 0;
static int64 lastTime = 0;

Timer::Timer(uint32 time/*0*/, bool useAcurateTiming /*false*/) {
    m_duration = time;
    m_startTime = currentTime;
    m_setAtTrigger = time;
    m_useAcurateTiming = useAcurateTiming;

    if (time)
        m_enabled = true;
    else
        m_enabled = false;
}

Timer::Timer(uint32 startAt, uint32 time, bool useAcurateTiming /*false*/) {
    m_duration = time;
    m_startTime = startAt;
    m_setAtTrigger = time;
    m_useAcurateTiming = useAcurateTiming;

    if (time)
        m_enabled = true;
    else
        m_enabled = false;
}

/* This function checks if the timer triggered */
bool Timer::Check(bool reset /*true*/)
{
    if (m_enabled)
        if ((currentTime - m_startTime) > m_duration) {
            if (reset) {
                if (m_useAcurateTiming)
                    m_startTime += m_duration; /* set start time to end of last timer */
                else
                    m_startTime = currentTime; /* set start time to now */
                m_duration = m_setAtTrigger;
            } else
                m_enabled = false;
            return true;
        }

    return false;
}

/* This function sets the timer and starts it */
void Timer::Start(uint32 setTimerTime, bool changeResetTimer /*true*/) {
    if (setTimerTime == 0) {
        m_enabled = false;
        return;
    }

    if (m_enabled) {    // this will allow resetting of the time without changing the start time
        if (m_duration != setTimerTime)
            m_duration = setTimerTime;
    } else {
        m_startTime = currentTime;
        m_duration = setTimerTime;
    }

    if (changeResetTimer)
        m_setAtTrigger = setTimerTime;

    m_enabled = true;
}

/* This updates the timer without restarting it */
void Timer::SetTimer(uint32 setTimerTime) {
    /* If we were disabled before => restart the timer */
    if (!m_enabled) {
        m_startTime = currentTime;
        m_enabled = true;
    }
    if (setTimerTime) {
        m_duration = setTimerTime;
        m_setAtTrigger = setTimerTime;
    }
}

uint32 Timer::GetRemainingTime() const {
    if (m_enabled)
        if ((currentTime - m_startTime) < m_duration)
            return (m_startTime + m_duration - currentTime);

    return 0;
}

void Timer::SetAtTrigger(uint32 setAtTrigger, bool enableIfDisabled) {
    m_setAtTrigger = setAtTrigger;
    if (!m_enabled && enableIfDisabled)
        Enable();
}

void Timer::Trigger() {
    m_enabled = true;
    m_duration = m_setAtTrigger;
    m_startTime = (currentTime - m_duration - 1);
}

uint32 Timer::GetCurrentTime() {
    return currentTime;
}

const void Timer::SetCurrentTime()
{
    int64 tickCount = GetSteadyTime();
    if (lastTime == 0) {
        currentTime = 0;
    } else {
        int64 timeDiff = tickCount - lastTime;
        
        // 检查时间差是否超出 uint32 范围
        if (timeDiff > static_cast<int64>(UINT32_MAX)) {
            sLog.Warning("Timer", "Time difference %lld exceeds uint32 range, capping to maximum", timeDiff);
            timeDiff = UINT32_MAX;
        } else if (timeDiff < 0) {
            sLog.Warning("Timer", "Negative time difference detected: %lld, using 0", timeDiff);
            timeDiff = 0;
        }
        
        currentTime += static_cast<uint32>(timeDiff);
    }
    lastTime = tickCount;
    //currentSeconds = (tickCount / 1000);
}
