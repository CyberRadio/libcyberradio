/***************************************************************************
 * \file NbddcComponent.cpp
 * \brief Defines the NBDDC interface for the NDR551.
 * \author DA
 * \author NH
 * \author MN
 * \copyright (c) 2017 CyberRadio Solutions, Inc.  All rights reserved.
 *
 ***************************************************************************/

#include "LibCyberRadio/Driver/NDR551/NbddcComponent.h"
#include "LibCyberRadio/Driver/RadioHandler.h"
#include <boost/format.hpp>
#include <boost/lexical_cast.hpp>
#include <json/json.h>

namespace LibCyberRadio
{

    namespace Driver
    {

        namespace NDR551
        {

            NbddcComponent::NbddcComponent(int index,
                    ::LibCyberRadio::Driver::RadioHandler* parent,
                     bool debug,
                     int dataPort,
                     int rateIndex,
                     int udpDestination,
                     int vitaEnable,
                     int streamId,
                     double frequency,
                     int source) :
                m551Parent( parent ),
                ::LibCyberRadio::Driver::NbddcComponent(
                        /* const std::string& name */ (boost::format("NDR551-NBDDC%02d") % \
                                index).str(),
                        /* int index */ index,
                        /* ::LibCyberRadio::Driver::RadioHandler* parent */ parent,
                        /* bool debug */ debug,
                        /* bool nbddcCommandSetsFreq */ true,
                        /* bool nbddcCommandSetsSource */ true,
                        /* bool selectableDataPort */ false,
                        /* double freqRangeMin */ -64.0e6,
                        /* double freqRangeMax */ 64.0e6,
                        /* double freqRes */ 1.0,
                        /* double freqUnits */ 1.0,
                        /* int source */ source,
                        /* int dataPort */ dataPort,
                        /* double frequency */ frequency,
                        /* int rateIndex */ rateIndex,
                        /* int udpDestination */ udpDestination,
                        /* int vitaEnable */ vitaEnable,
                        /* unsigned int streamId */ streamId)
            {
                initConfigurationDict();
                _rateSet[15] = 4.0e6; 
                _rateSet[14] = 3.2e6; 
                _rateSet[13] = 1.28e6; 
                _rateSet[12] = 400e3;
                _rateSet[11] = 256e3; 
                _rateSet[10] = 200e3; 
                _rateSet[9]  = 128e3; 
                _rateSet[8]  = 64e3;
                _rateSet[7]  = 32e3; 
                _rateSet[6]  = 16e3; 
                _rateSet[5]  = 8.0e3; 
                _rateSet[4]  = 4.0e3; 
                _rateSet[3]  = 2.0e3; 
                _rateSet[2]  = 1.0e3; 
                _rateSet[1]  = 0.5e3; 
                _rateSet[0]  = 0.25e3; 
            }

            NbddcComponent::~NbddcComponent()
            {
            }

            NbddcComponent::NbddcComponent(const NbddcComponent& other) :
                ::LibCyberRadio::Driver::NbddcComponent(other)
            {
            }

            NbddcComponent& NbddcComponent::operator=(const NbddcComponent& other)
            {
                ::LibCyberRadio::Driver::NbddcComponent::operator=(other);
                if ( this != &other )
                {
                }
                return *this;
            }

            void NbddcComponent::initConfigurationDict()
            {
                //this->debug("[NbddcComponent::initConfigurationDict] Called\n");
                _config.clear();
                // Call the base-class version
                RadioComponent::initConfigurationDict();
                // Define tuner-specific keys
                _config["rateIndex"] = _rateIndex;
                _config["udpDestination"] = _udpDestination;
                _config["vitaEnable"] = _vitaEnable;
                _config["streamId"] = _streamId;
                _config["frequency"] = _frequency;
                _config["source"] = _source;
                _config["mode"] = _mode;
                _config["audio"  ] = false;
                _config["bfo"    ] = 0;
                _config["cic0"   ] = 0;
                _config["cic1"   ] = 0;
                _config["dal"    ] = 0;
                _config["dao"    ] = 0;
                _config["dat"    ] = 0;
                _config["datc"   ] = 0;
                _config["ddl"    ] = 0;
                _config["ddo"    ] = 0;
                _config["ddt"    ] = 0;
                _config["ddtc"   ] = 0;
                _config["demod"  ] = "none";
                _config["dest"   ] = 0;
                _config["dgv"    ] = 0;
                _config["dll"    ] = 0;
                _config["dmdgain"] = 0;
                _config["dtl"    ] = 0;
                _config["dul"    ] = 0;
                _config["ovs"    ] = 0;
                //this->debug("[NbddcComponent::initConfigurationDict] Returning\n");
            }

