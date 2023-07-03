#pragma once

#include "daisy_seed.h"
#include "dev/oled_ssd130x.h"

/** Define simpler log syntax */
using Log = daisy::Logger<daisy::LOGGER_EXTERNAL>;

class PedalDevKit
{
public:
    PedalDevKit() {}

    void Init()
    {
        /** Initialize Daisy Seed */
        seed.Init();

        /** Initialize the Relay */
        relay_left.Init(daisy::seed::D20, daisy::GPIO::Mode::OUTPUT);
        relay_right.Init(daisy::seed::D21, daisy::GPIO::Mode::OUTPUT);

        /** Initialize the three Pots and expression */
        daisy::AdcChannelConfig adc_cfg[4];
        adc_cfg[0].InitSingle(daisy::seed::A0);
        adc_cfg[1].InitSingle(daisy::seed::A1);
        adc_cfg[2].InitSingle(daisy::seed::A2);
        adc_cfg[3].InitSingle(daisy::seed::A4);
        seed.adc.Init(adc_cfg, 4);

        /** Initialize the two way toggle */
        toggle1.Init(daisy::seed::D23, seed.AudioCallbackRate());

        /** Initialize the footswitch */
        footswitch.Init(daisy::seed::D22, seed.AudioCallbackRate());

        /** Initialize the three way toggle */
        toggle2.Init(daisy::seed::D25, daisy::seed::D24);

        /** Initialize the RGB LED */
        led2.Init(daisy::seed::D11, daisy::seed::D12, daisy::seed::D31, true);

        /** Initialize the Single Color LED */
        led1.Init(daisy::seed::D9, true);

        /** Initialize the encoder */
        encoder.Init(daisy::seed::D28, daisy::seed::D27, daisy::seed::D18, seed.AudioCallbackRate());

        /** Initialize the momentary push button */
        pushbutton.Init(daisy::seed::D26, seed.AudioCallbackRate());

        /** Initialize MIDI input/output */
        daisy::MidiUartHandler::Config midi_cfg; /**< Defaults are okay */
        midi.Init(midi_cfg);

        /** Initialize the SDMMC and attach to FatFS Interface */
        daisy::SdmmcHandler::Config sdmmc_cfg;
        sdmmc_cfg.Defaults();
        sdmmc_cfg.speed = daisy::SdmmcHandler::Speed::STANDARD;
        sdmmc_cfg.width = daisy::SdmmcHandler::BusWidth::BITS_4;
        sdmmc_handler.Init(sdmmc_cfg);

        /** Initialize the OLED */
        Display::Config display_cfg;
        display_cfg.driver_config.transport_config.spi_config.periph = daisy::SpiHandle::Config::Peripheral::SPI_1;
        display_cfg.driver_config.transport_config.spi_config.baud_prescaler = daisy::SpiHandle::Config::BaudPrescaler::PS_8;
        /** Set up the SPI Pins for SPI1 */
        display_cfg.driver_config.transport_config.spi_config.pin_config.sclk = daisy::seed::D8;
        display_cfg.driver_config.transport_config.spi_config.pin_config.miso = daisy::Pin();
        display_cfg.driver_config.transport_config.spi_config.pin_config.mosi = daisy::seed::D10;
        display_cfg.driver_config.transport_config.spi_config.pin_config.nss = daisy::seed::D7;
        /** Command and Reset Pins */
        display_cfg.driver_config.transport_config.pin_config.dc = daisy::seed::D0;
        display_cfg.driver_config.transport_config.pin_config.reset = daisy::seed::D32;
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
        Log::StartLog(true); /**< USB C Logging */
        seed.adc.Start();     /**< ADC */

        /** Set Relays to DSP path */
        relay_left.Write(true);
        relay_right.Write(true);

        toled = daisy::System::GetNow();
        tled = daisy::System::GetNow();
    }

    /** Test sequence for animating LEDs */
    void AnimateLeds()
    {
        // auto now = System::GetNow();
        // if (now - tled > 1)
        {
            uint32_t now = daisy::System::GetNow();
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
        auto now = daisy::System::GetNow();
        if (now - toled > 16)
        {
            /** Update Animation */
            toled = now;
            uint32_t line = (now >> 5) % 128;
            // bool polarity = (now & 1023) > 511;
            display.Fill(false);
            display.SetCursor(4, 16);
            display.WriteString("Pedal DevKit Test", Font_7x10, true);
            display.DrawLine(0, 40, line, 40, true);
            display.Update();
        }
    }

    /** Wrapper for starting up audio */
    void StartAudio(daisy::AudioHandle::AudioCallback cb)
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

    using Display = daisy::OledDisplay<daisy::SSD130x4WireSpi128x64Driver>;

    daisy::DaisySeed seed;
    daisy::Switch toggle1, footswitch, pushbutton;
    daisy::Switch3 toggle2;
    daisy::Encoder encoder;

    /** Relay Control/UI init */
    daisy::GPIO relay_left, relay_right;
    daisy::Led led1;
    daisy::RgbLed led2;

    daisy::SdmmcHandler sdmmc_handler;
    daisy::MidiUartHandler midi;
    Display display;

private:
    /** Timing trackers for animations */
    uint32_t toled;
    uint32_t tled;
};