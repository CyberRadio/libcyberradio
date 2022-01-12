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
#include <iostream>


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
                _config["filter"] = 40;
                _config["dest"] = 0;
                _config["enable"] = false;
                _config["vita"] = 0;
                _config["type"] = "auto";
                _config["decimation"] = _decimation;
                _config["ovs"] = 1;
                _config["mode"] = _mode;
                _config["dgv"]  = _dgv; 
                _config["dul"]  = _dul; 
                _config["dll"]  = _dll; 
                _config["dtl"]  = _dtl; 
                _config["dal"]  = _dal; 
                _config["ddl"]  = _ddl; 
                _config["dao"]  = _dao; 
                _config["ddo"]  = _ddo; 
                _config["datc"] = _datc;
                _config["ddtc"] = _ddtc;
                _config["ddt"]  = 90; 
                if ( _tunable )
                {
                    _config["offset"] = 0.0f;
                }
                if ( _selectableSource )
                {
                    _config["rfch"] = "0";
                }
                if ( _selectableDataPort )
                {
                    _config["link"] = 0;
                }
                //this->debug("[WbddcComponent::initConfigurationDict] Returning\n");
            }

            bool WbddcComponent::setConfiguration(ConfigurationDict& cfg)
            {
                bool ret = false;
                this->debug("[NDR551WbddcComponent::setConfiguration] Called\n");
                // Setup the JSON Command.
                Json::Value command;
                Json::Value params;
                command["cmd"] = "wbddc";
                command["msg"] = _parent->getMessageId();
                command["params"] = Json::objectValue;
                command["params"]["id"] = _index;

                for( ConfigurationDict::const_iterator it = cfg.begin();
                     it != cfg.end(); ++it)
                {
                    if ( (it->first == "mode") || 
                         (it->first == "rfch") || 
                         (it->first == "type") )
                    {
                        command["params"][it->first.c_str()] = it->second.c_str();
                    } 
                    else if ( (it->first == "enable") )
                    {
                        command["params"][it->first.c_str()] = it->second.asBool();
                    }
                    else
                    {
                        command["params"][it->first.c_str()] = it->second.asInt();
                    }                    
                }
                Json::FastWriter fastWriter;
                std::string output = fastWriter.write(command);
                std::cout << output << std::endl;
                BasicStringList rsp = _parent->sendCommand(output);
                Json::Reader reader;
                Json::Value returnVal; 
                std::string t = rsp.at(0);
                bool parsingSuccessful = reader.parse( t.c_str(), returnVal );     //parse process
                ret = returnVal["success"].asBool();
                return ret;
            }

            void WbddcComponent::updateConfigurationDict()
            {
                this->debug("[NDR551WbddcComponent::updateConfigurationDict] Called\n");
                RadioComponent::updateConfigurationDict();
                if ( _config.hasKey("filter") )
                    setConfigurationValueToInt("filter", _rateIndex);
                if ( _config.hasKey("dest") )
                    setConfigurationValueToInt("dest", _udpDestination);
                if ( _config.hasKey("enable") )
                    setConfigurationValueToInt("enable", _vitaEnable);
                if ( _config.hasKey("vita") )
                    setConfigurationValueToUInt("vita", _streamId);
                if ( _tunable && _config.hasKey("offset") )
                {
                    setConfigurationValueToDbl("offset", _frequency);
                }
                if ( _selectableSource && _config.hasKey("rfch") )
                {
                    setConfigurationValueToInt("rfch", _source);
                }
                if ( _selectableDataPort && _config.hasKey("link") )
                {
                    setConfigurationValueToInt("link", _dataPort);
                }
                if ( _config.hasKey("mode") )
                {
                    setConfigurationValue("mode", _mode);
                }
                if ( _config.hasKey("decimation") )
                {
                    setConfigurationValueToInt("decimation", _decimation);
                }
                if ( _config.hasKey("dgv" ) ) 
                {
                    setConfigurationValueToInt("dgv", _dgv);
                }
                if ( _config.hasKey("dul" ) ) 
                {
                    setConfigurationValueToInt("dul", _dul);
                }
                if ( _config.hasKey("dll" ) ) 
                {
                    setConfigurationValueToInt("dll", _dll);
                }
                if ( _config.hasKey("dtl" ) ) 
                {
                    setConfigurationValueToInt("dtl", _dtl);
                }
                if ( _config.hasKey("dal" ) ) 
                {
                    setConfigurationValueToInt("dal", _dal);
                }
                if ( _config.hasKey("ddl" ) ) 
                {
                    setConfigurationValueToInt("ddl", _ddl);
                }
                if ( _config.hasKey("dao" ) ) 
                {
                    setConfigurationValueToInt("dao", _dao);
                }
                if ( _config.hasKey("ddo" ) ) 
                {
                    setConfigurationValueToInt("ddo", _ddo);
                }
                if ( _config.hasKey("datc") )
                {
                    setConfigurationValueToInt("datc", _datc);
                }
                if ( _config.hasKey("ddtc") )
                {
                    setConfigurationValueToInt("ddtc", _ddtc);
                }
                if ( _config.hasKey("ddt" ) ) 
                {
                    setConfigurationValueToInt("ddt", _ddt);
                }
                if ( _config.hasKey("type") )
                {
                    setConfigurationValue("type", _type);
                }
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
                _enabled = boost::lexical_cast<bool>(returnVal["result"]["enable"].asBool());
                _frequency = boost::lexical_cast<double>(returnVal["result"]["offset"].asDouble());
                _source = boost::lexical_cast<int>(returnVal["result"]["rfch"].asString());
                _dataPort = boost::lexical_cast<int>(returnVal["result"]["link"].asInt());
                _udpDestination = boost::lexical_cast<int>(returnVal["result"]["dest"].asInt());
                _rateIndex = boost::lexical_cast<int>(returnVal["result"]["filter"].asInt());
                _streamId = boost::lexical_cast<int>(returnVal["result"]["vita"].asUInt());
                _ovs = boost::lexical_cast<int>(returnVal["result"]["ovs"].asInt());
                _decimation = boost::lexical_cast<int>(returnVal["result"]["decimation"].asInt());
                _type = boost::lexical_cast<std::string>(returnVal["result"]["type"].asString());
                _mode = boost::lexical_cast<std::string>(returnVal["result"]["mode"].asString());
                _dgv = boost::lexical_cast<int>(returnVal["result"]["dgv"].asInt());
                _dul = boost::lexical_cast<int>(returnVal["result"]["dul"].asInt());
                _dll = boost::lexical_cast<int>(returnVal["result"]["dll"].asInt());
                _dtl = boost::lexical_cast<int>(returnVal["result"]["dtl"].asInt());
                _dal = boost::lexical_cast<int>(returnVal["result"]["dal"].asInt());
                _ddl = boost::lexical_cast<int>(returnVal["result"]["ddl"].asInt());
                _dao = boost::lexical_cast<int>(returnVal["result"]["dao"].asInt());
                _ddo = boost::lexical_cast<int>(returnVal["result"]["ddo"].asInt());
                _datc = boost::lexical_cast<int>(returnVal["result"]["datc"].asInt());
                _ddtc = boost::lexical_cast<int>(returnVal["result"]["ddtc"].asInt());
                _dat = boost::lexical_cast<int>(returnVal["result"]["dat"].asInt());
                _ddt = boost::lexical_cast<int>(returnVal["result"]["ddt"].asInt());
                updateConfigurationDict();
                this->debug("[WbddcComponent::queryConfiguration] Returning\n");
            }

            /*!
            ** \brief Execute a WBDDC Query and respond with settings
            ** \param index The DDC index
            */
            bool WbddcComponent::executeWbddcQuery(int index, int& rateIndex,
                    int& udpDestination, bool& enabled, int& vitaEnable,
                    unsigned int& streamId)
            {
                bool ret = false;
                if ( (_parent != NULL) && (_parent->isConnected()) )
                {
                    Json::Value root(Json::objectValue);
                    root["msg"] = m551Parent->getMessageId();
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
                    enabled = boost::lexical_cast<int>(result["enable"].asBool());
                    vitaEnable = boost::lexical_cast<int>(result["enable"].asBool());
                    streamId = boost::lexical_cast<unsigned int>(result["vita"].asUInt());
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
                    params["enable"] = boost::lexical_cast<bool>(vitaEnable);
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

            bool WbddcComponent::setRateIndex(int index)
            {
                bool ret = false;
                if ( _config.hasKey("filter") )
                {
                    int adjRateIndex = index;
                    int adjUdpDest = _udpDestination;
                    int adjVita = _vitaEnable;
                    unsigned int adjStream = _streamId;
                    bool adjEnabled = _enabled;
                    ret = executeWbddcCommand(_index, adjRateIndex, adjUdpDest, adjEnabled, adjVita, adjStream);
                    if ( ret )
                    {
                        _rateIndex = adjRateIndex;
                        updateConfigurationDict();
                    }
                }
                return ret;
            }

            bool WbddcComponent::setSource(int source)
            {
                bool ret = false;
                if ( _config.hasKey("rfch") )
                {
                    int adjSource = source;
                    ret = executeSourceCommand(_index, adjSource);
                    if ( ret )
                    {
                        _source = adjSource;
                        updateConfigurationDict();
                    }
                }
                return ret;
            }

            bool WbddcComponent::setUdpDestination(int dest)
            {
                bool ret = false;
                if ( _config.hasKey("dest") )
                {
                    int adjRateIndex = _rateIndex;
                    int adjUdpDest = dest;
                    int adjVita = _vitaEnable;
                    unsigned int adjStream = _streamId;
                    bool adjEnabled = _enabled;
                    ret = executeWbddcCommand(_index, adjRateIndex, adjUdpDest, adjEnabled, adjVita, adjStream);
                    if ( ret )
                    {
                        _udpDestination = adjUdpDest;
                        updateConfigurationDict();
                    }
                }
                return ret;
            }

        } /* namespace NDR551 */

    } // namespace Driver

} // namespace LibCyberRadio

