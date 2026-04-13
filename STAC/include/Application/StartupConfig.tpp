// StartupConfig.tpp - Template implementation
// This file is included at the end of StartupConfig.h

#include "Config/Constants.h"
#include "Hardware/Display/Colors.h"
// Glyph definitions included via Device_Config.h -> board config
#include <Arduino.h>

namespace Application {

    template<uint8_t GLYPH_SIZE> StartupConfig<GLYPH_SIZE>::StartupConfig(
        Button* btn,
        Display::IDisplay* disp,
        Display::GlyphManager<GLYPH_SIZE> *glyphs,
        Storage::ConfigManager* cfg,
        Button* btnB
    )
        : button( btn )
        , buttonB( btnB )
        , display( disp )
        , glyphManager( glyphs )
        , configManager( cfg )
    {}

    template<uint8_t GLYPH_SIZE>
    bool StartupConfig<GLYPH_SIZE>::runStartupSequence( StacOperations& ops, bool autoStartBypass ) {
        if ( autoStartBypass ) {
            log_i( "Autostart bypass active - skipping startup config" );
            return false;  // No changes made
        }

        // Track if any values were changed
        bool configChanged = false;
        StacOperations originalOps = ops;

        // Show tally channel
        displayTallyChannel( ops );
        waitForButtonRelease();

        bool continueSequence = true;
        while ( continueSequence ) {
            button->read();
            checkButtonBReset();

            if ( button->wasReleased() ) {
                // Advance to tally mode
                break;
            }
            if ( button->pressedFor( Config::Timing::BUTTON_SELECT_MS ) ) {
                uint8_t oldChannel = ops.tallyChannel;
                changeTallyChannel( ops );
                if ( ops.tallyChannel != oldChannel ) {
                    configChanged = true;
                }
                displayTallyChannel( ops );
            }
        }

        // Show tally mode
        displayTallyMode( ops );
        waitForButtonRelease();

        continueSequence = true;
        while ( continueSequence ) {
            button->read();
            checkButtonBReset();

            if ( button->wasReleased() ) {
                // Advance to startup mode
                break;
            }
            if ( button->pressedFor( Config::Timing::BUTTON_SELECT_MS ) ) {
                bool oldMode = ops.cameraOperatorMode;
                changeCameraTalentMode( ops );
                if ( ops.cameraOperatorMode != oldMode ) {
                    configChanged = true;
                }
                displayTallyMode( ops );
            }
        }

        // Show startup mode
        displayStartupMode( ops );
        waitForButtonRelease();

        continueSequence = true;
        while ( continueSequence ) {
            button->read();
            checkButtonBReset();

            if ( button->wasReleased() ) {
                // Advance to brightness
                break;
            }
            if ( button->pressedFor( Config::Timing::BUTTON_SELECT_MS ) ) {
                bool oldAutoStart = ops.autoStartEnabled;
                changeStartupMode( ops );
                if ( ops.autoStartEnabled != oldAutoStart ) {
                    configChanged = true;
                }
                displayStartupMode( ops );
            }
        }

        // Show brightness
        displayBrightness( ops );
        waitForButtonRelease();

        continueSequence = true;
        while ( continueSequence ) {
            button->read();
            checkButtonBReset();

            if ( button->wasReleased() ) {
                // Exit to WiFi connect
                log_i( "Startup config completed, changes: %s", configChanged ? "YES" : "NO" );
                return configChanged;
            }
            if ( button->pressedFor( Config::Timing::BUTTON_SELECT_MS ) ) {
                uint8_t oldBrightness = ops.displayBrightnessLevel;
                changeBrightness( ops );
                if ( ops.displayBrightnessLevel != oldBrightness ) {
                    configChanged = true;
                }
                // After brightness change, show the new level and wait for click
                displayBrightness( ops );
            }
        }

        log_i( "Startup config completed, changes: %s", configChanged ? "YES" : "NO" );
        return configChanged;
    }

