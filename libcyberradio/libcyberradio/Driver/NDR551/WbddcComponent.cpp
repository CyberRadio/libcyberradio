/***************************************************************************
 * \file WbddcComponent.cpp
 * \brief Defines the WBDDC interface for the NDR551.
 * \author DA
 * \author NH
 * \author MN
 * \copyright (c) 2017 CyberRadio Solutions, Inc.  All rights reserved.
 *
 ***************************************************************************/

#include "LibCyberRadio/Driver/NDR551/WbddcComponent.h"
#include "LibCyberRadio/Driver/RadioHandler.h"
#include <boost/format.hpp>
#include <jsoncpp/json/json.h>
#include <boost/lexical_cast.hpp>


namespace LibCyberRadio
{
    namespace Driver
    {

        namespace NDR551
        {

            WbddcComponent::WbddcComponent(int index,
                    ::LibCyberRadio::Driver::RadioHandler* parent,
                     bool debug,
                     int dataPort,
                     int rateIndex,
                     int udpDestination,
                     int vitaEnable,
                     int streamId) :
                m551Parent( parent ),
                ::LibCyberRadio::Driver::WbddcComponent(
                        /* const std::string& name */ (boost::format("NDR551-WBDDC%02d") % \
                                index).str(),
                        /* int index */ index,
                        /* ::LibCyberRadio::Driver::RadioHandler* parent */ parent,
                        /* bool debug */ debug,
                        /* bool tunable */ true,
                        /* bool selectableSource */ true,
                        /* bool selectableDataPort */ true,
                        /* bool agc */ true,
                        /* double freqRangeMin */ 0.0,
                        /* double freqRangeMax */ 0.0,
                        /* double freqRes */ 1e6,
                        /* double freqUnits */ 1e6,
                        /* int source */ index,
                        /* int dataPort */ dataPort,
                        /* double frequency */ 0.0,
                        /* int rateIndex */ rateIndex,
                        /* int udpDestination */ udpDestination,
                        /* int vitaEnable */ vitaEnable,
                        /* unsigned int streamId */ streamId)
            {
                this->debug("[NDR551::WbddcComponent] index - %d\n", index);
                initConfigurationDict();
                // Set rate set
                _rateSet[40] = 128.0e6;
                _rateSet[39] = 64.0e6;
                _rateSet[38] = 32.0e6;
                _rateSet[37] = 32.0e6;
                _rateSet[36] = 16.0e6;
                _rateSet[35] = 16.0e6;
                _rateSet[34] = 16.0e6;
                _rateSet[33] = 8.0e6;
                _rateSet[32] = 8.0e6;
            }

            WbddcComponent::~WbddcComponent()
            {
            }

            WbddcComponent::WbddcComponent(const WbddcComponent& other) :
                ::LibCyberRadio::Driver::WbddcComponent(other)
            {
            }

            WbddcComponent& WbddcComponent::operator=(const WbddcComponent& other)
            {
                ::LibCyberRadio::Driver::WbddcComponent::operator=(other);
                if ( this != &other )
                {
                }
                return *this;
            }

            void WbddcComponent::initConfigurationDict()
            {
                //this->debug("[WbddcComponent::initConfigurationDict] Called\n");
                _config.clear();
                // Call the base-class version
                RadioComponent::initConfigurationDict();
                // Define tuner-specific keys
                _config["rateIndex"] = "";
                _config["udpDestination"] = "";
                _config["vitaEnable"] = "";
                _config["streamId"] = "";
                if ( _tunable )
                {
                    _config["frequency"] = "";
                }
                if ( _selectableSource )
                {
                    _config["source"] = "";
                }
                if ( _selectableDataPort )
                {
                    _config["dataPort"] = "";
                }
                //this->debug("[WbddcComponent::initConfigurationDict] Returning\n");
            }

            void WbddcComponent::queryConfiguration()
            {
                this->debug("[WbddcComponent::queryConfiguration] Called\n");
                Json::Value command;
                Json::Value params;
                command["cmd"] = "qwbddc";
                command["msg"] = _parent->getMessageId();
                command["params"] = Json::objectValue;
                command["params"]["id"] = _index;
                Json::FastWriter fastWriter;
                std::string output = fastWriter.write(command);
                BasicStringList rsp = _parent->sendCommand(output);
                Json::Reader reader;
                Json::Value returnVal; 
                std::string t = rsp.at(0);
                bool parsingSuccessful = reader.parse( t.c_str(), returnVal );     //parse process
                _enabled = boost::lexical_cast<bool>(returnVal["enable"].asBool());
                _frequency = boost::lexical_cast<double>(returnVal["freq"].asDouble());
                _source = boost::lexical_cast<int>(returnVal["source"].asInt());
                _dataPort = boost::lexical_cast<int>(returnVal["link"].asInt());
                _udpDestination = boost::lexical_cast<int>(returnVal["dest"].asInt());
                _rateIndex = boost::lexical_cast<int>(returnVal["filter"].asInt());
                _streamId = boost::lexical_cast<int>(returnVal["vita"].asUInt());
                updateConfigurationDict();
                this->debug("[WbddcComponent::queryConfiguration] Returning\n");
            }

