/***************************************************************************
 * \file RadioHandler.cpp
 * \brief Defines the radio handler interface for the NDR551.
 * \author DA
 * \author NH
 * \author MN
 * \copyright (c) 2017 CyberRadio Solutions, Inc.  All rights reserved.
 *
 ***************************************************************************/

#include "LibCyberRadio/Driver/NDR324/RadioHandler.h"
#include "LibCyberRadio/Driver/NDR551/DataPort.h"
#include "LibCyberRadio/Driver/NDR551/NbddcComponent.h"
#include "LibCyberRadio/Driver/NDR324/TunerComponent.h"
#include "LibCyberRadio/Driver/NDR324/VitaIfSpec.h"
#include "LibCyberRadio/Driver/NDR324/WbddcComponent.h"
//#include "LibCyberRadio/Driver/NDR551/WbddcGroupComponent.h"
//#include "LibCyberRadio/Driver/NDR551/NbddcGroupComponent.h"
#include "LibCyberRadio/Common/Pythonesque.h"
#include <json/json.h>
#include <sstream>
#include <cstdio>
#include <iostream>
#include <cstring>

static uint32_t msgCounter = 1;

namespace LibCyberRadio
{

    namespace Driver
    {
        namespace NDR324
        {

            RadioHandler::RadioHandler(bool debug) :
                    ::LibCyberRadio::Driver::RadioHandler(
                            /* const std::string& name */ "NDR324",
                            /* int numTuner */ 4,
                            /* int tunerIndexBase */ 0,
                            /* int numWbddc */ 4,
                            /* int wbddcIndexBase */ 0,
                            /* int numNbddc */ 0,
                            /* int nbddcIndexBase */ 0,
                            /* int numTunerBoards */ 1,
                            /* int maxTunerBw */ 6000,
                            /* int numTransmitters */ 0,
                            /* int transmitterIndexBase */ 0,
                            /* int numDuc */ 0,
                            /* int ducIndexBase */ 0,
                            /* int numWbddcGroups */ 0,
                            /* int wbddcGroupIndexBase */ 1,
                            /* int numNbddcGroups */ 0,
                            /* int nbddcGroupIndexBase */ 1,
                            /* int numDdcGroups */ 0,
                            /* int ddcGroupIndexBase */ 1,
                            /* int numDataPorts */ 4,
                            /* int dataPortIndexBase */ 0,
                            /* int numSimpleIpSetups */ 0,
                            /* double adcRate */ 1351.68e6,
                            /* VitaIfSpec ifSpec */ NDR324::VitaIfSpec(),
                            /* bool debug */ debug)
            {
                initConfigurationDict();
                _connModesSupported.push_back("udp");
                _defaultDeviceInfo = 19091;
                _transport.setJson(true);
                

                // Allocate tuner components
                for (int tuner = _tunerIndexBase;
                        tuner < (_tunerIndexBase + _numTuner); tuner++)
                {

                    _tuners[tuner] = new NDR324::TunerComponent(
                            /* int index */ tuner,
                            /* RadioHandler* parent */ this,
                            /* bool debug */ _debug,
                            /* double frequency */ 800e6,
                            /* double attenuation */ 0.0,
                            /* int filter */ 0);
                }               
                // Allocate WBDDC components
                for (int wbddc = _wbddcIndexBase;
                        wbddc < (_wbddcIndexBase + _numWbddc); wbddc++)
                {
                    _wbddcs[wbddc] = new NDR324::WbddcComponent(
                            /* int index */ wbddc,
                            /* RadioHandler* parent */ this,
                            /* bool debug */ _debug,
                            /* int dataPort */ 1,
                            /* int rateIndex */ 0,
                            /* int udpDestination */ 0,
                            /* int vitaEnable */ 0,
                            /* int streamId */ 0);
                } 
#if 0                               
                // Allocate NBDDC components
                for (int nbddc = _nbddcIndexBase;
                        nbddc < (_nbddcIndexBase + _numNbddc); nbddc++)
                {
                    _nbddcs[nbddc] = new NDR551::NbddcComponent(
                            /* int index */ nbddc,
                            /* RadioHandler* parent */ this,
                            /* bool debug */ _debug,
                            /* int dataPort */ 1,
                            /* int rateIndex */ 0,
                            /* int udpDestination */ 0,
                            /* int vitaEnable */ 0,
                            /* int streamId */ 0,
                            /* double frequency */ 0.0,
                            /* int source */ 1);
                }
                       
                // Allocate WBDDC group components
                for (int group = _wbddcGroupIndexBase;
                        group < (_wbddcGroupIndexBase + _numWbddcGroups); group++)
                {
                _wbddcGroups[group] = new NDR551::WbddcGroupComponent(
                        /* int index */ group,
                        /* RadioHandler* parent */ this,
                        /* bool debug */ _debug);
                }
                // Allocate NBDDC group components
                for (int group = _nbddcGroupIndexBase;
                        group < (_nbddcGroupIndexBase + _numNbddcGroups); group++)
                {
                _nbddcGroups[group] = new NDR551::NbddcGroupComponent(
                        /* int index */ group,
                        /* RadioHandler* parent */ this,
                        /* bool debug */ _debug);
                }
#endif                
                // Allocate data ports
                for (int dataPort = _dataPortIndexBase;
                        dataPort < (_dataPortIndexBase + _numDataPorts); dataPort++)
                {
                    _dataPorts[dataPort] = new NDR551::DataPort(
                            /* int index */ dataPort,
                            /* RadioHandler* parent */ this,
                            /* bool debug */ _debug,
                    /* const std::string& sourceIP */ "0.0.0.0");
                }                
            }

