#ifndef STAC_TALLY_STATE_MANAGER_H
#define STAC_TALLY_STATE_MANAGER_H

#include <cstdint>
#include <functional>
#include "Config/Types.h"
#include "Hardware/Display/Colors.h"
#include "StateManagerBase.h"


    namespace State {

        /**
         * @brief Manages tally state and transitions
         *
         * Tracks current tally state, handles state changes,
         * and notifies observers when state changes occur.
         */
        class TallyStateManager : public StateManagerBase<TallyState> {
          public:
            /**
             * @brief Callback function type for state changes
             * @param oldState Previous tally state
             * @param newState New tally state
             */
            using StateChangeCallback = std::function<void( TallyState oldState, TallyState newState )>;

            TallyStateManager();
            ~TallyStateManager() = default;

            /**
             * @brief Get current tally state
             * @return Current TallyState
             */
            TallyState getCurrentState() const {
                return currentState;
            }

            /**
             * @brief Set new tally state
             * @param newState State to transition to
             * @return true if state changed
             */
            bool setState( TallyState newState );

            /**
             * @brief Get previous tally state
             * @return Previous TallyState
             */
            TallyState getPreviousState() const {
                return previousState;
            }

            /**
             * @brief Get time since last state change
             * @return Milliseconds since last change
             */
            unsigned long getTimeSinceChange() const;

            /**
             * @brief Check if in error state
             * @return true if current state is ERROR
             */
            bool isError() const {
                return currentState == TallyState::ERROR;
            }

            /**
             * @brief Check if on-air (PROGRAM)
             * @return true if current state is PROGRAM
             */
            bool isOnAir() const {
                return currentState == TallyState::PROGRAM;
            }

            /**
             * @brief Check if in preview
             * @return true if current state is PREVIEW
             */
            bool isPreview() const {
                return currentState == TallyState::PREVIEW;
            }

            /**
             * @brief Get color for current tally state
             * @return Display color for current state
             */
            Display::color_t getStateColor() const;

            /**
             * @brief Get string representation of current state
             * @return State name as string
             */
            const char *getStateString() const;
 
            /**
            * @brief Convert TallyState to string
            * @param state State to convert
            * @return String representation
            */
            static const char *stateToString( TallyState state );

            /**
             * @brief Get display color for a state
             * @param state State to get color for
             * @return Display color
             */
            static Display::color_t stateToColor( TallyState state );

            /**
             * @brief Reset to default state (NO_TALLY)
             */
            void reset();
        };

    } // namespace State


#endif // STAC_TALLY_STATE_MANAGER_H


//  --- EOF --- //
