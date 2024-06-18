/*
 * image_helpers.h
 *
 *  Created on: 14.05.2018
 *      Author: oracion
 *      Code taken and adapted from tensorflowlite label_image example.
 */

#ifndef IMAGE_HELPERS_IMAGE_HELPERS_H_
#define IMAGE_HELPERS_IMAGE_HELPERS_H_

#include "image_helpers_impl.h"

namespace FilterManagementLibrary
{
	namespace TFIntegration
	{

		template<class T>
		void resize(T* out, uint8_t* in, int image_height, int image_width,
				int image_channels, int wanted_height, int wanted_width,
				int wanted_channels, NeuronalNetworkImageProcessSettings* s);

		// explicit instantiation
		template void resize<uint8_t>(uint8_t*, unsigned char*, int, int, int,
				int, int, int, NeuronalNetworkImageProcessSettings*);
		template void resize<float>(float*, unsigned char*, int, int, int, int,
				int, int, NeuronalNetworkImageProcessSettings*);
	}
}

#endif /* IMAGE_HELPERS_IMAGE_HELPERS_H_ */