            RadioHandler::~RadioHandler()
            {
            }

            RadioHandler::RadioHandler(const RadioHandler &other) :
                    ::LibCyberRadio::Driver::RadioHandler(other)
            {
            }
            // Default implementation is the NDR308 pattern
            void RadioHandler::initConfigurationDict()
            {
                //this->debug("[RadioHandler::initConfigurationDict] Called\n");
                _config.clear();
                _config["referenceMode"] = _referenceMode;
                //this->debug("[RadioHandler::initConfigurationDict] Returning\n");
            }

            RadioHandler& RadioHandler::operator=(const RadioHandler& other)
            {
                    ::LibCyberRadio::Driver::RadioHandler::operator=(other);
                    // Block self-assignment
                    if (this != &other)
                    {
                    }
                    return *this;
            }

            void RadioHandler::queryConfiguration()
            {
                    this->debug("[NDR324]::RadioHandler::queryConfiguration] Called\n");
                    // Purge the banner sent over when a connection is made.
                    //BasicStringList rsp = _transport.receive(_defaultTimeout);
                    // Call the base-class queryConfiguration() to retrieve identity info
                    // and query configuration for all components
                    Json::Value root(Json::objectValue);
                    root["msg"] = this->getMessageId();
                    root["cmd"] = "qstatus";
                    Json::Value params(Json::objectValue);
                    root["params"] = params;
                    Json::FastWriter fastWriter;
                    std::string output = fastWriter.write(root);
                    ::LibCyberRadio::Driver::RadioHandler::sendCommand(output,1.0);
                    if ( _config.hasKey("referenceMode") )
                    {
                        this->executeReferenceModeQuery(_referenceMode);
                    }
                    this->debug("[NDR324::RadioHandler::queryConfiguration] Returning\n");

                    for ( TunerComponentDict::iterator it = _tuners.begin();
                            it != _tuners.end(); it++)
                    {
                        it->second->queryConfiguration();
                    }
                    for ( WbddcComponentDict::iterator it = _wbddcs.begin();
                            it != _wbddcs.end(); it++)
                    {
                        it->second->queryConfiguration();
                    }
                    for ( DataPortDict::iterator it = _dataPorts.begin();
                            it != _dataPorts.end(); it++)
                    {
                        it->second->queryConfiguration();   
                    }
                    this->queryVersionInfo();
            }

            bool RadioHandler::query324Specifics()
            {
                Json::Value root(Json::objectValue);
                root["msg"] = this->getMessageId();
                root["cmd"] = "cli";
                Json::Value params(Json::objectValue);
                params["input"] = "version";
                root["params"] = params;
                Json::FastWriter fastWriter;
                std::string output = fastWriter.write(root);
                BasicStringList rsp = ::LibCyberRadio::Driver::RadioHandler::sendCommand(output,1.0);
                Json::Reader reader;
                Json::Value returnVal; 
                std::string t = rsp.at(0);
                bool parsingSuccessful = reader.parse( t.c_str(), returnVal );     //parse process
                if( parsingSuccessful ){
                    std::string result = returnVal["result"].asString();
                    LibCyberRadio::BasicStringList l = Pythonesque::Split(Pythonesque::Lstrip(result), "\n"); //Pythonesque::Split(Pythonesque::Replace(result," ", ""), "\n");
                    for(int i = 0; i < l.size(); i++)
                    {
                        std::string temp = l.at(i);
                        temp = Pythonesque::Replace(temp," ", "").c_str();
                        BasicStringList J = Pythonesque::Split(temp, ":");
                        const char *unitsn = "UnitSN";                        
                        if ( std::strstr(temp.c_str(), unitsn) != nullptr )
                        {   
                            _versionInfo["serialNumber"] = J.at(1).c_str();
                        }
                    }
                }
                return true;
            }

