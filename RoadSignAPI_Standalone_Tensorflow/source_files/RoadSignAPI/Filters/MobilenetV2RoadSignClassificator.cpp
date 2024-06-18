/*
 * MobilenetV2_RoadSignClassificator.cpp
 *
 * NOTE: Although TensorflowNNInstanceClassifier *does* work with
 * MobilenetV2 and we could use it here, I wanted to keep everything
 * that has to do with the RoadSigns detection / classification in their
 * corresponding filters. This might come in handy if we use different
 * neuronal models in future versions.
 * Also, we don't need the feature of TensorflowNNInstanceClassifier
 * to provide N top predictions (we always go with the hightest
 * confidence value). So to do this as performant as possible, this is
 * the way we go!
 *
 *  Created on: 03.06.2018
 *      Author: Patrick Langer
 */


#include "RoadSignAPI/Filters/MobilenetV2RoadSignClassificator.h"
#include "RoadSignAPI/DetectedSignDescriptor.h"
#include "RoadSignAPI/RSAPIPipeRegisteredFilters/RSAPIPipeRegisteredFilters.h"
#include "FilterManagementLibrary/Logger.h"
#include "FilterManagementLibrary/TensorflowOpenCVUtils.h"



#ifdef __ANDROID__
/**
 * @brief Constructor of MobilenetV2RoadSignClassificator for Android environments.
 * It basically just calls the TFNNBasedPipeFilter super constructor.
 * Refer to it for a more detailed description.
 * Under Android, the RoadSignAPI expects the model files of our neuronal
 * networks to be placed in the assets folder of the app.
 * Thus, this constructor needs to be passed a pointer to an AssetManager
 * (from java), which is MANDATORY for the RoadSignAPI under Android!
 *
 * All other class-member variables will be initialized to their default values.
 *
 * @param TensorflowNNModelDescription nnModelDescription a description of
 * the Tensorflow model which will be used by the filter.
 *
 * @param int numThreads number of threads Tensorflow is allowed to use
 * for computation.
 *
 * @param AAssetManager* const assetManager pointer to an Android Asset
 * Manager which needs to be passed from the Java part of the Android App
 * by any means. Manadatory to load the neuronal network models from
 * the assets directory of the app.
 */
RoadSignAPI::MobilenetV2RoadSignClassificator::MobilenetV2RoadSignClassificator(
		FilterManagementLibrary::TFIntegration::
		TensorflowNNModelDescription nnModelDescription, int numThreads,
		AAssetManager* const assetManager) :
		TFNNBasedPipeFilter(nnModelDescription, numThreads),
		castedWorkingDataSet(NULL), nnModelInputHeight(0),
		nnModelInputWidth(0), assetManager(assetManager)
{

}
#else
/**
 * @brief Constructor of MobilenetV2RoadSignClassificator for non Android environments.
 * It basically just calls the TFNNBasedPipeFilter super constructor.
 * Refer to it for a more detailed description.
 *
 * All class-member variables will be initialized to their default values.
 *
 * @param TensorflowNNModelDescription nnModelDescription a description of
 * the Tensorflow model which will be used by the filter.
 *
 * @param int numThreads number of threads Tensorflow is allowed to use
 * for computation.
 */
RoadSignAPI::MobilenetV2RoadSignClassificator::MobilenetV2RoadSignClassificator(
		FilterManagementLibrary::TFIntegration::
		TensorflowNNModelDescription nnModelDescription, int numThreads) :
		TFNNBasedPipeFilter(nnModelDescription, numThreads),
		castedWorkingDataSet(NULL), nnModelInputHeight(0),
		nnModelInputWidth(0)
{

}
#endif

/**
 * @brief Initializes the filter.
 *
 * Will be called by ProcessingPipeline.
 * Here, basically just the neuronal network model will be loaded.
 * For this, the setupModelFromFile() or setupModelFromAssets() function
 * will be called accordingly, depending whether we are under an Android
 * environment or not.
 * Refer to TFNNBasedPipeFilter for a more detailed description.
 *
 *
 * @return true, if the model could be loaded successfully, false otherwise
 */
bool RoadSignAPI::MobilenetV2RoadSignClassificator::initByPipeSetup()
{
	this->castedWorkingDataSet =
			(RSAPIWorkingDataSet*) this->pipeWorkingDataSet;

	FilterManagementLibrary::Logger::printfln("RoadSignClassificator init");
	FilterManagementLibrary::Logger::printfln("Loading model ...");


	// As this code belongs to the RoadSignAPI, and RoadSignAPI does not solve
	// any general tasks but rather has a really specific task: road sign,
	// detection we do not care about a more fancy way to provide a platform
	// independent way to setup the model files: As under Android, both
	// setupModelFromAssets(...) and setupModelFromFile() are available, as
	// the FilterManagementLibrary provides them, we would need a separate
	// initByPipeSetup() function (like one for Android, one for other
	// systems), if not using the #ifdef approach (because either how,
	// somwehere has to be checked if we are under Android and if that's the
	// case, we want to load the model file from Assets, but this code would
	// not be able to compile under other systems if just using a normal
	// variable to determine whether we are under Android, for example).
	#ifdef __ANDROID__
		if(this->setupModelFromAssets(this->assetManager))
	#else
		if(this->setupModelFromFile())
	#endif
	{
		FilterManagementLibrary::Logger::printfln("Model loaded");

		const FilterManagementLibrary::
				TFIntegration::TensorflowNNModelDescription*
				modelDescription = this->getNNModelDescription();
		this->nnModelInputHeight = modelDescription->inputHeight;
		this->nnModelInputWidth = modelDescription->inputWidth;

		this->currentCutOutImage =
						cv::Mat(this->nnModelInputWidth,
								this->nnModelInputHeight, CV_8UC3);


		return true;
	}
	else
	{
		FilterManagementLibrary::Logger::printfln("Setup model failed");
		return false;
	}
}

