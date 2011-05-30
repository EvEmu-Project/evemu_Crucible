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

#ifndef TIMER_H
#define TIMER_H

class Timer
{
public:
	Timer(int32 timer_time, bool iUseAcurateTiming = false);
	Timer(int32 start, int32 timer, bool iUseAcurateTiming);
	~Timer() { }

	bool Check(bool iReset = true);
	void Enable();
	void Disable();
	void Start(int32 set_timer_time=0, bool ChangeResetTimer = true);
	void SetTimer(int32 set_timer_time=0);
	int32 GetRemainingTime() const;
	inline const int32& GetTimerTime()		{ return timer_time; }
	inline const int32& GetSetAtTrigger()	{ return set_at_trigger; }
	void Trigger();
	void SetAtTrigger(int32 set_at_trigger, bool iEnableIfDisabled = false);

	inline bool Enabled() const { return enabled; }
	inline int32 GetStartTime() const { return(start_time); }
	inline int32 GetDuration() const { return(timer_time); }

	static const int32 SetCurrentTime();
	static const int32 GetCurrentTime();
	static const int32 GetTimeSeconds();

private:
	int32	start_time;
	int32	timer_time;
	bool	enabled;
	int32	set_at_trigger;

	// Tells the timer to be more accurate about happening every X ms.
	// Instead of Check() setting the start_time = now,
	// it it sets it to start_time += timer_time
	bool	pUseAcurateTiming;

//	static int32 current_time;
//	static int32 last_time;
};

#endif
