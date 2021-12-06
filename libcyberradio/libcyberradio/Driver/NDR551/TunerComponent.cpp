/***************************************************************************
 * \file TunerComponent.cpp
 * \brief Defines the tuner interface for the NDR551.
 * \author DA
 * \author NH
 * \author MN
 * \copyright (c) 2017 CyberRadio Solutions, Inc.  All rights reserved.
 *
 ***************************************************************************/

#include "LibCyberRadio/Driver/NDR551/TunerComponent.h"
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

            TunerComponent::TunerComponent(int index,
                    ::LibCyberRadio::Driver::RadioHandler* parent,
                     bool debug,
                     double frequency,
                     double attenuation,
                     int filter) :
                ::LibCyberRadio::Driver::TunerComponent(
                        /* const std::string& name */ (boost::format("NDR551-TUNER%02d") % \
                                index).str(),
                        /* int index */ index,
                        /* ::LibCyberRadio::Driver::RadioHandler* parent */ parent,
                        /* bool debug */ debug,
                        /* double freqRangeMin */ 20e6,
                        /* double freqRangeMax */ 18000e6,
                        /* double freqRes */ 10e6,
                        /* double freqUnits */ 1e6,
                        /* double attRangeMin */ 0.0,
                        /* double attRangeMax */ 40.0,
                        /* double attRes */ 1.0,
                        /* bool agc */ true,
                        /* double frequency */ frequency,
                        /* double attenuation */ attenuation,
                        /* int filter */ filter)
            {
                this->debug("[NDR551::TunerComponent] index - %d\n", index);
                this->initConfigurationDict();
            }

            void TunerComponent::initConfigurationDict()
            {
                _config.clear();
                _config["enable"] = "";
                _config["frequency"] = "";
                _config["attenuation"] = "";
                _config["mode"] = "";
                
            }

            TunerComponent::~TunerComponent()
            {
            }

            TunerComponent::TunerComponent(const TunerComponent& other) :
                ::LibCyberRadio::Driver::TunerComponent(other)
            {
            }

            TunerComponent& TunerComponent::operator=(const TunerComponent& other)
            {
                ::LibCyberRadio::Driver::TunerComponent::operator=(other);
                if ( this != &other )
                {
                }
                return *this;
            }

            bool TunerComponent::executeFreqCommand(int index, double& freq)
            {
                bool ret = false;
                if ( (_parent != NULL) && (_parent->isConnected()) )
                {
                    Json::Value command;
                    Json::Value params;
                    command["cmd"] = "tuner";
                    command["msg"] = _parent->getMessageId();
                    command["params"] = Json::objectValue;
                    command["params"]["id"] = index;
                    command["params"]["freq"] = freq;
                    Json::FastWriter fastWriter;
                    std::string output = fastWriter.write(command);
                    BasicStringList rsp = _parent->sendCommand(output);
                    Json::Reader reader;
                    Json::Value returnVal; 
                    std::string t = rsp.at(0);
                    bool parsingSuccessful = reader.parse( t.c_str(), returnVal );     //parse process
                    ret = true;
                    ///Json::Value result = returnVal["result"];
                    //ret = boost::lexical_cast<bool>(result["success"]);
                }
                return ret;
            }

            void TunerComponent::queryConfiguration()
            {
                this->debug("[TunerComponent::queryConfiguration] Called\n");
                Json::Value command;
                Json::Value params;
                command["cmd"] = "qtuner";
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
                _attenuation = boost::lexical_cast<double>(returnVal["atten"].asDouble());
                updateConfigurationDict();
                this->debug("[NDR551] [TunerComponent::queryConfiguration] Returning\n");
            }

            bool TunerComponent::executeEnableCommand(int index, bool& enabled)
            {
                bool ret = false;
                if ( (_parent != NULL) && (_parent->isConnected()) )
                {
                    Json::Value command;
                    Json::Value params;
                    command["cmd"] = "tuner";
                    command["msg"] = _parent->getMessageId();
                    command["params"] = Json::objectValue;
                    command["params"]["id"] = index;
                    command["params"]["enable"] = enabled;
                    Json::FastWriter fastWriter;
                    std::string output = fastWriter.write(command);
                    BasicStringList rsp = _parent->sendCommand(output);
                    Json::Reader reader;
                    Json::Value returnVal; 
                    std::string t = rsp.at(0);
                    bool parsingSuccessful = reader.parse( t.c_str(), returnVal );     //parse process
                    ret = returnVal["success"].asBool();
                }
                return ret;
            }

            bool TunerComponent::executeAttenCommand(int index, double& atten)
            {
                bool ret = false;
                if ( (_parent != NULL) && (_parent->isConnected()) )
                {
                    Json::Value command;
                    Json::Value params;
                    command["cmd"] = "tuner";
                    command["msg"] = _parent->getMessageId();
                    command["params"] = Json::objectValue;
                    command["params"]["id"] = index;
                    command["params"]["atten"] = atten;
                    Json::FastWriter fastWriter;
                    std::string output = fastWriter.write(command);
                    BasicStringList rsp = _parent->sendCommand(output);
                    Json::Reader reader;
                    Json::Value returnVal; 
                    std::string t = rsp.at(0);
                    bool parsingSuccessful = reader.parse( t.c_str(), returnVal );     //parse process
                    ret = returnVal["success"].asBool();
                }
                return ret;
            }


        } /* namespace NDR551 */

    } // namespace Driver

} // namespace LibCyberRadio
