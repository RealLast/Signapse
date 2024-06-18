/*
 * TFNNBasedFilter.h
 *
 *  Created on: 30.05.2018
 *      Author: Patrick Langer
 */

#ifndef HEADER_FILES_FILTERMANAGEMENTLIBRARY_PIPESYSTEM_TFNNBASEDPIPEFILTER_H_
#define HEADER_FILES_FILTERMANAGEMENTLIBRARY_PIPESYSTEM_TFNNBASEDPIPEFILTER_H_

// Tensorflow Neuronal Network Based Filter
#include <stdint.h>
#include "FilterManagementLibrary/TensorflowIntegration/TensorflowNNInstance.h"
#include "FilterManagementLibrary/TensorflowIntegration/TensorflowResultContainer.h"
#include "FilterManagementLibrary/TensorflowIntegration/TensorflowNNModelDescription.h"
#include "FilterManagementLibrary/PipeSystem/PipeWorkingDataSet.h"
#include "FilterManagementLibrary/PipeSystem/PipeFilter.h"


namespace FilterManagementLibrary
{
	namespace PipeSystem
	{
		/**
		 * @brief Abstract template class to derive Tensorflow based filters from.
		 *
		 * A TFNNBasedPipeFilter is a special type of filter (thus derived from
		 * PipeFilter) already providing some functions to easily implemented
		 * filters based on Tensorflow Neuronal Networks.
		 * Strictly speaking the main processing of these filters is done by
		 * a preloaded Tensorflow model. The filter may take a part of the
		 * working data set and feed it into the model using the output of
		 * it's inference to generate the result of this filter.
		 * If you use it, make sure to call evaluateInputVectorByNN() in order
		 * to let the neuronal network do it's predictions.
		 *
		 * For this, the TFNNBasedPipeFilter abstract class uses a
		 * TensorflowNNInstance (TensorflowNNInstanceClassifier is not used,
		 * as this is just a test class for basic classification models
		 * with a predefined way of interpreting their outputs, which is
		 * not assured to be a general way to interprete the output
		 * other classificator networks may use).
		 *
		 * It provides a predefined easy way to apply input vectors from a
		 * 3 dimensional array of uint8_t's (applyNNImageInputVector).
		 * If the underlying model expects floats however, the conversion
		 * will be done automatically (see TensorflowNNInstance for
		 * more details).
		 * If a more specific way to apply an input is needed, the function
		 * getInputTensor() is provided which provides a direct access to the
		 * input tensor of the Neural Network Instance.
		 * Also keep in mind we provide some utility functions for easy
		 * OpenCV integration! In detail, offering some functions to directly
		 * apply a cv::Mat to an input tensor (see TensorflowOpenCVUtils).
		 *
		 * A basic example to performantely effective
		 * apply a matrix to the underlying neuronal network, expecting
		 * uint8_t as input vector, from within a derivate of
		 * TFNNBasedPipeFilter would be:
		 * TensorflowOpenCVUtils::fastApplyCVMatOnInputTensorUInt8(&yourMat,
		 * 	this->getNNInputTensor());
		 * 	simple as that!
		 *
		 * 	However, keep in mind TensorflowOpenCVUtils do NOT have to be
		 * 	compiled with and linked to a exectuable or libary using the
		 * 	FilterManagementLibrary, if for example you don't use OpenCV
		 * 	(Mat's as input methods).
		 */
		class TFNNBasedPipeFilter : public PipeFilter
		{
			private:
				/**
				 * The filter local instance of the underlying
				 * tensorflow neuronal network model.
				 */
				TFIntegration::TensorflowNNInstance tfNNInstance;

			protected:

				bool applyNNImageInputVector(uint8_t*** image,
									const int height,
									const int width, const int channels);
				bool evaluateInputVectorByNN();

				tensorflow::Tensor* getNNInputTensor();

				const FilterManagementLibrary::TFIntegration::
				TensorflowNNModelDescription* getNNModelDescription() const;

				TFIntegration::TensorflowNNInstance::
						ErrorType getNeuralNetLastError() const;

				virtual void onNNEvaluationFinished
								(const TFIntegration::TensorflowResultContainer
										resultContainer) = 0;

			public:

				TFNNBasedPipeFilter(TFIntegration::TensorflowNNModelDescription
						nnModelDescription,
						int numThreads);



				void adjustModelFile(std::string modelFile);

				bool setupModelFromFile();

				#ifdef __ANDROID__
					bool setupModelFromAssets(
							AAssetManager* const assetManager);
				#endif

				virtual ~TFNNBasedPipeFilter();



		};
	}
}



#endif /* HEADER_FILES_FILTERMANAGEMENTLIBRARY_PIPESYSTEM_TFNNBASEDPIPEFILTER_H_ */
