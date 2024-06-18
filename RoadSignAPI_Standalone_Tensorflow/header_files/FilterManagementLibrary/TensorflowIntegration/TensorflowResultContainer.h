/*
 * ResultContainer.h
 *
 *  Created on: 30.05.2018
 *      Author: Patrick Langer
 */

#ifndef HEADER_FILES_FILTERMANAGEMENTLIBRARY_TENSORFLOWINTEGRATION_TENSORFLOWRESULTCONTAINER_H_
#define HEADER_FILES_FILTERMANAGEMENTLIBRARY_TENSORFLOWINTEGRATION_TENSORFLOWRESULTCONTAINER_H_

// Wrapper around Tensorflows output Tensor to
// provide some additional functionalities

#include <vector>

#include <tensorflow/core/platform/env.h>
#include <tensorflow/core/public/session.h>

namespace FilterManagementLibrary
{
	namespace TFIntegration
	{
		/**
		 * This class is a container for the results of a
		 * {@link TensorflowNNInstance}. It wraps the output tensors of
		 * an inference and their corresponding names so the internal
		 * structurization of those tensors can be hidden.
		 * It allows to get the Tensor of interest by name, so one
		 * does not have to remember the ID (which is the order
		 * of the output layer names provided in the
		 * {@link TensorflowNNModelDescription} used to create the
		 * {@link TensorflowNNInstance}.
		 *
		 */
		class TensorflowResultContainer
		{
			private:

				/**
				 * List of output tensors created by
				 * {@link TensorflowNNInstance} according to the
				 * output layer names wich were provided in
				 * {@link TensorflowNNModelDescription}.
				 * {@link TensorflowNNInstance} assures that the order
				 * of output layer names provided matches the order of
				 * the output tensors.
				 * So the ID of the first output layer name in the
				 * model description is also the ID of the corresponding
				 * output tensor in this vector.
				 */
				std::vector<tensorflow::Tensor>* outputTensors;

				/**
				  * List of output tensors provided by the
				  * {@link TensorflowNNModelDescription} of
				  * the corresponding {@link TensorflowNNInstancce}.
				  * The order of the tensor names here matches the
				  * order of the tensor names in the model description!
				  * So the ID of the first output layer name in the
				  * model description is also the ID of the corresponding
				  * output tensor in this vector.
				  */
				std::vector<std::string> outputTensorNames;

			public:
				TensorflowResultContainer
				(std::vector<tensorflow::Tensor>* outputTensors,
						std::vector<std::string> outputTensorNames);

				tensorflow::Tensor* getOutputTensorByID(int id) const;
				tensorflow::Tensor* getOutputTensorByLayerName(
						std::string layerName) const;
		};
	}
}


#endif /* HEADER_FILES_FILTERMANAGEMENTLIBRARY_TENSORFLOWINTEGRATION_TENSORFLOWRESULTCONTAINER_H_ */
