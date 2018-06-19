/***************************************************************************
 * \file NbddcComponent.cpp
 * \brief Defines the basic NBDDC interface for an NDR-class radio.
 * \author NH
 * \author DA
 * \author MN
 * \copyright (c) 2017 CyberRadio Solutions, Inc.  All rights reserved.
 *
 ***************************************************************************/

#include "LibCyberRadio/Driver/RadioHandler.h"
#include "LibCyberRadio/Driver/NbddcComponent.h"
#include "LibCyberRadio/Common/Pythonesque.h"
#include <boost/lexical_cast.hpp>
#include <sstream>
#include <iomanip>


namespace LibCyberRadio
{
    namespace Driver
    {

        NbddcComponent::NbddcComponent(const std::string& name, int index,
        		                       RadioHandler* parent,
									   bool debug,
									   bool nbddcCommandSetsFreq,
									   bool nbddcCommandSetsSource,
									   bool selectableDataPort,
									   double freqRangeMin,
									   double freqRangeMax,
		                               double freqRes,
		                               double freqUnits,
									   int source,
									   int dataPort,
									   double frequency,
									   int rateIndex,
									   int udpDestination,
									   int vitaEnable,
									   unsigned int streamId) :
            RadioComponent(name, index, parent, debug),
			_nbddcCommandSetsFreq(nbddcCommandSetsFreq),
			_nbddcCommandSetsSource(nbddcCommandSetsSource),
            _selectableDataPort(selectableDataPort),
            _freqRangeMin(freqRangeMin),
            _freqRangeMax(freqRangeMax),
			_freqRes(freqRes),
			_freqUnits(freqUnits),
            _source(source),
			_dataPort(dataPort),
            _frequency(frequency),
            _rateIndex(rateIndex),
            _udpDestination(udpDestination),
            _vitaEnable(vitaEnable),
            _streamId(streamId)
        {
        	initConfigurationDict();
        }

        NbddcComponent::~NbddcComponent()
        {
        }

        NbddcComponent::NbddcComponent(const NbddcComponent& other) :
            RadioComponent(other),
			_nbddcCommandSetsFreq(other._nbddcCommandSetsFreq),
			_nbddcCommandSetsSource(other._nbddcCommandSetsSource),
			_selectableDataPort(other._selectableDataPort),
            _freqRangeMin(other._freqRangeMin),
            _freqRangeMax(other._freqRangeMax),
            _freqRes(other._freqRes),
            _freqUnits(other._freqUnits),
            _source(other._source),
			_dataPort(other._dataPort),
            _frequency(other._frequency),
			_rateIndex(other._rateIndex),
			_udpDestination(other._udpDestination),
			_vitaEnable(other._vitaEnable),
			_streamId(other._streamId)
        {
        }

        NbddcComponent& NbddcComponent::operator=(const NbddcComponent& other)
        {
            RadioComponent::operator=(other);
            if ( this != &other )
            {
            	_nbddcCommandSetsFreq = other._nbddcCommandSetsFreq;
                _nbddcCommandSetsSource = other._nbddcCommandSetsSource;
    			_selectableDataPort = other._selectableDataPort;
                _freqRangeMin = other._freqRangeMin;
                _freqRangeMax = other._freqRangeMax;
                _freqRes = other._freqRes;
                _freqUnits = other._freqUnits;
                _source = other._source;
    			_dataPort = other._dataPort;
                _frequency = other._frequency;
    			_rateIndex = other._rateIndex;
    			_udpDestination = other._udpDestination;
    			_vitaEnable = other._vitaEnable;
    			_streamId = other._streamId;
            }
            return *this;
        }

        bool NbddcComponent::enable(bool enabled)
        {
        	int adjRateIndex = _rateIndex;
        	int adjUdpDest = _udpDestination;
        	int adjVita = _vitaEnable;
        	unsigned int adjStream = _streamId;
            bool adjEnabled = enabled;
            double adjFreq = _frequency;
            int adjSource = _source;
            bool ret = executeNbddcCommand(_index, adjRateIndex, adjUdpDest, adjEnabled,
            		                       adjVita, adjStream, adjFreq, adjSource);
            if ( ret )
            {
                _enabled = adjEnabled;
                updateConfigurationDict();
                // If the hardware call succeeds, call the base class version
                RadioComponent::enable(enabled);
            }
            return ret;
        }

