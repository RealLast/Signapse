/*
 * MobilenetV2_RoadSignClassificator.h
 *
 *  Created on: 03.06.2018
 *      Author: Patrick Langer
 */

#ifndef HEADER_FILES_ROADSIGNAPI_FILTERS_MOBILENETV2ROADSIGNCLASSIFICATOR_H_
#define HEADER_FILES_ROADSIGNAPI_FILTERS_MOBILENETV2ROADSIGNCLASSIFICATOR_H_



#include "FilterManagementLibrary/PipeSystem/TFNNBasedPipeFilter.h"
#include "RoadSignAPI/RSAPIWorkingDataSet/RSAPIWorkingDataSet.h"

#ifdef __ANDROID__
	#include "FilterManagementLibrary/TensorflowAndroidJNIUtils.h"
#endif

namespace RoadSignAPI
{
	/**
	 * This filter is based on TFNNBasedPipeFilter and uses MobilenetV2 for
	 * (roadsign) classification.
	 */
	class MobilenetV2RoadSignClassificator :
			public FilterManagementLibrary::PipeSystem::TFNNBasedPipeFilter
	{
		private:

			/**
			  * Just a pointer casted from {@link PipeWorkingDataSet}*
			  * to {@link RSAPIWorkingDataSet}*, so we just don't have to do
			  * the casting every time we need it ;)
			  */
			RSAPIWorkingDataSet* castedWorkingDataSet;

			/**
			 * Just a copy from the input height provided in the
			 * {@link TensorflowNNModelDescription} of the underlaying
			 * {@link TensorflowNNInstance}.
			 */
			int nnModelInputHeight;

			/**
			  * Just a copy from the input height provided in the
			  * {@link TensorflowNNModelDescription} of the underlying
			  * {@link TensorflowNNInstance}.
			  */
			int nnModelInputWidth;

			/**
			 * Threshold for the output confidence of the
			 * neuronal network model. Means a result needs to achieve
			 * a confidence >= threshold to be candidate for list of
			 * top N predictions.
			 */
			float threshold = 0.95;

			/**
			 * A cv::Mat were we will temporarily store a resized image matrix
			 * while we iterate over {@see RSAPIWorkingDataSet}::cutOutImages
			 * (we need to resize them to the size our network model expects).
			 */
			cv::Mat currentCutOutImage;

			/**
			 * Inter-class copy of top rated recognition we determined in on
			 * onNNEvaluationFinished(...). You may refer to the process()
			 * function of this class for a description of where this is used.
			 */
			std::pair<float, int> currentRecognition;

			/**
			 * If none of the outputs of the network beats the threshold,
			 * we currentRecognition may store the result of a previous
			 * classification, which would lead to wrong results.
			 * This variable states if there was a valid recognition in the last
			 * evaluateImageByNN() call.
			 */
			bool validRecognition = false;


			#ifdef __ANDROID__
				/**
				 * A pointer to an AssetManager which can be passed via
				 * the constructor.
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

			bool isInUnwantedClasses(int classID) const;

		public:

			// We provide a slightly different constructor for Android environments
			// to be able to pass in an AAssetManager to load model files from
			// the Android assets folder of an application using this.
			#ifdef __ANDROID__
				MobilenetV2RoadSignClassificator(FilterManagementLibrary::
						TFIntegration::TensorflowNNModelDescription,
						int numThreads,
						AAssetManager* const assetManager);
			#else
				MobilenetV2RoadSignClassificator(FilterManagementLibrary::
						TFIntegration::TensorflowNNModelDescription,
						int numThreads);
			#endif

	};
}


#endif /* HEADER_FILES_ROADSIGNAPI_FILTERS_MOBILENETV2ROADSIGNCLASSIFICATOR_H_ */