            void NbddcComponent::updateConfigurationDict()
            {
                this->debug("[NbddcComponent::updateConfigurationDict] Called\n");
                RadioComponent::updateConfigurationDict();
                if ( _config.hasKey("rateIndex") )
                    setConfigurationValueToInt("rateIndex", _rateIndex);
                if ( _config.hasKey("udpDestination") )
                    setConfigurationValueToInt("udpDestination", _udpDestination);
                if ( _config.hasKey("vitaEnable") )
                    setConfigurationValueToInt("vitaEnable", _vitaEnable);
                if ( _config.hasKey("streamId") )
                    setConfigurationValueToUInt("streamId", _streamId);
                if ( _config.hasKey("frequency") )
                    setConfigurationValueToDbl("frequency", _frequency);
                if ( _config.hasKey("source") )
                    setConfigurationValueToInt("source", _source);
                if ( _config.hasKey("mode") ){
                    setConfigurationValue("mode", _mode);
                }
                if ( _selectableDataPort && _config.hasKey("dataPort") )
                {
                    setConfigurationValueToInt("dataPort", _dataPort);
                }
                this->debug("[NbddcComponent::updateConfigurationDict] Returning\n");
            }

            void NbddcComponent::queryConfiguration()
            {
                this->debug("[NbddcComponent::queryConfiguration] Called\n");
                Json::Value command;
                Json::Value params;
                command["cmd"] = "qnbddc";
                command["msg"] = _parent->getMessageId();
                command["params"] = Json::objectValue;
                command["params"]["id"] = _index;
                Json::FastWriter fastWriter;
                std::string output = fastWriter.write(command);
                BasicStringList rsp = _parent->sendCommand(output);
                Json::Reader reader;
                Json::Value returnVal; 
                std::string t = rsp.at(0);
                bool parsingSuccessful = reader.parse( t.c_str(), returnVal["result"] );     //parse process
                _enabled = boost::lexical_cast<bool>(returnVal["enable"].asBool());
                _frequency = boost::lexical_cast<double>(returnVal["freq"].asDouble());
                _source = boost::lexical_cast<int>(returnVal["source"].asInt());
                _udpDestination = boost::lexical_cast<int>(returnVal["dest"].asInt());
                _rateIndex = boost::lexical_cast<int>(returnVal["filter"].asInt());
                _streamId = boost::lexical_cast<int>(returnVal["vita"].asUInt());
                _mode = boost::lexical_cast<std::string>(returnVal["mode"].asString());
            }

            // Default implementation uses the NDR308 syntax.
            // WBDDC? <index>
            bool NbddcComponent::executeNbddcQuery(int index,
                        int& rateIndex,
                        int& udpDestination,
                        bool& enabled,
                        int& vitaEnable,
                        unsigned int& streamId,
                        double& frequency,
                        int& source)
            {
                bool ret = false;
                if ( (_parent != NULL) && (_parent->isConnected()) )
                {
                    Json::Value root(Json::objectValue);
                    root["msg"] = _parent->getMessageId();
                    root["cmd"] = "qnbddc";
                    Json::Value params(Json::objectValue);
                    params["id"] = index;
                    root["params"] = params;
                    Json::FastWriter fastWriter;
                    std::string output = fastWriter.write(root);
                    LibCyberRadio::BasicStringList recv = _parent->sendCommand(output,1.0);
                    Json::Reader reader;
                    Json::Value returnVal; 
                    std::string t = recv.at(0);
                    bool parsingSuccessful = reader.parse( t.c_str(), returnVal );     //parse process
                    Json::Value result = returnVal["result"];
                    rateIndex = boost::lexical_cast<int>(result["filter"].asInt());
                    udpDestination = boost::lexical_cast<int>(result["dest"].asInt());
                    enabled = boost::lexical_cast<bool>(result["enable"].asBool());
                    vitaEnable = boost::lexical_cast<int>(result["enable"].asBool());
                    streamId = boost::lexical_cast<unsigned int>(result["vita"].asUInt());
                }
                return ret;
            }