/**
 * @brief Uses the neuronal network to classify the previous detected signs.
 *
 * Iterates over all the images the DetectionBasedImageSlicer generated
 * from the originalBGRImage and uses the neuronal network model to
 * classify them (uses evaluateInputVectorByNN() of TFNNBasedPipeFilter super class.
 * Will set the class ID and the confidence accordingly.
 * All classes which are unwanted (i.e. misc classes) will be filtered out,
 * in other words they won't be added to classifierApprovedSigns
 * of RSAPIWorkingDataSet's.
 *
 * @return true if evaluateInputVectorByNN() return true, false otherwise
 * (does NOT return false if no signs could be classified!)
 */
bool RoadSignAPI::MobilenetV2RoadSignClassificator::process()
{
	this->castedWorkingDataSet->classifierApprovedSigns.clear();
	float confidence = -1.0f;
	int classID = -1;
	for(int i = 0; i < this->castedWorkingDataSet->cutOutImages.size(); i++)
	{
		// Copies and scales the current cutted out image to the input size the
		// classificator model expects.
		cv::resize(this->castedWorkingDataSet->cutOutImages[i],
					this->currentCutOutImage,
					cv::Size(this->nnModelInputWidth, this->nnModelInputHeight),
					0.0, 0.0, cv::INTER_CUBIC );


		this->applyImageVectorFromOpenCVMat(
				&this->currentCutOutImage);

		if(this->evaluateInputVectorByNN())
		{
			if(this->validRecognition)
			{
				confidence = this->currentRecognition.first;
				classID = this->currentRecognition.second;


				// Check if the class is of any interest for us
				if(!this->isInUnwantedClasses(classID))
				{
					this->castedWorkingDataSet->
						classifierApprovedSigns.push_back(i);


					this->castedWorkingDataSet->
							detectedSigns[i].classifierConfidence = confidence;
					this->castedWorkingDataSet->
							detectedSigns[i].classifierApprovedClassID = classID;
				}
			}

		}
		else
		{
			return false;
		}
	}

	if(this->castedWorkingDataSet->classifierApprovedSigns.size() > 0)
	{
		this->invokeNext(((RSAPIPipeRegisteredFilters*)
						this->pipeRegisteredFilters)->
							CLASSIFIED_SIGNS_GROUPER_FILTER);
	}
	else
	{

		this->indicateProcessingFinished();
	}

	return true;
}

/**
 * @brief Checks if the given class ID is in the list of unwanted ID's
 *
 * @param int classID ID of the class which shall be checked.
 *
 * @return true if the given ID is in the list of unwanted class ID's
 * (i.e. misc classes), false otherwise.
 */
bool RoadSignAPI::MobilenetV2RoadSignClassificator::isInUnwantedClasses(int classID) const
{
	// MISC Classes
	if(classID >= 1 && classID <= 7)
		return true;
	else
		return false;
}

/**
 * @brief Callback, will be called when the network finished it's prediction
 *
 * In process(), the TFNNBasedPipeFilter's (super class)
 * evaluteInputVectorByNN() will be called. If it was successfull,
 * this callback will be called and passed a TensorflowResultContainer
 * with the result of the inference of the neuronal network model.
 * Here we will check the prediction and find the highest confidence
 * prediction, which exceeds a certain threshold.
 * The prediction itself will be examined in process() itself
 * (remember, this is a sync function! It will be called immediately
 * after evaluteInputVectorByNN() was called and then will return to process()
 * again).
 *
 * @param const TensorflowResultContainer resultContainer contains the
 * results (in Tensorflow tensors) of the network prediction.
 */
void RoadSignAPI::MobilenetV2RoadSignClassificator::onNNEvaluationFinished
				(const FilterManagementLibrary::TFIntegration::
						TensorflowResultContainer resultContainer)
{

	this->validRecognition = false;

	// CODE FROM TENSORFLOW LABEL IMAGE EXAMPLE!
	// We only need one prediction!
	const int numPredictions = 1;
	auto prediction = resultContainer.getOutputTensorByID(0)->flat<float>();
	// Will contain top N results in ascending order.
	std::priority_queue<std::pair<float, int>,
	std::vector<std::pair<float, int> >,
	       std::greater<std::pair<float, int> > >
				top_result_pq;

	  const int count = prediction.size();

	  for (int i = 0; i < count; ++i) {
	    const float value = prediction(i);

	    // Only add it if it beats the threshold and has a chance at being in
	    // the top N.

	    if (value < this->threshold) {
	      continue;
	    }



	    top_result_pq.push(std::pair<float, int>(value, i));


	    // If at capacity, kick the smallest value out.
	    if (top_result_pq.size() > 1) {
	      top_result_pq.pop();
	    }
	  }

	  // Copy to output vector and reverse into descending order.
	  if(!top_result_pq.empty())
	  {
		  this->validRecognition = true;
		  this->currentRecognition = top_result_pq.top();
	  }

}

/**
 * @brief Uses TensorflowOpenCVUtils to apply a mat as input to the network.
 *
 * @param cv::Mat *mat pointer to an OpenCV Mat which shall be used
 * as input.
 */
void RoadSignAPI::MobilenetV2RoadSignClassificator::
		applyImageVectorFromOpenCVMat(cv::Mat *mat)
{
	FilterManagementLibrary::TensorflowOpenCVUtils::
		fastApplyCVMatOnInputTensorFloat(mat,
				this->getNNInputTensor(),
				this->getNNModelDescription()->input_mean,
				this->getNNModelDescription()->input_std);
}




