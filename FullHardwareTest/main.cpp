/** Daisy Seed2 DFM Pedal Dev Kit - Full Hardware Test
 *
 *  This program initializes and tests all of the hardware on the Pedal Dev Kit
 *
 *  The following test data is printed via serial over the USB-C connector.
 *
 */
#include "daisy_seed.h"
#include "dev/oled_ssd130x.h"

using namespace daisy;

/** Define simpler log syntax */
using Log = Logger<LOGGER_EXTERNAL>;

class PedalDevKit
{
public:
    PedalDevKit() {}

    void Init()
    {
        /** Initialize Daisy Seed */
        seed.Init();

        /** Initialize the Relay */
        relay_left.Init(seed::D20, GPIO::Mode::OUTPUT);
        relay_right.Init(seed::D21, GPIO::Mode::OUTPUT);

        /** Initialize the three Pots and expression */
        AdcChannelConfig adc_cfg[4];
        adc_cfg[0].InitSingle(seed::A0);
        adc_cfg[1].InitSingle(seed::A1);
        adc_cfg[2].InitSingle(seed::A2);
        adc_cfg[3].InitSingle(seed::A4);
        seed.adc.Init(adc_cfg, 4);

        /** Initialize the two way toggle */
        toggle1.Init(seed::D23, seed.AudioCallbackRate());

        /** Initialize the footswitch */
        footswitch.Init(seed::D22, seed.AudioCallbackRate());

        /** Initialize the three way toggle */
        toggle2.Init(seed::D24, seed::D25);

        /** Initialize the RGB LED */
        led2.Init(seed::D11, seed::D12, seed::D31, true);

        /** Initialize the Single Color LED */
        led1.Init(seed::D11, true);

        /** Initialize the encoder */
        encoder.Init(seed::D27, seed::D28, seed::D18, seed.AudioCallbackRate());

        /** Initialize the momentary push button */
        pushbutton.Init(seed::D26, seed.AudioCallbackRate());

        /** Initialize MIDI input/output */
        MidiUartHandler::Config midi_cfg; /**< Defaults are okay */
        midi.Init(midi_cfg);

        /** Initialize the SDMMC */
        SdmmcHandler::Config sdmmc_cfg;
        sdmmc_cfg.speed = SdmmcHandler::Speed::STANDARD;
        sdmmc_cfg.width = SdmmcHandler::BusWidth::BITS_4;

        /** Initialize the OLED */
        Display::Config display_cfg;
        display_cfg.driver_config.transport_config.pin_config.dc = seed::D0;
        display_cfg.driver_config.transport_config.pin_config.reset = seed::D32;
        display.Init(display_cfg);
        display.Fill(false);
        display.Update();

        /** Start up background bits */
        Log::StartLog();  /**< USB C Logging */
        seed.adc.Start(); /**< ADC */
    }

    /** Test sequence for animating LEDs */
    void AnimateLeds()
    {
        auto now = System::GetNow();
        if (now - tled > 2)
        {
            /** Update Animation */
            toled = now;
        }
    }

    /** Test sequence for animating the OLED */
    void AnimateOLED()
    {
        auto now = System::GetNow();
        if (now - toled > 16)
        {
            /** Update Animation */
            toled = now;
        }
    }

    /** Wrapper for starting up audio */
    void StartAudio(AudioHandle::AudioCallback cb)
    {
        seed.StartAudio(cb);
    }

    using Display = OledDisplay<SSD130x4WireSpi128x64Driver>;

    DaisySeed seed;
    Switch toggle1, footswitch, pushbutton;
    Switch3 toggle2;
    Encoder encoder;

    /** Relay Control/UI init */
    GPIO relay_left, relay_right;
    Led led1;
    RgbLed led2;

    SdmmcHandler sdmmc_handler;
    MidiUartHandler midi;
    Display display;

private:
    /** Timing trackers for animations */
    uint32_t toled;
    uint32_t tled;
};

/** Global objects that need to be accessed in both main() and the audio callback */
PedalDevKit hardware;

/** The audio callback that fires whenever new audio samples can be prepared */
void AudioCallback(AudioHandle::InputBuffer in,
                   AudioHandle::OutputBuffer out,
                   size_t size)
{

    /** Process the AnalogControl to do some basic filtering from the ADC reads.
     *  To prevent quantization error, it would be prudent to add additional filtering
     *  at audio rate.
     */
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
    hardware.Init();

    /** Start the the Audio */
    hardware.StartAudio(AudioCallback);

    while (true)
    {
    }
}
