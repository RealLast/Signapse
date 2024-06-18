/*
 * RoadSignAPI.cpp
 *
 *  Created on: 05.06.2018
 *      Author: Patrick Langer
 */


#include "RoadSignAPI/RoadSignAPI.h"

// Instantiation
RoadSignAPI::RoadSignAPI* RoadSignAPI::RoadSignAPI::instance;

FilterManagementLibrary::TFIntegration::TensorflowNNModelDescription RoadSignAPI::RoadSignAPI::ssdLiteModelDescription;
FilterManagementLibrary::TFIntegration::TensorflowNNModelDescription RoadSignAPI::RoadSignAPI::mobilenetModelDescription;


#ifdef __ANDROID__
/**
 * @brief Constructor of the RoadSignAPI for Android environments.
 *
 * Under Android, we need a slightly different constructor for the
 * RoadSignAPI: As we want to store the neuronal network model files
 * in the assets folder of the app, we need a Pointer to an AssetManager
 * passed from the Java part of the app (by any means).
 * The Android-platform dependent code of the RoadSignAPI expects this
 * (also refer to {@link SSDLiteRoadSignDetector}
 * and {@link MobilenetV2RoadSignClassificator} for more details).
 * Of course this constructor is only available under Android environments.
 * For other platforms, please use the corresponding constructor.
 * Calls the constructor of the ProcessingPipeline and all filters.
 * Afterwards the filters are registered to the pipe.
 *
 * @param TensorflowNNModelDescription The model description for the detector
 * network.
 *
 * @param TensorflowNNModelDescription The model description for the
 * classificator network.
 *
 * @param const int numThreads Number of threads Tensorflow shall be allowed
 * to use at max.
 *
 * @param AAssetManager* const assetManager pointer to an AssetManager, which
 * we will use to load neuronal network model files from the assets folder of
 * the Android app. Needs to be passed from the Java part of the app
 * by any means.
 */
RoadSignAPI::RoadSignAPI::RoadSignAPI(FilterManagementLibrary::
			TFIntegration::TensorflowNNModelDescription
			detectorModelDescription,
			FilterManagementLibrary::
			TFIntegration::TensorflowNNModelDescription
			classificatorModelDescription,
			const int numThreads, AAssetManager* const assetManager) :

	processingPipeline(&workingDataSet, &pipeRegisteredFilters, false),
	roadSignDetector(detectorModelDescription, numThreads, assetManager),
	roadSignClassificator(
			classificatorModelDescription, numThreads, assetManager),
			classifiedSignsGrouper(), roadSignDuplicationDeleter(),

			assetManager(assetManager)
{
	this->processingPipeline.registerFilter(
			&this->roadSignDetector,
			&this->pipeRegisteredFilters.SIGN_DETECTION_FILTER);

	this->processingPipeline.registerFilter(
				&this->roadSignClassificator,
				&this->pipeRegisteredFilters.SIGN_RECOGNITION_FILTER);

	this->processingPipeline.registerFilter(
				&this->detectionBasedImageSlicer,
				&this->pipeRegisteredFilters.
				DETECTION_BASED_IMAGE_SLICER_FILTER);

	this->processingPipeline.registerFilter(
				&this->classifiedSignsGrouper,
				&this->pipeRegisteredFilters.
				CLASSIFIED_SIGNS_GROUPER_FILTER);

	this->processingPipeline.registerFilter(
				&this->roadSignDuplicationDeleter,
				&this->pipeRegisteredFilters.
				SIGN_DUPLICATION_DELETER_FILTER);
}
#else
/**
 * @brief Constructor of the RoadSignAPI for non Android environments.
 *
 * Calls the constructor of the ProcessingPipeline and all filters.
 * Afterwards the filters are registered to the pipe.
 *
 * @param TensorflowNNModelDescription The model description for the detector
 * network.
 *
 * @param TensorflowNNModelDescription The model description for the
 * classificator network.
 *
 * @param const int numThreads Number of threads Tensorflow shall be allowed
 * to use at max.
 *
 */
