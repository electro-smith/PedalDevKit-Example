/** Daisy Seed2 DFM Pedal Dev Kit Basic Example
 *
 *  This example does a few things that can serve as a great jumping off
 *  point for new effects pedal projects.
 *
 *  1. Initializes Switch and GPIO for control of the true-bypass relays
 *  2. Initializes an LED to indicate bypass state (illuminate RED when audio runs through software)
 *  3. Initializes POT 1 and 2 to be used to control the volume of the audio channels
 *  4. Sets up basic audio with volume control via POT 1 and 2
 */
#include "daisy_seed.h"

using namespace daisy;

/** Global objects that need to be accessed in both main() and the audio callback */
DaisySeed hardware;

int main()
{

    /** Initialize the Daisy Seed at maximum clock speed */
    hardware.Init(true);

    Switch toggle;
    toggle.Init(seed::D23, hardware.AudioCallbackRate());

    hardware.StartLog();

    while (true)
    {
        bool toggle_state = toggle.RawState();
        if (toggle_state)
            hardware.PrintLine("Toggle On");
        else
            hardware.PrintLine("Toggle Off");

        /** Delay 100ms and then repeat */
        System::Delay(100);
    }
}
