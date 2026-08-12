#ifndef EVEMU_EVESERVER_SERVERLIFECYCLE_H_
#define EVEMU_EVESERVER_SERVERLIFECYCLE_H_

namespace ServerLifecycle {

inline bool ShouldContinue(bool processRunning, bool consoleRunning) noexcept {
  return processRunning && consoleRunning;
}

} // namespace ServerLifecycle

#endif // EVEMU_EVESERVER_SERVERLIFECYCLE_H_