RoadSignAPI::RoadSignAPI::RoadSignAPI(FilterManagementLibrary::
			TFIntegration::TensorflowNNModelDescription
			detectorModelDescription,
			FilterManagementLibrary::
			TFIntegration::TensorflowNNModelDescription
			classificatorModelDescription,
			const int numThreads) :
	processingPipeline(&workingDataSet, &pipeRegisteredFilters, false),
	roadSignDetector(detectorModelDescription, numThreads),
	roadSignClassificator(classificatorModelDescription, numThreads),
	classifiedSignsGrouper(), roadSignDuplicationDeleter()
{

	this->processingPipeline.registerFilter(
			&this->roadSignDetector,
			&this->pipeRegisteredFilters.SIGN_DETECTION_FILTER);

	this->processingPipeline.registerFilter(
				&this->roadSignClassificator,
				&this->pipeRegisteredFilters.SIGN_RECOGNITION_FILTER);

	this->processingPipeline.registerFilter(
				&this->detectionBasedImageSlicer,
				&this->pipeRegisteredFilters.
				DETECTION_BASED_IMAGE_SLICER_FILTER);

	this->processingPipeline.registerFilter(
			&this->classifiedSignsGrouper,
			&this->pipeRegisteredFilters.
			CLASSIFIED_SIGNS_GROUPER_FILTER);

	this->processingPipeline.registerFilter(
				&this->roadSignDuplicationDeleter,
				&this->pipeRegisteredFilters.
				SIGN_DUPLICATION_DELETER_FILTER);


}
#endif

/**
 * @brief Initializes the RoadSignAPI's static interface.
 *
 * We use a Singleton to provide a static interface to the RoadSignAPI.
 * This Singleton will be initialized in this function:
 * The model descriptions for the SSDLite and MobilenetV2 neuronal networks
 * will be created, the RoadSignAPI will be instantiated and the
 * ProcessingPipeline's setUp function is called.
 */
bool RoadSignAPI::RoadSignAPI::init()
{
	return this->processingPipeline.setup();
}


/**
 * @brief Takes an OpenCV Mat uses the filter to examine it for road signs.
 */
bool RoadSignAPI::RoadSignAPI::feedImage(cv::Mat image)
{
	this->workingDataSet.originalBGRImage = image;
	this->workingDataSet.originalImageHeight = image.rows;
	this->workingDataSet.originalImageWidth = image.cols;

	return this->processingPipeline.processCurrentDataSet();
}

/**
 * @brief Returns a vector of all detected (not classified!) signs
 *
 * Should only be called after the feedImage(...) function was used.
 * The vector contains all signs which were detected on the previous image.
 *
 * @return const std::vector<DetectedSignDescriptor> a vector containing
 * all detected signs.
 */
const std::vector<RoadSignAPI::DetectedSignDescriptor>*
				RoadSignAPI::RoadSignAPI::getDetectedSigns()
{
	return &this->workingDataSet.detectedSigns;
}

/**
 * @brief Returns a vector of all classified signs.
 *
 * Should only be called after the feedImage(...) function was used.
 * The vector contains all signs which were detected AND classified on
 * the previous image. All images which were detected but could not be
 * classified or are classified as a part of the unwanted classes won't
 * be contained in this list!
 *
 * @param std::vector<const DetectedSignDescriptor*>* a pointer to a
 * vector of pointers to a DetectedSignDescriptor. The vector
 * will be filled with the DetectedSignDescriptors of the RSAPIWorkingDataSet
 * which were successfully classified as relevant roadsigns.
 *
 * @return void
 */
void RoadSignAPI::RoadSignAPI::getClassifierApprovedDetectedSigns(
		std::vector<const DetectedSignDescriptor*>* detectedSigns)
{


	detectedSigns->clear();

	for(int i = 0; i < this->
	workingDataSet.classifierApprovedSigns.size(); i++)
	{
		detectedSigns->push_back(&this->workingDataSet.detectedSigns
				[this->workingDataSet.classifierApprovedSigns[i]]);
	}
}


/**
 * @brief Returns a vector of all @link{DetectedSignCombination}s.
 *
 * Should only be called after the feedImage(...) function was used.
 * The vector contains all signs which were detected AND classified on
 * the previous image grouped into @link{DetectedSignCombination}s.
 * All images which were detected but could not be
 * classified or are classified as a part of the unwanted classes won't
 * be contained in this list!
 *
 * @return const std::vector<DetectedSignCombination>* const pointer to
 * a vector containing all @link{DetectedSignCombination}s, that were
 * found in the image that was lastly provided to feedImage(...).
 */
const std::vector<RoadSignAPI::DetectedSignCombination>*
			const RoadSignAPI::RoadSignAPI::getDetectedSignCombinations() const
{
	return &this->workingDataSet.detectedSignCombinations;
}

