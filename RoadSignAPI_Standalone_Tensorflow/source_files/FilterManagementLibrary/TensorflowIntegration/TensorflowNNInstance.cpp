/*
 * TensorflowNNInstance.cpp
 *
 *  Created on: 19.05.2018
 *      Author: Patrick Langer
 *
 *      Supports only one input layer, but multiple output layers by now.
 */



#include "FilterManagementLibrary/TensorflowIntegration/TensorflowNNInstance.h"

#include "FilterManagementLibrary/Utilities.h"

#include "FilterManagementLibrary/Logger.h"

/**
 * @brief Constructor of TensorflowNNInstance.
 *
 * This is the only constructor the TensorflowNNInstance has, and
 * the only one that should be allowed!
 * As an object of type TensorflowNNInstance is useless without a
 * TensorflowNNModelDescription, it is MANDATORY to have a constructor
 * to which an object of that type is passed.
 * Additionally, it needs to be specified how many threads
 * Tensorflow is allowed to use for computation!
 * An Tensor (of type tensorflow::Tensor), which is used as input Tensor,
 * of type specified in the model description will be created
 * (for now, only uint8_t and float are supported!)
 * All other class members will be initialised to their default values.
 *
 * @param TensorflowNNModelDescription nnModelDescription the model description
 * containing all information needed to load and use the Tensorflow model.
 *
 * @param int numThreads number of threads Tensorflow is allowed to use.
 */
FilterManagementLibrary::TFIntegration::TensorflowNNInstance::
	TensorflowNNInstance(
			TensorflowNNModelDescription nnModelDescription,
			int numThreads):
	numThreads(numThreads),
	lastError(ErrorType::ERROR_NONE), nnModelDescription(nnModelDescription),
	outputTensors()
{
	if(nnModelDescription.input_floating)
	{
		tensorflow::Tensor inputTensor(tensorflow::DT_FLOAT,
							tensorflow::TensorShape({1,
							this->nnModelDescription.inputHeight,
							this->nnModelDescription.inputWidth,
							this->nnModelDescription.channels}));
			this->inputTensor = inputTensor;
	}
	else
	{
		tensorflow::Tensor inputTensor(tensorflow::DT_UINT8,
							tensorflow::TensorShape({1,
							this->nnModelDescription.inputHeight,
							this->nnModelDescription.inputWidth,
							this->nnModelDescription.channels}));
			this->inputTensor = inputTensor;
	}

}

/**
 * @brief Sets up the Tensorflow model using the provieded model description.
 *
 * Tries to load the Tensorflow model from the file specified in the
 * TensorflowNNModelDescription which is passed to the constructor.
 * If the file could be loaded, a graph is built from it and a new
 * Tensorflow Session will be created.
 * On failure, lastError will be set accordingly.
 * @bool if the model was loaded and setup successfully, false otherwise
 */
bool FilterManagementLibrary::TFIntegration::
		TensorflowNNInstance::setupModelFromFile()
{
	if(Utilities::fileExists(this->nnModelDescription.modelFile))
	{
		// Proto configs
		tensorflow::ConfigProto& configProto = this->sessionOptions.config;

		tensorflow::Session* session;
		tensorflow::Status status = NewSession(this->sessionOptions, &session);

		if(!status.ok())
		{
			this->lastError = TensorflowNNInstance::ErrorType::
					ERROR_FAILED_TO_CONSTRUCT_NEW_SESSION;

			FilterManagementLibrary::Logger::
				printfln(status.ToString().c_str());

			return false;
		}


		tensorflow::Env* env = tensorflow::Env::Default();

		this->inputLayerName.reset(&this->nnModelDescription.inputLayerNameStr);
		//this->outputLayerName.reset(&this->nnModelDescription.outputLayerNameStr);

		this->tensorflowSession.reset(session);

		status = tensorflow::ReadBinaryProto(env,
				this->nnModelDescription.modelFile, &this->graphDef);

		if(!status.ok())
		{
			this->lastError = TensorflowNNInstance::ErrorType::
					ERROR_COULD_NOT_LOAD_MODEL;

			FilterManagementLibrary::Logger::
				printfln(status.ToString().c_str());
			return false;
		}

		status = this->tensorflowSession->Create(this->graphDef);

		if(!status.ok())
		{

			//*debugString = status.ToString();
			this->lastError = TensorflowNNInstance::ErrorType::
					ERROR_COULD_NOT_ADD_GRAPH_TO_SESSION;

			FilterManagementLibrary::Logger::
				printfln(status.ToString().c_str());
			return false;
		}

		// Setup was successfull!

		// clear graph to save memory
		this->graphDef.Clear();
		return true;
	}
	else
	{
		this->lastError = TensorflowNNInstance::ErrorType::
				ERROR_INVALID_MODEL_FILE;
		return false;
	}
}

