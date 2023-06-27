/** Daisy Seed2 DFM Pedal Dev Kit Basic Example
 *
 *  This example does a few things that can serve as a great jumping off
 *  point for new effects pedal projects.
 *
 *  1. Initializes Switch and GPIO for control of the true-bypass relays
 *  2. Initializes an LED to indicate bypass state (illuminate RED when audio runs through software)
 *  3. Sets up basic audio passthru
 */
#include "daisy_seed.h"

using namespace daisy;

/** Global objects that need to be accessed in both main() and the audio callback */
DaisySeed hardware;

/** The audio callback that fires whenever new audio samples can be prepared */
void AudioCallback(AudioHandle::InputBuffer in,
                   AudioHandle::OutputBuffer out,
                   size_t size)
{
    for (size_t i = 0; i < size; i++)
    {
        /** For each sample in the loop we'll multiply the input by our volume control
         *  OUT_x and IN_x are macros that access the out, and in buffers respectively.
         *  This would be equivalent to: out[0][i] = in[0][i] * volume;, etc.
         */
        OUT_L[i] = IN_L[i];
        OUT_R[i] = IN_R[i];

        /** Due to minor errata on the Rev4 hardware we need to phase invert the
         *  left channel so both outputs have the same phase */
        OUT_L[i] *= -1.f;
    }
}

int main()
{

    /** Initialize the Daisy Seed at maximum clock speed */
    hardware.Init(true);

    /** Relay Control/UI init */
    GPIO relay_left, relay_right;
    Switch relay_control;
    Led relay_led;
    relay_left.Init(seed::D20, GPIO::Mode::OUTPUT);
    relay_right.Init(seed::D21, GPIO::Mode::OUTPUT);
    relay_control.Init(seed::D23, hardware.AudioCallbackRate());
    relay_led.Init(seed::D11, true);

    /** Start the Audio */
    hardware.StartAudio(AudioCallback);

    while (true)
    {
        /** Debounce the toggle */
        relay_control.Debounce();

        /** Update the relay states based on the toggle position */
        bool bypass_state = relay_control.Pressed();
        relay_left.Write(bypass_state);
        relay_right.Write(bypass_state);

        /** Update the LED brightness based on true bypass state */
        if (bypass_state)
            relay_led.Set(1);
        else
            relay_led.Set(0);
        /** Update the LED (required in case of software PWM for values between 0-1)*/
        relay_led.Update();

        /** We'll delay for one millisecond before looping back over to keep everything nice, and predictable
         *  The hardware control work could be moved to the audio callback as well.
         */
        System::Delay(1);
    }
}
