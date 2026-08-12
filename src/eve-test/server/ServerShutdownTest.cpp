#include "eve-test.h"

#include "ServerLifecycle.h"

#include <cstdio>

int server_ServerShutdownTest(int, char **) {
  if (ServerLifecycle::ShouldContinue(false, true)) {
    std::fprintf(stderr, "Signal shutdown was re-enabled\n");
    return EXIT_FAILURE;
  }

  if (ServerLifecycle::ShouldContinue(true, false)) {
    std::fprintf(stderr, "Console shutdown was ignored\n");
    return EXIT_FAILURE;
  }

  if (!ServerLifecycle::ShouldContinue(true, true)) {
    std::fprintf(stderr, "Active server was stopped unexpectedly\n");
    return EXIT_FAILURE;
  }

  return EXIT_SUCCESS;
}
