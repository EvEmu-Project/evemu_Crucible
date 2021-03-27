/**
 * @name CrimeWatch.h
 *   EvEmu CrimeWatch code.
 *
 * @Author:         Allan
 * @date:   13 March 2016
 */


#ifndef EVEMU_SERVER_CRIMEWATCH_H_
#define EVEMU_SERVER_CRIMEWATCH_H_


#include "eve-common.h"

class CrimeWatch
{
public:
    CrimeWatch();
    ~CrimeWatch()   { /* do nothing here */ }

    void Init();
protected:

private:
    Timer t_crimFlag;
    Timer t_aggression;
    Timer t_weapon;
    Timer t_session;
    Timer t_killRight;
};

#endif  // EVEMU_SERVER_CRIMEWATCH_H_