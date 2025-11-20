// StartupConfig.tpp - Template implementation
// This file is included at the end of StartupConfig.h

#include "Config/Constants.h"
#include "Hardware/Display/Colors.h"
#include "Hardware/Display/Glyphs5x5.h"
#include "Hardware/Display/Glyphs8x8.h"
#include <Arduino.h>

namespace Application {

    template<uint8_t GLYPH_SIZE>
    StartupConfig<GLYPH_SIZE>::StartupConfig(
        Button* btn,
        Display::IDisplay* disp,
        Display::GlyphManager<GLYPH_SIZE>* glyphs,
        Storage::ConfigManager* cfg
    )
        : button(btn)
        , display(disp)
        , glyphManager(glyphs)
        , configManager(cfg)
    {}

    template<uint8_t GLYPH_SIZE>
    bool StartupConfig<GLYPH_SIZE>::runStartupSequence(StacOperations& ops, bool autoStartBypass) {
        if (autoStartBypass) {
            log_i("Autostart bypass active - skipping startup config");
            return false;  // No changes made
        }

        // Track if any values were changed
        bool configChanged = false;
        StacOperations originalOps = ops;

        // Show tally channel
        displayTallyChannel(ops);
        waitForButtonRelease();

        bool continueSequence = true;
        while (continueSequence) {
            button->read();
            
            if (button->wasReleased()) {
                // Advance to tally mode
                break;
            }
            if (button->pressedFor(Config::Timing::BUTTON_SELECT_MS)) {
                uint8_t oldChannel = ops.tallyChannel;
                changeTallyChannel(ops);
                if (ops.tallyChannel != oldChannel) {
                    configChanged = true;
                }
                displayTallyChannel(ops);
            }
        }

        // Show tally mode
        displayTallyMode(ops);
        waitForButtonRelease();

        continueSequence = true;
        while (continueSequence) {
            button->read();
            
            if (button->wasReleased()) {
                // Advance to startup mode
                break;
            }
            if (button->pressedFor(Config::Timing::BUTTON_SELECT_MS)) {
                bool oldMode = ops.cameraOperatorMode;
                changeCameraTalentMode(ops);
                if (ops.cameraOperatorMode != oldMode) {
                    configChanged = true;
                }
                displayTallyMode(ops);
            }
        }

        // Show startup mode
        displayStartupMode(ops);
        waitForButtonRelease();

        continueSequence = true;
        while (continueSequence) {
            button->read();
            
            if (button->wasReleased()) {
                // Advance to brightness
                break;
            }
            if (button->pressedFor(Config::Timing::BUTTON_SELECT_MS)) {
                bool oldAutoStart = ops.autoStartEnabled;
                changeStartupMode(ops);
                if (ops.autoStartEnabled != oldAutoStart) {
                    configChanged = true;
                }
                displayStartupMode(ops);
            }
        }

        // Show brightness
        displayBrightness(ops);
        waitForButtonRelease();

        continueSequence = true;
        while (continueSequence) {
            button->read();
            
            if (button->wasReleased()) {
                // Exit to WiFi connect
                log_i("Startup config completed, changes: %s", configChanged ? "YES" : "NO");
                return configChanged;
            }
            if (button->pressedFor(Config::Timing::BUTTON_SELECT_MS)) {
                uint8_t oldBrightness = ops.displayBrightnessLevel;
                changeBrightness(ops);
                if (ops.displayBrightnessLevel != oldBrightness) {
                    configChanged = true;
                }
                // After brightness change, show the new level and wait for click
                displayBrightness(ops);
            }
        }

        log_i("Startup config completed, changes: %s", configChanged ? "YES" : "NO");
        return configChanged;
    }

