/*
 * NeuronalNetworkImageProcessSettings.h
 *
 *  Created on: 19.05.2018
 *      Author: Patrick Langer
 */

#ifndef HEADER_FILES_FILTERMANAGEMENTLIBRARY_NEURONALNETWORKIMAGEPROCESSSETTINGS_H_
#define HEADER_FILES_FILTERMANAGEMENTLIBRARY_NEURONALNETWORKIMAGEPROCESSSETTINGS_H_

#include <memory>
#include <string>

namespace FilterManagementLibrary
{
	namespace TFIntegration
	{

		struct TensorflowNNModelDescription
		{

			/**
			 * Specifys whether the input type of the neuronal network model
			 * is tensorflow::DT_UINT8 or tensorflow::DT_FLOAT (true means float,
			 * false means uint8). Others are not supported for now.
			 */
			bool input_floating = false;

			/**
			 * Only used if input_floating = true.
			 * As the default operations of the FilterManagementLibrary are
			 * implemented to use uint8_t, we need a way to convert those
			 * values to their float counterparts.
			 * For this, we need a mean and a std value for normalization.
			 * Formula is:
			 * float_value = (uint8_t_value - input_mean) / input_std;
			 */
			float input_mean = 127.5f; // default value for mobilenet v1 and v2

			/**
			 * Only used if input_floating = true.
			 * As the default operations of the FilterManagementLibrary are
			 * implemented to use uint8_t, we need a way to convert those
			 * values to their float counterparts.
			 * For this, we need a mean and a std value for normalization.
			 * Formula is:
			 * float_value = (uint8_t_value - input_mean) / input_std;
			*/
			float input_std = 127.5f; // default value for mobilenet v1 and v2

			/**
			 * Width of the input layer. Although if it may be flattened,
			 * Tensorflow provides functions to map tensors to access the data
			 * using x, y and z coordinates, which is what is used in
			 * TensorflowNNInstance::applyImageInputVector(...)
			 */
			int inputWidth;

			/**
			  * Height of the input layer. Although if it may be flattened,
			  * Tensorflow provides functions to map tensors to access the data
			  * using x, y and z coordinates, which is what is used in
			  * TensorflowNNInstance::applyImageInputVector(...)
			  */
			int inputHeight;

			/**
			  * Amount of channels of the input layer (channels(RGB) = 3)
			  * Although if it may be flattened,
			  * Tensorflow provides functions to map tensors to access the data
			  * using x, y and z coordinates, which is what is used in
			  * TensorflowNNInstance::applyImageInputVector(...)
			  */
			int channels = 3;

			/**
			 * Path to the protobuf model file which contains the model and
			 * where the Tensorflow graph is built from.
			 */
			std::string modelFile;

			/**
			 * Name of the input layer of the Neuronal Network model.
			 */
			std::string inputLayerNameStr;

			/**
			 * List of names of the output layers that shall be used
			 * during inference. For output interpretation, you can also
			 * use these names to get the corresponding output tensor
			 * from a {@link TensorflowResultContainer}.
			 * But if your network model has really silly or annoying
			 * 1042 character long names, be aware that you could also
			 * use an ID tou get the corresponding tensor form
			 * {@link TensorflowResultContainer}.
			 * The ID corresponds to the position of the
			 * layer in this vector, so be sure to remember it!
			 */
			std::vector<std::string> outputLayerNames;
		};
	}
}




#endif /* HEADER_FILES_FILTERMANAGEMENTLIBRARY_NEURONALNETWORKIMAGEPROCESSSETTINGS_H_ */
