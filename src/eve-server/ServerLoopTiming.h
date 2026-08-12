#ifndef EVEMU_EVESERVER_SERVERLOOPTIMING_H_
#define EVEMU_EVESERVER_SERVERLOOPTIMING_H_

#include <cstdint>

namespace ServerLoop {

inline std::uint32_t
RemainingSleepMilliseconds(std::uint32_t targetMilliseconds,
                           std::uint32_t elapsedMilliseconds) {
    if (elapsedMilliseconds >= targetMilliseconds)
        return 0;

    return targetMilliseconds - elapsedMilliseconds;
}

} // namespace ServerLoop

#endif // EVEMU_EVESERVER_SERVERLOOPTIMING_H_