        bool NbddcComponent::setConfiguration(ConfigurationDict& cfg)
        {
        	// Call the "grandparent" version of this method instead of the
        	// parent version. We want the normalization, but not the
        	// automatic enabling.
        	bool ret = Configurable::setConfiguration(cfg);
        	// Use the keys provided in the *incoming* dictionary to determine
        	// what needs to be changed via hardware calls.
        	int adjRateIndex = _rateIndex;
        	int adjUdpDest = _udpDestination;
        	int adjVita = _vitaEnable;
        	unsigned int adjStream = _streamId;
            bool adjEnabled = _enabled;
            double adjFreq = _frequency;
            int adjSource = _source;
            int adjDataPort = _dataPort;
            bool ddcCmdNeedsExecuting = false;
            bool freqCmdNeedsExecuting = false;
            bool srcCmdNeedsExecuting = false;
            bool dpCmdNeedsExecuting = false;
            if ( cfg.find("enable") != cfg.end() )
            {
            	adjEnabled = getConfigurationValueAsBool("enable");
            	ddcCmdNeedsExecuting = true;
            }
            if ( cfg.find("rateIndex") != cfg.end() )
            {
            	adjRateIndex = getConfigurationValueAsInt("rateIndex");
            	ddcCmdNeedsExecuting = true;
            }
            if ( cfg.find("udpDestination") != cfg.end() )
            {
            	adjUdpDest = getConfigurationValueAsInt("udpDestination");
            	ddcCmdNeedsExecuting = true;
            }
            if ( cfg.find("vitaEnable") != cfg.end() )
            {
            	adjVita = getConfigurationValueAsInt("vitaEnable");
            	ddcCmdNeedsExecuting = true;
            }
            if ( cfg.find("streamId") != cfg.end() )
            {
            	adjStream = getConfigurationValueAsUInt("streamId");
            	ddcCmdNeedsExecuting = true;
            }
			if ( cfg.find("frequency") != cfg.end() )
			{
				adjFreq = getConfigurationValueAsDbl("frequency");
				if ( _nbddcCommandSetsFreq )
					ddcCmdNeedsExecuting = true;
				else
					freqCmdNeedsExecuting = true;
			}
			if ( cfg.find("source") != cfg.end() )
			{
				adjSource = getConfigurationValueAsInt("source");
				if ( _nbddcCommandSetsSource )
					ddcCmdNeedsExecuting = true;
				else
					srcCmdNeedsExecuting = true;
			}
			if ( cfg.find("dataPort") != cfg.end() )
			{
				adjDataPort = getConfigurationValueAsInt("dataPort");
	            if ( _selectableDataPort )
					dpCmdNeedsExecuting = true;
			}
            if ( ddcCmdNeedsExecuting )
            {
				ret &= executeNbddcCommand(_index, adjRateIndex, adjUdpDest,
						                   adjEnabled, adjVita, adjStream,
										   adjFreq, adjSource);
            }
            if ( freqCmdNeedsExecuting )
            {
				ret &= executeFreqCommand(_index, adjFreq);
            }
            if ( srcCmdNeedsExecuting )
            {
				ret &= executeSourceCommand(_index, adjSource);
            }
            if ( dpCmdNeedsExecuting )
            {
				ret &= executeDataPortCommand(_index, adjDataPort);
            }
            if ( ret )
            {
            	_rateIndex = adjRateIndex;
            	_udpDestination = adjUdpDest;
            	_vitaEnable = adjVita;
            	_streamId = adjStream;
            	_enabled = adjEnabled;
            	_frequency = adjFreq;
            	_source = adjSource;
            	_dataPort = adjDataPort;
            	updateConfigurationDict();
            }
            return ret;
        }

        void NbddcComponent::queryConfiguration()
        {
            this->debug("[queryConfiguration] Called\n");
            executeNbddcQuery(_index, _rateIndex, _udpDestination, _enabled,
            		          _vitaEnable, _streamId, _frequency, _source);
            if ( !_nbddcCommandSetsFreq )
            {
            	executeFreqQuery(_index, _frequency);
            }
            if ( !_nbddcCommandSetsSource )
            {
            	executeSourceQuery(_index, _source);
            }
            if ( _selectableDataPort )
            {
            	executeDataPortQuery(_index, _dataPort);
            }
            updateConfigurationDict();
//            this->debug("[queryConfiguration] Config:\n");
//            this->debug("[queryConfiguration] -- enabled=%s\n",
//                                 debugBool(_enabled));
//            this->debug("[queryConfiguration] -- rate=%d\n", _rateIndex);
//            this->debug("[queryConfiguration] -- dest=%d\n", _udpDestination);
//            this->debug("[queryConfiguration] -- vita=%d\n", _vitaEnable);
//            this->debug("[queryConfiguration] -- sid=%u\n", _streamId);
//			this->debug("[queryConfiguration] -- freq=%0.1f\n", _frequency);
//            this->debug("[queryConfiguration] -- source=%d\n", _source);
//            if ( _selectableDataPort )
//            {
//            	this->debug("[queryConfiguration] -- port=%d\n", _dataPort);
//            }
//            this->debug("[queryConfiguration] Returning\n");
        }