            bool RadioHandler::queryVersionInfo()
            {
                this->debug("[RadioHandler::queryVersionInfo] Called\n");
                // First, call the base-class version
                bool ret = true;
                Json::Value root(Json::objectValue);
                root["msg"] = this->getMessageId();
                root["cmd"] = "qstatus";
                Json::Value params(Json::objectValue);
                root["params"] = params;
                Json::FastWriter fastWriter;
                std::string output = fastWriter.write(root);
                BasicStringList rsp = ::LibCyberRadio::Driver::RadioHandler::sendCommand(output,1.0);
                Json::Reader reader;
                Json::Value returnVal; 
                std::string t = rsp.at(0);
                bool parsingSuccessful = reader.parse( t.c_str(), returnVal );     //parse process
                if( parsingSuccessful ){
                    _versionInfo["model"] = returnVal["result"]["model"].asString();
                    _versionInfo["softwareVersion"] = returnVal["result"]["sw"].asString();
                    _versionInfo["firmwareVersion"] = returnVal["result"]["fw"].asString();
                    _versionInfo["unitRevision"] = returnVal["result"]["unit"].asString();
                    _versionInfo["hardwareVersion"] = returnVal["result"]["unit"].asString();
                    _versionInfo["serialNumber"] = returnVal["result"]["sn"].asString();
                }
                //this->query324Specifics();               
                this->debug("[NDR324::RadioHandler::queryVersionInfo] Returning %s\n", debugBool(ret));
                return ret;
            }

            // NOTE: The default implementation is the NDR324 implementation,
            // but this just makes it explicit in the code.
            bool RadioHandler::executeQueryIDN(std::string& model,
                    std::string& serialNumber)
            {
                    return ::LibCyberRadio::Driver::RadioHandler::executeQueryIDN(model, serialNumber);
            }

            // NOTE: The default implementation is the NDR324 implementation,
            // but this just makes it explicit in the code.
            bool RadioHandler::executeQueryVER(std::string& softwareVersion,
                    std::string& firmwareVersion,
                    std::string& referenceVersion,
                    std::string& firmwareDate)
            {

                    return ::LibCyberRadio::Driver::RadioHandler::executeQueryVER(softwareVersion,
                            firmwareVersion,
                            referenceVersion,
                            firmwareDate);
            }

            // NOTE: The default implementation is the NDR324 implementation,
            // but this just makes it explicit in the code.
            bool RadioHandler::executeQueryHREV(std::string& hardwareInfo)
            {
                return ::LibCyberRadio::Driver::RadioHandler::executeQueryHREV(hardwareInfo);
            }

            uint32_t RadioHandler::getMessageId( void )
            {
                return msgCounter++;
            }

            bool RadioHandler::executeReferenceModeQuery(int& refMode)
            {
                this->debug("[NDR324::RadioHandler::queryVersionInfo] Called\n");
                // First, call the base-class version
                bool ret = false;
                Json::Value root(Json::objectValue);
                root["msg"] = this->getMessageId();
                root["cmd"] = "qref";
                Json::Value params(Json::objectValue);
                root["params"] = params;
                Json::FastWriter fastWriter;
                std::string output = fastWriter.write(root);
                BasicStringList rsp = this->sendCommand(output);
                Json::Reader reader;
                Json::Value returnVal; 
                std::string t = rsp.at(0);
                bool parsingSuccessful = reader.parse( t.c_str(), returnVal );     //parse process
                if( parsingSuccessful ){
                    refMode = boost::lexical_cast<int>(returnVal["result"]["cfg10m"].asInt());
                    ret = true;
                }
                return ret;
            }

            bool RadioHandler::executeReferenceModeCommand(int& refMode)
            {
                this->debug("[NDR324::RadioHandler::queryVersionInfo] Called\n");
                // First, call the base-class version
                bool ret = false;
                Json::Value root(Json::objectValue);
                root["msg"] = this->getMessageId();
                root["cmd"] = "ref";
                Json::Value params(Json::objectValue);
                params["cfg10m"] = refMode;
                root["params"] = params;
                Json::FastWriter fastWriter;
                std::string output = fastWriter.write(root);
                BasicStringList rsp = this->sendCommand(output);
                Json::Reader reader;
                Json::Value returnVal; 
                std::string t = rsp.at(0);
                bool parsingSuccessful = reader.parse( t.c_str(), returnVal );     //parse process
                if( parsingSuccessful ){
                    ret = boost::lexical_cast<bool>(returnVal["success"].asBool());
                }
                return ret;
            }


        } /* namespace NDR324 */

    } /* namespace Driver */

} /* namespace LibCyberRadio */
