/***************************************************************************
 * \file DataPort.h
 * \brief Defines the 10GigE data port interface for an NDR551.
 * \author DA
 * \author NH
 * \author MN
 * \copyright (c) 2017 CyberRadio Solutions, Inc.  All rights reserved.
 *
 ***************************************************************************/

#ifndef INCLUDED_LIBCYBERRADIO_DRIVER_NDR551_DATAPORT_H
#define INCLUDED_LIBCYBERRADIO_DRIVER_NDR551_DATAPORT_H

#include "LibCyberRadio/Driver/DataPort.h"
#include <json/json.h>
#include <boost/lexical_cast.hpp>
#include <string>

/**
 * \brief Provides programming elements for controlling CyberRadio Solutions products.
 */
namespace LibCyberRadio
{
    /**
     * \brief Provides programming elements for driving CRS NDR-class radios.
     */
    namespace Driver
    {
        // Forward declaration for RadioHandler
        class RadioHandler;

        /**
         * \brief Provides programming elements for driving NDR551 radios.
         */
        namespace NDR551
        {
            /**
             * \brief 10GigE data port class.
             *
             * A radio handler object maintains one DataPort object for each
             * 10GigE data port on the radio.
             *
             */
            class DataPort : public ::LibCyberRadio::Driver::DataPort
            {
                public:
                    /**
                     * \brief Constructs a DataPort object.
                     * \param index The index number of this object.
                     * \param parent A pointer to the RadioHandler object that "owns" this
                     *    object.
                     * \param debug Whether the object supports debug output.
                     * \param sourceIP Source IP address.
                     */
                    DataPort(int index = 0,
                            ::LibCyberRadio::Driver::RadioHandler* parent = NULL,
                             bool debug = false,
                             const std::string& sourceIP = "0.0.0.0");
                    /**
                     * \brief Destroys a DataPort object.
                     */
                    virtual ~DataPort();
                    /**
                     * \brief Copies a DataPort object.
                     * \param other The DataPort object to copy.
                     */
                    DataPort(const DataPort& other);
                    /**
                     * \brief Assignment operator for DataPort objects.
                     * \param other The DataPort object to copy.
                     * \returns A reference to the assigned object.
                     */
                    virtual DataPort& operator=(const DataPort& other);
                    /**
                     * \brief Initializes the configuration dictionary, defining the allowed
                     *    keys.
                     */
                    void initConfigurationDict();
                    /**
                     * \brief Updates the configuration dictionary from object settings.
                     */
                    void queryConfiguration();

                protected:
                    /**
                     * \brief Executes the source IP query command.
                     * \note The return value from this method only indicates if the command
                     *    succeeded or failed. This method uses reference parameters to return
                     *    the results of the query.
                     * \param index Data port index.
                     * \param ipAddr Source IP address (return).
                     * \returns True if the command succeeded, false otherwise.
                     */
                    bool executeSourceIPQuery(int index, std::string& ipAddr);
                    /**
                     * \brief Executes the source IP set command.
                     * \param index Data port index.
                     * \param ipAddr Source IP address (return).
                     * \returns True if the command succeeded, false otherwise.
                     */
                    bool executeSourceIPCommand(int index, std::string& ipAddr);
                    /**
                     * \brief Executes the destination IP query command.
                     * \note The return value from this method only indicates if the command
                     *    succeeded or failed. This method uses reference parameters to return
                     *    the results of the query.
                     * \param index Data port index.
                     * \param dipIndex DIP table entry index.
                     * \param ipAddr Destination IP address (return).
                     * \param macAddr Destination MAC address (return).
                     * \param sourcePort Source UDP port (return).
                     * \param destPort Destination UDP port (return).
                     * \returns True if the command succeeded, false otherwise.
                     */
                    bool executeDestIPQuery(int index,
                            int dipIndex,
                            std::string& ipAddr,
                            std::string& macAddr,
                            unsigned int& sourcePort,
                            unsigned int& destPort);
                    /**
                     * \brief Executes the destination IP set command.
                     * \param index Data port index.
                     * \param dipIndex DIP table entry index.
                     * \param ipAddr Destination IP address.
                     * \param macAddr Destination MAC address.
                     * \param sourcePort Source UDP port.
                     * \param destPort Destination UDP port.
                     * \returns True if the command succeeded, false otherwise.
                     */
                    bool executeDestIPCommand(int index,
                            int dipIndex,
                            std::string& ipAddr,
                            std::string& macAddr,
                            unsigned int& sourcePort,
                            unsigned int& destPort);

                    std::string _sourceMacAddr;
                    uint16_t _sourcePort;

            }; /* class DataPort */

        } /* namespace NDR551 */


    } /* namespace Driver */

} /* namespace LibCyberRadio */

#endif /* INCLUDED_LIBCYBERRADIO_DRIVER_NDR551_DATAPORT_H */