    template<uint8_t GLYPH_SIZE>
    void StartupConfig<GLYPH_SIZE>::displayTallyChannel( const StacOperations& ops ) {
        using namespace Display;

        // Get the glyph for the channel number
        // For V-160HD SDI channels (9-16), display the channel within bank (1-8)
        // For V-80HD SDI channels (5-8), display the channel within bank (1-4)
        // For ATEM channels 10-40, only the ones digit fits the single-character display
        uint8_t displayChannel = ops.tallyChannel;
        if ( ops.isV160HD() && ops.tallyChannel > 8 ) {
            displayChannel = ops.tallyChannel - 8;  // SDI 9 displays as 1, SDI 10 as 2, etc.
        }
        else if ( ops.isV80HD() && ops.tallyChannel > 4 ) {
            displayChannel = ops.tallyChannel - 4;  // SDI 5 displays as 1, SDI 6 as 2, etc.
        }
        else if ( ops.isATEM() && displayChannel > 9 ) {
            displayChannel = displayChannel % 10;   // ATEM 10+ shows ones digit only
        }
        const uint8_t *channelGlyph = glyphManager->getDigitGlyph( displayChannel );

        // Color depends on switch model and bank
        color_t foreground, background;

        if ( ops.isV60HD() ) {
            // V-60HD: Blue for all channels
            background = StandardColors::BLACK;
            foreground = StandardColors::BLUE;
        }
        else if ( ops.isATEM() ) {
            // ATEM: Teal for all channels
            background = StandardColors::BLACK;
            foreground = StandardColors::TEAL;
        }
        else if ( ops.isV80HD() ) {
            // V-80HD: Check bank (HDMI 1-4, SDI 5-8)
            if ( ops.channelBank == "hdmi_" || ops.tallyChannel <= 4 ) {
                // HDMI channels: Blue
                background = StandardColors::BLACK;
                foreground = StandardColors::BLUE;
            }
            else {
                // SDI channels: Light green
                background = StandardColors::BLACK;
                foreground = 0x1a800d; // RGB_COLOR_GREENLT from baseline
            }
        }
        else {
            // V-160HD: Check bank (HDMI 1-8, SDI 9-16)
            if ( ops.channelBank == "hdmi_" || ops.tallyChannel <= 8 ) {
                // HDMI channels: Blue
                background = StandardColors::BLACK;
                foreground = StandardColors::BLUE;
            }
            else {
                // SDI channels: Light green
                background = StandardColors::BLACK;
                foreground = 0x1a800d; // RGB_COLOR_GREENLT from baseline
            }
        }

        // TFT displays show the full channel number (1-40 for ATEM, etc.)
        if constexpr ( GLYPH_SIZE == 1 ) {
            if ( ops.isATEM() ) {
                display->drawChannelNumber( ops.tallyChannel, foreground, background );
                return;
            }
        }

        display->drawGlyph( channelGlyph, foreground, background, true );

        // LED matrix: overlay bank corner pixels for ATEM channels 10-40
        if ( ops.isATEM() && ops.tallyChannel > 9 ) {
            uint8_t bankGlyphIndex;
            if ( ops.tallyChannel < 20 ) {
                bankGlyphIndex = GLF_BANK1_CORNERS;
            }
            else if ( ops.tallyChannel < 30 ) {
                bankGlyphIndex = GLF_BANK2_CORNERS;
            }
            else if ( ops.tallyChannel < 40 ) {
                bankGlyphIndex = GLF_BANK3_CORNERS;
            }
            else {
                bankGlyphIndex = GLF_BANK4_CORNERS; // channel 40 = all four inner-corner pixels
            }
            const uint8_t *bankGlyph = glyphManager->getGlyph( bankGlyphIndex );
            display->drawGlyphOverlay( bankGlyph, StandardColors::PURPLE, true );
        }
    }

