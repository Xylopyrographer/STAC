#ifndef STAC_ROLAND_CLIENT_FACTORY_H
#define STAC_ROLAND_CLIENT_FACTORY_H

#include <memory>
#include "IRolandClient.h"
#include "V60HDClient.h"
#include "V160HDClient.h"


    namespace Net {

        /**
         * @brief Switch model types
         */
        enum class SwitchModel {
            V60HD,      ///< Roland V-60HD
            V160HD,     ///< Roland V-160HD
            UNKNOWN     ///< Unknown or uninitialized
        };

        /**
         * @brief Factory for creating Roland client instances
         *
         * Creates the appropriate Roland client implementation based on
         * the switch model type. Supports runtime selection.
         */
        class RolandClientFactory {
          public:
            /**
             * @brief Create Roland client for specified switch model
             * @param model Switch model type
             * @return Unique pointer to IRolandClient implementation
             */
            static std::unique_ptr<IRolandClient> create( SwitchModel model ) {
                switch ( model ) {
                    case SwitchModel::V60HD:
                        return std::make_unique<V60HDClient>();

                    case SwitchModel::V160HD:
                        return std::make_unique<V160HDClient>();

                    case SwitchModel::UNKNOWN:
                    default:
                        return nullptr;
                }
            }

            /**
             * @brief Create Roland client from string identifier
             * @param modelString Switch model string ("V-60HD" or "V-160HD")
             * @return Unique pointer to IRolandClient implementation
             */
            static std::unique_ptr<IRolandClient> createFromString( const String& modelString ) {
                SwitchModel model = stringToSwitchModel( modelString );
                return create( model );
            }

            /**
             * @brief Convert string to SwitchModel enum
             * @param modelString Switch model string
             * @return SwitchModel enum value
             */
            static SwitchModel stringToSwitchModel( const String& modelString ) {
                if ( modelString == "V-60HD" ) {
                    return SwitchModel::V60HD;
                }
                else if ( modelString == "V-160HD" ) {
                    return SwitchModel::V160HD;
                }
                else {
                    return SwitchModel::UNKNOWN;
                }
            }

            /**
             * @brief Convert SwitchModel enum to string
             * @param model SwitchModel enum value
             * @return String representation
             */
            static String switchModelToString( SwitchModel model ) {
                switch ( model ) {
                    case SwitchModel::V60HD:
                        return "V-60HD";
                    case SwitchModel::V160HD:
                        return "V-160HD";
                    case SwitchModel::UNKNOWN:
                    default:
                        return "Unknown";
                }
            }

          private:
            RolandClientFactory() = delete;  // Static class, no instantiation
        };

    } // namespace Net


#endif // STAC_ROLAND_CLIENT_FACTORY_H


//  --- EOF --- //