#ifdef __ANDROID__
/**
 * @brief Initializes the RoadSignAPI's static interface.
 *
 * This is the staticInit function for Android environments. As under Android,
 * we want the neuronal network model files to be stored inside the assets
 * folder of the app, we need an AssetManager passed from the Java part of the
 * app to be able to do so. On non Android environments, this function just lacks
 * it's last parameter.
 *
 * We use a Singleton to provide a static interface to the RoadSignAPI.
 * This Singleton will be initialized in this function:
 * The model descriptions for the SSDLite and MobilenetV2 neuronal networks
 * will be created, the RoadSignAPI will be instantiated and the
 * ProcessingPipeline's setUp function is called.
 *
 */
bool RoadSignAPI::RoadSignAPI::staticInit(
		const int numThreads, AAssetManager* const assetManager)
{


	RoadSignAPI::RoadSignAPI::
		ssdLiteModelDescription.inputHeight = 300;
	RoadSignAPI::RoadSignAPI::
		ssdLiteModelDescription.inputWidth = 300;
	RoadSignAPI::RoadSignAPI::
		ssdLiteModelDescription.channels = 3;
	RoadSignAPI::RoadSignAPI::
		ssdLiteModelDescription.input_floating = false;
	RoadSignAPI::RoadSignAPI::
		ssdLiteModelDescription.inputLayerNameStr = "image_tensor";
	RoadSignAPI::RoadSignAPI::
		ssdLiteModelDescription.outputLayerNames =
		{"detection_boxes","detection_scores","detection_classes","num_detections"};
	RoadSignAPI::RoadSignAPI::
		ssdLiteModelDescription.modelFile = "ssdlite.pb";



	RoadSignAPI::RoadSignAPI::
		mobilenetModelDescription.inputHeight = 96;
	RoadSignAPI::RoadSignAPI::
		mobilenetModelDescription.inputWidth = 96;
	RoadSignAPI::RoadSignAPI::
	mobilenetModelDescription.channels = 3;
	RoadSignAPI::RoadSignAPI::
		mobilenetModelDescription.input_floating = true;
	RoadSignAPI::RoadSignAPI::
		mobilenetModelDescription.inputLayerNameStr = "input_1";
	RoadSignAPI::RoadSignAPI::
		mobilenetModelDescription.outputLayerNames = {"output_node0"};
	RoadSignAPI::RoadSignAPI::
		mobilenetModelDescription.modelFile = "mobilenetv2.pb";
	//RoadSignAPI::RoadSignAPI::instance->processingPipeline.registerFilter((FilterManagementLibrary::PipeSystem::PipeFilter*)(&(RoadSignAPI::RoadSignAPI::instance->roadSignDetector)), &RoadSignAPI::RoadSignAPI::instance->pipeRegisteredFilters.SIGN_DETECTION_FILTER);
	RoadSignAPI::RoadSignAPI::instance = new RoadSignAPI(RoadSignAPI::RoadSignAPI::
			ssdLiteModelDescription, RoadSignAPI::RoadSignAPI::
			mobilenetModelDescription, numThreads, assetManager);
	return RoadSignAPI::RoadSignAPI::instance->init();
}
#else
/**
 * @brief Initializes the RoadSignAPI's static interface.
 *
 * This is the staticInit function for non-Android environments.
 * We use a Singleton to provide a static interface to the RoadSignAPI.
 * This Singleton will be initialized in this function:
 * The model descriptions for the SSDLite and MobilenetV2 neuronal networks
 * will be created, the RoadSignAPI will be instantiated and the
 * ProcessingPipeline's setUp function is called.
 */
