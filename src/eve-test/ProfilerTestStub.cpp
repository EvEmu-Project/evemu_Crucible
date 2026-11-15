// The eve-core database translation unit references the server profiler.
// Unit tests link eve-core without the full server executable, so provide the
// no-op timing sink needed by this standalone test binary.
#include "Profiler.h"

void Profiler::AddTime(uint8 key, double value)
{
    (void)key;
    (void)value;
}
