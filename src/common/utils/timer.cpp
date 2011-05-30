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
	Author:		Zhur
*/

#include "CommonPCH.h"

#include "utils/timer.h"

static int32 current_time = 0;
static int32 current_seconds = 0;
static int32 last_time = 0;

Timer::Timer(int32 in_timer_time, bool iUseAcurateTiming) {
	timer_time = in_timer_time;
	start_time = current_time;
	set_at_trigger = timer_time;
	pUseAcurateTiming = iUseAcurateTiming;
	if (timer_time == 0) {
		enabled = false;
	}
	else {
		enabled = true;
	}
}

Timer::Timer(int32 start, int32 timer, bool iUseAcurateTiming = false) {
	timer_time = timer;
	start_time = start;
	set_at_trigger = timer_time;
	pUseAcurateTiming = iUseAcurateTiming;
	if (timer_time == 0) {
		enabled = false;
	}
	else {
		enabled = true;
	}
}

/* This function checks if the timer triggered */
bool Timer::Check(bool iReset)
{
	//_CP(Timer_Check);
    if (this==0) { 
                printf( "Null timer during ->Check()!?\n" );
		return true; 
	}
    if (enabled && current_time-start_time > timer_time) {
		if (iReset) {
			if (pUseAcurateTiming)
				start_time += timer_time;
			else
				start_time = current_time; // Reset timer
			timer_time = set_at_trigger;
		}
		return true;
    }
	
    return false;
}

/* This function disables the timer */
void Timer::Disable() {
	enabled = false;
}

void Timer::Enable() {
	enabled = true;
}

/* This function set the timer and restart it */
void Timer::Start(int32 set_timer_time, bool ChangeResetTimer) {	
    start_time = current_time;
	enabled = true;
    if (set_timer_time != 0)
    {	
		timer_time = set_timer_time;
		if (ChangeResetTimer == true)
			set_at_trigger = set_timer_time;
    }
}

/* This timer updates the timer without restarting it */
void Timer::SetTimer(int32 set_timer_time) {
    /* If we were disabled before => restart the timer */
    if (!enabled) {
		start_time = current_time;
		enabled = true;
    }
    if (set_timer_time != 0) {
		timer_time = set_timer_time;
		set_at_trigger = set_timer_time;
    }
}

int32 Timer::GetRemainingTime() const {
    if (enabled) {
	    if (current_time-start_time > timer_time)
			return 0;
		else
			return (start_time + timer_time) - current_time;
    }
	else {
		return 0xFFFFFFFF;
	}
}

void Timer::SetAtTrigger(int32 in_set_at_trigger, bool iEnableIfDisabled) {
	set_at_trigger = in_set_at_trigger;
	if (!Enabled() && iEnableIfDisabled) {
		Enable();
	}
}

void Timer::Trigger()
{
	enabled = true;

	timer_time = set_at_trigger;
	start_time = current_time-timer_time-1;
}

const int32 Timer::GetCurrentTime()
{	
    return current_time;
}

//just to keep all time related crap in one place... not really related to timers.
const int32 Timer::GetTimeSeconds() {
    return(current_seconds);
}

const int32 Timer::SetCurrentTime()
{
    struct timeval read_time;	
    int32 this_time;

    gettimeofday(&read_time,0);
    this_time = read_time.tv_sec * 1000 + read_time.tv_usec / 1000;

    if (last_time == 0)
    {
		current_time = 0;
    }
    else
    {
		current_time += this_time - last_time;
    }
    
	last_time = this_time;
	current_seconds = read_time.tv_sec;

	return current_time;
}
