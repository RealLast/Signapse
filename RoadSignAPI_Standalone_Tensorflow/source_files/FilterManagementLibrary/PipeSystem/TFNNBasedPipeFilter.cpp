/*
 * TFNNBasedPipeFilter.cpp
 *
 *  Created on: 03.06.2018
 *      Author: Patrick Langer
 */


#include "FilterManagementLibrary/PipeSystem/TFNNBasedPipeFilter.h"
#include "FilterManagementLibrary/Logger.h"


/**
 * @brief Standard constructor of TFNNBasedPipeFilter
 *
 * The TFNNBasedPipeFilter will use a TensorflowNNInstance to load and instantiate
 * Tensorflow model (and a corresponding session). For this, it needs
 * to tell the TensorflowNNInstance a description of a model to use
 * (TensorflowNNModelDescription) and how many threads Tensorflow is allowed
 * to use for computation.
 *
 * @param TensorflowNNModelDescription nnModelDescription a description of
 * the Tensorflow model which will be used by the filter.
 *
 * @param int numThreads number of threads Tensorflow is allowed to use
 * for computation.
 */
FilterManagementLibrary::PipeSystem::TFNNBasedPipeFilter::TFNNBasedPipeFilter(
		TFIntegration::TensorflowNNModelDescription nnModelDescription,
		int numThreads) : tfNNInstance(nnModelDescription, numThreads)
{

}


/**
 * @brief Applies an image vector to the TensorflowNNInstance
 *
 * This takes a 3D uint8_t vector and will call the TensorflowNNInstance's
 * applyImageInputVector(...)function accordingly, which will set the data on
 * the input Tensor of the model. It originally was intended to be used
 * for 3 Channel RGB images, as the TensorflowNNInstance's function
 * may scale the input as needed (i.e. normalization using floats and
 * a mean and std value).
 *
 * @param uint8_t*** image 3D array of uint8_t values which will be
 * applied to the model
 *
 * @param int height height of the image (columns of the vector)
 * @param int width width of the image (rows of the vector)
 * @param int channels channels of the image (depth of the vector)
 *
 * @return bool true if the vector was successfully applied to the model,
 * false otherwise
 */
bool FilterManagementLibrary::PipeSystem::
	TFNNBasedPipeFilter::applyNNImageInputVector(uint8_t*** image,
				const int height, const int width,
				const int channels)
{
	return this->tfNNInstance.
			applyImageInputVector(image,height, width, channels);
}

/**
 * @brief Let's the neuronal network evaluate the given input.
 *
 * An input vector should have been applied to the neuronal network
 * (the TensorflowNNInstance) beforehand, by any means.
 * This will call the TensorflowNNInstance's runInference() function
 * accordingly and will call the onNNEvaluationFinished() callback function
 * on success (synchronously).
 *
 * @return bool true if the inference (TensorflowNNInstance) was successfull,
 * false otherwise.
 */
bool FilterManagementLibrary::PipeSystem::
	TFNNBasedPipeFilter::evaluateInputVectorByNN()
{
	if(this->tfNNInstance.runInference())
		{
			//FilterManagementLibrary::Logger::printfln("Inference success\n");
			this->onNNEvaluationFinished(this->tfNNInstance.getResultContainer());
			return true;
		}
		else
		{
			FilterManagementLibrary::Logger::printfln("Inference failed\n");
			return false;
		}
}

/**
 * @brief Returns the input tensor of the underlaying neuronal network.
 *
 * As we want to hide the object of TensorflowNNInstance from a
 * derived filter, we provide this function to directly access the input tensor
 * of the underlaying tensorflow neuronal network.
 *
 * @return tensorflow::Tensor* pointer to the input tensor
 */
tensorflow::Tensor* FilterManagementLibrary::PipeSystem::
						TFNNBasedPipeFilter::getNNInputTensor()
{
	return this->tfNNInstance.getInputTensor();
}


/**
 * @brief Returns the model description of the underlaying neuronal network.
 *
 * As we want to hide the object of TensorflowNNInstance from a
 * derived filter, we provide this function to a const pointer
 * to the model description of the underlaying neuronal network.
 *
 * @return FilterManagementLibrary::TFIntegration::TensorflowNNModelDescription*
 * pointer to the model description.
 */
const FilterManagementLibrary::TFIntegration::
				TensorflowNNModelDescription*
				FilterManagementLibrary::
				PipeSystem::TFNNBasedPipeFilter::getNNModelDescription() const
{
	return this->tfNNInstance.getModelDescription();
}

/**
 * @brief Set's up the Tensorflow model of the TensorflowNNInstance.
 *
 * This will call TensorflowNNInstance's setupModelFromFile() function
 * accordingly, which will try to load and build the Tensorflow model
 * from the given TensorflowNNModelDescription (passed to constructor)
 *
 * @return bool true if network model was setup successfully, false otherwise
 */
