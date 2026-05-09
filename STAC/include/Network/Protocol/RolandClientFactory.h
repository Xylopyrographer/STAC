#ifndef STAC_ROLAND_CLIENT_FACTORY_H
#define STAC_ROLAND_CLIENT_FACTORY_H

#include <memory>
#include "IRolandClient.h"
#include "V60HDClient.h"
#include "V160HDClient.h"
#include "ATEMClient.h"


namespace Net {

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
                case SwitchModel::V80HD:  // V-80HD uses same protocol as V-160HD
                    return std::make_unique<V160HDClient>();

                case SwitchModel::ATEM:
                    return std::make_unique<ATEMClient>();

                case SwitchModel::UNKNOWN:
                default:
                    return nullptr;
            }
        }

        /**
         * @brief Create Roland client from string identifier
         * @param modelString Switch model string ("V-60HD", "V-160HD", or "V-80HD")
         * @return Unique pointer to IRolandClient implementation
         */
        static std::unique_ptr<IRolandClient> createFromString( const String &modelString ) {
            return create( switchModelFromString( modelString ) );
        }

      private:
        RolandClientFactory() = delete;  // Static class, no instantiation
    };

} // namespace Net


#endif // STAC_ROLAND_CLIENT_FACTORY_H


//  --- EOF --- //