		double NbddcComponent::getFrequency() const
		{
			return _frequency;
		}

		bool NbddcComponent::setFrequency(double freq)
		{
			bool ret = false;
            double adjFreq = freq;
			if ( _nbddcCommandSetsFreq )
			{
	        	int adjRateIndex = _rateIndex;
	        	int adjUdpDest = _udpDestination;
	        	int adjVita = _vitaEnable;
	        	unsigned int adjStream = _streamId;
	            bool adjEnabled = _enabled;
	            int adjSource = _source;
				ret = executeNbddcCommand(_index, adjRateIndex, adjUdpDest, adjEnabled,
										  adjVita, adjStream, adjFreq, adjSource);
			}
			else
			{
	            ret = executeFreqCommand(_index, adjFreq);
			}
			if ( ret )
			{
				_frequency = adjFreq;
                updateConfigurationDict();
			}
			return ret;
		}

		BasicDoubleList NbddcComponent::getFrequencyRange() const
		{
			BasicDoubleList ret;
			ret.push_back(_freqRangeMin);
			ret.push_back(_freqRangeMax);
			return ret;
		}

		double NbddcComponent::getFrequencyRes() const
		{
			return _freqRes;
		}

		double NbddcComponent::getFrequencyUnit() const
		{
			return _freqUnits;
		}

		int NbddcComponent::getSource() const
		{
			return _source;
		}

		bool NbddcComponent::setSource(int source)
		{
			bool ret = false;
            int adjSource = source;
			if ( _nbddcCommandSetsSource )
			{
	        	int adjRateIndex = _rateIndex;
	        	int adjUdpDest = _udpDestination;
	        	int adjVita = _vitaEnable;
	        	unsigned int adjStream = _streamId;
	            bool adjEnabled = _enabled;
	            double adjFreq = _frequency;
				ret = executeNbddcCommand(_index, adjRateIndex, adjUdpDest, adjEnabled,
										  adjVita, adjStream, adjFreq, adjSource);
			}
			else
			{
	            ret = executeSourceCommand(_index, adjSource);
			}
			if ( ret )
			{
				_source = adjSource;
                updateConfigurationDict();
			}
			return ret;
		}

		int NbddcComponent::getRateIndex() const
		{
			return _rateIndex;
		}

		bool NbddcComponent::setRateIndex(int index)
		{
        	int adjRateIndex = index;
        	int adjUdpDest = _udpDestination;
        	int adjVita = _vitaEnable;
        	unsigned int adjStream = _streamId;
            bool adjEnabled = _enabled;
            double adjFreq = _frequency;
            int adjSource = _source;
            bool ret = executeNbddcCommand(_index, adjRateIndex, adjUdpDest,
            		                       adjEnabled, adjVita, adjStream,
										   adjFreq, adjSource);
            if ( ret )
            {
                _rateIndex = adjRateIndex;
                updateConfigurationDict();
            }
            return ret;
		}

		int NbddcComponent::getUdpDestination() const
		{
			return _udpDestination;
		}

		bool NbddcComponent::setUdpDestination(int dest)
		{
        	int adjRateIndex = _rateIndex;
        	int adjUdpDest = dest;
        	int adjVita = _vitaEnable;
        	unsigned int adjStream = _streamId;
            bool adjEnabled = _enabled;
            double adjFreq = _frequency;
            int adjSource = _source;
            bool ret = executeNbddcCommand(_index, adjRateIndex, adjUdpDest,
            		                       adjEnabled, adjVita, adjStream,
										   adjFreq, adjSource);
            if ( ret )
            {
                _udpDestination = adjUdpDest;
                updateConfigurationDict();
            }
            return ret;
		}

		int NbddcComponent::getVitaEnable() const
		{
			return _vitaEnable;
		}