bool FilterManagementLibrary::PipeSystem::
	TFNNBasedPipeFilter::setupModelFromFile()
{

	if(this->tfNNInstance.setupModelFromFile())
	{
		return true;
	}
	else
	{

		switch(this->tfNNInstance.getLastError())
		{
			case FilterManagementLibrary::TFIntegration::
				TensorflowNNInstance::ErrorType::ERROR_INVALID_MODEL_FILE:
			{
				Logger::printfln("Setting up model failed:\
						 Model file not found");
			}
			break;
			case FilterManagementLibrary::TFIntegration::
				TensorflowNNInstance::ErrorType::ERROR_COULD_NOT_LOAD_MODEL:
			{
				Logger::printfln("Setting up model failed: Model could not be\
						 loaded (invalid format or missing permissions");

			}
			break;
			case FilterManagementLibrary::TFIntegration::TensorflowNNInstance::
				ErrorType::ERROR_COULD_NOT_ADD_GRAPH_TO_SESSION:
			{
				Logger::printfln("Setting up model failed: \
										Could not add graph to session");
			}
			break;
			case FilterManagementLibrary::TFIntegration::TensorflowNNInstance::
				ErrorType::ERROR_FAILED_TO_CONSTRUCT_NEW_SESSION:
			{
				Logger::printfln("Setting up model failed: \
										Failed to add graph to session.");
			}
			break;
		}
		return false;
	}
}

#ifdef __ANDROID__
/**
 * @brief Set's up the Tensorflow model of the TensorflowNNInstance.
 *
 * CAUTION: THIS FUNCTION IS ONLY AVAILABLE UNDER ANDROID ENVIRONMENTS!
 * This will call TensorflowNNInstance's setupModelFromAssets() function
 * accordingly, which will try to load and build the Tensorflow model
 * from the given TensorflowNNModelDescription (passed to constructor).
 * In contrast to setupModelFromFile(), the modelFile given in the
 * {@link TensorflowNNModelDescription} will be threated as an android
 * assets file and loaded via an AssetsManager which needs to be passed
 * to this function.
 *
 * @param const AAssetManager* assetManager Pointer to an AssetManager which,
 * by any means, needs to to be passed from the Java side of the code.
 *
 * @return bool true if network model was setup successfully, false otherwise
 */
bool FilterManagementLibrary::PipeSystem::
	TFNNBasedPipeFilter::setupModelFromAssets(
			AAssetManager* const assetManager)
{

	if(this->tfNNInstance.setupModelFromAssets(assetManager))
	{
		return true;
	}
	else
	{

		switch(this->tfNNInstance.getLastError())
		{
			case FilterManagementLibrary::TFIntegration::
				TensorflowNNInstance::ErrorType::ERROR_INVALID_MODEL_FILE:
			{
				Logger::printfln("Setting up model failed:\
						 Model file not found");
			}
			break;
			case FilterManagementLibrary::TFIntegration::
				TensorflowNNInstance::ErrorType::ERROR_COULD_NOT_LOAD_MODEL:
			{
				Logger::printfln("Setting up model failed: Model could not be\
						 loaded (invalid format or missing permissions");

			}
			break;
			case FilterManagementLibrary::TFIntegration::TensorflowNNInstance::
				ErrorType::ERROR_COULD_NOT_ADD_GRAPH_TO_SESSION:
			{
				Logger::printfln("Setting up model failed: \
										Could not add graph to session");
			}
			break;
			case FilterManagementLibrary::TFIntegration::TensorflowNNInstance::
				ErrorType::ERROR_FAILED_TO_CONSTRUCT_NEW_SESSION:
			{
				Logger::printfln("Setting up model failed: \
										Failed to add graph to session.");
			}
			break;
		}
		return false;
	}
}
#endif

/**
 * @brief Allows to change ONLY the model file path set in the model description.
 *
 * Can be used to adjust the path of the model file of the
 * TensorflowNNModelDescription at run-time.
 * TensorflowNNInstance's adjustModelFile(...) function will
 * be called accordingly.
 *
 * @param std::string new path to the modelFile to use.
 * @return void
 */
void FilterManagementLibrary::PipeSystem::
	TFNNBasedPipeFilter::adjustModelFile(std::string modelFile)
{
	this->tfNNInstance.adjustModelFile(modelFile);
}

/**
 * @brief Returns the last error that happened concerning the neuronal instance
 *
 * @return TensorflowNNInstance::ErrorType the enum value of the last error
 * that happened concering the TensorflowNNInstance
 */
FilterManagementLibrary::TFIntegration::TensorflowNNInstance::ErrorType
		FilterManagementLibrary::PipeSystem::TFNNBasedPipeFilter::
		getNeuralNetLastError() const
{
	return this->tfNNInstance.getLastError();
}

/**
 * @brief Destructor of TFNNBasedPipeFilter. Does nothing by now.
 */
FilterManagementLibrary::PipeSystem::
			TFNNBasedPipeFilter::~TFNNBasedPipeFilter()
{

}

