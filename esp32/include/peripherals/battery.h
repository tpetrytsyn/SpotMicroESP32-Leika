#pragma once

#include <peripherals/sensor.hpp>
#include <peripherals/drivers/ads1115.h>

#ifndef BATTERY_ADC_CHANNEL
#define BATTERY_ADC_CHANNEL 0
#endif

// Pack volts per volt at the ADC input: the voltage module's nominal 30 kΩ + 7.5 kΩ divider
#ifndef BATTERY_DIVIDER_RATIO
#define BATTERY_DIVIDER_RATIO 5.0f
#endif

struct BatteryMsg {
    float voltage {-1};
    bool success {false};
};

// Pack voltage through the voltage module on an ADS1115 input
class Battery : public SensorBase<BatteryMsg> {
  public:
    bool initialize() override {
        _msg.success = _adc.begin(BATTERY_ADC_CHANNEL);
        if (_msg.success) {
            ESP_LOGI("Battery", "ADS1115 initialized on A%d", BATTERY_ADC_CHANNEL);
        } else {
            ESP_LOGE("Battery", "ADS1115 initialization failed");
        }
        return _msg.success;
    }

    bool update() override {
        if (!_msg.success) return false;
        float volts = 0;
        if (!_adc.readVolts(volts)) return false;
        float voltage = volts * BATTERY_DIVIDER_RATIO;
        // Light smoothing: servo load steps make single samples jump by a few tenths of a volt
        _msg.voltage = _msg.voltage < 0 ? voltage : _msg.voltage + SMOOTHING * (voltage - _msg.voltage);
        return true;
    }

    float getVoltage() { return _msg.voltage; }

  private:
    static constexpr float SMOOTHING = 0.2f;
    ADS1115Driver _adc;
};
