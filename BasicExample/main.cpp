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
AnalogControl volume_left_pot, volume_right_pot;

/** The audio callback that fires whenever new audio samples can be prepared */
void AudioCallback(AudioHandle::InputBuffer in,
                   AudioHandle::OutputBuffer out,
                   size_t size)
{

    /** Process the AnalogControl to do some basic filtering from the ADC reads.
     *  To prevent quantization error, it would be prudent to add additional filtering
     *  at audio rate.
     */
    float volume_left = volume_left_pot.Process();
    float volume_right = volume_right_pot.Process();
    for (size_t i = 0; i < size; i++)
    {
        /** For each sample in the loop we'll multiply the input by our volume control
         *  OUT_x and IN_x are macros that access the out, and in buffers respectively.
         *  This would be equivalent to: out[0][i] = in[0][i] * volume;, etc.
         */
        OUT_L[i] = IN_L[i] * volume_left;
        OUT_R[i] = IN_R[i] * volume_right;
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
    AdcChannelConfig adc_cfg[2];
    adc_cfg[0].InitSingle(seed::A0); /**< alias for seed::D15 */
    adc_cfg[1].InitSingle(seed::A1); /**< alias for seed::D16 */
    hardware.adc.Init(adc_cfg, 2);
    volume_left_pot.Init(hardware.adc.GetPtr(0), hardware.AudioCallbackRate());
    volume_right_pot.Init(hardware.adc.GetPtr(1), hardware.AudioCallbackRate());

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
