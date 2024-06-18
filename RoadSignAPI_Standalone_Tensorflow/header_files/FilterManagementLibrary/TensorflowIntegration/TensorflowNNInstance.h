/*
 * TensorflowNNInstance.h
 *
 *  Created on: 19.05.2018
 *      Author: Patrick Langer
 */

#ifndef HEADER_FILES_TENSORFLOWNNINSTANCE_H_
#define HEADER_FILES_TENSORFLOWNNINSTANCE_H_


#include <tensorflow/core/platform/env.h>
#include <tensorflow/core/public/session.h>

#include <cstdint>

#include <string>
#include "TensorflowNNModelDescription.h"
#include "TensorflowResultContainer.h"

// If we're under Android, we want to provide functionality to load
// network models from the assets folder so they don't hav to be
// placed into normal user space.
#ifdef __ANDROID__
	#include "FilterManagementLibrary/TensorflowAndroidJNIUtils.h"
#endif

namespace FilterManagementLibrary
{
	/**
	 * Provides functionalitys to load Tensorflow models and use them
	 * to do calculations / perform operations, interprete their output
	 * and so on.
	 */
	namespace TFIntegration
	{
		/**
		 * Provides all the functionality needed to load tensorflow models,
		 * add them to a tensorflow session, provide an input vector, functions
		 * to run inferences and methods to process the output.
		 */
		class TensorflowNNInstance
		{
			public:

				enum class ErrorType
				{
					/**
					  * Standard value set on initialization.
					  * Needed because getLastError() shall not return a
					  * random value (which could cause unexpected behavior)
					  */
					ERROR_NONE,

					/**
					 * If the model file does not exist at the given path.
					 */
					ERROR_INVALID_MODEL_FILE,

					/**
					 * Tensorflow related error. Thrown, if the NewSession()
					 * function in setupModel() fails.
					 */
					ERROR_FAILED_TO_CONSTRUCT_NEW_SESSION,

					// If the given file could not be loaded
					// (wrong format, missing permissions etc.)
					/**
					 * If the given model file could not be loaded because it
					 * is in wrong format or we do not have sufficient
					 * permission to read it.
					 */
					ERROR_COULD_NOT_LOAD_MODEL,

					// If Session::Create(GraphDef) fails
					/**
					 * Tensorflow related error. Thrown, if the function
					 * Create(...) of the Tensorflow Session fails with the
					 * graph definition build from the model file.
					 */
					ERROR_COULD_NOT_ADD_GRAPH_TO_SESSION,

					/**
					 * Tensorflow related error. Thrown, if we could not run
					 * the session (maybe incorrect model file, missing ops
					 * in the Tensorflow library or any other
					 * configuration error).
					 */
					ERROR_SESSION_RUN_FAILED,

					/**
					 * If the dimensions of the given input vector do not
					 * match with the input sized in the provided
					 * {@link TensorflowNNModelDescription}.
					 */
					ERROR_INPUT_SIZE_MISMATCH,

					/**
					 * Only available under Android environments.
					 * If using the setupModelFromAssets(...) function,
					 * but the modelFile provided in
					 * {@link TensorflowNNModelDescription} could not be
					 * loaded (for example is not found in the assets folder).
					 */
					#ifdef __ANDROID__
					READ_FILE_TO_PROTO_FROM_ASSETS_FAILED,
					#endif

					// If the specified labels file does not exist
					/**
					 * Only relevant for {@link TensorflowNNInstanceClassifier}
					 * if the specified labels file does not exist and thus
					 * could not be loaded.
					 */
					ERROR_TFNN_CLASSIFIER_INVALID_LABELS_FILE,
				};

			private:

				/**
				 * Everything we need to load, build and run the model.
				 * See {@link TensorflowNNModelDescription} for a detailed
				 * description.
				 */
				TensorflowNNModelDescription nnModelDescription;


				/**
				 * Number of threads Tensorflow is allowed to use.
				 * Standard value is 1, but can be set via constructor.
				 */
				int numThreads = 1;



			protected:

				/**
				 * Tensor where our input is copied to.
				 */
				tensorflow::Tensor inputTensor;

				/**
				 * Stores the result of the inference
				 */
				std::vector<tensorflow::Tensor> outputTensors;

				/**
				 * Stores and holds the instance of the used model
				 */
				std::unique_ptr<tensorflow::Session> tensorflowSession;

				/**
				 * Options for the session holding the model
				 */
				tensorflow::SessionOptions sessionOptions;

				/**
				 * Tensorflow graph (built from the model)
				 */
				tensorflow::GraphDef graphDef;

				/**
				 * Name of the input layer built from
				 * NeuronalNetwork
				 */
				std::unique_ptr<std::string> inputLayerName;


				/**
				 * Error code of the last error that happened.
				 * See TensorflowNNInstance::ErrorType
				 */
				ErrorType lastError;


			public:


				bool setupModelFromFile();

				#ifdef __ANDROID__
					bool setupModelFromAssets(
							AAssetManager* const assetManager);
				#endif

				bool runInference();

				bool applyImageInputVector(uint8_t*** inputVector,
							int imageHeight, int imageWidth, int channels);

				bool applyInputVectorFromFloatData(float* floatData, const int heigth,
						const int width, const int channels);

				void adjustModelFile(std::string modelFile);

				tensorflow::Tensor *getInputTensor();
				std::vector<tensorflow::Tensor>* getOutputTensors();

				TensorflowResultContainer getResultContainer();

				const TensorflowNNModelDescription* getModelDescription() const;

				ErrorType getLastError() const;

				TensorflowNNInstance(
						TensorflowNNModelDescription nnModelDescription,
						int numThreads = 1);

				~TensorflowNNInstance();




		};
	}

}

#endif /* HEADER_FILES_TENSORFLOWNNINSTANCE_H_ */
