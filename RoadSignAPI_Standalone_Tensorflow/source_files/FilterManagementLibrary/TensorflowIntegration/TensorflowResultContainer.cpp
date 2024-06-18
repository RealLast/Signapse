/*
 * TensorflowResultContainer.cpp
 *
 *  Created on: 03.06.2018
 *      Author: Patrick Langer
 */

#include "FilterManagementLibrary/TensorflowIntegration/TensorflowResultContainer.h"
#include <assert.h>


/**
 * @brief Constructor of TensorflowResultContainer
 * All information / data contained in an objecct of type
 * TensorflowResultContainer will be set using this constructor.
 * It does not contain any other class member variables!
 *
 * @param std::vector<tensorflow::Tensor>* outputTensors the list of
 * output tensors of the corresponding inference
 * (the Tensors containing the result)
 *
 * @param std::vector<std::string> outputTensorNames layer names of the
 * output tensors.
 * The names *HAVE* to be in the same order as the
 * outputTensors vector! Otherwise a wrong tensor would be returned when
 * using the getOutTensorByLayerName(...) function.
 */
FilterManagementLibrary::TFIntegration::
					TensorflowResultContainer::TensorflowResultContainer(
							std::vector<tensorflow::Tensor>* outputTensors,
							std::vector<std::string> outputTensorNames):
							outputTensors(outputTensors), outputTensorNames(outputTensorNames)
{

}


/**
 * @brief Returns a pointer to the output Tensor with the given ID.
 *
 * @return tensorflow::Tensor the output Tensor with the given ID.
 */
tensorflow::Tensor* FilterManagementLibrary::
	TFIntegration::TensorflowResultContainer::getOutputTensorByID(int id) const
{
	return &this->outputTensors->at(id);
}

/**
 * @brief Returns a pointer to the output Tensor at the given layer name.
 *
 * @return tensorflow::Tensor the output Tensor at the given layer name.
 */
tensorflow::Tensor* FilterManagementLibrary::TFIntegration::
	TensorflowResultContainer::getOutputTensorByLayerName(
			std::string layerName) const
{
	// Calculate index in vector by name
	ptrdiff_t pos = std::distance(this->outputTensorNames.begin(),
			std::find(this->outputTensorNames.begin(),
					this->outputTensorNames.end(), layerName));

	assert (pos < this->outputTensorNames.size());

	return &this->outputTensors->at(pos);

}