    template<uint8_t GLYPH_SIZE>
    void StartupConfig<GLYPH_SIZE>::changeTallyChannel( StacOperations& ops ) {
        using namespace Display;
        using namespace Config::Timing;

        // Validate and clamp channel to valid range before displaying
        if ( ops.isV60HD() ) {
            if ( ops.tallyChannel < 1 || ops.tallyChannel > ops.maxChannelCount ) {
                ops.tallyChannel = 1;
            }
        }
        else if ( ops.isATEM() ) {
            // ATEM supports inputs 1-40
            if ( ops.tallyChannel < 1 || ops.tallyChannel > 40 ) {
                ops.tallyChannel = 1;
            }
        }
        else if ( ops.isV80HD() ) {
            // V-80HD validation (channels 1-4 HDMI, 5-8 SDI)
            if ( ops.tallyChannel < 1 || ops.tallyChannel > ( ops.maxSDIChannel + 4 ) ) {
                ops.tallyChannel = 1;
                ops.channelBank = "hdmi_";
            }
        }
        else {
            // V-160HD validation (channels 1-8 HDMI, 9-16 SDI)
            if ( ops.tallyChannel < 1 || ops.tallyChannel > ( ops.maxSDIChannel + 8 ) ) {
                ops.tallyChannel = 1;
                ops.channelBank = "hdmi_";
            }
        }

        // ATEM: Use two-tier bank select state machine (ones cycling and bank cycling)
        if ( ops.isATEM() ) {
            uint8_t originalChannel = ops.tallyChannel;
            unsigned long timeout = millis() + OP_MODE_TIMEOUT_MS;
            bool bankMode = false;      // false=ones cycling (+1/press), true=bank cycling (×10/press)
            bool longPressHandled = false;

            // Draw channel in ONES select mode: orange digit on dark-teal bg, teal bank corners
            auto drawOnesSelect = [&]() {
                if constexpr ( GLYPH_SIZE == 1 ) {
                    display->drawChannelNumber( ops.tallyChannel, StandardColors::ORANGE, static_cast<color_t>( 0x003333 ) );
                }
                else {
                    uint8_t ones = ops.tallyChannel % 10;
                    display->drawGlyph( glyphManager->getDigitGlyph( ones ), StandardColors::ORANGE, static_cast<color_t>( 0x003333 ), true );
                    if ( ops.tallyChannel > 9 ) {
                        uint8_t bgIdx;
                        if ( ops.tallyChannel < 20 ) { bgIdx = GLF_BANK1_CORNERS; }
                        else if ( ops.tallyChannel < 30 ) { bgIdx = GLF_BANK2_CORNERS; }
                        else if ( ops.tallyChannel < 40 ) { bgIdx = GLF_BANK3_CORNERS; }
                        else { bgIdx = GLF_BANK4_CORNERS; }
                        display->drawGlyphOverlay( glyphManager->getGlyph( bgIdx ), StandardColors::PURPLE, true );
                    }
                }
            };

            // Draw channel in BANK select mode: tens digit on black bg, orange bank corners
            auto drawBankSelect = [&]() {
                if constexpr ( GLYPH_SIZE == 1 ) {
                    uint8_t bankBase = ( ops.tallyChannel < 10 ) ? 1 : ( ops.tallyChannel / 10 ) * 10;
                    display->drawChannelNumber( bankBase, StandardColors::ORANGE, StandardColors::BLACK );
                }
                else {
                    uint8_t tens = ( ops.tallyChannel < 10 ) ? 0 : ops.tallyChannel / 10;
                    display->drawGlyph( glyphManager->getDigitGlyph( tens ), StandardColors::ORANGE, StandardColors::BLACK, true );
                    if ( ops.tallyChannel >= 10 ) {
                        uint8_t bgIdx;
                        if ( ops.tallyChannel < 20 ) { bgIdx = GLF_BANK1_CORNERS; }
                        else if ( ops.tallyChannel < 30 ) { bgIdx = GLF_BANK2_CORNERS; }
                        else if ( ops.tallyChannel < 40 ) { bgIdx = GLF_BANK3_CORNERS; }
                        else { bgIdx = GLF_BANK4_CORNERS; }
                        display->drawGlyphOverlay( glyphManager->getGlyph( bgIdx ), StandardColors::RED, true );
                    }
                }
            };

            drawOnesSelect();
            while ( button->read() ); // Wait for button release

            while ( millis() < timeout ) {
                button->read();
                checkButtonBReset();

                if ( button->wasReleased() ) {
                    timeout = millis() + OP_MODE_TIMEOUT_MS;

                    if ( !longPressHandled ) {
                        if ( !bankMode ) {
                            // ONES mode: cycle +1 (1→2→...→40→1)
                            ops.tallyChannel = ( ops.tallyChannel >= 40 ) ? 1 : ops.tallyChannel + 1;
                            drawOnesSelect();
                        }
                        else {
                            // BANK mode: cycle to next bank start (1→10→20→30→40→1)
                            if ( ops.tallyChannel < 10 )      { ops.tallyChannel = 10; }
                            else if ( ops.tallyChannel < 20 ) { ops.tallyChannel = 20; }
                            else if ( ops.tallyChannel < 30 ) { ops.tallyChannel = 30; }
                            else if ( ops.tallyChannel < 40 ) { ops.tallyChannel = 40; }
                            else                              { ops.tallyChannel = 1; }
                            drawBankSelect();
                        }
                    }
                    longPressHandled = false;
                }

                // 4s hold: confirm+exit (checked before 2s to prevent double-trigger)
                if ( button->pressedFor( BUTTON_ATEM_CONFIRM_MS ) ) {
                    showConfirmation();
                    if ( originalChannel != ops.tallyChannel ) {
                        if ( !configManager->saveAtemConfig( ops ) ) {
                            log_e( "Failed to save ATEM tally channel" );
                        }
                    }
                    waitForButtonRelease();
                    delay( GUI_PAUSE_SHORT_MS );
                    return;
                }
                // 2s hold: toggle ONES ↔ BANK mode
                else if ( button->pressedFor( BUTTON_SELECT_MS ) && !longPressHandled ) {
                    longPressHandled = true;
                    bankMode = !bankMode;
                    if ( bankMode ) {
                        drawBankSelect();
                    }
                    else {
                        drawOnesSelect();
                    }
                }
            }

            // Timeout: revert and restore
            ops.tallyChannel = originalChannel;
            displayTallyChannel( ops );
            return;
        }

        uint8_t originalChannel = ops.tallyChannel;
        unsigned long timeout = millis() + OP_MODE_TIMEOUT_MS;

        // Show SELECT state with different colors
        // For V-160HD SDI channels (9-16), display the channel within bank (1-8)
        // For V-80HD SDI channels (5-8), display the channel within bank (1-4)
        // For ATEM channels 10-40, only the ones digit fits the single-character display
        uint8_t displayChannel = ops.tallyChannel;
        if ( ops.isV160HD() && ops.tallyChannel > 8 ) {
            displayChannel = ops.tallyChannel - 8;
        }
        else if ( ops.isV80HD() && ops.tallyChannel > 4 ) {
            displayChannel = ops.tallyChannel - 4;
        }
        else if ( ops.isATEM() && displayChannel > 9 ) {
            displayChannel = displayChannel % 10;   // ATEM 10+ shows ones digit only
        }
        const uint8_t *channelGlyph = glyphManager->getDigitGlyph( displayChannel );

        color_t selectForeground = StandardColors::ORANGE;
        color_t selectBackground;

        if ( ops.isV60HD() ) {
            selectBackground = 0x00007f; // RGB_COLOR_BLUEDK
        }
        else if ( ops.isATEM() ) {
            selectBackground = 0x003333; // Dark teal
        }
        else if ( ops.isV80HD() ) {
            if ( ops.channelBank == "hdmi_" || ops.tallyChannel <= 4 ) {
                selectBackground = 0x00007f; // RGB_COLOR_BLUEDK (HDMI)
            }
            else {
                selectBackground = 0x0d4007; // RGB_COLOR_GREENDK (SDI)
            }
        }
        else {
            // V-160HD
            if ( ops.channelBank == "hdmi_" || ops.tallyChannel <= 8 ) {
                selectBackground = 0x00007f; // RGB_COLOR_BLUEDK (HDMI)
            }
            else {
                selectBackground = 0x0d4007; // RGB_COLOR_GREENDK (SDI)
            }
        }

        display->drawGlyph( channelGlyph, selectForeground, selectBackground, true );

        while ( button->read() ); // Wait for button release

        while ( millis() < timeout ) {
            button->read();
            checkButtonBReset();

            // Click: Advance to next channel
            if ( button->wasReleased() ) {
                timeout = millis() + OP_MODE_TIMEOUT_MS; // Reset timeout

                if ( ops.isV60HD() ) {
                    // V-60HD: Check if at max, wrap to 1, else increment
                    if ( ops.tallyChannel >= ops.maxChannelCount ) {
                        ops.tallyChannel = 1;
                    }
                    else {
                        ops.tallyChannel++;
                    }
                }
                else if ( ops.isATEM() ) {
                    // ATEM: Cycle 1–40
                    if ( ops.tallyChannel >= 40 ) {
                        ops.tallyChannel = 1;
                    }
                    else {
                        ops.tallyChannel++;
                    }
                }
                else if ( ops.isV80HD() ) {
                    // V-80HD: Handle HDMI/SDI banks (channels 1-4 HDMI, 5-8 SDI)
                    if ( ops.tallyChannel < 5 && ops.tallyChannel == ops.maxHDMIChannel ) {
                        // At last HDMI, wrap to first SDI (channel 5)
                        ops.tallyChannel = 5;
                        ops.channelBank = "sdi_";
                    }
                    else if ( ops.tallyChannel >= 5 && ops.tallyChannel == ( ops.maxSDIChannel + 4 ) ) {
                        // At last SDI, wrap to first HDMI
                        ops.tallyChannel = 1;
                        ops.channelBank = "hdmi_";
                    }
                    else {
                        ops.tallyChannel++;
                    }
                }
                else {
                    // V-160HD: Handle HDMI/SDI banks - check before increment
                    if ( ops.tallyChannel < 9 && ops.tallyChannel == ops.maxHDMIChannel ) {
                        // At last HDMI, wrap to first SDI
                        ops.tallyChannel = 9;
                        ops.channelBank = "sdi_";
                    }
                    else if ( ops.tallyChannel > 8 && ops.tallyChannel == ( ops.maxSDIChannel + 8 ) ) {
                        // At last SDI, wrap to first HDMI
                        ops.tallyChannel = 1;
                        ops.channelBank = "hdmi_";
                    }
                    else {
                        ops.tallyChannel++;
                    }
                }

                // Update display with new channel
                // For V-160HD SDI channels (9-16), display the channel within bank (1-8)
                // For V-80HD SDI channels (5-8), display the channel within bank (1-4)
                // For ATEM channels 10-40, only the ones digit fits the single-character display
                displayChannel = ops.tallyChannel;
                if ( ops.isV160HD() && ops.tallyChannel > 8 ) {
                    displayChannel = ops.tallyChannel - 8;
                }
                else if ( ops.isV80HD() && ops.tallyChannel > 4 ) {
                    displayChannel = ops.tallyChannel - 4;
                }
                else if ( ops.isATEM() && displayChannel > 9 ) {
                    displayChannel = displayChannel % 10;   // ATEM 10+ shows ones digit only
                }
                channelGlyph = glyphManager->getDigitGlyph( displayChannel );

                if ( ops.isV60HD() ) {
                    selectBackground = 0x00007f; // RGB_COLOR_BLUEDK
                }
                else if ( ops.isATEM() ) {
                    selectBackground = 0x003333; // Dark teal
                }
                else if ( ops.isV80HD() ) {
                    if ( ops.channelBank == "hdmi_" || ops.tallyChannel <= 4 ) {
                        selectBackground = 0x00007f; // RGB_COLOR_BLUEDK (HDMI)
                    }
                    else {
                        selectBackground = 0x0d4007; // RGB_COLOR_GREENDK (SDI)
                    }
                }
                else {
                    // V-160HD
                    if ( ops.channelBank == "hdmi_" || ops.tallyChannel <= 8 ) {
                        selectBackground = 0x00007f; // RGB_COLOR_BLUEDK (HDMI)
                    }
                    else {
                        selectBackground = 0x0d4007; // RGB_COLOR_GREENDK (SDI)
                    }
                }

                display->drawGlyph( channelGlyph, selectForeground, selectBackground, true );
            }

            // Long press: Confirm and exit
            if ( button->pressedFor( Config::Timing::BUTTON_SELECT_MS ) ) {
                showConfirmation();

                // Save if changed
                if ( originalChannel != ops.tallyChannel ) {
                    bool saved = false;
                    if ( ops.isV60HD() ) {
                        saved = configManager->saveV60HDConfig( ops );
                    }
                    else if ( ops.isV160HD() ) {
                        saved = configManager->saveV160HDConfig( ops );
                    }
                    else if ( ops.isV80HD() ) {
                        saved = configManager->saveV80HDConfig( ops );
                    }
                    else if ( ops.isATEM() ) {
                        saved = configManager->saveAtemConfig( ops );
                    }
                    if ( !saved ) {
                        log_e( "Failed to save tally channel" );
                    }
                }

                waitForButtonRelease();
                delay( GUI_PAUSE_SHORT_MS );
                return;
            }
        }

        // Timeout: Revert and restore display
        ops.tallyChannel = originalChannel;
        displayTallyChannel( ops ); // Restore normal display
    }