    template<uint8_t GLYPH_SIZE>
    void StartupConfig<GLYPH_SIZE>::displayTallyChannel(const StacOperations& ops) {
        using namespace Display;

        if constexpr (GLYPH_SIZE == 5) {
            using namespace Glyphs5x5::GlyphIndex;
        } else {
            using namespace Glyphs8x8::GlyphIndex;
        }

        // Get the glyph for the channel number
        // For V-160HD SDI channels (9-20), display the channel within bank (1-8)
        uint8_t displayChannel = ops.tallyChannel;
        if (ops.switchModel != "V-60HD" && ops.tallyChannel > 8) {
            displayChannel = ops.tallyChannel - 8;  // SDI 9 displays as 1, SDI 10 as 2, etc.
        }
        const uint8_t* channelGlyph = glyphManager->getDigitGlyph(displayChannel);

        // Color depends on switch model and bank
        color_t foreground, background;
        
        if (ops.switchModel == "V-60HD") {
            // V-60HD: Blue for all channels
            background = StandardColors::BLACK;
            foreground = StandardColors::BLUE;
        } else {
            // V-160HD: Check bank
            if (ops.channelBank == "hdmi_" || ops.tallyChannel <= 8) {
                // HDMI channels: Blue
                background = StandardColors::BLACK;
                foreground = StandardColors::BLUE;
            } else {
                // SDI channels: Light green
                background = StandardColors::BLACK;
                foreground = 0x1a800d; // RGB_COLOR_GREENLT from baseline
            }
        }

        display->drawGlyph(channelGlyph, foreground, background, true);
    }

    template<uint8_t GLYPH_SIZE>
    void StartupConfig<GLYPH_SIZE>::changeTallyChannel(StacOperations& ops) {
        using namespace Display;
        using namespace Config::Timing;

        if constexpr (GLYPH_SIZE == 5) {
            using namespace Glyphs5x5::GlyphIndex;
        } else {
            using namespace Glyphs8x8::GlyphIndex;
        }

        // Validate and clamp channel to valid range before displaying
        if (ops.switchModel == "V-60HD") {
            if (ops.tallyChannel < 1 || ops.tallyChannel > ops.maxChannelCount) {
                ops.tallyChannel = 1;
            }
        } else {
            // V-160HD validation
            if (ops.tallyChannel < 1 || ops.tallyChannel > (ops.maxSDIChannel + 8)) {
                ops.tallyChannel = 1;
                ops.channelBank = "hdmi_";
            }
        }

        uint8_t originalChannel = ops.tallyChannel;
        unsigned long timeout = millis() + OP_MODE_TIMEOUT_MS;

        // Show SELECT state with different colors
        // For V-160HD SDI channels (9-20), display the channel within bank (1-8)
        uint8_t displayChannel = ops.tallyChannel;
        if (ops.switchModel != "V-60HD" && ops.tallyChannel > 8) {
            displayChannel = ops.tallyChannel - 8;
        }
        const uint8_t* channelGlyph = glyphManager->getDigitGlyph(displayChannel);
        
        color_t selectForeground = StandardColors::ORANGE;
        color_t selectBackground;
        
        if (ops.switchModel == "V-60HD") {
            selectBackground = 0x00007f; // RGB_COLOR_BLUEDK
        } else {
            if (ops.channelBank == "hdmi_" || ops.tallyChannel <= 8) {
                selectBackground = 0x00007f; // RGB_COLOR_BLUEDK
            } else {
                selectBackground = 0x0d4007; // RGB_COLOR_GREENDK
            }
        }

        display->drawGlyph(channelGlyph, selectForeground, selectBackground, true);
        
        while (button->read());  // Wait for button release

        while (millis() < timeout) {
            button->read();
            
            // Click: Advance to next channel
            if (button->wasReleased()) {
                timeout = millis() + OP_MODE_TIMEOUT_MS; // Reset timeout
                
                if (ops.switchModel == "V-60HD") {
                    // V-60HD: Check if at max, wrap to 1, else increment
                    if (ops.tallyChannel >= ops.maxChannelCount) {
                        ops.tallyChannel = 1;
                    } else {
                        ops.tallyChannel++;
                    }
                } else {
                    // V-160HD: Handle HDMI/SDI banks - check before increment
                    if (ops.tallyChannel < 9 && ops.tallyChannel == ops.maxHDMIChannel) {
                        // At last HDMI, wrap to first SDI
                        ops.tallyChannel = 9;
                        ops.channelBank = "sdi_";
                    } else if (ops.tallyChannel > 8 && ops.tallyChannel == (ops.maxSDIChannel + 8)) {
                        // At last SDI, wrap to first HDMI
                        ops.tallyChannel = 1;
                        ops.channelBank = "hdmi_";
                    } else {
                        ops.tallyChannel++;
                    }
                }

                // Update display with new channel
                // For V-160HD SDI channels (9-20), display the channel within bank (1-8)
                displayChannel = ops.tallyChannel;
                if (ops.switchModel != "V-60HD" && ops.tallyChannel > 8) {
                    displayChannel = ops.tallyChannel - 8;
                }
                channelGlyph = glyphManager->getDigitGlyph(displayChannel);
                
                if (ops.switchModel == "V-60HD") {
                    selectBackground = 0x00007f;
                } else {
                    if (ops.channelBank == "hdmi_" || ops.tallyChannel <= 8) {
                        selectBackground = 0x00007f;
                    } else {
                        selectBackground = 0x0d4007;
                    }
                }
                
                display->drawGlyph(channelGlyph, selectForeground, selectBackground, true);
            }

            // Long press: Confirm and exit
            if (button->pressedFor(Config::Timing::BUTTON_SELECT_MS)) {
                showConfirmation();
                
                // Save if changed
                if (originalChannel != ops.tallyChannel) {
                    if (!configManager->saveOperations(ops)) {
                        log_e("Failed to save tally channel");
                    }
                }
                
                waitForButtonRelease();
                delay(GUI_PAUSE_SHORT_MS);
                return;
            }
        }

        // Timeout: Revert and restore display
        ops.tallyChannel = originalChannel;
        displayTallyChannel(ops);  // Restore normal display
    }

