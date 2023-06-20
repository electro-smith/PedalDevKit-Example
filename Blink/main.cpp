/** Daisy Blink Example
 *
 *  This example blinks the Daisy Seed's LED
 *  once per second, as explained in the
 *  C++ Getting Started guide for the Daisy.
*/
#include "daisy_seed.h"

using namespace daisy;

DaisySeed hardware;

int main()
{
    hardware.Init();

    while (true)
    {
        hardware.SetLed(true);
        hardware.DelayMs(500);
        hardware.SetLed(false);
        hardware.DelayMs(500);
    }
}
