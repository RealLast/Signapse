/*
 * SSDLite_RoadSignDetector.cpp
 *
 *  Created on: 03.06.2018
 *      Author: Patrick Langer
 */


#include "RoadSignAPI/Filters/SSDLiteRoadSignDetector.h"
#include "RoadSignAPI/DetectedSignDescriptor.h"
#include "RoadSignAPI/RSAPIPipeRegisteredFilters/RSAPIPipeRegisteredFilters.h"
#include "FilterManagementLibrary/Logger.h"
#include "FilterManagementLibrary/TensorflowOpenCVUtils.h"


#ifdef __ANDROID__
/**
 * @brief Constructor of SSDLiteRoadSignDetector for Android environments.
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
RoadSignAPI::SSDLiteRoadSignDetector::SSDLiteRoadSignDetector(
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
 * @brief Constructor of SSDLiteRoadSignDetector
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
RoadSignAPI::SSDLiteRoadSignDetector::SSDLiteRoadSignDetector(
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
 * For this, the setupModel() function will be called, refer to
 * TFNNBasedPipeFilter for a more detailed description.
 * Also the RSAPIWorkingDataSet's detectorSgaledBGRImage
 * will be initialized (with the corresponding size and type).
 *
 * @return true, if the model could be loaded successfully, flase otherwise
 */
bool RoadSignAPI::SSDLiteRoadSignDetector::initByPipeSetup()
{
	this->castedWorkingDataSet =
			(RSAPIWorkingDataSet*) this->pipeWorkingDataSet;
			
	FilterManagementLibrary::Logger::printfln("RoadSignDetector init");
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

		this->castedWorkingDataSet->detectorScaledBGRImage =
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
 * @brief Uses the neuronal network to detect signs on the current image.
 *
 * The originalBGR image will be scaled to the size the network model expects.
 * Afterfards, it uses the TFNNBasedPipeFilter super class's
 * evaluateInputVectorByNN() function to detect the road signs.
 * @return true if evaluateInputVectorByNN() return true, false otherwise
 * (does NOT return false if no signs were detected!)
 */
bool RoadSignAPI::SSDLiteRoadSignDetector::process()
{
	this->castedWorkingDataSet->detectedSigns.clear();
	this->castedWorkingDataSet->detectedSignCombinations.clear();

	cv::resize(this->castedWorkingDataSet->originalBGRImage,
			this->castedWorkingDataSet->detectorScaledBGRImage,
			cv::Size(this->nnModelInputWidth, this->nnModelInputHeight),
			0.0, 0.0, cv::INTER_CUBIC );


	this->applyImageVectorFromOpenCVMat(
			&this->castedWorkingDataSet->detectorScaledBGRImage);


	
	if(this->evaluateInputVectorByNN())
	{
		// Will be ignored if this->indicateProcessingFinished(); was called
		this->invokeNext(((RSAPIPipeRegisteredFilters*)
				this->pipeRegisteredFilters)->
					SIGN_DUPLICATION_DELETER_FILTER);
		return true;
	}
	else
	{
		FilterManagementLibrary::Logger::printfln("Failed.");
		return false;
	}
}

/**
 * @brief Callback, will be called when the network finished it's prediction
 *
 * In process(), the TFNNBasedPipeFilter's (super class)
 * evaluteInputVectorByNN() will be called. If it was successfull,
 * this callback will be called and passed a TensorflowResultContainer
 * with the result of the inference of the neuronal network model.
 * Here, we will interpete the output of the network.
 * We iterate over all it's detections and add them to a
 * DetectedSignDescriptor, if the cofidence exceeds a certain threshold value.
 * This descriptor is then added to RSAPIWorkingDataSet.
 *
 * @param const TensorflowResultContainer resultContainer contains the
 * results (in Tensorflow tensors) of the network prediction.
 */
void RoadSignAPI::SSDLiteRoadSignDetector::onNNEvaluationFinished
				(const FilterManagementLibrary::TFIntegration::
						TensorflowResultContainer resultContainer)
{

	tensorflow::TTypes<float>::Flat scores = resultContainer.getOutputTensorByLayerName(
			"detection_scores")->flat<float>();
	tensorflow::TTypes<float>::Flat classes = resultContainer.getOutputTensorByLayerName(
			"detection_classes")->flat<float>();
	tensorflow::TTypes<float>::Flat num_detections = resultContainer.getOutputTensorByLayerName(
			"num_detections")->flat<float>();
	auto boxes = resultContainer.getOutputTensorByLayerName(
			"detection_boxes")->flat_outer_dims<float,3>();

	if(num_detections(0) > 0)
	{
		for(size_t i = 0; i < num_detections(0) && i < 20;++i)
		{

			if(scores(i) > this->threshold)
			{

				RoadSignAPI::DetectedSignDescriptor detectedSignDescriptor;


				detectedSignDescriptor.upperLeft =
						cv::Point(this->castedWorkingDataSet->
								originalImageWidth * boxes(0, i, 1),
								this->castedWorkingDataSet->
								originalImageHeight * boxes(0, i, 0));
								
				detectedSignDescriptor.lowerRight =
						cv::Point(this->castedWorkingDataSet->
								originalImageWidth * boxes(0, i, 3) ,
								this->castedWorkingDataSet->
								originalImageHeight * boxes(0, i, 2));

				detectedSignDescriptor.detectionPredictedClassID = (int) classes(i);

				detectedSignDescriptor.detectorConfidence = (float) scores(i);



				this->castedWorkingDataSet->detectedSigns.push_back(
						detectedSignDescriptor);

			}
		}
	}
	else
	{
		// If we did not detect any sign on the image, that's okay!
		// It's not an error. As in this case any following hasn't any
		// work to do, we just indicate that the current working data
		// set has been fully processed!

		this->indicateProcessingFinished();
	}



}

/**
 * @brief Uses TensorflowOpenCVUtils to apply a mat as input to the network.
 *
 * @param cv::Mat *mat pointer to an OpenCV Mat which shall be used as input.
 *
 * @return void
 */
void RoadSignAPI::SSDLiteRoadSignDetector::
		applyImageVectorFromOpenCVMat(cv::Mat *mat)
{
	FilterManagementLibrary::TensorflowOpenCVUtils::
		fastApplyCVMatOnInputTensorUInt8(mat,
				this->getNNInputTensor());
}