    template<uint8_t GLYPH_SIZE>
    void StartupConfig<GLYPH_SIZE>::displayTallyMode(const StacOperations& ops) {
        using namespace Display;

        const uint8_t* modeGlyph;
        if constexpr (GLYPH_SIZE == 5) {
            using namespace Glyphs5x5::GlyphIndex;
            modeGlyph = ops.cameraOperatorMode ? 
                glyphManager->getGlyph(GLF_C) : glyphManager->getGlyph(GLF_T);
        } else {
            using namespace Glyphs8x8::GlyphIndex;
            modeGlyph = ops.cameraOperatorMode ? 
                glyphManager->getGlyph(GLF_C) : glyphManager->getGlyph(GLF_T);
        }

        display->drawGlyph(modeGlyph, StandardColors::PURPLE, StandardColors::BLACK, true);
    }

    template<uint8_t GLYPH_SIZE>
    void StartupConfig<GLYPH_SIZE>::changeCameraTalentMode(StacOperations& ops) {
        using namespace Display;
        using namespace Config::Timing;

        // Get glyph indices based on display size
        uint8_t GLF_C_IDX, GLF_T_IDX;
        if constexpr (GLYPH_SIZE == 5) {
            GLF_C_IDX = Glyphs5x5::GlyphIndex::GLF_C;
            GLF_T_IDX = Glyphs5x5::GlyphIndex::GLF_T;
        } else {
            GLF_C_IDX = Glyphs8x8::GlyphIndex::GLF_C;
            GLF_T_IDX = Glyphs8x8::GlyphIndex::GLF_T;
        }

        bool originalMode = ops.cameraOperatorMode;
        bool currentMode = originalMode;
        unsigned long timeout = millis() + OP_MODE_TIMEOUT_MS;

        // Show SELECT state
        const uint8_t* modeGlyph = currentMode ? 
            glyphManager->getGlyph(GLF_C_IDX) : glyphManager->getGlyph(GLF_T_IDX);
        
        display->drawGlyph(modeGlyph, StandardColors::ORANGE, 0x380070, true); // RGB_COLOR_PRPLEDK
        while (button->read());  // Wait for button release

        while (millis() < timeout) {
            button->read();
            
            // Click: Toggle mode
            if (button->wasReleased()) {
                timeout = millis() + OP_MODE_TIMEOUT_MS;
                currentMode = !currentMode;
                
                modeGlyph = currentMode ? 
                    glyphManager->getGlyph(GLF_C_IDX) : glyphManager->getGlyph(GLF_T_IDX);
                display->drawGlyph(modeGlyph, StandardColors::ORANGE, 0x380070, true);
            }

            // Long press: Confirm and exit
            if (button->pressedFor(Config::Timing::BUTTON_SELECT_MS)) {
                showConfirmation();
                
                // Save if changed
                if (originalMode != currentMode) {
                    ops.cameraOperatorMode = currentMode;
                    if (!configManager->saveOperations(ops)) {
                        log_e("Failed to save tally mode");
                    }
                }
                
                waitForButtonRelease();
                delay(GUI_PAUSE_SHORT_MS);
                return;
            }
        }

        // Timeout: Revert to original and restore display
        ops.cameraOperatorMode = originalMode;
        displayTallyMode(ops);
    }

