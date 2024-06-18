/*
 * TensorflowNNInstance.cpp
 *
 *  Created on: 19.05.2018
 *      Author: oracion
 */


#include "FilterManagementLibrary/TensorflowIntegration/TensorflowNNInstance.h"

#include "FilterManagementLibrary/Utilities.h"
#include "FilterManagementLibrary/TensorflowIntegration/image_helpers/image_helpers.h"



FilterManagementLibrary::TFIntegration::TensorflowNNInstance::
	TensorflowNNInstance(std::string modelFile,
			NeuronalNetworkImageProcessSettings imageProcessSettings,
			int numThreads, bool useNNAPI):
	inputVector(NULL), modelFile(modelFile), numThreads(numThreads),
	useNNAPI(useNNAPI), lastError(ErrorType::ERROR_NONE)
{

}

bool FilterManagementLibrary::TFIntegration::TensorflowNNInstance::setupModel()
{
	if(Utilities::fileExists(this->modelFile))
	{

		this->model =
				tflite::FlatBufferModel::BuildFromFile(this->modelFile.c_str(),
						NULL);

		if (this->model)
		{

			this->model->error_reporter();

			tflite::InterpreterBuilder(*model, this->resolver)
				(&this->tfLiteInterpreter);

			if (this->tfLiteInterpreter)
			{
				this->tfLiteInterpreter->SetNumThreads(this->numThreads);
				this->tfLiteInterpreter->UseNNAPI(this->useNNAPI);
				if (this->tfLiteInterpreter->AllocateTensors() == kTfLiteOk)
				{
					return true;
				}
				else
				{
					this->lastError = TensorflowNNInstance::ErrorType::
							ERROR_NN_FAILED_TO_ALLOCATE_TENSORS;
					return false;
				}

			}
			else
			{
				this->lastError = TensorflowNNInstance::ErrorType::
						ERROR_FAILED_TO_CONSTRUCT_INTERPRETER;
				return true;
			}
		  }
		  else
		  {
			  this->lastError = TensorflowNNInstance::ErrorType::
					  ERROR_COULD_NOT_MMAP_MODEL;
			  return false;
		  }
	}
	else
	{
		this->lastError = TensorflowNNInstance::ErrorType::
				ERROR_INVALID_MODEL_FILE;
		return false;
	}
}

bool FilterManagementLibrary::TFIntegration::TensorflowNNInstance::
applyImageInputVector(uint8_t* inputVector,
							int imageWidth, int imageHeight, int channels)
{

	// get input dimension from the input tensor metadata
	// assuming one input only

	int input = tfLiteInterpreter->inputs()[0];
	TfLiteIntArray* dims = tfLiteInterpreter->tensor(input)->dims;
	int wanted_height = dims->data[1];
	int wanted_width = dims->data[2];
	int wanted_channels = dims->data[3];

	printf("6\n");

	switch (this->tfLiteInterpreter->tensor(input)->type)
	{
		case kTfLiteFloat32:
	    {
	    	imageProcessSettings.input_floating = true;
	    	resize<float>(tfLiteInterpreter->typed_tensor<float>(input),
	    			inputVector, imageHeight,
	            imageWidth, channels, wanted_height, wanted_width,
				wanted_channels, &imageProcessSettings);
	    	return true;
	    }
	    break;
	    case kTfLiteUInt8:
	    {
	    	imageProcessSettings.input_floating = false;
	        resize<uint8_t>(tfLiteInterpreter->typed_tensor<uint8_t>(input),
	        		inputVector, imageHeight,
					imageWidth, channels, wanted_height, wanted_width,
	        		wanted_channels, &imageProcessSettings);
	        return true;

	    }
	    break;
	    default:
	    {
	    	this->lastError = TensorflowNNInstance::ErrorType::
	    			ERROR_NN_UNSUPPORTED_INPUT_TYPE;
	    	return false;
	    }
	    printf("7\n");
	}
}


bool FilterManagementLibrary::TFIntegration::TensorflowNNInstance::runInference()
{
	 if (this->tfLiteInterpreter->Invoke() == kTfLiteOk)
	 {
		 return true;
	 }
	 else
	 {
		 this->lastError = TensorflowNNInstance::ErrorType::
				 ERROR_NN_INVOKE_FAILED;
		 return false;
	 }
}