    template<uint8_t GLYPH_SIZE>
    void StartupConfig<GLYPH_SIZE>::displayTallyMode( const StacOperations& ops ) {
        using namespace Display;

        const uint8_t *modeGlyph = ops.cameraOperatorMode ?
                                   glyphManager->getGlyph( GLF_C ) : glyphManager->getGlyph( GLF_T );

        display->drawGlyph( modeGlyph, StandardColors::PURPLE, StandardColors::BLACK, true );
    }

    template<uint8_t GLYPH_SIZE>
    void StartupConfig<GLYPH_SIZE>::changeCameraTalentMode( StacOperations& ops ) {
        using namespace Display;
        using namespace Config::Timing;

        // Get glyph indices based on display size
        uint8_t GLF_C_IDX = Display::GLF_C;
        uint8_t GLF_T_IDX = Display::GLF_T;

        bool originalMode = ops.cameraOperatorMode;
        bool currentMode = originalMode;
        unsigned long timeout = millis() + OP_MODE_TIMEOUT_MS;

        // Show SELECT state
        const uint8_t *modeGlyph = currentMode ?
                                   glyphManager->getGlyph( GLF_C_IDX ) : glyphManager->getGlyph( GLF_T_IDX );

        display->drawGlyph( modeGlyph, StandardColors::ORANGE, 0x380070, true ); // RGB_COLOR_PRPLEDK
        while ( button->read() ); // Wait for button release

        while ( millis() < timeout ) {
            button->read();
            checkButtonBReset();

            // Click: Toggle mode
            if ( button->wasReleased() ) {
                timeout = millis() + OP_MODE_TIMEOUT_MS;
                currentMode = !currentMode;

                modeGlyph = currentMode ?
                            glyphManager->getGlyph( GLF_C_IDX ) : glyphManager->getGlyph( GLF_T_IDX );
                display->drawGlyph( modeGlyph, StandardColors::ORANGE, 0x380070, true );
            }

            // Long press: Confirm and exit
            if ( button->pressedFor( Config::Timing::BUTTON_SELECT_MS ) ) {
                showConfirmation();

                // Save if changed
                if ( originalMode != currentMode ) {
                    ops.cameraOperatorMode = currentMode;
                    bool saved = false;
                    if ( ops.isV60HD() ) {
                        saved = configManager->saveV60HDConfig( ops );
                    }
                    else if ( ops.isV160HD() ) {
                        saved = configManager->saveV160HDConfig( ops );
                    }
                    else if ( ops.isV80HD() ) {
                        saved = configManager->saveV80HDConfig( ops );
                    }
                    else if ( ops.isATEM() ) {
                        saved = configManager->saveAtemConfig( ops );
                    }
                    if ( !saved ) {
                        log_e( "Failed to save tally mode" );
                    }
                }

                waitForButtonRelease();
                delay( GUI_PAUSE_SHORT_MS );
                return;
            }
        }

        // Timeout: Revert to original and restore display
        ops.cameraOperatorMode = originalMode;
        displayTallyMode( ops );
    }

