#pragma once

#include <peripherals/i2c_bus.h>

// ADS1115 16-bit ADC, run in continuous mode on one single-ended channel: after setup a read is a single
// 2-byte register fetch, so it is cheap enough for the control task.
class ADS1115Driver {
  public:
    static constexpr uint8_t DEFAULT_ADDR = 0x48;

    enum class Gain : uint16_t {
        FSR_6_144V = 0x0000,
        FSR_4_096V = 0x0200,
        FSR_2_048V = 0x0400,
        FSR_1_024V = 0x0600,
        FSR_0_512V = 0x0800,
        FSR_0_256V = 0x0A00,
    };

    ADS1115Driver(uint8_t addr = DEFAULT_ADDR) : _addr(addr) {}

    bool begin(uint8_t channel, Gain gain = Gain::FSR_4_096V) {
        _initialized = false;
        if (channel > 3 || !I2CBus::instance().probe(_addr)) return false;

        uint16_t config = MUX_SINGLE_0 | (static_cast<uint16_t>(channel) << 12) | static_cast<uint16_t>(gain) |
                          MODE_CONTINUOUS | DR_128SPS | COMP_QUE_DISABLE;
        if (writeReg(REG_CONFIG, config) != ESP_OK) return false;

        uint16_t readback = 0;
        if (readReg(REG_CONFIG, readback) != ESP_OK || (readback & ~OS_BIT) != config) return false;

        _lsbVolts = lsbVolts(gain);
        _initialized = true;
        return true;
    }

    // Latest conversion of the configured channel, in volts at the input pin.
    bool readVolts(float &volts) {
        if (!_initialized) return false;
        uint16_t raw = 0;
        if (readReg(REG_CONVERSION, raw) != ESP_OK) return false;
        volts = static_cast<int16_t>(raw) * _lsbVolts;
        return true;
    }

    bool isInitialized() const { return _initialized; }

  private:
    static constexpr uint8_t REG_CONVERSION = 0x00;
    static constexpr uint8_t REG_CONFIG = 0x01;

    static constexpr uint16_t OS_BIT = 0x8000;
    static constexpr uint16_t MUX_SINGLE_0 = 0x4000; // AIN0 vs GND; OR the channel in at bit 12 for AIN1..3
    static constexpr uint16_t MODE_CONTINUOUS = 0x0000;
    static constexpr uint16_t DR_128SPS = 0x0080;
    static constexpr uint16_t COMP_QUE_DISABLE = 0x0003;

    static float lsbVolts(Gain gain) {
        switch (gain) {
            case Gain::FSR_6_144V: return 6.144f / 32768.0f;
            case Gain::FSR_4_096V: return 4.096f / 32768.0f;
            case Gain::FSR_2_048V: return 2.048f / 32768.0f;
            case Gain::FSR_1_024V: return 1.024f / 32768.0f;
            case Gain::FSR_0_512V: return 0.512f / 32768.0f;
            case Gain::FSR_0_256V: return 0.256f / 32768.0f;
        }
        return 0;
    }

    esp_err_t writeReg(uint8_t reg, uint16_t value) {
        uint8_t buf[2] = {static_cast<uint8_t>(value >> 8), static_cast<uint8_t>(value & 0xFF)};
        return I2CBus::instance().writeReg(_addr, reg, buf, 2);
    }

    esp_err_t readReg(uint8_t reg, uint16_t &value) {
        uint8_t buf[2] = {0, 0};
        esp_err_t err = I2CBus::instance().readReg(_addr, reg, buf, 2);
        if (err == ESP_OK) value = (static_cast<uint16_t>(buf[0]) << 8) | buf[1];
        return err;
    }

    uint8_t _addr;
    bool _initialized = false;
    float _lsbVolts = 0;
};
