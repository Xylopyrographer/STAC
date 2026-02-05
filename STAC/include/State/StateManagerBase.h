#ifndef STAC_STATE_MANAGER_BASE_H
#define STAC_STATE_MANAGER_BASE_H

#include <cstdint>
#include <functional>
#include <Arduino.h>


namespace State {

    /**
     * @brief Base class for state managers
     *
     * Provides common state management functionality including:
     * - Current/previous state tracking
     * - State change detection and callbacks
     * - Time tracking since last change
     *
     * @tparam StateType Enum type for states (TallyState, OperatingMode, etc.)
     */
    template<typename StateType>
    class StateManagerBase {
      public:
        /**
         * @brief Callback function type for state changes
         * @param oldState Previous state
         * @param newState New state
         */
        using StateChangeCallback = std::function<void( StateType oldState, StateType newState )>;

        /**
         * @brief Construct a new State Manager Base
         * @param initialState Initial state value
         */
        explicit StateManagerBase( StateType initialState )
            : currentState( initialState )
            , previousState( initialState )
            , lastChangeTime( 0 )
            , callback( nullptr ) {
        }

        virtual ~StateManagerBase() = default;

        /**
         * @brief Get current state
         * @return Current state value
         */
        StateType getCurrentState() const {
            return currentState;
        }

        /**
         * @brief Set new state
         * @param newState State to transition to
         * @param stateToString Function to convert state to string for logging
         * @return true if state changed
         */
        bool setState( StateType newState, const char* ( *stateToString )( StateType ) ) {
            if ( newState == currentState ) {
                return false;  // No change
            }

            // Store previous state
            previousState = currentState;
            currentState = newState;
            lastChangeTime = millis();

            // Log state change
            log_i( "State: %s -> %s",
                   stateToString( previousState ),
                   stateToString( currentState ) );

            // Notify callback if registered
            if ( callback ) {
                callback( previousState, currentState );
            }

            return true;
        }

        /**
         * @brief Get previous state
         * @return Previous state value
         */
        StateType getPreviousState() const {
            return previousState;
        }

        /**
         * @brief Get time since last state change
         * @return Milliseconds since last change
         */
        unsigned long getTimeSinceChange() const {
            return millis() - lastChangeTime;
        }

        /**
         * @brief Set callback for state changes
         * @param cb Function to call on state change
         */
        void setStateChangeCallback( StateChangeCallback cb ) {
            callback = cb;
        }

      protected:
        StateType currentState;         ///< Current state
        StateType previousState;        ///< Previous state
        unsigned long lastChangeTime;   ///< Time of last state change
        StateChangeCallback callback;   ///< State change callback function
    };

} // namespace State


#endif // STAC_STATE_MANAGER_BASE_H


//  --- EOF --- //
