/***************************************************************************
 * \file DataPort.cpp
 * \brief Defines the 10GigE data port interface for an NDR551.
 * \author DA
 * \author NH
 * \author MN
 * \copyright (c) 2017 CyberRadio Solutions, Inc.  All rights reserved.
 *
 ***************************************************************************/

#include "LibCyberRadio/Driver/NDR551/DataPort.h"
#include "LibCyberRadio/Driver/RadioHandler.h"
#include <boost/format.hpp>


namespace LibCyberRadio
{

    namespace Driver
    {

        namespace NDR551
        {

            DataPort::DataPort(int index,
                    ::LibCyberRadio::Driver::RadioHandler* parent,
                     bool debug,
                     const std::string& sourceIP) :
                ::LibCyberRadio::Driver::DataPort(/* const std::string& name */ (boost::format("NDR551-DP%02d") % \
                        index).str(),
                        /* int index */ index,
                        /* RadioHandler* parent */ parent,
                        /* bool debug */ debug,
                        /* const std::string& sourceIP */ sourceIP,
                        /* int numDataPortDipEntries */ 64,
                        /* int dataPortDipEntryIndexBase */ 0)
            {
                this->debug("[NDR551::DataPort] index - %d\n", index);
                
                initConfigurationDict();
            }

            DataPort::~DataPort()
            {
            }

            DataPort::DataPort(const DataPort& other) :
                ::LibCyberRadio::Driver::DataPort(other)
            {
            }

            DataPort& DataPort::operator=(const DataPort& other)
            {
                ::LibCyberRadio::Driver::DataPort::operator=(other);
                if ( this != &other )
                {
                }
                return *this;
            }

            void DataPort::initConfigurationDict()
            {
                _config.clear();
                _config["sourceIP"] = _sourceIP;
                _config["sourcePort"] = _sourcePort;
                _config["sourceMac"] = _sourceMacAddr;
            }
            void DataPort::queryConfiguration()
            {
                Json::Value command;
                Json::Value params;
                command["cmd"] = "qcfge10g";
                command["msg"] = _parent->getMessageId();
                command["params"] = Json::objectValue;
                command["params"]["link"] = _index;
                Json::FastWriter fastWriter;
                std::string output = fastWriter.write(command);
                BasicStringList rsp = _parent->sendCommand(output);
                Json::Reader reader;
                Json::Value returnVal; 
                std::string t = rsp.at(0);
                bool parsingSuccessful = reader.parse( t.c_str(), returnVal );     //parse process
                _sourceIP = boost::lexical_cast<std::string>(returnVal["result"]["ip"].asString());
                _sourceMacAddr = boost::lexical_cast<std::string>(returnVal["result"]["mac"].asString());
                _sourcePort = boost::lexical_cast<uint16_t>(returnVal["result"]["port"].asUInt());
                for(int i = _dipEntryIndexBase; i < _numDipEntries; i++ )
                {
                    this->executeDestIPQuery(_index, i, _ipAddresses[i],
                                             _macAddresses[i], 
                                             _sourcePorts[i],
                                             _destPorts[i]);
                }
                updateConfigurationDict();
            }

            bool DataPort::executeSourceIPQuery(int index, std::string& ipAddr)
            {
                bool ret = false;
                Json::Value command;
                Json::Value params;
                command["cmd"] = "qcfge10g";
                command["msg"] = _parent->getMessageId();
                command["params"] = Json::objectValue;
                command["params"]["link"] = index;
                Json::FastWriter fastWriter;
                std::string output = fastWriter.write(command);
                BasicStringList rsp = _parent->sendCommand(output);
                Json::Reader reader;
                Json::Value returnVal; 
                std::string t = rsp.at(0);
                bool parsingSuccessful = reader.parse( t.c_str(), returnVal );     //parse process
                ret = returnVal["success"].asBool();
                if(ret){
                    ipAddr = boost::lexical_cast<std::string>(returnVal["result"]["ip"].asString());
                } else {
                    ipAddr = "0.0.0.0";
                }
                return ret;
            }
            bool DataPort::executeSourceIPCommand(int index, std::string& ipAddr)
            {
                bool ret = false;
                Json::Value command;
                Json::Value params;
                command["cmd"] = "cfge10g";
                command["msg"] = _parent->getMessageId();
                command["params"] = Json::objectValue;
                command["params"]["link"] = index;
                command["params"]["ip"] = ipAddr.c_str();
                Json::FastWriter fastWriter;
                std::string output = fastWriter.write(command);
                BasicStringList rsp = _parent->sendCommand(output);
                Json::Reader reader;
                Json::Value returnVal; 
                std::string t = rsp.at(0);
                bool parsingSuccessful = reader.parse( t.c_str(), returnVal );     //parse process
                ret = returnVal["success"].asBool();
                return ret;
            }
            // Default implementation uses the NDR308 pattern
            bool DataPort::executeDestIPQuery(int index,
                    int dipIndex,
                    std::string& ipAddr,
                    std::string& macAddr,
                    unsigned int& sourcePort,
                    unsigned int& destPort)
            {
                bool ret = false;
                ipAddr = "";
                macAddr = "";
                sourcePort = 0;
                destPort = 0;
                if ( (_parent != NULL) && (_parent->isConnected()) &&
                        ( _macAddresses.find(dipIndex) != _macAddresses.end()) )
                {
                    Json::Value command;
                    Json::Value params;
                    command["cmd"] = "qe10g";
                    command["msg"] = _parent->getMessageId();
                    command["params"] = Json::objectValue;
                    command["params"]["link"] = index;
                    command["params"]["dest"] = dipIndex;
                    Json::FastWriter fastWriter;
                    std::string output = fastWriter.write(command);
                    BasicStringList rsp = _parent->sendCommand(output);
                    Json::Reader reader;
                    Json::Value returnVal; 
                    std::string t = rsp.at(0);
                    bool parsingSuccessful = reader.parse( t.c_str(), returnVal );     //parse process
                    ret = returnVal["success"].asBool();
                    if(ret)
                    {
                        ipAddr = boost::lexical_cast<std::string>(returnVal["result"]["ip"].asString());
                        macAddr = boost::lexical_cast<std::string>(returnVal["result"]["mac"].asString());
                        //sourcePort = boost::lexical_cast<uint16_t>(returnVal["result"]["port"].asUInt());
                        sourcePort = 0;
                        destPort = boost::lexical_cast<uint16_t>(returnVal["result"]["port"].asUInt());
                    }
                }
                return ret;
            }
            bool DataPort::executeDestIPCommand(int index,
                            int dipIndex,
                            std::string& ipAddr,
                            std::string& macAddr,
                            unsigned int& sourcePort,
                            unsigned int& destPort)
            {
                bool ret = false;
                Json::Value command;
                Json::Value params;
                command["cmd"] = "e10g";
                command["msg"] = _parent->getMessageId();
                command["params"] = Json::objectValue;
                command["params"]["link"] = index;
                command["params"]["dest"] = dipIndex;
                command["params"]["ip"] = ipAddr.c_str();
                command["params"]["port"] = destPort;
                command["params"]["mac"] = macAddr.c_str();
                command["params"]["arp"] = false;
                Json::FastWriter fastWriter;
                std::string output = fastWriter.write(command);
                BasicStringList rsp = _parent->sendCommand(output);
                Json::Reader reader;
                Json::Value returnVal; 
                std::string t = rsp.at(0);
                bool parsingSuccessful = reader.parse( t.c_str(), returnVal );     //parse process
                ret = returnVal["success"].asBool();
                return ret;
            }

        } // namespace NDR551

    } /* namespace Driver */

} /* namespace LibCyberRadio */






