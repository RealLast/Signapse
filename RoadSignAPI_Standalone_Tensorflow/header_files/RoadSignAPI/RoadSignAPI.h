/*
 * RoadSignAPI.h
 *
 *  Created on: 05.06.2018
 *      Author: Patrick Langer
 */

#ifndef SOURCE_FILES_ROADSIGNAPI_ROADSIGNAPI_H_
#define SOURCE_FILES_ROADSIGNAPI_ROADSIGNAPI_H_

#include <vector>
#include "RoadSignAPI/DetectedSignDescriptor.h"

#include "FilterManagementLibrary/PipeSystem/ProcessingPipeline.h"
#include "RoadSignAPI/Filters/SSDLiteRoadSignDetector.h"
#include "RoadSignAPI/Filters/DetectionBasedImageSlicer.h"
#include "RoadSignAPI/Filters/MobilenetV2RoadSignClassificator.h"
#include "RoadSignAPI/Filters/ClassifiedSignsGrouper.h"
#include "RoadSignAPI/Filters/RoadSignDuplicationDeleter.h"
#include "RoadSignAPI/RSAPIWorkingDataSet/RSAPIWorkingDataSet.h"
#include "RoadSignAPI/RSAPIPipeRegisteredFilters/RSAPIPipeRegisteredFilters.h"





// Class of static functions which provides an entry to all
// of the functionality of RoadSignAPI

namespace RoadSignAPI
{
	/**
	 * Implemenation of the RoadSignAPI.
	 * Here all filters used for roadsign detection and classification will
	 * be set up and managed.
	 * It provides a static interface using a static instance of RoadSignAPI
	 * to easily feed an image and analyze it from static environments
	 * (i.e. Java Native Interface)
	 *
     * 
     * @image html api_class_roadsign2.svg
     * @image latex api_class_roadsign2.png width=16cm
    **/
	class RoadSignAPI
	{
		private:
			/**
			 * Pipeline which is used for sequencial processing of filters
			 * used for roadsign detection, classification, grouping
			 * and filtering.
			 */
			FilterManagementLibrary::
			PipeSystem::ProcessingPipeline processingPipeline;

			/**
			 * Static description for the ssdLite network, used for static interface.
			 */
			static FilterManagementLibrary::TFIntegration::TensorflowNNModelDescription ssdLiteModelDescription;

			/**
			 * Static description for the mobilenetv2 network, used for static interface.
			 */
			static FilterManagementLibrary::TFIntegration::TensorflowNNModelDescription mobilenetModelDescription;

			/**
			 * Instance of the SSDLiteRoadSignDetector used for roadsign
			 * detection, which is registered to the pipe of an instance
			 * of RoadSignAPI.
			 */
			SSDLiteRoadSignDetector roadSignDetector;

			/**
			 * Instance of the MobilenetV2RoadSignClassificator used for
			 * roadsign classification, which is registered to the pipe of
			 * an instance of RoadSignAPI.
			 */
			MobilenetV2RoadSignClassificator roadSignClassificator;

			/**
			 * Instance of DetectionBasedImageSlicer used to crop the boxes
			 * in which the roadsigns are contained, predicted by the
			 * SSDLiteRoadSignDetector.
			 */
			DetectionBasedImageSlicer detectionBasedImageSlicer;

			/**
			 * Instance of ClassifiedSignsGrouper which groups all signs
			 * that where detected *AND* classified into
			 * @link{DetectedSignCombination}s
			 */
			ClassifiedSignsGrouper classifiedSignsGrouper;

			/**
			 * Instance of RoadSignDuplicationDeleter which is used to
			 * delete signs that were detected multiple times (with a little
			 * offset) by the SSDLiteRoadSignDetector.
			 */
			RoadSignDuplicationDeleter roadSignDuplicationDeleter;

			/**
			 * Instance of RSAPIWorkingDataSet, which is the shared resource
			 * in which all results of the filters used in this class
			 * are stored.
			 */
			RSAPIWorkingDataSet workingDataSet;

			/**
			 * Instance of RSAPIPipeRegisteredFilters, which is a struct in
			 * which all IDs of the filters which are added to the
			 * ProcessingPipeline of this class are stored.
			 * It is passed to all filters so they know the IDs of all other
			 * filters.
			 */
			RSAPIPipeRegisteredFilters pipeRegisteredFilters;

			/**
			 * Static instance of RoadSignAPI for static interface.
			 */
			static RoadSignAPI* instance;

			#ifdef __ANDROID__
				/**
				 * A pointer to an AssetManager which can be passed via
				 * the constructor.
				 * We need it to be able to load model files from the Android
				 * assets folder, which we want to do so the files do not have
				 * to be placed in the normal user space.
				 * Will be passed to SSDLiteRoadSignDetector and
				 * MobilenetV2RoadSignClassificator.
				 */
				AAssetManager* const assetManager;
			#endif


		public:

			bool init();

			bool feedImage(cv::Mat iamge);

			const
				std::vector<DetectedSignDescriptor>* getDetectedSigns();

			void getClassifierApprovedDetectedSigns(
				std::vector<const DetectedSignDescriptor*>* detectedSigns);

			const std::vector<DetectedSignCombination>*
					const getDetectedSignCombinations() const;

			// Static interface

			#ifdef __ANDROID__
				// Under Android, we need an AssetManager passed from Java
				// in order to be able to load the model files from the
				// assets folder of the Android app.
			static bool staticInit(
						const int numThreads,
						AAssetManager* const assetManager);
			#else
				static bool staticInit(const int numThreads);
			#endif


			// cv::Mat uses Smartpointers, so we
			// don't have to use pointers to pass it around
			static bool staticFeedImage(cv::Mat image);

			// Remember: If you have a const vector<int> you cannot modify the container,
			// nor can you modify any of the elements in the container.
			static const
				std::vector<DetectedSignDescriptor>* staticGetDetectedSigns();

			static void staticGetClassifierApprovedDetectedSigns(
					std::vector<const DetectedSignDescriptor*>* detectedSigns);


			static const std::vector<DetectedSignCombination>*
						const staticGetDetectedSignCombinations();



			// We provide a slightly different constructor for Android environments
			// to be able to pass in an AAssetManager to load model files from
			// the Android assets folder of an application using this.
			#ifdef __ANDROID__
			RoadSignAPI(
					FilterManagementLibrary::
					TFIntegration::TensorflowNNModelDescription
					detectorModelDescription,
					FilterManagementLibrary::
					TFIntegration::TensorflowNNModelDescription
					classificatorModelDescription,
					const int numThreads, AAssetManager* const assetManager);
			#else
			RoadSignAPI(
					FilterManagementLibrary::
					TFIntegration::TensorflowNNModelDescription
					detectorModelDescription,
					FilterManagementLibrary::
					TFIntegration::TensorflowNNModelDescription
					classificatorModelDescription,
					const int numThreads);
			#endif

	};
}


#endif /* SOURCE_FILES_ROADSIGNAPI_ROADSIGNAPI_H_ */
