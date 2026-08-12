#include "eve-test.h"

class Profiler
{
public:
    void AddTime(uint8, double);
};

void Profiler::AddTime(uint8 key, double value)
{
    (void)key;
    (void)value;
}