            // Default implementation returns false, since it is based on
            // the NDR308, which does not support selectable-source WBDDCs.
            bool NbddcComponent::executeSourceCommand(int index, int& source)
            {
                bool ret = false;
                if ( (_parent != NULL) && (_parent->isConnected()) )
                {
                    Json::Value root(Json::objectValue);
                    root["msg"] = m551Parent->getMessageId();
                    root["cmd"] = "nbddc";
                    Json::Value params(Json::objectValue);
                    params["id"] = index;
                    params["rfch"] = std::to_string(source);
                    root["params"] = params;
                    Json::FastWriter fastWriter;
                    std::string output = fastWriter.write(root);
                    LibCyberRadio::BasicStringList recv = _parent->sendCommand(output,1.0);
                    Json::Reader reader;
                    Json::Value returnVal; 
                    std::string t = recv.at(0);
                    bool parsingSuccessful = reader.parse( t.c_str(), returnVal );     //parse process
                    ret = returnVal["success"].asBool();
                }
                return ret;
            }

            // Default implementation returns false, since it is based on
            // the NDR308, which does not support tunable WBDDCs.
            bool NbddcComponent::executeFreqCommand(int index, double& freq)
            {
                bool ret = false;
                if ( (_parent != NULL) && (_parent->isConnected()) )
                {
                    Json::Value root(Json::objectValue);
                    root["msg"] = m551Parent->getMessageId();
                    root["cmd"] = "nbddc";
                    Json::Value params(Json::objectValue);
                    params["id"] = index;
                    params["offset"] = freq;
                    root["params"] = params;
                    Json::FastWriter fastWriter;
                    std::string output = fastWriter.write(root);
                    LibCyberRadio::BasicStringList recv = _parent->sendCommand(output,1.0);
                    Json::Reader reader;
                    Json::Value returnVal; 
                    std::string t = recv.at(0);
                    bool parsingSuccessful = reader.parse( t.c_str(), returnVal );     //parse process
                    ret = returnVal["success"].asBool();
                }
                return ret;
            }

            // Default implementation uses the NDR308 syntax.
            // WBDDC <index>, <rate index>, <udp dest>, <enable>, <vita enable>, <stream id>
            bool NbddcComponent::executeNbddcCommand(int index,
                                                     int& rateIndex,
                                                     int& udpDestination,
                                                     bool& enabled,
                                                     int& vitaEnable,
                                                     unsigned int& streamId,
                                                     double& frequency,
                                                     int& source)
            {
                bool ret = false;
                if ( (_parent != NULL) && (_parent->isConnected()) )
                {
                    Json::Value root(Json::objectValue);
                    root["msg"] = _parent->getMessageId();
                    root["cmd"] = "nbddc";
                    Json::Value params(Json::objectValue);
                    params["id"] = index;
                    params["filter"] = rateIndex;
                    params["dest"] = udpDestination;
                    params["enable"] = boost::lexical_cast<bool>(vitaEnable);
                    params["vita"] = streamId;
                    params["rfch"] = std::to_string(source);
                    root["params"] = params;
                    Json::FastWriter fastWriter;
                    std::string output = fastWriter.write(root);
                    LibCyberRadio::BasicStringList recv = _parent->sendCommand(output,1.0);
                    Json::Reader reader;
                    Json::Value returnVal; 
                    std::string t = recv.at(0);
                    bool parsingSuccessful = reader.parse( t.c_str(), returnVal );     //parse process
                    ret = returnVal["success"].asBool();
                }
                return ret;
            }

        } /* namespace NDR551 */

    } // namespace Driver

} // namespace LibCyberRadio

