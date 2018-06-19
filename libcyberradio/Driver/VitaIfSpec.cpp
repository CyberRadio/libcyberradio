/***************************************************************************
 * \file VitaIfSpec.cpp
 * \brief Defines the VITA interface specification for an NDR-class radio.
 * \author DA
 * \author NH
 * \author MN
 * \copyright (c) 2017 CyberRadio Solutions, Inc.  All rights reserved.
 *
 ***************************************************************************/

#include "LibCyberRadio/Driver/VitaIfSpec.h"

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
		VitaIfSpec::VitaIfSpec(int headerSizeWords,
				               int payloadSizeWords,
							   int tailSizeWords,
							   const char* byteOrder,
				               bool iqSwapped) :
			headerSizeWords(headerSizeWords),
			payloadSizeWords(payloadSizeWords),
			tailSizeWords(tailSizeWords),
			byteOrder(byteOrder),
			iqSwapped(iqSwapped)
		{
		}

		VitaIfSpec::~VitaIfSpec()
		{
		}

		VitaIfSpec::VitaIfSpec(const VitaIfSpec& other) :
			headerSizeWords(other.headerSizeWords),
			payloadSizeWords(other.payloadSizeWords),
			tailSizeWords(other.tailSizeWords),
			byteOrder(other.byteOrder),
			iqSwapped(other.iqSwapped)
		{
		}

		VitaIfSpec& VitaIfSpec::operator=(const VitaIfSpec& other)
		{
			if ( this != &other )
			{
				headerSizeWords = other.headerSizeWords;
				payloadSizeWords = other.payloadSizeWords;
				tailSizeWords = other.tailSizeWords;
				byteOrder = other.byteOrder;
				iqSwapped = other.iqSwapped;
			}
			return *this;
		}

    } /* namespace Driver */

} /* namespace LibCyberRadio */