		bool NbddcComponent::setVitaEnable(int enable)
		{
        	int adjRateIndex = _rateIndex;
        	int adjUdpDest = _udpDestination;
        	int adjVita = enable;
        	unsigned int adjStream = _streamId;
            bool adjEnabled = _enabled;
            double adjFreq = _frequency;
            int adjSource = _source;
            bool ret = executeNbddcCommand(_index, adjRateIndex, adjUdpDest,
            		                       adjEnabled, adjVita, adjStream,
										   adjFreq, adjSource);
            if ( ret )
            {
            	_vitaEnable = adjVita;
                updateConfigurationDict();
            }
            return ret;
		}

		unsigned int NbddcComponent::getStreamId() const
		{
			return _streamId;
		}

		bool NbddcComponent::setStreamId(unsigned int sid)
		{
        	int adjRateIndex = _rateIndex;
        	int adjUdpDest = _udpDestination;
        	int adjVita = _vitaEnable;
        	unsigned int adjStream = sid;
            bool adjEnabled = _enabled;
            double adjFreq = _frequency;
            int adjSource = _source;
            bool ret = executeNbddcCommand(_index, adjRateIndex, adjUdpDest,
            		                       adjEnabled, adjVita, adjStream,
										   adjFreq, adjSource);
            if ( ret )
            {
            	_streamId = adjStream;
                updateConfigurationDict();
            }
            return ret;
		}

		int NbddcComponent::getDataPort() const
		{
			return _dataPort;
		}

		bool NbddcComponent::setDataPort(int port)
		{
			bool ret = false;
			if ( _selectableDataPort )
			{
				int adjDataPort = port;
				ret = executeDataPortCommand(_index, adjDataPort);
				if ( ret )
				{
					_dataPort = adjDataPort;
					updateConfigurationDict();
				}
			}
            return ret;
		}

		NbddcRateSet NbddcComponent::getRateSet() const
		{
			return _rateSet;
		}

		bool NbddcComponent::setRateSet(const NbddcRateSet& set)
		{
			_rateSet = set;
            return true;
		}

		BasicDoubleList NbddcComponent::getRateList() const
		{
			BasicDoubleList ret;
			for (NbddcRateSet::const_iterator it = _rateSet.begin(); it != _rateSet.end(); it++)
			{
				ret.push_back(it->second);
			}
			return ret;
		}

		void NbddcComponent::initConfigurationDict()
        {
            RadioComponent::initConfigurationDict();
            _config["rateIndex"] = "";
            _config["udpDestination"] = "";
            _config["vitaEnable"] = "";
            _config["streamId"] = "";
			_config["frequency"] = "";
			_config["source"] = "";
            if ( _selectableDataPort )
            {
            	_config["dataPort"] = "";
            }
        }

		void NbddcComponent::updateConfigurationDict()
        {
            this->debug("[NbddcComponent::updateConfigurationDict] Called\n");
            RadioComponent::updateConfigurationDict();
            setConfigurationValueToInt("rateIndex", _rateIndex);
            setConfigurationValueToInt("udpDestination", _udpDestination);
            setConfigurationValueToInt("vitaEnable", _vitaEnable);
            setConfigurationValueToUInt("streamId", _streamId);
            setConfigurationValueToDbl("frequency", _frequency);
            setConfigurationValueToInt("source", _source);
            if ( _selectableDataPort )
            {
                setConfigurationValueToInt("dataPort", _dataPort);
            }
            //this->debug("[NbddcComponent::updateConfigurationDict] Current configuration\n");
            //this->dumpConfiguration();
            this->debug("[NbddcComponent::updateConfigurationDict] Returning\n");
        }

