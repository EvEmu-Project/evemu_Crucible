/**
 * @name Threading.h
 *   lightweight threading code specifically for EvEmu concurrency
 *   this code is very basic, and very specific.
 * @Author:         Allan
 * @date:   05 March 2016
 */


#ifndef EVE_THREADING_H
#define EVE_THREADING_H

#include <thread>

#include "../eve-core.h"
#include "utils/Singleton.h"

class Threading
: public Singleton<Threading>
{
public:
    Threading();
    ~Threading();

    void Initialize();
    void RunLoop();
    void Process();
    void AddSocket(SOCKET soc);
    void AddThread(std::thread* thread);
    void RemoveThread(std::thread* thread);
    std::thread* CreateThread(void *(*start_routine) (void *), void *args);
    void EndThreads();
    void ListThreads();


    uint8 Count()                           { return (uint8)(m_threads.size()); }

protected:
    char* buf;
    fd_set rSoc, wSoc;
    struct timeval tv;

    uint8 nfds;
    uint8 sleepTime;
    uint32 bufferLen;

private:
    std::vector<std::thread*> m_threads;
};

//Singleton
#define sThread \
    ( Threading::get() )

#endif  // EVE_THREADING_H