#ifdef __ANDROID__
/**
 * @brief Sets up the Tensorflow model using the provieded model description.
 *
 * In contrast to setupModelFromFile(), this functions interpretes the
 * modelFile specified in the {@link TensorflowNNModelDescriotion} as an
 * Android Assets file and thus tries to load the model from the Assets
 * belonging to the App. Of course, this is only available under Android
 * environments.
 * If the file could be loaded, a graph is built from it and a new
 * Tensorflow Session will be created.
 * On failure, lastError will be set accordingly.
 * @bool if the model was loaded and setup successfully, false otherwise
 */
bool FilterManagementLibrary::TFIntegration::
		TensorflowNNInstance::setupModelFromAssets(
				AAssetManager* const assetManager)
{

		// Proto configs
		tensorflow::ConfigProto& configProto = this->sessionOptions.config;

		tensorflow::Session* session;
		tensorflow::Status status = NewSession(this->sessionOptions, &session);

		if(!status.ok())
		{
			this->lastError = TensorflowNNInstance::ErrorType::
					ERROR_FAILED_TO_CONSTRUCT_NEW_SESSION;

			FilterManagementLibrary::Logger::
				printfln(status.ToString().c_str());

			return false;
		}


		tensorflow::Env* env = tensorflow::Env::Default();

		this->inputLayerName.reset(&this->nnModelDescription.inputLayerNameStr);
		//this->outputLayerName.reset(&this->nnModelDescription.outputLayerNameStr);

		this->tensorflowSession.reset(session);

		std::string modelFileInAssets = "file:///android_asset/";
		modelFileInAssets.append(this->nnModelDescription.modelFile);
		FilterManagementLibrary::TensorflowAndroidJNIUtils::
		  ReadFileToProto(assetManager,
				  modelFileInAssets, &this->graphDef);


		  LOG(INFO) << "Creating session.";
		  tensorflow::Status s = session->Create(this->graphDef);

	      if(!s.ok())
		  {
		  	this->lastError = TensorflowNNInstance::ErrorType::
		  		ERROR_COULD_NOT_ADD_GRAPH_TO_SESSION;
		  	FilterManagementLibrary::Logger::
		  		printfln(status.ToString().c_str());
		  	return false;
		  }

		// Setup was successfull!

		// clear graph to save memory
		this->graphDef.Clear();
		return true;
}
#endif

/**
 * @brief Applies an image vector to the instance of the Tensorflow model.
 *
 * This takes a 3D uint8_t vector and will set the data on
 * the input Tensor of the model. It is intended to be used
 * for 3 Channel RGB images. If the model expects float as input type instead,
 * the uint8_t values will be converted accordingly using the
 * mean and std value provided with the model description.
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
bool FilterManagementLibrary::TFIntegration::TensorflowNNInstance::
applyImageInputVector(uint8_t*** inputVector,
							int imageHeight, int imageWidth, int channels)
{

	// Assume the image already has the required dimensions for now.


	//types.pb.h
	//enum DataType { DT_INVALID = 0, DT_FLOAT = 1, DT_DOUBLE = 2, DT_INT32 = 3, DT_UINT8 = 4,
	// DT_INT16 = 5, DT_INT8 = 6, DT_STRING = 7, DT_COMPLEX64 = 8, DT_INT64 = 9,

	if(!(imageWidth == this->nnModelDescription.inputWidth &&
			imageHeight == this->nnModelDescription.inputHeight))
	{
		this->lastError = FilterManagementLibrary::TFIntegration::
				TensorflowNNInstance::ErrorType::ERROR_INPUT_SIZE_MISMATCH;
		return false;
	}


	if(this->nnModelDescription.input_floating)
	{

		// Creates a Tensor of dimension 1xwidthxheigthxchannels

		auto inputTensorMapped = inputTensor.tensor<float, 4>();
		for (int i = 0; i < this->nnModelDescription.inputHeight; ++i)
		{
		    for (int j = 0; j < this->nnModelDescription.inputWidth; ++j)
		    {
		    	for(int k = 0; k < 3; k++)
		    	{

		    		inputTensorMapped(0, i, j, k) =
		    				     (static_cast<float>(inputVector[i][j][k]) -
		    				        this->nnModelDescription.input_mean) /
									this->nnModelDescription.input_std;

		    	}
		    }
		}

	}
	else
	{
		auto inputTensorMapped = inputTensor.tensor<uint8_t, 4>();
		for (int i = 0; i < this->nnModelDescription.inputHeight; ++i)
		{
		    for (int j = 0; j < this->nnModelDescription.inputWidth; ++j)
		    {
		    	for(int k = 0; k < 3; k++)
		    	{
		    		inputTensorMapped(0, i, j, k) = inputVector[i][j][k];
		    	}
		    }
		}
	}

	return true;
}

/**
 * @brief Applies an already float input vector to the neuronal network model.
 *
 * Used std::copy_n to directly copy the flat float vector to the input tensor
 * May not always work (or at all).
 */
