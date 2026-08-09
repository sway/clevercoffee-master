/**
 * @file displayTemplateMinimalWithStatus.h
 *
 * @brief Minimal display template with status line and heater power bar
 *
 */

#pragma once

inline void displayUptime(const int x, const int y) {
    const unsigned long totalSeconds = millis() / 1000;
    const unsigned long hours = totalSeconds / 3600;
    const unsigned long minutes = totalSeconds % 3600 / 60;

    char uptimeString[11];
    if (hours < 1) {
        snprintf(uptimeString, sizeof(uptimeString), "%lum", minutes);
    }
    else {
        snprintf(uptimeString, sizeof(uptimeString), "%luh %02lum", hours, minutes);
    }

    u8g2->setFont(u8g2_font_profont11_tf);
    const int autox = x + 11 - u8g2->getStrWidth(uptimeString);
    u8g2->drawStr(autox, y, uptimeString);
}

inline void customDisplayStatusbar() {
    // For status info
    u8g2->drawLine(0, 12, 128, 12);

    if (!offlineMode) {
        displayWiFiStatus(4, 1);
        displayMQTTStatus(40, 0);
    }
    else {
        u8g2->setCursor(40, 0);
        u8g2->setFont(u8g2_font_profont10_tf);
        u8g2->print(langstring_offlinemode);
    }

    displayBluetoothStatus(24, 1);

    displayUptime(113, 0);
}

inline void printScreen() {

    if (displayMachineState()) {
        return;
    }

    u8g2->clearBuffer();

    customDisplayStatusbar();

    const float delta = config.get<float>("display.blinking.delta");
    const bool nearSetpoint = fabs(temperature - setpoint) <= delta;
    const int blinkMode = config.get<int>("display.blinking.mode");

    // Large temperature display, centered
    if (!(isrCounter < 500 && ((nearSetpoint && blinkMode == 1) || (!nearSetpoint && blinkMode == 2)))) {
        u8g2->setFont(custom_helvB24);

        char tempBuf[12];
        const int decimals = (temperature < 99.95) ? 1 : 0;
        snprintf(tempBuf, sizeof(tempBuf), decimals ? "%.1f\xb0" : "%.0f\xb0", temperature);
        const int startX = (128 - u8g2->getStrWidth(tempBuf)) / 2;

        u8g2->setCursor(startX, 16);
        u8g2->print(tempBuf);
    }

    // State text, centered, all caps
    u8g2->setFont(custom_helvB08);
    char stateBuf[32] = "";

    if (machineState == kSteam) {
        strncpy(stateBuf, "STEAM ON", sizeof(stateBuf));
    }
    else if (machineState == kManualFlush && config.get<bool>("hardware.switches.brew.enabled")) {
        snprintf(stateBuf, sizeof(stateBuf), "FLUSHING %.0fs", currBrewTime / 1000.0);
    }
    else if (machineState == kHotWater && config.get<bool>("hardware.switches.brew.enabled")) {
        snprintf(stateBuf, sizeof(stateBuf), "HOT WATER %.0fs", currPumpOnTime / 1000.0);
    } else if (config.get<bool>("hardware.switches.brew.enabled") && shouldDisplayBrewTimer()) {
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
        snprintf(stateBuf, sizeof(stateBuf),
                 "HEATING UP TO %.0f\xb0"
                 "C",
                 setpoint);
    }
    else if (!nearSetpoint && temperature > setpoint) {
        strncpy(stateBuf, "TEMP TOO HIGH", sizeof(stateBuf));
    } else if (nearSetpoint) {
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