    template<uint8_t GLYPH_SIZE>
    void StartupConfig<GLYPH_SIZE>::displayStartupMode( const StacOperations& ops ) {
        using namespace Display;

        // Get glyph indices based on display size
        uint8_t GLF_A_IDX = Display::GLF_A;
        uint8_t GLF_S_IDX = Display::GLF_S;

        const uint8_t *modeGlyph = ops.autoStartEnabled ?
                                   glyphManager->getGlyph( GLF_A_IDX ) : glyphManager->getGlyph( GLF_S_IDX );

        display->drawGlyph( modeGlyph, StandardColors::TEAL, StandardColors::BLACK, true );
    }

    template<uint8_t GLYPH_SIZE>
    void StartupConfig<GLYPH_SIZE>::changeStartupMode( StacOperations& ops ) {
        using namespace Display;
        using namespace Config::Timing;

        // Get glyph indices based on display size
        uint8_t GLF_A_IDX = Display::GLF_A;
        uint8_t GLF_S_IDX = Display::GLF_S;

        bool originalMode = ops.autoStartEnabled;
        bool currentMode = originalMode;
        unsigned long timeout = millis() + OP_MODE_TIMEOUT_MS;

        // Show SELECT state
        const uint8_t *modeGlyph = currentMode ?
                                   glyphManager->getGlyph( GLF_A_IDX ) : glyphManager->getGlyph( GLF_S_IDX );

        display->drawGlyph( modeGlyph, StandardColors::ORANGE, 0x003a21, true ); // RGB_COLOR_TEALDK
        while ( button->read() ); // Wait for button release

        while ( millis() < timeout ) {
            button->read();
            checkButtonBReset();

            // Click: Toggle mode
            if ( button->wasReleased() ) {
                timeout = millis() + OP_MODE_TIMEOUT_MS;
                currentMode = !currentMode;

                modeGlyph = currentMode ?
                            glyphManager->getGlyph( GLF_A_IDX ) : glyphManager->getGlyph( GLF_S_IDX );
                display->drawGlyph( modeGlyph, StandardColors::ORANGE, 0x003a21, true );
            }

            // Long press: Confirm and exit
            if ( button->pressedFor( Config::Timing::BUTTON_SELECT_MS ) ) {
                showConfirmation();

                // Save if changed
                if ( originalMode != currentMode ) {
                    ops.autoStartEnabled = currentMode;
                    bool saved = false;
                    if ( ops.isV60HD() ) {
                        saved = configManager->saveV60HDConfig( ops );
                    }
                    else if ( ops.isV160HD() ) {
                        saved = configManager->saveV160HDConfig( ops );
                    }
                    else if ( ops.isV80HD() ) {
                        saved = configManager->saveV80HDConfig( ops );
                    }
                    else if ( ops.isATEM() ) {
                        saved = configManager->saveAtemConfig( ops );
                    }
                    if ( !saved ) {
                        log_e( "Failed to save startup mode" );
                    }
                }

                waitForButtonRelease();
                delay( GUI_PAUSE_SHORT_MS );
                return;
            }
        }

        // Timeout: Revert to original and restore display
        ops.autoStartEnabled = originalMode;
        displayStartupMode( ops );
    }

