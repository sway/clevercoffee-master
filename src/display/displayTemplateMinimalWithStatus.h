/**
 * @file displayTemplateMinimalWithStatus.h
 *
 * @brief Minimal display template with status line and heater power bar
 *
 */

#pragma once

inline void printScreen() {

    if (displayMachineState()) {
        return;
    }

    u8g2->clearBuffer();

    displayStatusbar();

    const float delta = config.get<float>("display.blinking.delta");
    const bool nearSetpoint = fabs(temperature - setpoint) <= delta;
    const int blinkMode = config.get<int>("display.blinking.mode");
    const bool brewEnabled = config.get<bool>("hardware.switches.brew.enabled");

    // Large temperature display, centered
    if (!(isrCounter < 500 && ((nearSetpoint && blinkMode == 1) || (!nearSetpoint && blinkMode == 2)))) {
        u8g2->setFont(custom_helvB24);

        const int decimals = (temperature >= 99.95) ? 0 : 1;
        const int startX = decimals ? 26 : 30;

        u8g2->setCursor(startX, 16);
        u8g2->print(temperature, decimals);
        u8g2->print(static_cast<char>(176));
    }

    // State text, centered, all caps
    u8g2->setFont(custom_helvB08);
    char stateBuf[32] = "";

    if (machineState == kSteam) {
        strncpy(stateBuf, "STEAM ON", sizeof(stateBuf));
    }
    else if (machineState == kManualFlush && brewEnabled) {
        snprintf(stateBuf, sizeof(stateBuf), "FLUSHING %.0fs", currBrewTime / 1000.0);
    }
    else if (machineState == kHotWater && brewEnabled) {
        snprintf(stateBuf, sizeof(stateBuf), "HOT WATER %.0fs", currPumpOnTime / 1000.0);
    }
    else if (brewEnabled && shouldDisplayBrewTimer()) {
        const bool automaticBrewingEnabled = config.get<int>("brew.mode") == 1;

        // Time — show target if set
        const bool hasTimeTarget = totalTargetBrewTime > 0 && config.get<bool>("brew.by_time.enabled") && automaticBrewingEnabled;
        if (hasTimeTarget) {
            snprintf(stateBuf, sizeof(stateBuf), "%.0f/%.0fs", currBrewTime / 1000.0, totalTargetBrewTime / 1000.0);
        }
        else {
            snprintf(stateBuf, sizeof(stateBuf), "%.1fs", currBrewTime / 1000.0);
        }

        // Weight — show target if set
        if (scale && !scaleFailure && config.get<bool>("hardware.sensors.scale.enabled")) {
            const bool hasWeightTarget = automaticBrewingEnabled && config.get<bool>("brew.by_weight.enabled");
            if (hasWeightTarget) {
                const auto targetBrewWeight = ParameterRegistry::getInstance().getParameterById("brew.by_weight.target_weight")->getValueAs<float>();
                snprintf(stateBuf + strlen(stateBuf), sizeof(stateBuf) - strlen(stateBuf), "  %.0f/%.0fg", currBrewWeight, targetBrewWeight);
            }
            else {
                snprintf(stateBuf + strlen(stateBuf), sizeof(stateBuf) - strlen(stateBuf), "  %.0fg", currBrewWeight);
            }
        }

        // Pressure
        if (config.get<bool>("hardware.sensors.pressure.enabled")) {
            snprintf(stateBuf + strlen(stateBuf), sizeof(stateBuf) - strlen(stateBuf), "  %.1fB", inputPressure);
        }
    }
    else if (!nearSetpoint && temperature < setpoint) {
        snprintf(stateBuf, sizeof(stateBuf), "HEATING UP TO %.0f%cC", setpoint, static_cast<char>(176));
    }
    else if (!nearSetpoint && temperature > setpoint) {
        strncpy(stateBuf, "TEMP TOO HIGH", sizeof(stateBuf));
    }
    else if (nearSetpoint) {
        strncpy(stateBuf, "READY TO BREW", sizeof(stateBuf));
    }

    if (stateBuf[0] != '\0') {
        u8g2->setCursor((128 - u8g2->getStrWidth(stateBuf)) / 2, 45);
        u8g2->print(stateBuf);
    }

    // Heater power bar with bolt icon (open_iconic_other, encoding 64)
    u8g2->setFont(u8g2_font_open_iconic_other_1x_t);
    u8g2->setCursor(4, 57);
    u8g2->print(static_cast<char>(64));

    constexpr int barX = 14;
    constexpr int barY = 59;
    constexpr int barW = 110;
    constexpr int barH = 5;
    u8g2->drawRFrame(barX, barY, barW, barH, 2);

    if (const int fillW = map(static_cast<int>(pidOutput / 10), 0, 100, 0, barW - 2); fillW > 0) {
        u8g2->drawBox(barX + 1, barY + 1, fillW, barH - 2);
    }

    displayBufferReady = true;
}