            // Default implementation uses the NDR308 syntax.
            // WBDDC? <index>
            bool WbddcComponent::executeWbddcQuery(int index, int& rateIndex,
                    int& udpDestination, bool& enabled, int& vitaEnable,
                    unsigned int& streamId)
            {
                bool ret = false;
                if ( (_parent != NULL) && (_parent->isConnected()) )
                {
                    Json::Value root(Json::objectValue);
                    root["msg"] = m551Parent->getMessageId();
                    root["cmd"] = "qwbddc";
                    Json::Value params(Json::objectValue);
                    params["id"] = index;
                    root["params"] = params;
                    Json::FastWriter fastWriter;
                    std::string output = fastWriter.write(root);
                    LibCyberRadio::BasicStringList recv = _parent->sendCommand(output,1.0);
                    this->debug("DATA RETURNED ----- %s\n", recv.at(0));
                    Json::Reader reader;
                    Json::Value returnVal; 
                    std::string t = recv.at(0);
                    bool parsingSuccessful = reader.parse( t.c_str(), returnVal );     //parse process
                    Json::Value result = returnVal["result"];
                    rateIndex = boost::lexical_cast<int>(result["filter"].asInt());
                    udpDestination = boost::lexical_cast<int>(result["dest"].asInt());
                    enabled = boost::lexical_cast<int>(result["enable"].asBool());
                    vitaEnable = boost::lexical_cast<int>(result["enable"].asBool());
                    streamId = boost::lexical_cast<unsigned int>(result["vita"].asUInt());
                    
                    //std::ostringstream oss;
                    //oss << "WBDDC? " << index << "\n";
                    //BasicStringList rsp = _parent->sendCommand(oss.str(), 2.0);
                    //if ( _parent->getLastCommandErrorInfo() == "" )
                    //{
                    //    BasicStringList vec = Pythonesque::Split(
                    //            Pythonesque::Replace(rsp.front(), "WBDDC ", ""),
                    //            ", ");
                    //    // vec[0] = Index
                    //    // vec[1] = Rate index
                    //    rateIndex = boost::lexical_cast<int>(vec[1]);
                    //    udpDestination = boost::lexical_cast<int>(vec[2]);
                    //    enabled = (boost::lexical_cast<int>(vec[3]) == 1);
                    //    vitaEnable = boost::lexical_cast<int>(vec[4]);
                    //    streamId = boost::lexical_cast<unsigned int>(vec[5]);
                    //    ret = true;
                    //}
                }
                return ret;
            }

            bool WbddcComponent::executeDataPortCommand(int index, int& dataPort)
            {
                bool ret = false;
                if ( (_parent != NULL) && (_parent->isConnected()) )
                {
                    Json::Value root(Json::objectValue);
                    root["msg"] = m551Parent->getMessageId();
                    root["cmd"] = "wbddc";
                    Json::Value params(Json::objectValue);
                    params["id"] = index;
                    params["link"] = dataPort;
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
            // the NDR308, which does not support selectable-source WBDDCs.
            bool WbddcComponent::executeSourceCommand(int index, int& source)
            {
                bool ret = false;
                if ( (_parent != NULL) && (_parent->isConnected()) )
                {
                    Json::Value root(Json::objectValue);
                    root["msg"] = m551Parent->getMessageId();
                    root["cmd"] = "wbddc";
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
            bool WbddcComponent::executeFreqCommand(int index, double& freq)
            {
                bool ret = false;
                if ( (_parent != NULL) && (_parent->isConnected()) )
                {
                    Json::Value root(Json::objectValue);
                    root["msg"] = m551Parent->getMessageId();
                    root["cmd"] = "wbddc";
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
            bool WbddcComponent::executeWbddcCommand(int index, int& rateIndex,
                    int& udpDestination, bool& enabled, int& vitaEnable,
                    unsigned int& streamId)
            {
                bool ret = false;
                if ( (_parent != NULL) && (_parent->isConnected()) )
                {
                    Json::Value root(Json::objectValue);
                    root["msg"] = m551Parent->getMessageId();
                    root["cmd"] = "wbddc";
                    Json::Value params(Json::objectValue);
                    params["id"] = index;
                    params["filter"] = rateIndex;
                    params["dest"] = udpDestination;
                    params["enable"] = enabled;
                    params["vita"] = streamId;
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

