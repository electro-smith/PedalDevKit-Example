/** Daisy Seed2 DFM Pedal Eval Basic Example
 *
 *  This example does a few things
 *
 *  1. Initializes Switch and GPIO for control of the true-bypass relays
 *  2. Initializes an LED to indicate bypass state
 *  3. Initializes POT 1 to be used to control the volume of the Audio
 *  4. Sets up basic audio with volume control via POT 1
 */
#include "daisy_seed.h"

using namespace daisy;

/** Global objects that need to be accessed in both main() and the audio callback */
DaisySeed hardware;
AnalogControl volume_pot;


/** The audio callback that fires whenever new audio samples can be prepared */
void AudioCallback(AudioHandle::InputBuffer in,
                   AudioHandle::OutputBuffer out,
                   size_t size)
{

    /** Process the AnalogControl to do some basic filtering from the ADC reads.
     *  To prevent quantization error, it would be prudent to add additional filtering
     *  at audio rate.
     */
    float volume = volume_pot.Process();
    for (size_t i = 0; i < size; i++)
    {
        /** For each sample in the loop we'll multiply the input by our volume control
         *  OUT_x and IN_x are macros that access the out, and in buffers respectively.
         *  This would be equivalent to: out[0][i] = in[0][i] * volume;, etc.
         */
        OUT_L[i] = IN_L[i] * volume;
        OUT_R[i] = IN_R[i] * volume;
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

    /** ADC and Volume Control setup */
    AdcChannelConfig pot1_cfg;
    pot1_cfg.InitSingle(seed::A0); /**< alias for seed::D15 */
    hardware.adc.Init(&pot1_cfg, 1);
    volume_pot.Init(hardware.adc.GetPtr(0), hardware.AudioCallbackRate());

    /** Start the ADC and then the Audio */
    hardware.adc.Start();
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