    template<uint8_t GLYPH_SIZE>
    void StartupConfig<GLYPH_SIZE>::displayStartupMode(const StacOperations& ops) {
        using namespace Display;

        // Get glyph indices based on display size
        uint8_t GLF_A_IDX, GLF_S_IDX;
        if constexpr (GLYPH_SIZE == 5) {
            GLF_A_IDX = Glyphs5x5::GlyphIndex::GLF_A;
            GLF_S_IDX = Glyphs5x5::GlyphIndex::GLF_S;
        } else {
            GLF_A_IDX = Glyphs8x8::GlyphIndex::GLF_A;
            GLF_S_IDX = Glyphs8x8::GlyphIndex::GLF_S;
        }

        const uint8_t* modeGlyph = ops.autoStartEnabled ? 
            glyphManager->getGlyph(GLF_A_IDX) : glyphManager->getGlyph(GLF_S_IDX);

        display->drawGlyph(modeGlyph, StandardColors::TEAL, StandardColors::BLACK, true);
    }

    template<uint8_t GLYPH_SIZE>
    void StartupConfig<GLYPH_SIZE>::changeStartupMode(StacOperations& ops) {
        using namespace Display;
        using namespace Config::Timing;

        // Get glyph indices based on display size
        uint8_t GLF_A_IDX, GLF_S_IDX;
        if constexpr (GLYPH_SIZE == 5) {
            GLF_A_IDX = Glyphs5x5::GlyphIndex::GLF_A;
            GLF_S_IDX = Glyphs5x5::GlyphIndex::GLF_S;
        } else {
            GLF_A_IDX = Glyphs8x8::GlyphIndex::GLF_A;
            GLF_S_IDX = Glyphs8x8::GlyphIndex::GLF_S;
        }

        bool originalMode = ops.autoStartEnabled;
        bool currentMode = originalMode;
        unsigned long timeout = millis() + OP_MODE_TIMEOUT_MS;

        // Show SELECT state
        const uint8_t* modeGlyph = currentMode ? 
            glyphManager->getGlyph(GLF_A_IDX) : glyphManager->getGlyph(GLF_S_IDX);
        
        display->drawGlyph(modeGlyph, StandardColors::ORANGE, 0x003a21, true); // RGB_COLOR_TEALDK
        while (button->read());  // Wait for button release

        while (millis() < timeout) {
            button->read();
            
            // Click: Toggle mode
            if (button->wasReleased()) {
                timeout = millis() + OP_MODE_TIMEOUT_MS;
                currentMode = !currentMode;
                
                modeGlyph = currentMode ? 
                    glyphManager->getGlyph(GLF_A_IDX) : glyphManager->getGlyph(GLF_S_IDX);
                display->drawGlyph(modeGlyph, StandardColors::ORANGE, 0x003a21, true);
            }

            // Long press: Confirm and exit
            if (button->pressedFor(Config::Timing::BUTTON_SELECT_MS)) {
                showConfirmation();
                
                // Save if changed
                if (originalMode != currentMode) {
                    ops.autoStartEnabled = currentMode;
                    if (!configManager->saveOperations(ops)) {
                        log_e("Failed to save startup mode");
                    }
                }
                
                waitForButtonRelease();
                delay(GUI_PAUSE_SHORT_MS);
                return;
            }
        }

        // Timeout: Revert to original and restore display
        ops.autoStartEnabled = originalMode;
        displayStartupMode(ops);
    }

    template<uint8_t GLYPH_SIZE>
    void StartupConfig<GLYPH_SIZE>::displayBrightness(const StacOperations& ops) {
        using namespace Display;

        // Get glyph indices based on display size
        uint8_t GLF_CBD_IDX, GLF_EN_IDX;
        if constexpr (GLYPH_SIZE == 5) {
            GLF_CBD_IDX = Glyphs5x5::GlyphIndex::GLF_CBD;
            GLF_EN_IDX = Glyphs5x5::GlyphIndex::GLF_EN;
        } else {
            GLF_CBD_IDX = Glyphs8x8::GlyphIndex::GLF_CBD;
            GLF_EN_IDX = Glyphs8x8::GlyphIndex::GLF_EN;
        }

        const uint8_t* checkboard = glyphManager->getGlyph(GLF_CBD_IDX);
        const uint8_t* centerBlank = glyphManager->getGlyph(GLF_EN_IDX);
        const uint8_t* brightnessGlyph = glyphManager->getGlyph(ops.displayBrightnessLevel);

        // Draw checkerboard background
        display->drawGlyph(checkboard, StandardColors::RED, StandardColors::GREEN, false);

        // Blank center columns
        display->drawGlyphOverlay(centerBlank, StandardColors::BLACK, false);

        // Overlay brightness number
        display->drawGlyphOverlay(brightnessGlyph, StandardColors::WHITE, true);
    }

