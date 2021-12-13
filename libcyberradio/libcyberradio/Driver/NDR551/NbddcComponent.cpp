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
#include <jsoncpp/json/json.h>

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
                _config["rateIndex"] = "";
                _config["udpDestination"] = "";
                _config["vitaEnable"] = "";
                _config["streamId"] = "";
                _config["frequency"] = "";
                _config["source"] = "";
                //this->debug("[NbddcComponent::initConfigurationDict] Returning\n");
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
                bool parsingSuccessful = reader.parse( t.c_str(), returnVal );     //parse process
                _enabled = boost::lexical_cast<bool>(returnVal["enable"].asBool());
                _frequency = boost::lexical_cast<double>(returnVal["freq"].asDouble());
                _source = boost::lexical_cast<int>(returnVal["source"].asInt());
                _udpDestination = boost::lexical_cast<int>(returnVal["dest"].asInt());
                _rateIndex = boost::lexical_cast<int>(returnVal["filter"].asInt());
                _streamId = boost::lexical_cast<int>(returnVal["vita"].asUInt());
                updateConfigurationDict();
                this->debug("[NbddcComponent::queryConfiguration] Returning\n");
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
                    //this->debug("DATA RETURNED ----- %s\n", recv.at(0));
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

