/*
 * TensorflowNNInstanceClassifier.cpp
 *
 *  Created on: 19.05.2018
 *      Author: Patrick Langer
 */


#include "FilterManagementLibrary/TensorflowIntegration/TensorflowNNInstanceClassifier.h"


#include <pthread.h>
#include <sys/stat.h>
#include <unistd.h>
#include <queue>
#include <sstream>
#include <string>

#include "tensorflow/core/framework/step_stats.pb.h"
#include "tensorflow/core/framework/tensor.h"
#include "tensorflow/core/framework/types.pb.h"
#include "tensorflow/core/lib/strings/stringprintf.h"
#include "tensorflow/core/platform/env.h"
#include "tensorflow/core/platform/logging.h"
#include "tensorflow/core/platform/mutex.h"
#include "tensorflow/core/platform/types.h"
#include "tensorflow/core/public/session.h"
#include "tensorflow/core/util/stat_summarizer.h"

#include <sstream>
#include <fstream>

/**
 * @brief Constructor of TensorflowNNInstanceClassifier.
 *
 * Basically just calls the TensorflowNNInstance super constructor,
 * which you may refer to for a more detailed description.
 *
 * @see
 *
 * @param TensorflowNNModelDescription nnModelDescription the model description
 * containing all information needed to load and use the Tensorflow model.
 *
 * @param int numThreads number of threads Tensorflow is allowed to use.
 */
FilterManagementLibrary::TFIntegration::TensorflowNNInstanceClassifier::
	TensorflowNNInstanceClassifier(
		TensorflowNNModelDescription nnModelDescription,
		int numThreads):
		TensorflowNNInstance(nnModelDescription, numThreads),
		labelCount(-1)
{

}

/**
 * @brief Reads labels from a file.
 *
 * The specified file will be read line by line whereas each line represents
 * an entry in the loadedLabels vector.
 *
 * @param std::string path path to the labels file.
 *
 * @return bool true if the labels file exists and could be read,
 * false otherwise
 */

bool FilterManagementLibrary::TFIntegration::TensorflowNNInstanceClassifier::
		readLabelsFile(std::string path)
{
	std::ifstream file(path);
	if (!file)
	{
		this->lastError = TensorflowNNInstance::ErrorType::
				ERROR_TFNN_CLASSIFIER_INVALID_LABELS_FILE;
	    return false;
	}

	loadedLabels.clear();
	std::string line;
	while (std::getline(file, line))
	{
		loadedLabels.push_back(line);
	}

	this->labelCount = loadedLabels.size();
	const int padding = 16;
	while (loadedLabels.size() % padding)
	{
		loadedLabels.emplace_back();
	}
	return true;
}

/**
 * @brief Return the top N predictions of the network and their confidences.
 *
 * Returns the top N confidence values over threshold in the provided vector,
 * sorted by confidence in descending order.
 *
 * @param const int numPredictions number of predictions that the
 * outputList shall contain (the 'N' in getTopNPredictions(...))
 *
 * @param const float threshold min confidence value a prediction needs to
 * have to be included in the outputList
 *
 * @param std::vector<std::pair<float, int> >* outputList pointer to
 * a vector where the results will be written to.
 */
bool FilterManagementLibrary::TFIntegration::TensorflowNNInstanceClassifier::
 getTopNPredictions(const int numPredictions,
		const float threshold,
		std::vector<std::pair<float, int> >* outputList)
{


	tensorflow::Tensor* output =  &this->outputTensors[0];
	auto prediction = output->flat<float>();

  // Will contain top N results in ascending order.
  std::priority_queue<std::pair<float, int>,
                      std::vector<std::pair<float, int> >,
                      std::greater<std::pair<float, int> > >
      top_result_pq;

  const int count = prediction.size();
  std::cout << "count " << count;
  for (int i = 0; i < count; ++i) {
    const float value = prediction(i);

    // Only add it if it beats the threshold and has a chance at being in
    // the top N.

    if (value < threshold) {
      continue;
    }

    top_result_pq.push(std::pair<float, int>(value, i));

    // If at capacity, kick the smallest value out.
    if (top_result_pq.size() > numPredictions) {
      top_result_pq.pop();
    }
  }

  // Copy to output vector and reverse into descending order.
  while (!top_result_pq.empty()) {
	  outputList->push_back(top_result_pq.top());
    top_result_pq.pop();
  }
  std::reverse(outputList->begin(), outputList->end());
  return true;
}

/**
 * @brief Return the top N labels of the network and their confidences.
 *
 * Uses getTopNPredictions(...) to get a list of the top N confidence
 * predictions and their confidence values, and then uses the ID's
 * to get the corresponding label tag from the labels file which
 * should have been loaded prior to calling this function.
 *
 * @param const int numPredictions number of predictions that the
 * outputList shall contain (the 'N' in getTopNPredictions(...))
 *
 * @param const float threshold min confidence value a prediction needs to
 * have to be included in the outputList
 *
 * @param std::vector<std::string>* outputList pointer to
 * a vector where the labels will be written to.
 */
bool FilterManagementLibrary::TFIntegration::TensorflowNNInstanceClassifier::
getTopNPredictionsStringVector(const int numPredictions,
						const float threshold,
						std::vector<std::string> *outputList)
{
	std::vector<std::pair<float, int> > predictions;

	bool success = this->getTopNPredictions(numPredictions, threshold, &predictions);

	outputList->clear();

	if(success)
	{
		std::stringstream stringstream;

		for(std::pair<float, int> prediction : predictions)
		{
			stringstream.str("");
			std::cout << prediction.second << "  " << this->labelCount << std::endl;
			if(prediction.second <= this->labelCount)
			{
				stringstream << this->loadedLabels[prediction.second]
							 << ": " << prediction.first;
				outputList->push_back(stringstream.str());
			}
			else
			{
				std::cout << "second \n";
				return false;

			}
		}
		return true;
	}
	else
		return false;
}


