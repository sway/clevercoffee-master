/**
 * @file hwConfig.h
 *
 * @brief Cached copies of reboot-required hardware configuration
 *
 * These settings can only change with a device restart, so they are read from
 * the Config store exactly once during setup() via loadHwConfig(). Hot-path code
 * (the main loop and its handlers) then reads these plain variables instead of
 * doing a JSON lookup with a String allocation on every loop iteration.
 */

#pragma once

#include "Config.h"
#include "hardware/Switch.h"

struct HwConfig {
        int brewSwitchType = Switch::TOGGLE;
        int steamSwitchType = Switch::TOGGLE;
        int powerSwitchType = Switch::TOGGLE;
        int hotWaterSwitchType = Switch::TOGGLE;
        bool pressureEnabled = false;
};

inline HwConfig hwConfig;

inline void loadHwConfig(const Config& config) {
    hwConfig.brewSwitchType = config.get<int>("hardware.switches.brew.type");
    hwConfig.steamSwitchType = config.get<int>("hardware.switches.steam.type");
    hwConfig.powerSwitchType = config.get<int>("hardware.switches.power.type");
    hwConfig.hotWaterSwitchType = config.get<int>("hardware.switches.hot_water.type");
    hwConfig.pressureEnabled = config.get<bool>("hardware.sensors.pressure.enabled");
}
