#ifndef STAC_STARTUP_CONFIG_H
#define STAC_STARTUP_CONFIG_H

#include <memory>
#include <functional>
#include <XP_Button.h>
#include "../Hardware/Display/IDisplay.h"
#include "../Storage/ConfigManager.h"
#include "../Config/Types.h"
#include "../Config/Constants.h"

// Forward declare glyph manager templates
namespace Display {
    template<uint8_t SIZE>
    class GlyphManager;
}

namespace Application {

    /**
     * @brief Handles interactive startup parameter configuration
     * 
     * Template class that works with 5x5 or 8x8 glyph managers.
     * 
     * Provides user interface for configuring:
     * - Tally channel selection
     * - Display mode (Camera Operator / Talent)
     * - Startup mode (Standard / Autostart)
     * - Display brightness
     * 
     * Uses button press patterns:
     * - Click: Advance to next parameter or cycle through options
     * - Long press: Enter/exit select state, confirm selection
     * - 30s timeout: Cancel changes and revert
     */
    template<uint8_t GLYPH_SIZE>
    class StartupConfig {
    public:
        /**
         * @brief Construct startup configurator
         * @param btn Button interface for user input
         * @param disp Display interface for visual feedback
         * @param glyphs Glyph manager for rendering text/icons
         * @param cfg Configuration manager for persistence
         * @param btnB Optional secondary button for reset (M5StickC Plus)
         */
        StartupConfig(
            Button* btn,
            Display::IDisplay* disp,
            Display::GlyphManager<GLYPH_SIZE>* glyphs,
            Storage::ConfigManager* cfg,
            Button* btnB = nullptr
        );

        /**
         * @brief Run the startup configuration sequence
         * @param ops Current operations configuration (modified by user)
         * @param autoStartBypass If true, skip all interactive config (autostart mode)
         * @return true if WiFi connect should proceed, false otherwise
         */
        bool runStartupSequence(StacOperations& ops, bool autoStartBypass);

        /**
         * @brief Allow runtime brightness adjustment (called from STACApp during operation)
         * @param ops Operations configuration (brightness level will be modified)
         */
        void changeBrightness(StacOperations& ops);

        /**
         * @brief Allow runtime brightness adjustment with custom save callback (for peripheral mode)
         * @param currentBrightness Current brightness level (1-6 for 5x5, 1-8 for 8x8)
         * @param saveCallback Function to save brightness to NVS
         * @return New brightness level (or original if timeout/cancelled)
         */
        uint8_t changeBrightness(uint8_t currentBrightness, std::function<void(uint8_t)> saveCallback);

        /**
         * @brief Change Camera/Talent mode setting
         * @param ops Operations configuration (cameraOperatorMode will be modified)
         */
        void changeCameraTalentMode(StacOperations& ops);

        /**
         * @brief Change Camera/Talent mode with custom save callback (for peripheral mode)
         * @param currentMode Current mode (true=camera, false=talent)
         * @param saveCallback Function to save mode to NVS
         * @return New mode (or original if timeout/cancelled)
         */
        bool changeCameraTalentMode(bool currentMode, std::function<void(bool)> saveCallback);

    private:
        Button* button;
        Button* buttonB;  // Optional secondary button for reset (nullptr if not available)
        Display::IDisplay* display;
        Display::GlyphManager<GLYPH_SIZE>* glyphManager;
        Storage::ConfigManager* configManager;

        // Configuration step handlers
        void displayTallyChannel(const StacOperations& ops);
        void changeTallyChannel(StacOperations& ops);
        
        void displayTallyMode(const StacOperations& ops);
        
        void displayStartupMode(const StacOperations& ops);
        void changeStartupMode(StacOperations& ops);
        
        void displayBrightness(const StacOperations& ops);

        // Helper functions
        void showConfirmation();
        void waitForButtonRelease();
        bool checkForButtonClick();
        bool checkForLongPress();
        bool waitForSelectInput(unsigned long timeoutMs);
        
        /**
         * @brief Check if Button B was pressed and restart if so
         * Polls buttonB and calls ESP.restart() if released
         */
        void checkButtonBReset();
    };

    // Type aliases for convenience
    using StartupConfig5x5 = StartupConfig<5>;
    using StartupConfig8x8 = StartupConfig<8>;

} // namespace Application

// Include template implementation
#include "StartupConfig.tpp"

#endif // STAC_STARTUP_CONFIG_H

//  --- EOF --- //
