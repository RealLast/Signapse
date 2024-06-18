/*
 * NeuronalNetworkImageProcessSettings.h
 *
 *  Created on: 19.05.2018
 *      Author: oracion
 */

#ifndef HEADER_FILES_FILTERMANAGEMENTLIBRARY_NEURONALNETWORKIMAGEPROCESSSETTINGS_H_
#define HEADER_FILES_FILTERMANAGEMENTLIBRARY_NEURONALNETWORKIMAGEPROCESSSETTINGS_H_

namespace FilterManagementLibrary
{
	namespace TFIntegration
	{
		struct NeuronalNetworkImageProcessSettings
		{
			  bool input_floating = false; // false means uint8_t, true means float
			  float input_mean = 127.5f; // default value for mobilenet v1 and v2
			  float input_std = 127.5f; // default value for mobilenet v1 and v2
		};
	}
}




#endif /* HEADER_FILES_FILTERMANAGEMENTLIBRARY_NEURONALNETWORKIMAGEPROCESSSETTINGS_H_ */