        // Default implementation uses the NDR308 syntax.
        // NBDDC? <index>
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
                std::ostringstream oss;
                oss << "NBDDC? " << index << "\n";
                BasicStringList rsp = _parent->sendCommand(oss.str(), 2.0);
                if ( _parent->getLastCommandErrorInfo() == "" )
                {
                   BasicStringList vec = Pythonesque::Split(
                                             Pythonesque::Replace(rsp.front(), "NBDDC ", ""),
                                             ", ");
                   // vec[0] = Index
                   // vec[1] = Frequency
                   // vec[2] = Rate index
                   frequency = boost::lexical_cast<double>(vec[1]);
                   rateIndex = boost::lexical_cast<int>(vec[2]);
                   udpDestination = boost::lexical_cast<int>(vec[3]);
                   enabled = (boost::lexical_cast<int>(vec[4]) == 1);
                   vitaEnable = boost::lexical_cast<int>(vec[5]);
                   streamId = boost::lexical_cast<unsigned int>(vec[6]);
                   // NDR308 doesn't use NBDDC command to set the data source
                   ret = true;
                }
            }
            return ret;
		}

        // Default implementation uses the NDR308 syntax.
        // NBDDC <index>, <frequency>, <rate index>, <udp dest>, <enable>,
		// <vita enable>, <stream id>
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
                std::ostringstream oss;
                oss << "NBDDC " << index
                	<< ", " << frequency
                    << ", " << rateIndex
                    << ", " << udpDestination
                    << ", " << (enabled ? 1 : 0)
                    << ", " << vitaEnable
                    << ", " << streamId
                    << "\n";
                BasicStringList rsp = _parent->sendCommand(oss.str(), 2.0);
                if ( _parent->getLastCommandErrorInfo() == "" )
                {
                   ret = true;
                }
            }
            return ret;
		}

        // Default implementation returns false, since it is based on
		// the NDR308, which does not support tunable NBDDCs.
        bool NbddcComponent::executeFreqQuery(int index, double& freq)
        {
        	return false;
        }

        // Default implementation returns false, since it is based on
		// the NDR308, which does not support tunable NBDDCs.
        bool NbddcComponent::executeFreqCommand(int index, double& freq)
        {
        	return false;
        }

        // Default implementation is based on the NDR308, which sets
        // NBDDC source through the NBSS command.
        bool NbddcComponent::executeSourceQuery(int index, int& source)
        {
            bool ret = false;
            if ( (_parent != NULL) && (_parent->isConnected()) )
            {
                std::ostringstream oss;
                oss << "NBSS? " << index << "\n";
                BasicStringList rsp = _parent->sendCommand(oss.str(), 2.0);
                if ( _parent->getLastCommandErrorInfo() == "" )
                {
                   BasicStringList vec = Pythonesque::Split(
                                             Pythonesque::Replace(rsp.front(), "NBSS ", ""),
                                             ", ");
                   // vec[0] = Index
                   // vec[1] = Source
                   source = boost::lexical_cast<int>(vec[1]);
                   ret = true;
                }
            }
            return ret;
        }

        // Default implementation is based on the NDR308, which sets
        // NBDDC source through the NBSS command.
        bool NbddcComponent::executeSourceCommand(int index, int& source)
        {
            bool ret = false;
            if ( (_parent != NULL) && (_parent->isConnected()) )
            {
                std::ostringstream oss;
                oss << "NBSS " << index
                	<< ", " << source
                    << "\n";
                BasicStringList rsp = _parent->sendCommand(oss.str(), 2.0);
                if ( _parent->getLastCommandErrorInfo() == "" )
                {
                   ret = true;
                }
            }
            return ret;
        }

        // Default implementation uses the NDR308 syntax.
        // NBDP? <index>
        bool NbddcComponent::executeDataPortQuery(int index, int& dataPort)
        {
            bool ret = false;
            if ( (_parent != NULL) && (_parent->isConnected()) )
            {
                std::ostringstream oss;
                oss << "NBDP? " << index << "\n";
                BasicStringList rsp = _parent->sendCommand(oss.str(), 2.0);
                if ( _parent->getLastCommandErrorInfo() == "" )
                {
                   BasicStringList vec = Pythonesque::Split(
                                             Pythonesque::Replace(rsp.front(), "NBDP ", ""),
                                             ", ");
                   // vec[0] = Index
                   // vec[1] = Data Port
                   dataPort = boost::lexical_cast<int>(vec[1]);
                   ret = true;
                }
            }
            return ret;
        }

        // Default implementation uses the NDR308 syntax.
        // NBDP <index>, <data port>
        bool NbddcComponent::executeDataPortCommand(int index, int& dataPort)
        {
            bool ret = false;
            if ( (_parent != NULL) && (_parent->isConnected()) )
            {
                std::ostringstream oss;
                oss << "NBDP " << index
                    << ", " << dataPort
                    << "\n";
                BasicStringList rsp = _parent->sendCommand(oss.str(), 2.0);
                if ( _parent->getLastCommandErrorInfo() == "" )
                {
                   ret = true;
                }
            }
            return ret;
        }

    } // namespace Driver

} // namespace LibCyberRadio

