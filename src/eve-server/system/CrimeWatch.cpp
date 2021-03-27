/**
 * @name CrimeWatch.cpp
 *   EvEmu CrimeWatch code.
 *
 * @Author:         Allan
 * @date:   13 March 2016
 */


#include "CrimeWatch.h"
#include "EVEServerConfig.h"

/*
 * each client(character) will have it's own cw code.
the cw system will consist of the following:

    criminal flags
    aggression timers
    weapon timers
    jump/dock timers
    kill rights
    */

/** @todo  set up config variables for timers here */
CrimeWatch::CrimeWatch()
: t_crimFlag(sConfig.crime.CrimFlagTime *1000),
t_aggression(sConfig.crime.AggFlagTime *1000),
t_weapon(sConfig.crime.WeaponFlagTime *1000),
t_session(sConfig.crime.CWSessionTime *1000),
t_killRight(sConfig.crime.KillRightTime *1000)
{
    t_crimFlag.Disable();
    t_aggression.Disable();
    t_weapon.Disable();
    t_session.Disable();
    t_killRight.Disable();
}

void CrimeWatch::Init() {

}
