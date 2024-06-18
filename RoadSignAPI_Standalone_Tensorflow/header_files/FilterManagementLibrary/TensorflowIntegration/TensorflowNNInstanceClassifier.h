/*
 * TensorflowNNInstanceClassifier.h
 *
 *  Created on: 19.05.2018
 *      Author: Patrick Langer
 */

#ifndef HEADER_FILES_FILTERMANAGEMENTLIBRARY_TENSORFLOWNNINSTANCECLASSIFIER_H_
#define HEADER_FILES_FILTERMANAGEMENTLIBRARY_TENSORFLOWNNINSTANCECLASSIFIER_H_

#include <string>
#include <vector>
#include "FilterManagementLibrary/TensorflowIntegration/TensorflowNNInstance.h"

namespace FilterManagementLibrary
{
	namespace TFIntegration
	{
		/**
		 * This is just a very basic class to fastly implement classification
		 * models like MobilenetV1/2 or Inception(V3) as they use the same
		 * method for output encoding (check getTopNPredictions(...) for more).
		 * It is not used in any of the productive PipeSystem Code, so see it more
		 * as an example or class for debugging and testing.
		 *
		 * It only needs to be provided a correct label file for the
		 * corresponding trained model and a correct model description.
		 * Afterwards, getTopNPredictions(...) and
		 * getTopNPredictionsStringVector(...) can be used for easy output
		 * interpretation (don't forget to call runInference() first though).
		 */
		class TensorflowNNInstanceClassifier : public TensorflowNNInstance
		{
			private:

				/**
				 * List of labels loaded from file
				 */
				std::vector<std::string> loadedLabels;

				/**
				 * Number of labels that were read.
				 * Assume these are equal to the output size of the used model!
				 */
				int labelCount;

			public:
				bool readLabelsFile(std::string path);
				bool getTopNPredictions(const int numPredictions,
						const float threshold,
						std::vector<std::pair<float, int> >* outputList);

				bool getTopNPredictionsStringVector(const int numPredictions,
						const float threshold,
						std::vector<std::string> *outputList);

				TensorflowNNInstanceClassifier(
						TensorflowNNModelDescription nnModelDescription,
					int numThreads = 1);

		};
	}
}


#endif /* HEADER_FILES_FILTERMANAGEMENTLIBRARY_TENSORFLOWNNINSTANCECLASSIFIER_H_ */