    template<uint8_t GLYPH_SIZE>
    void StartupConfig<GLYPH_SIZE>::changeBrightness(StacOperations& ops) {
        using namespace Display;
        using namespace Config::Timing;

        // Get glyph index based on display size
        uint8_t GLF_EN_IDX;
        if constexpr (GLYPH_SIZE == 5) {
            GLF_EN_IDX = Glyphs5x5::GlyphIndex::GLF_EN;
        } else {
            GLF_EN_IDX = Glyphs8x8::GlyphIndex::GLF_EN;
        }

        const uint8_t* centerBlank = glyphManager->getGlyph(GLF_EN_IDX);

        // Get max brightness level based on display size
        uint8_t maxBrightnessLevel;
        if constexpr (GLYPH_SIZE == 5) {
            maxBrightnessLevel = Config::Display::BRIGHTNESS_LEVELS_5X5;
        } else {
            maxBrightnessLevel = Config::Display::BRIGHTNESS_LEVELS_8X8;
        }

        // Validate and clamp brightness to valid range (1 to max)
        if (ops.displayBrightnessLevel < 1 || ops.displayBrightnessLevel > maxBrightnessLevel) {
            ops.displayBrightnessLevel = 1;
        }

        uint8_t originalBrightness = ops.displayBrightnessLevel;
        uint8_t currentBrightness = originalBrightness;
        unsigned long timeout = millis() + OP_MODE_TIMEOUT_MS;

        // Apply current brightness and show SELECT state BEFORE waiting for button release
        {
            uint8_t absoluteBrightness;
            if constexpr (GLYPH_SIZE == 5) {
                absoluteBrightness = Config::Display::BRIGHTNESS_MAP_5X5[currentBrightness];
            } else {
                absoluteBrightness = Config::Display::BRIGHTNESS_MAP_8X8[currentBrightness];
            }
            display->setBrightness(absoluteBrightness, false);
        }

        // Show SELECT state - white background with orange number
        display->fill(StandardColors::WHITE, false);
        display->drawGlyphOverlay(centerBlank, StandardColors::BLACK, false);
        const uint8_t* brightnessGlyph = glyphManager->getDigitGlyph(currentBrightness);
        display->drawGlyphOverlay(brightnessGlyph, StandardColors::ORANGE, true);

        while (button->read());  // Wait for button release

        while (millis() < timeout) {
            button->read();
            
            // Click: Cycle brightness
            if (button->wasReleased()) {
                timeout = millis() + OP_MODE_TIMEOUT_MS;
                
                // Check if at max, wrap to 1, else increment
                if (currentBrightness >= maxBrightnessLevel) {
                    currentBrightness = 1;
                } else {
                    currentBrightness++;
                }

                // Update stored value
                ops.displayBrightnessLevel = currentBrightness;
                
                // Map level to absolute brightness using appropriate brightMap
                uint8_t absoluteBrightness;
                if constexpr (GLYPH_SIZE == 5) {
                    absoluteBrightness = Config::Display::BRIGHTNESS_MAP_5X5[currentBrightness];
                } else {
                    absoluteBrightness = Config::Display::BRIGHTNESS_MAP_8X8[currentBrightness];
                }
                
                // Update display with new brightness level number
                display->fill(StandardColors::WHITE, false);
                display->drawGlyphOverlay(centerBlank, StandardColors::BLACK, false);
                brightnessGlyph = glyphManager->getDigitGlyph(currentBrightness);
                display->drawGlyphOverlay(brightnessGlyph, StandardColors::ORANGE, false);
                
                // Apply new brightness value immediately for visual feedback
                display->setBrightness(absoluteBrightness, true);
            }

            // Long press: Confirm and exit
            if (button->pressedFor(Config::Timing::BUTTON_SELECT_MS)) {
                showConfirmation();
                
                // Save if changed
                if (originalBrightness != currentBrightness) {
                    if (!configManager->saveOperations(ops)) {
                        log_e("Failed to save brightness level");
                    }
                }
                
                waitForButtonRelease();
                delay(GUI_PAUSE_SHORT_MS);
                return;
            }
        }

        // Timeout: Revert to original brightness and restore display
        ops.displayBrightnessLevel = originalBrightness;
        
        // Map level to absolute brightness using appropriate brightMap
        uint8_t absoluteBrightness;
        if constexpr (GLYPH_SIZE == 5) {
            absoluteBrightness = Config::Display::BRIGHTNESS_MAP_5X5[originalBrightness];
        } else {
            absoluteBrightness = Config::Display::BRIGHTNESS_MAP_8X8[originalBrightness];
        }
        
        // Revert display brightness to original value
        display->setBrightness(absoluteBrightness, false);
        
        // Restore the brightness indicator display
        displayBrightness(ops);
        
        // Restore normal brightness display
        displayBrightness(ops);
    }

