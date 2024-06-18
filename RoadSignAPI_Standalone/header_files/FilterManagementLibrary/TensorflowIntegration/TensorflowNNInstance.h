/*
 * TensorflowNNInstance.h
 *
 *  Created on: 19.05.2018
 *      Author: oracion
 */

#ifndef HEADER_FILES_TENSORFLOWNNINSTANCE_H_
#define HEADER_FILES_TENSORFLOWNNINSTANCE_H_


#include "FilterManagementLibrary/TensorflowIntegration/NeuronalNetworkImageProcessSettings.h"

#include "tensorflow/contrib/lite/interpreter.h"

#include "tensorflow/contrib/lite/kernels/register.h"
#include "tensorflow/contrib/lite/model.h"
#include "tensorflow/contrib/lite/optional_debug_tools.h"
#include "tensorflow/contrib/lite/string_util.h"

#include <cstdint>

#include <string>

namespace FilterManagementLibrary
{
	namespace TFIntegration
	{
		class TensorflowNNInstance
		{
			public:

				enum class ErrorType
				{
					ERROR_NONE,

					// If specified file does not exist.
					ERROR_INVALID_MODEL_FILE,

					// If the given file is of wrong format (i.e. not *.tflite)
					ERROR_COULD_NOT_MMAP_MODEL,
					ERROR_FAILED_TO_CONSTRUCT_INTERPRETER,

					// If the given network model uses an input type
					// we don't support / did not consider.
					ERROR_NN_UNSUPPORTED_INPUT_TYPE,

					// If the given network model uses an output type
					// we don't support / did not consider.
					ERROR_NN_UNSUPPORTED_OUTPUT_TYPE,

					// If interpreter.AllocateTensors() fails
					ERROR_NN_FAILED_TO_ALLOCATE_TENSORS,

					// If interpreter.invoke() fails
					ERROR_NN_INVOKE_FAILED,

					// ====== TensorflowNNIntegrationClassifier ====== //

					// If the specified labels file does not exist
					ERROR_TFNN_CLASSIFIER_INVALID_LABELS_FILE,
				};

			private:

				NeuronalNetworkImageProcessSettings imageProcessSettings;



				bool useNNAPI = false;
				int numThreads = 1;
				std::string modelFile;

				uint8_t* inputVector;

			protected:
				std::unique_ptr<tflite::FlatBufferModel> model;
				std::unique_ptr<tflite::Interpreter> tfLiteInterpreter;
				tflite::ops::builtin::BuiltinOpResolver resolver;

				ErrorType lastError;

			public:


				bool setupModel();

				bool runInference();

				bool applyImageInputVector(uint8_t* inputVector,
								int imageWidth, int imageHeight, int channels);

				TensorflowNNInstance(std::string modelFile,
					NeuronalNetworkImageProcessSettings imageProcessSettings,
					int numThreads = 1, bool useNNAPI = false);


		};
	}

}

#endif /* HEADER_FILES_TENSORFLOWNNINSTANCE_H_ */