    template<uint8_t GLYPH_SIZE>
    void StartupConfig<GLYPH_SIZE>::displayBrightness( const StacOperations& ops ) {
        using namespace Display;

        // Get glyph indices based on display size
        uint8_t GLF_CBD_IDX = Display::GLF_CBD;
        uint8_t GLF_EN_IDX = Display::GLF_EN;

        const uint8_t *checkerboard = glyphManager->getGlyph( GLF_CBD_IDX );
        const uint8_t *centerBlank = glyphManager->getGlyph( GLF_EN_IDX );
        const uint8_t *brightnessGlyph = glyphManager->getGlyph( ops.displayBrightnessLevel );

        // Draw checkerboard background
        display->drawGlyph( checkerboard, StandardColors::RED, StandardColors::GREEN, false );

        // Blank center columns
        display->drawGlyphOverlay( centerBlank, StandardColors::BLACK, false );

        // Overlay brightness number
        display->drawGlyphOverlay( brightnessGlyph, StandardColors::WHITE, true );
    }

    template<uint8_t GLYPH_SIZE>
    void StartupConfig<GLYPH_SIZE>::changeBrightness( StacOperations& ops ) {
        using namespace Display;
        using namespace Config::Timing;

        // Get glyph index based on display size
        uint8_t GLF_EN_IDX = Display::GLF_EN;

        const uint8_t *centerBlank = glyphManager->getGlyph( GLF_EN_IDX );

        // Get max brightness level
        uint8_t maxBrightnessLevel = Config::Display::BRIGHTNESS_LEVELS;

        // Validate and clamp brightness to valid range (1 to max)
        if ( ops.displayBrightnessLevel < 1 || ops.displayBrightnessLevel > maxBrightnessLevel ) {
            ops.displayBrightnessLevel = 1;
        }

        uint8_t originalBrightness = ops.displayBrightnessLevel;
        uint8_t currentBrightness = originalBrightness;
        unsigned long timeout = millis() + OP_MODE_TIMEOUT_MS;

        // Apply current brightness and show SELECT state
        {
            uint8_t absoluteBrightness = Config::Display::BRIGHTNESS_MAP[ currentBrightness ];
            display->setBrightness( absoluteBrightness, false );
        }

        // Show SELECT state - white background with orange number
        display->fill( StandardColors::WHITE, false );
        display->drawGlyphOverlay( centerBlank, StandardColors::BLACK, false );
        const uint8_t *brightnessGlyph = glyphManager->getDigitGlyph( currentBrightness );
        display->drawGlyphOverlay( brightnessGlyph, StandardColors::ORANGE, true );

        while ( button->read() ); // Wait for button release

        while ( millis() < timeout ) {
            button->read();
            checkButtonBReset();

            // Click: Cycle brightness
            if ( button->wasReleased() ) {
                timeout = millis() + OP_MODE_TIMEOUT_MS;

                // Check if at max, wrap to 1, else increment
                if ( currentBrightness >= maxBrightnessLevel ) {
                    currentBrightness = 1;
                }
                else {
                    currentBrightness++;
                }

                // Update stored value
                ops.displayBrightnessLevel = currentBrightness;

                // Map level to absolute brightness
                uint8_t absoluteBrightness = Config::Display::BRIGHTNESS_MAP[ currentBrightness ];

                // Update display with new brightness level number
                display->fill( StandardColors::WHITE, false );
                display->drawGlyphOverlay( centerBlank, StandardColors::BLACK, false );
                brightnessGlyph = glyphManager->getDigitGlyph( currentBrightness );
                display->drawGlyphOverlay( brightnessGlyph, StandardColors::ORANGE, false );

                // Apply new brightness value immediately for visual feedback
                display->setBrightness( absoluteBrightness, true );
            }

            // Long press: Confirm and exit
            if ( button->pressedFor( Config::Timing::BUTTON_SELECT_MS ) ) {
                showConfirmation();

                // Save if changed
                if ( originalBrightness != currentBrightness ) {
                    bool saved = false;
                    if ( ops.isV60HD() ) {
                        saved = configManager->saveV60HDConfig( ops );
                    }
                    else if ( ops.isV160HD() ) {
                        saved = configManager->saveV160HDConfig( ops );
                    }
                    else if ( ops.isV80HD() ) {
                        saved = configManager->saveV80HDConfig( ops );
                    }
                    else if ( ops.isATEM() ) {
                        saved = configManager->saveAtemConfig( ops );
                    }
                    if ( !saved ) {
                        log_e( "Failed to save brightness level" );
                    }
                }

                waitForButtonRelease();
                delay( GUI_PAUSE_SHORT_MS );
                return;
            }
        }

        // Timeout: Revert to original brightness and restore display
        ops.displayBrightnessLevel = originalBrightness;

        // Map level to absolute brightness
        uint8_t absoluteBrightness = Config::Display::BRIGHTNESS_MAP[ originalBrightness ];

        // Revert display brightness to original value
        display->setBrightness( absoluteBrightness, false );

        // Restore the brightness indicator display
        displayBrightness( ops );
    }