    template<uint8_t GLYPH_SIZE>
    void StartupConfig<GLYPH_SIZE>::showConfirmation() {
        using namespace Display;

        // Get glyph index based on display size
        uint8_t GLF_CK_IDX;
        if constexpr (GLYPH_SIZE == 5) {
            GLF_CK_IDX = Glyphs5x5::GlyphIndex::GLF_CK;
        } else {
            GLF_CK_IDX = Glyphs8x8::GlyphIndex::GLF_CK;
        }

        const uint8_t* checkmark = glyphManager->getGlyph(GLF_CK_IDX);

        display->drawGlyph(checkmark, StandardColors::GREEN, StandardColors::BLACK, true);
    }

    template<uint8_t GLYPH_SIZE>
    void StartupConfig<GLYPH_SIZE>::waitForButtonRelease() {
        while (button->read()) {
            yield();
        }
    }

    // ===== Overloads for Peripheral Mode (custom save callbacks) =====

    template<uint8_t GLYPH_SIZE>
    uint8_t StartupConfig<GLYPH_SIZE>::changeBrightness(uint8_t currentBrightness, std::function<void(uint8_t)> saveCallback) {
        using namespace Display;
        using namespace Config::Timing;

        // Get glyph index based on display size
        uint8_t GLF_EN_IDX;
        if constexpr (GLYPH_SIZE == 5) {
            GLF_EN_IDX = Glyphs5x5::GlyphIndex::GLF_EN;
        } else {
            GLF_EN_IDX = Glyphs8x8::GlyphIndex::GLF_EN;
        }

        const uint8_t* centerBlank = glyphManager->getGlyph(GLF_EN_IDX);

        // Get max brightness level based on display size
        uint8_t maxBrightnessLevel;
        if constexpr (GLYPH_SIZE == 5) {
            maxBrightnessLevel = Config::Display::BRIGHTNESS_LEVELS_5X5;
        } else {
            maxBrightnessLevel = Config::Display::BRIGHTNESS_LEVELS_8X8;
        }

        // Validate and clamp brightness to valid range (1 to max)
        if (currentBrightness < 1 || currentBrightness > maxBrightnessLevel) {
            currentBrightness = 1;
        }

        uint8_t originalBrightness = currentBrightness;
        unsigned long timeout = millis() + OP_MODE_TIMEOUT_MS;

        // Apply current brightness and show SELECT state BEFORE waiting for button release
        {
            uint8_t absoluteBrightness;
            if constexpr (GLYPH_SIZE == 5) {
                absoluteBrightness = Config::Display::BRIGHTNESS_MAP_5X5[currentBrightness];
            } else {
                absoluteBrightness = Config::Display::BRIGHTNESS_MAP_8X8[currentBrightness];
            }
            display->setBrightness(absoluteBrightness, false);
        }

        // Show SELECT state - white background with orange number
        display->fill(StandardColors::WHITE, false);
        display->drawGlyphOverlay(centerBlank, StandardColors::BLACK, false);
        const uint8_t* brightnessGlyph = glyphManager->getDigitGlyph(currentBrightness);
        display->drawGlyphOverlay(brightnessGlyph, StandardColors::ORANGE, true);

        while (button->read());  // Wait for button release

        while (millis() < timeout) {
            button->read();
            
            // Click: Cycle brightness
            if (button->wasReleased()) {
                timeout = millis() + OP_MODE_TIMEOUT_MS;
                
                // Check if at max, wrap to 1, else increment
                if (currentBrightness >= maxBrightnessLevel) {
                    currentBrightness = 1;
                } else {
                    currentBrightness++;
                }
                
                // Map level to absolute brightness using appropriate brightMap
                uint8_t absoluteBrightness;
                if constexpr (GLYPH_SIZE == 5) {
                    absoluteBrightness = Config::Display::BRIGHTNESS_MAP_5X5[currentBrightness];
                } else {
                    absoluteBrightness = Config::Display::BRIGHTNESS_MAP_8X8[currentBrightness];
                }
                
                // Update display with new brightness level number
                display->fill(StandardColors::WHITE, false);
                display->drawGlyphOverlay(centerBlank, StandardColors::BLACK, false);
                brightnessGlyph = glyphManager->getDigitGlyph(currentBrightness);
                display->drawGlyphOverlay(brightnessGlyph, StandardColors::ORANGE, false);
                
                // Apply new brightness value immediately for visual feedback
                display->setBrightness(absoluteBrightness, true);
            }

            // Long press: Confirm and exit
            if (button->pressedFor(Config::Timing::BUTTON_SELECT_MS)) {
                showConfirmation();
                
                // Save if changed using custom callback
                if (originalBrightness != currentBrightness && saveCallback) {
                    saveCallback(currentBrightness);
                }
                
                waitForButtonRelease();
                delay(GUI_PAUSE_SHORT_MS);
                return currentBrightness;
            }
        }

        // Timeout: Revert to original brightness
        uint8_t absoluteBrightness;
        if constexpr (GLYPH_SIZE == 5) {
            absoluteBrightness = Config::Display::BRIGHTNESS_MAP_5X5[originalBrightness];
        } else {
            absoluteBrightness = Config::Display::BRIGHTNESS_MAP_8X8[originalBrightness];
        }
        display->setBrightness(absoluteBrightness, false);
        return originalBrightness;
    }

