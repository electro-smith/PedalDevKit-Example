/** Daisy Seed2 DFM Pedal Dev Kit - Full Hardware Test
 *
 *  This program initializes and tests all of the hardware on the Pedal Dev Kit
 *
 *  The following test data is printed via serial over the USB-C connector.
 *
 */
#include "PedalDevKit.h"

using namespace daisy;

/** Global objects that need to be accessed in both main() and the audio callback */
PedalDevKit hardware;      /**< Global hardware object */
int32_t enc_value_tracker; /**< tracks value for encoder interactionse*/
FatFSInterface fsi;        /**< Inteface for linking FatFS to the hardware */
FIL test_file;             /**< test file for SD Card verification*/

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

    /** Test the SD card by writing and reading back a file */
    bool sd_card_success = false;
    fsi.Init(FatFSInterface::Config::MEDIA_SD);
    const char *test_string = "Daisy Pedal Dev Kit - Testing... OK.";
    char verify_string[64];
    std::fill(verify_string, verify_string + 64, 0);
    f_mount(&fsi.GetSDFileSystem(), "/", 0);

    if (f_open(&test_file, "pedal_dev_test.txt", (FA_CREATE_ALWAYS | FA_WRITE)) == FR_OK)
    {
        UINT bw = 0;
        /** write the test string to the file */
        if (f_write(&test_file, test_string, strlen(test_string), &bw) == FR_OK)
        {
            f_close(&test_file);
            /** re open file for reading */
            if (f_open(&test_file,
                       "pedal_dev_test.txt",
                       (FA_OPEN_EXISTING | FA_READ)) == FR_OK)
            {
                UINT br = 0;
                /** verify read data matches write data */
                if (f_read(&test_file, verify_string, strlen(test_string), &br) == FR_OK)
                {
                    if (br == bw && strcmp(verify_string, test_string) == 0)
                        sd_card_success = true;
                }
            }
        }
        /** close the file for good here */
        f_close(&test_file);
    }

    while (true)
    {
        now = System::GetNow();
        hardware.AnimateOLED();

        if (hardware.midi.HasEvents())
        {
            while (hardware.midi.HasEvents())
            {
                auto msg = hardware.midi.PopEvent();

                /** Always store the most recent message for display purposes */
                last_midi_event = msg;

                /** If the MIDI note is a note message we'll send that out to the MIDI output */
                if (msg.type == NoteOn || msg.type == NoteOff)
                {
                    uint8_t outmsg[3];
                    outmsg[0] = 0x90;
                    outmsg[1] = msg.data[0];
                    if (msg.type == NoteOff)
                        outmsg[2] = 0;
                    else
                        outmsg[2] = msg.data[1];
                    hardware.midi.SendMessage(outmsg, 3);
                }
            }
        }

        /** Print Data every 200ms */
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
            Log::PrintLine("SD Card Test:\t%s", sd_card_success ? "Pass" : "Fail");
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