bool RoadSignAPI::RoadSignAPI::staticInit(const int numThreads)
{


	RoadSignAPI::RoadSignAPI::
		ssdLiteModelDescription.inputHeight = 300;
	RoadSignAPI::RoadSignAPI::
		ssdLiteModelDescription.inputWidth = 300;
	RoadSignAPI::RoadSignAPI::
		ssdLiteModelDescription.channels = 3;
	RoadSignAPI::RoadSignAPI::
		ssdLiteModelDescription.input_floating = false;
	RoadSignAPI::RoadSignAPI::
		ssdLiteModelDescription.inputLayerNameStr = "image_tensor";
	RoadSignAPI::RoadSignAPI::
		ssdLiteModelDescription.outputLayerNames =
		{"detection_boxes","detection_scores","detection_classes","num_detections"};
	RoadSignAPI::RoadSignAPI::
		ssdLiteModelDescription.modelFile = "ssdlite.pb";



	RoadSignAPI::RoadSignAPI::
		mobilenetModelDescription.inputHeight = 96;
	RoadSignAPI::RoadSignAPI::
		mobilenetModelDescription.inputWidth = 96;
	RoadSignAPI::RoadSignAPI::
	mobilenetModelDescription.channels = 3;
	RoadSignAPI::RoadSignAPI::
		mobilenetModelDescription.input_floating = true;
	RoadSignAPI::RoadSignAPI::
		mobilenetModelDescription.input_mean = 127.5f;
	RoadSignAPI::RoadSignAPI::
		mobilenetModelDescription.input_std = 127.5f;
	RoadSignAPI::RoadSignAPI::
		mobilenetModelDescription.inputLayerNameStr = "input_1";
	RoadSignAPI::RoadSignAPI::
		mobilenetModelDescription.outputLayerNames = {"output_node0"};
	RoadSignAPI::RoadSignAPI::
		mobilenetModelDescription.modelFile = "mobilenetv2.pb";
	//RoadSignAPI::RoadSignAPI::instance->processingPipeline.registerFilter((FilterManagementLibrary::PipeSystem::PipeFilter*)(&(RoadSignAPI::RoadSignAPI::instance->roadSignDetector)), &RoadSignAPI::RoadSignAPI::instance->pipeRegisteredFilters.SIGN_DETECTION_FILTER);
	RoadSignAPI::RoadSignAPI::instance = new RoadSignAPI(RoadSignAPI::RoadSignAPI::
			ssdLiteModelDescription, RoadSignAPI::RoadSignAPI::
			mobilenetModelDescription, numThreads);
	return RoadSignAPI::RoadSignAPI::instance->init();
}
#endif

/**
 * @brief Takes an OpenCV Mat uses the filter to examine it for road signs.
 */
bool RoadSignAPI::RoadSignAPI::staticFeedImage(cv::Mat image)
{
	return RoadSignAPI::RoadSignAPI::instance->feedImage(image);
}

/**
 * @brief Returns a vector of all detected (not classified!) signs
 *
 * Should only be called after the feedImage(...) function was used.
 * The vector contains all signs which were detected on the previous image.
 *
 * @return const std::vector<DetectedSignDescriptor> a vector containing
 * all detected signs.
 */
const std::vector<RoadSignAPI::DetectedSignDescriptor>*
				RoadSignAPI::RoadSignAPI::staticGetDetectedSigns()
{
	return RoadSignAPI::RoadSignAPI::instance->getDetectedSigns();
}

/**
 * @brief Returns a vector of all classified signs.
 *
 * Should only be called after the staticFeedImage(...) function was used.
 * The vector contains all signs which were detected AND classified on
 * the previous image. All images which were detected but could not be
 * classified or are classified as a part of the unwanted classes won't
 * be contained in this list!
 *
 * @param std::vector<const DetectedSignDescriptor*>* a pointer to a
 * vector of pointers to a DetectedSignDescriptor. The vector
 * will be filled with the DetectedSignDescriptors of the RSAPIWorkingDataSet
 * which were successfully classified as relevant roadsigns.
 *
 * @return void
 */
void RoadSignAPI::RoadSignAPI::staticGetClassifierApprovedDetectedSigns(
		std::vector<const DetectedSignDescriptor*>* detectedSigns)
{


	return RoadSignAPI::RoadSignAPI::instance->
			getClassifierApprovedDetectedSigns(detectedSigns);
}

/**
 * @brief Returns a vector of all @link{DetectedSignCombination}s.
 *
 * Should only be called after the staticFeedImage(...) function was used.
 * The vector contains all signs which were detected AND classified on
 * the previous image grouped into @link{DetectedSignCombination}s.
 * All images which were detected but could not be
 * classified or are classified as a part of the unwanted classes won't
 * be contained in this list!
 *
 * @return const std::vector<DetectedSignCombination>* const pointer to
 * a vector containing all @link{DetectedSignCombination}s, that were
 * found in the image that was lastly provided to staticFeedImage(...).
 */
const std::vector<RoadSignAPI::DetectedSignCombination>*
			const RoadSignAPI::RoadSignAPI::staticGetDetectedSignCombinations()
{
	return RoadSignAPI::RoadSignAPI::instance->getDetectedSignCombinations();
}

