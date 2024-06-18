/*
 * TensorflowNNInstanceClassifier.h
 *
 *  Created on: 19.05.2018
 *      Author: oracion
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
		class TensorflowNNInstanceClassifier : public TensorflowNNInstance
		{
			private:

				// List of labels loaded from file
				std::vector<std::string> loadedLabels;

				// Number of labels that were read.
				// Assume these are equal to the output size of the used model!
				int labelCount;

			public:
				bool readLabelsFile(std::string path);
				bool getTopNPredictions(int desiredPredictions,
						float threshold, std::vector<std::string>& outputList);

				TensorflowNNInstanceClassifier(std::string modelFile,
					NeuronalNetworkImageProcessSettings imageProcessSettings,
					int numThreads = 1);

		};
	}
}


#endif /* HEADER_FILES_FILTERMANAGEMENTLIBRARY_TENSORFLOWNNINSTANCECLASSIFIER_H_ */