    template<uint8_t GLYPH_SIZE>
    void StartupConfig<GLYPH_SIZE>::showConfirmation() {
        using namespace Display;

        // Get glyph index based on display size
        uint8_t GLF_CK_IDX = Display::GLF_CK;

        const uint8_t *checkmark = glyphManager->getGlyph( GLF_CK_IDX );

        display->drawGlyph( checkmark, StandardColors::GREEN, StandardColors::BLACK, true );
    }

    template<uint8_t GLYPH_SIZE>
    void StartupConfig<GLYPH_SIZE>::waitForButtonRelease() {
        while ( button->read() ) {
            yield();
        }
    }

    // ===== Overloads for Peripheral Mode (custom save callbacks) =====

    template<uint8_t GLYPH_SIZE>
    uint8_t StartupConfig<GLYPH_SIZE>::changeBrightness( uint8_t currentBrightness, std::function<void( uint8_t )> saveCallback ) {
        using namespace Display;
        using namespace Config::Timing;

        // Get glyph index based on display size
        uint8_t GLF_EN_IDX = Display::GLF_EN;

        const uint8_t *centerBlank = glyphManager->getGlyph( GLF_EN_IDX );

        // Get max brightness level
        uint8_t maxBrightnessLevel = Config::Display::BRIGHTNESS_LEVELS;

        // Validate and clamp brightness to valid range (1 to max)
        if ( currentBrightness < 1 || currentBrightness > maxBrightnessLevel ) {
            currentBrightness = 1;
        }

        uint8_t originalBrightness = currentBrightness;
        unsigned long timeout = millis() + OP_MODE_TIMEOUT_MS;

        // Apply current brightness and show SELECT state BEFORE waiting for button release
        {
            uint8_t absoluteBrightness = Config::Display::BRIGHTNESS_MAP[ currentBrightness ];
            display->setBrightness( absoluteBrightness, false );
        }

        // Show SELECT state - white background with orange number
        display->fill( StandardColors::WHITE, false );
        display->drawGlyphOverlay( centerBlank, StandardColors::BLACK, false );
        const uint8_t *brightnessGlyph = glyphManager->getDigitGlyph( currentBrightness );
        display->drawGlyphOverlay( brightnessGlyph, StandardColors::ORANGE, true );

        while ( button->read() ); // Wait for button release

        while ( millis() < timeout ) {
            button->read();
            checkButtonBReset();

            // Click: Cycle brightness
            if ( button->wasReleased() ) {
                timeout = millis() + OP_MODE_TIMEOUT_MS;

                // Check if at max, wrap to 1, else increment
                if ( currentBrightness >= maxBrightnessLevel ) {
                    currentBrightness = 1;
                }
                else {
                    currentBrightness++;
                }

                // Map level to absolute brightness using appropriate brightMap
                uint8_t absoluteBrightness = Config::Display::BRIGHTNESS_MAP[ currentBrightness ];

                // Update display with new brightness level number
                display->fill( StandardColors::WHITE, false );
                display->drawGlyphOverlay( centerBlank, StandardColors::BLACK, false );
                brightnessGlyph = glyphManager->getDigitGlyph( currentBrightness );
                display->drawGlyphOverlay( brightnessGlyph, StandardColors::ORANGE, false );

                // Apply new brightness value immediately for visual feedback
                display->setBrightness( absoluteBrightness, true );
            }

            // Long press: Confirm and exit
            if ( button->pressedFor( Config::Timing::BUTTON_SELECT_MS ) ) {
                showConfirmation();

                // Save if changed using custom callback
                if ( originalBrightness != currentBrightness && saveCallback ) {
                    saveCallback( currentBrightness );
                }

                waitForButtonRelease();
                delay( GUI_PAUSE_SHORT_MS );
                return currentBrightness;
            }
        }

        // Timeout: Revert to original brightness
        uint8_t absoluteBrightness = Config::Display::BRIGHTNESS_MAP[ originalBrightness ];
        display->setBrightness( absoluteBrightness, false );
        return originalBrightness;
    }