bool FilterManagementLibrary::TFIntegration::TensorflowNNInstance::
applyInputVectorFromFloatData(float* floatData, const int heigth,
		const int width, const int channels)
{
	// create input shape
	tensorflow::TensorShape image_shape = tensorflow::TensorShape{1, heigth, width, channels};


	// create input tensor
	this->inputTensor = tensorflow::Tensor(tensorflow::DT_FLOAT, image_shape);

	std::copy_n((char*) floatData, image_shape.num_elements() * sizeof(float),
	const_cast<char*>(this->inputTensor.tensor_data().data()));

	return true;
}


/**
 * @brief Returns the last error that happened.
 *
 * @return TensorflowNNInstance::ErrorType the enum value of the last error
 * that happened when executing any previous function of TensorflowNNInstance
 */
FilterManagementLibrary::TFIntegration::TensorflowNNInstance::ErrorType
FilterManagementLibrary::TFIntegration::TensorflowNNInstance::
												getLastError() const
{
	return this->lastError;
}

/**
 * @brief Returns the tensors containg the output of the previous inference.
 *
 * Returns the output tensors of this instance. If an inference was
 * successfully executed before, those tensors will hold the output of the
 * neuronal network model used.
 * However, to examine the output, getResultContainer() should be used.
 *
 * @return std::vector<stensorflow::tensor>* vector containg the output Tensors
 */
std::vector<tensorflow::Tensor>* FilterManagementLibrary::TFIntegration::
		TensorflowNNInstance::getOutputTensors()
{
	return &this->outputTensors;
}

/**
 * @brief Let's the neuronal network examine the applied input.
 *
 * An input vector should have been applied prior to calling this function.
 * It will run the Tensorflow Session's run function which calculates
 * the output to the given input.
 * On failure, lastError will be set accordingly.
 *
 * @return true if the inference was successfull, false otherwise
 */
bool FilterManagementLibrary::TFIntegration::TensorflowNNInstance::
	runInference()
{
	std::vector<std::pair<std::string, tensorflow::Tensor> > inputTensors(
	      {{*this->inputLayerName, this->inputTensor}});


	  this->outputTensors.clear();


	  tensorflow::Tensor tensor;

	  tensorflow::Status s;
	  s = this->tensorflowSession->Run(inputTensors,
			  this->nnModelDescription.outputLayerNames, {},
			  &this->outputTensors);

	  if(!s.ok())
	  {
		  this->lastError = TensorflowNNInstance::ErrorType::
				  ERROR_SESSION_RUN_FAILED;
		  FilterManagementLibrary::Logger::printfln(s.ToString().c_str());
		  return false;
	  }
	  else
	  {
		  return true;
	  }

}

/**
 * @brief Allows to change ONLY the model file path set in the model description.
 *
 * Can be used to adjust the path of the model file of the
 * TensorflowNNModelDescription at run-time.
 *
 * @param std::string new path to the modelFile to use.
 * @return void
 */
void FilterManagementLibrary::TFIntegration::
	TensorflowNNInstance::adjustModelFile(std::string modelFile)
{
	this->nnModelDescription.modelFile = modelFile;
}

/**
 * @brief Returns a container containing the result of the last inference.
 *
 * Packs Tensorflow output tensors of this instance to a
 * TensorflowNNResultContainer which can be used to process
 * the result of the last inference.
 *
 * @return TensorflowResultContainer container containting the result of
 * the last inference.
 */
FilterManagementLibrary::TFIntegration::TensorflowResultContainer
	FilterManagementLibrary::TFIntegration::TensorflowNNInstance::getResultContainer()
{
	return TensorflowResultContainer(&this->outputTensors,
			this->nnModelDescription.outputLayerNames);
}

/**
 * @brief Returns the model description which was passed to this instance
 *
 * @return TensorflowNNModelDescription the model description which was passed
 * to this instance in constructor.
 */
const FilterManagementLibrary::TFIntegration::TensorflowNNModelDescription*
	FilterManagementLibrary::TFIntegration::
	TensorflowNNInstance::getModelDescription() const
{
	return &this->nnModelDescription;
}

/**
 * @brief Returns the input Tensor of this instance.
 *
 * Normally the Tensorflow input Tensor for the corresponding neuronal
 * network model should'nt be visible outside this class.
 * However it can be useful to provide a direct access to this Tensor,
 * because specific tasks may need a specific way to apply an input
 * to the network. TensorflowOpenCVUtils, for example, use this function
 * for the fastApplyCVMatOnInputTensor functions.
 * This, only use this function if you know what you're doing!
 */
tensorflow::Tensor* FilterManagementLibrary::TFIntegration::
						TensorflowNNInstance::getInputTensor()
{
	return &this->inputTensor;
}

/**
 * @brief Destructor of the TensorflowNNInstance class.
 *
 * Destructor whose only task is to close the Tensorflow Session which was
 * used for this instance.
 */
FilterManagementLibrary::TFIntegration::
						TensorflowNNInstance::~TensorflowNNInstance()
{
	this->tensorflowSession->Close();
}