    template<uint8_t GLYPH_SIZE>
    bool StartupConfig<GLYPH_SIZE>::changeCameraTalentMode(bool currentMode, std::function<void(bool)> saveCallback) {
        using namespace Display;
        using namespace Config::Timing;

        // Get glyph indices based on display size
        uint8_t GLF_C_IDX, GLF_T_IDX;
        if constexpr (GLYPH_SIZE == 5) {
            GLF_C_IDX = Glyphs5x5::GlyphIndex::GLF_C;
            GLF_T_IDX = Glyphs5x5::GlyphIndex::GLF_T;
        } else {
            GLF_C_IDX = Glyphs8x8::GlyphIndex::GLF_C;
            GLF_T_IDX = Glyphs8x8::GlyphIndex::GLF_T;
        }

        bool originalMode = currentMode;
        unsigned long timeout = millis() + OP_MODE_TIMEOUT_MS;

        // Show SELECT state - purple background
        const uint8_t* modeGlyph = currentMode ? 
            glyphManager->getGlyph(GLF_C_IDX) : glyphManager->getGlyph(GLF_T_IDX);
        
        display->drawGlyph(modeGlyph, StandardColors::PURPLE, StandardColors::BLACK, true);
        while (button->read());  // Wait for button release

        while (millis() < timeout) {
            button->read();
            
            // Click: Toggle mode
            if (button->wasReleased()) {
                timeout = millis() + OP_MODE_TIMEOUT_MS;
                currentMode = !currentMode;
                
                modeGlyph = currentMode ? 
                    glyphManager->getGlyph(GLF_C_IDX) : glyphManager->getGlyph(GLF_T_IDX);
                display->drawGlyph(modeGlyph, StandardColors::PURPLE, StandardColors::BLACK, true);
            }

            // Long press: Confirm and exit
            if (button->pressedFor(Config::Timing::BUTTON_SELECT_MS)) {
                showConfirmation();
                
                // Save if changed using custom callback
                if (originalMode != currentMode && saveCallback) {
                    saveCallback(currentMode);
                }
                
                waitForButtonRelease();
                delay(GUI_PAUSE_SHORT_MS);
                return currentMode;
            }
        }

        // Timeout: Revert to original mode
        return originalMode;
    }

} // namespace Application

//  --- EOF --- //
