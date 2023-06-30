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
        toggle2.Init(seed::D25, seed::D24);

        /** Initialize the RGB LED */
        led2.Init(seed::D11, seed::D12, seed::D31, true);

        /** Initialize the Single Color LED */
        led1.Init(seed::D9, true);

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
        display_cfg.driver_config.transport_config.spi_config.periph = SpiHandle::Config::Peripheral::SPI_1;
        display_cfg.driver_config.transport_config.spi_config.baud_prescaler = SpiHandle::Config::BaudPrescaler::PS_8;
        /** Set up the SPI Pins for SPI1 */
        display_cfg.driver_config.transport_config.spi_config.pin_config.sclk = seed::D8;
        display_cfg.driver_config.transport_config.spi_config.pin_config.miso = Pin();
        display_cfg.driver_config.transport_config.spi_config.pin_config.mosi = seed::D10;
        display_cfg.driver_config.transport_config.spi_config.pin_config.nss = seed::D7;
        /** Command and Reset Pins */
        display_cfg.driver_config.transport_config.pin_config.dc = seed::D0;
        display_cfg.driver_config.transport_config.pin_config.reset = seed::D32;
        display.Init(display_cfg);
        display.Fill(false);
        display.SetCursor(4, 16);
        display.WriteString("Insert USB-C", Font_7x10, true);
        display.SetCursor(4, 36);
        display.WriteString("And open serial", Font_6x8, true);
        display.SetCursor(4, 44);
        display.WriteString("monitor. . .", Font_6x8, true);
        display.Update();

        /** Start up background bits */
        Log::StartLog(true);  /**< USB C Logging */
        seed.adc.Start(); /**< ADC */

        toled = System::GetNow();
        tled = System::GetNow();
    }

    /** Test sequence for animating LEDs */
    void AnimateLeds()
    {
        // auto now = System::GetNow();
        // if (now - tled > 1)
        {
            uint32_t now = System::GetNow();
            /** Update Animation */
            // tled = now;
            /** 0-1 */
            float b1 = (now & 1023) / 1023.f;
            led1.Set(b1);
            led1.Update();
            led1.Update();
            led1.Update();
            led1.Update();

            /** 0-4 */
            float b2 = (now & 4095) / 1023.f;
            float r = b2 < 1.0f ? b2 : 0.f;
            float g = b2 < 2.0f && b2 > 0.99f ? b2 - 1.f : 0.f;
            float b = b2 < 3.0f && b2 > 1.99f ? b2 - 2.f : 0.f;
            led2.Set(r, g, b);
            led2.Update();
            led2.Update();
            led2.Update();
            led2.Update();
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
            bool polarity = (now & 1023) > 511;
            display.Fill(polarity);
            display.SetCursor(4, 16);
            display.WriteString("Pedal DevKit Test", Font_7x10, !polarity);
            display.Update();
        }
    }

    /** Wrapper for starting up audio */
    void StartAudio(AudioHandle::AudioCallback cb)
    {
        seed.StartAudio(cb);
    }

    /** Debounces controls, and updates values */
    void ProcessAllControls()
    {
        toggle1.Debounce();
        footswitch.Debounce();
        pushbutton.Debounce();
        encoder.Debounce();
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
int32_t enc_value_tracker;

/** The audio callback that fires whenever new audio samples can be prepared */
void AudioCallback(AudioHandle::InputBuffer in,
                   AudioHandle::OutputBuffer out,
                   size_t size)
{
    /** Debounce encoders */
    hardware.ProcessAllControls();
    /** LEDs are driven from software PWM.
     *  So having them in the audio loop helps to keep their PWM consistent.
     */
    hardware.AnimateLeds();

    /** Update Encoder tracker value
     *  +/- 1 from increment
     *  reset to 0 with click
     */
    enc_value_tracker += hardware.encoder.Increment();
    if (hardware.encoder.RisingEdge())
        enc_value_tracker = 0;

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

/** Fills string with string representation of MidiEvent::Type
 *  str needs to be at least 16 bytes long to store the data
 */
void GetMidiTypeAsString(MidiEvent &msg, char *str)
{
    switch (msg.type)
    {
    case NoteOff:
        strcpy(str, "NoteOff");
        break;
    case NoteOn:
        strcpy(str, "NoteOn");
        break;
    case PolyphonicKeyPressure:
        strcpy(str, "PolyKeyPres.");
        break;
    case ControlChange:
        strcpy(str, "CC");
        break;
    case ProgramChange:
        strcpy(str, "Prog. Change");
        break;
    case ChannelPressure:
        strcpy(str, "Chn. Pressure");
        break;
    case PitchBend:
        strcpy(str, "PitchBend");
        break;
    case SystemCommon:
        strcpy(str, "Sys. Common");
        break;
    case SystemRealTime:
        strcpy(str, "Sys. Realtime");
        break;
    case ChannelMode:
        strcpy(str, "Chn. Mode");
        break;
    default:
        strcpy(str, "Unknown");
        break;
    }
}

int main()
{

    /** Initialize the Daisy Seed at maximum clock speed */
    hardware.Init();

    /** Start the the Audio */
    hardware.StartAudio(AudioCallback);

    uint32_t now, tprint;
    now = tprint = System::GetNow();

    MidiEvent last_midi_event;
    hardware.midi.StartReceive();

    while (true)
    {
        now = System::GetNow();
        hardware.AnimateOLED();

        if (hardware.midi.HasEvents())
        {
            while (hardware.midi.HasEvents())
            {
                auto msg = hardware.midi.PopEvent();
                /** Print all messages when there are events */
                // Log::PrintLine("--- MIDI ---");
                // char outstr[128];
                // char type_str[16];
                // GetMidiTypeAsString(msg, type_str);
                // sprintf(outstr,
                //         "time:\t%ld\ttype: %s\tChannel:  %d\tData MSB: "
                //         "%d\tData LSB: %d\n",
                //         now,
                //         type_str,
                //         msg.channel,
                //         msg.data[0],
                //         msg.data[1]);
                // Log::PrintLine(outstr);
                last_midi_event = msg;

                /** If the MIDI note is a note message we'll send that out to the MIDI output */
                if (msg.type == NoteOn || msg.type == NoteOff)
                {
                    uint8_t outmsg[3];
                    outmsg[0] = 0x90;
                    outmsg[1] = msg.data[0];
                    outmsg[2] = msg.data[1];
                    hardware.midi.SendMessage(outmsg, 3);
                }
            }
        }

        /** Print Data */
        if (now - tprint > 200)
        {
            /** Reset time so next print happens 100ms from now */
            tprint = now;

            /** Print all the stuff */
            Log::PrintLine("--- Controls ---");
            /** Pot values */
            Log::PrintLine("Pot1:\t%5d\tPot2:\t%5d\tPot3:\t%5d\tExpression:\t%5d",
                           hardware.seed.adc.Get(0),
                           hardware.seed.adc.Get(1),
                           hardware.seed.adc.Get(2),
                           hardware.seed.adc.Get(3));
            /** Buttons and Toggles */
            auto tog_state = hardware.toggle2.Read();
            Log::PrintLine("3-way Toggle:\t%s",
                           tog_state == Switch3::POS_LEFT ? "Left" : tog_state == Switch3::POS_CENTER ? "Center"
                                                                                                      : "Right");
            Log::PrintLine("2-way Toggle:\t%s", hardware.toggle1.Pressed() ? "Right" : "Left");
            Log::PrintLine("Footswitch:\t%s\tMomentary:\t%s",
                           hardware.footswitch.Pressed() ? "On" : "Off",
                           hardware.pushbutton.Pressed() ? "On" : "Off");
            /** Encoders */
            Log::PrintLine("Encoder:\t%d", enc_value_tracker);
            /** Persistent data for last midi message received gets printed */
            Log::PrintLine("--- MIDI ---");
            char outstr[128];
            char type_str[16];
            GetMidiTypeAsString(last_midi_event, type_str);
            sprintf(outstr,
                    "time:\t%ld\ttype: %s\tChannel:  %d\tData MSB: "
                    "%d\tData LSB: %d\n",
                    now,
                    type_str,
                    last_midi_event.channel,
                    last_midi_event.data[0],
                    last_midi_event.data[1]);
            Log::PrintLine(outstr);
        }
    }
}
