#pragma once

#include "displayCommon.h"

namespace Templates {
    namespace Standard {
#include "displayTemplateStandard.h"
    }

    namespace Minimal {
#include "displayTemplateMinimal.h"
    }

    namespace TempOnly {
#include "displayTemplateTempOnly.h"
    }

    namespace Scale {
#include "displayTemplateScale.h"
    }

    namespace Upright {
#include "displayTemplateUpright.h"
    }

    namespace MinimalWithStatus {
#include "displayTemplateMinimalWithStatus.h"
    }
} // namespace Templates

class DisplayTemplateManager {
    public:
        enum Template {
            STANDARD = 0,
            MINIMAL = 1,
            TEMP_ONLY = 2,
            SCALE = 3,
            UPRIGHT = 4,
            MINIMAL_WITH_STATUS = 5,
        };

        static void initializeDisplay(const int templateId) {
            currentTemplate = templateId;

            switch (templateId) {
                case STANDARD:
                    currentPrintScreen = &Templates::Standard::printScreen;
                    break;
                case MINIMAL:
                    currentPrintScreen = &Templates::Minimal::printScreen;
                    break;
                case TEMP_ONLY:
                    currentPrintScreen = &Templates::TempOnly::printScreen;
                    break;
                case SCALE:
                    currentPrintScreen = &Templates::Scale::printScreen;
                    break;
                case UPRIGHT:
                    currentPrintScreen = &Templates::Upright::printScreen;
                    break;
                case MINIMAL_WITH_STATUS:
                    currentPrintScreen = &Templates::MinimalWithStatus::printScreen;
                    break;
                default:
                    currentPrintScreen = &Templates::Standard::printScreen;
                    break;
            }
        }

        static void printScreen() {
            if (currentPrintScreen) {
                currentPrintScreen();
            }
        }

    private:
        static inline int currentTemplate = STANDARD;
        static inline void (*currentPrintScreen)() = nullptr;
};
