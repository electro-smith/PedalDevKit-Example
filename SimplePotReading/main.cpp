/** Daisy Seed2 DFM Pedal Dev Kit Basic Example
 *
 *  In this example we set up two pots, and print their values over serial.
 *  To see the serial data printing on your computer, you can use any serial monitor
 *  (Serial Monitor extension in VS Code, Arduino Serial Monitor, PutTy, etc.) 
 */
#include "daisy_seed.h"

using namespace daisy;

/** Global objects that need to be accessed in both main() and the audio callback */
DaisySeed hardware;

int main()
{

    /** Initialize the Daisy Seed at maximum clock speed */
    hardware.Init(true);

    /** ADC and Volume Control setup */
    AdcChannelConfig adc_cfg[2];
    adc_cfg[0].InitSingle(seed::A0); /**< alias for seed::D15 */
    adc_cfg[1].InitSingle(seed::A1); /**< alias for seed::D16 */
    hardware.adc.Init(adc_cfg, 2);

    /** Start the serial logger, and the ADC */
    hardware.StartLog();
    hardware.adc.Start();


    while (true)
    {
        /** Every 100ms print the data,
         *  the basic adc.Get function will return a value from 0-65535 
         */
        hardware.PrintLine("--- Pot Data ---");
        hardware.PrintLine("POT 1:\t%d", hardware.adc.Get(0));
        hardware.PrintLine("POT 2:\t%d", hardware.adc.Get(1));
        System::Delay(100);
    }
}
