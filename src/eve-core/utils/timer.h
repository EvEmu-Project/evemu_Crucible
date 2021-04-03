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
    Updates:    Allan
*/

#ifndef TIMER_H
#define TIMER_H

class Timer
{
public:
    Timer(uint32 time=0, bool useAcurateTiming = false);
    Timer(uint32 startAt, uint32 time, bool useAcurateTiming = false);

    ~Timer()                                            { /* do nothing here */ }

    inline void Enable()                                { m_enabled = true; }
    inline void Disable()                               { m_enabled = false; }
    inline const uint32& GetSetAtTrigger()              { return m_setAtTrigger; }
    inline bool Enabled() const                         { return m_enabled; }
    inline uint32 GetStartTime() const                  { return m_startTime; }
    inline uint32 GetDuration() const                   { return m_duration; }

    void Trigger();
    void SetTimer(uint32 setTimerTime = 0);
    void Start(uint32 setTimerTime = 0, bool changeResetTimer = true);
    void SetAtTrigger(uint32 setAtTrigger, bool enableIfDisabled = false);

    bool TimerFinished();
    bool Check(bool reset = true);

    static const void SetCurrentTime();
    // return remaining time in ms
    uint32 GetRemainingTime() const;
    uint32 GetCurrentTime();


private:
    bool    m_enabled;
    /* useAcurateTiming resets timer (on check()) to start_time += timer_time to eliminate discrepencies in calling time */
    bool    m_useAcurateTiming;

    uint32	m_duration;
    uint32	m_startTime;
    uint32	m_setAtTrigger;

};

#endif
