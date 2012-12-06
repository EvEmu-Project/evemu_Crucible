/*
    ------------------------------------------------------------------------------------
    LICENSE:
    ------------------------------------------------------------------------------------
    This file is part of EVEmu: EVE Online Server Emulator
    Copyright 2006 - 2011 The EVEmu Team
    For the latest information visit http://evemu.org
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
*/

#ifndef TIMER_H
#define TIMER_H

class Timer
{
public:
    Timer(uint32 timerTime, bool useAcurateTiming = false);
    Timer(uint32 start, uint32 timer, bool useAcurateTiming);
    ~Timer() { }

    bool Check(bool reset = true);
    void Enable();
    void Disable();
    void Start(uint32 setTimerTime=0, bool changeResetTimer = true);
	bool TimerFinished();
    void SetTimer(uint32 setTimerTime=0);
    uint32 GetRemainingTime() const;
    inline const uint32& GetTimerTime()        { return m_timerTime; }
    inline const uint32& GetSetAtTrigger()    { return m_setAtTrigger; }
    void Trigger();
    void SetAtTrigger(uint32 setAtTrigger, bool enableIfDisabled = false);

    inline bool Enabled() const { return m_enabled; }
    inline uint32 GetStartTime() const { return(m_startTime); }
    inline uint32 GetDuration() const { return(m_timerTime); }

    static const uint32 SetCurrentTime();
    static const uint32 GetCurrentTime();
    static const uint32 GetTimeSeconds();

private:
    uint32		m_startTime;
    uint32		m_timerTime;
    bool		m_enabled;
    uint32		m_setAtTrigger;

    // Tells the timer to be more accurate about happening every X ms.
    // Instead of Check() setting the start_time = now,
    // it it sets it to start_time += timer_time
    bool    m_useAcurateTiming;

//    static int32 current_time;
//    static int32 last_time;
};

#endif
