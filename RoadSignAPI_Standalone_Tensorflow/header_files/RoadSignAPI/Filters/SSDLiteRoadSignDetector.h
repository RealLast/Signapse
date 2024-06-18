/*
 * SSDLite_RoadSignDetector.h
 *
 *  Created on: 03.06.2018
 *      Author: Patrick Langer
 */

#ifndef HEADER_FILES_ROADSIGNAPI_FILTERS_SSDLITEROADSIGNDETECTOR_H_
#define HEADER_FILES_ROADSIGNAPI_FILTERS_SSDLITEROADSIGNDETECTOR_H_


#include "FilterManagementLibrary/PipeSystem/TFNNBasedPipeFilter.h"
#include "RoadSignAPI/RSAPIWorkingDataSet/RSAPIWorkingDataSet.h"
#include "opencv2/opencv.hpp"

#ifdef __ANDROID__
	#include "FilterManagementLibrary/TensorflowAndroidJNIUtils.h"
#endif

namespace RoadSignAPI
{
	/**
	 * This filter is based on TFNNBasedPipeFilter and uses SSDLite for
	 * (roadsign) detection.
	 */
	class SSDLiteRoadSignDetector :
			public FilterManagementLibrary::PipeSystem::TFNNBasedPipeFilter
	{
		private:

			/**
			  * Just a copy from the input height provided in the
			  * {@link TensorflowNNModelDescription} of the underlaying
			  * {@link TensorflowNNInstance}.
			  */
			int nnModelInputHeight;

			/**
			  * Just a copy from the input height provided in the
			  * {@link TensorflowNNModelDescription} of the underlaying
			  * {@link TensorflowNNInstance}.
			  */
			int nnModelInputWidth;
			
			/**
			  * Threshold the confidence of a prediction from our underlying
			  * neuronal network model needs to exceed in order to be included
			  * to {@link RSAPIWorkingDataSet}::detectedSigns
			  *
			  */
			float threshold = 0.28f;

			/**
			  * Just a pointer casted from {@link PipeWorkingDataSet}*
			  * to {@link RSAPIWorkingDataSet}*, so we just don't have to do
			  * the casting every time we need it ;)
			  */
			RSAPIWorkingDataSet* castedWorkingDataSet;

			#ifdef __ANDROID__
				/**
				 * A pointer to an AssetManager which can be passed via
				 * the constructor (only available under Android environments).
				 * We need it to be able to load model files from the Android
				 * assets folder, which we want to do so the files do not have
				 * to be placed in the normal user space.
				 */
				AAssetManager* const assetManager;
			#endif

			// Get's called when the pipe's setup function is called
			// after all the filters have been registered.
			bool initByPipeSetup();

			// The main function of this filter, will be called
			// by the pipe when data shall be processed.
			bool process();

			void onNNEvaluationFinished
				(const FilterManagementLibrary::TFIntegration::
						TensorflowResultContainer resultContainer);

			void applyImageVectorFromOpenCVMat(cv::Mat *mat);

		public:
			// We provide a slightly different constructor for Android environments
			// to be able to pass in an AAssetManager to load model files from
			// the Android assets folder of an application using this.
			#ifdef __ANDROID__
				SSDLiteRoadSignDetector(FilterManagementLibrary::
						TFIntegration::TensorflowNNModelDescription,
						int numThreads,
						AAssetManager* const assetManager);
			#else
				SSDLiteRoadSignDetector(FilterManagementLibrary::
				TFIntegration::TensorflowNNModelDescription,
				int numThreads);
			#endif
	};
}

#endif /* HEADER_FILES_ROADSIGNAPI_FILTERS_SSDLITEROADSIGNDETECTOR_H_ */