    template<uint8_t GLYPH_SIZE>
    bool StartupConfig<GLYPH_SIZE>::changeCameraTalentMode( bool currentMode, std::function<void( bool )> saveCallback ) {
        using namespace Display;
        using namespace Config::Timing;

        // Get glyph indices based on display size
        uint8_t GLF_C_IDX = Display::GLF_C;
        uint8_t GLF_T_IDX = Display::GLF_T;

        bool originalMode = currentMode;
        unsigned long timeout = millis() + OP_MODE_TIMEOUT_MS;

        // Show SELECT state - purple background
        const uint8_t *modeGlyph = currentMode ?
                                   glyphManager->getGlyph( GLF_C_IDX ) : glyphManager->getGlyph( GLF_T_IDX );

        display->drawGlyph( modeGlyph, StandardColors::PURPLE, StandardColors::BLACK, true );
        while ( button->read() ); // Wait for button release

        while ( millis() < timeout ) {
            button->read();
            checkButtonBReset();

            // Click: Toggle mode
            if ( button->wasReleased() ) {
                timeout = millis() + OP_MODE_TIMEOUT_MS;
                currentMode = !currentMode;

                modeGlyph = currentMode ?
                            glyphManager->getGlyph( GLF_C_IDX ) : glyphManager->getGlyph( GLF_T_IDX );
                display->drawGlyph( modeGlyph, StandardColors::PURPLE, StandardColors::BLACK, true );
            }

            // Long press: Confirm and exit
            if ( button->pressedFor( Config::Timing::BUTTON_SELECT_MS ) ) {
                showConfirmation();

                // Save if changed using custom callback
                if ( originalMode != currentMode && saveCallback ) {
                    saveCallback( currentMode );
                }

                waitForButtonRelease();
                delay( GUI_PAUSE_SHORT_MS );
                return currentMode;
            }
        }

        // Timeout: Revert to original mode
        return originalMode;
    }

    template<uint8_t GLYPH_SIZE>
    void StartupConfig<GLYPH_SIZE>::checkButtonBReset() {
        if ( buttonB ) {
            buttonB->read();
            if ( buttonB->wasReleased() ) {
                log_i( "Button B pressed during startup config - restarting" );
                ESP.restart();
            }
        }
    }

} // namespace Application

//  --- EOF --- //
