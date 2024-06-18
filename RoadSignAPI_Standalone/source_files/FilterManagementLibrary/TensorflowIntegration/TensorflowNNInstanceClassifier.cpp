/*
 * TensorflowNNInstanceClassifier.cpp
 *
 *  Created on: 19.05.2018
 *      Author: oracion
 */


#include "FilterManagementLibrary/TensorflowIntegration/TensorflowNNInstanceClassifier.h"
#include "FilterManagementLibrary/TensorflowIntegration/label_helpers/get_top_n.h"

#include <sstream>
#include <fstream>

FilterManagementLibrary::TFIntegration::TensorflowNNInstanceClassifier::
	TensorflowNNInstanceClassifier(std::string modelFile,
		NeuronalNetworkImageProcessSettings imageProcessSettings,
		int numThreads, bool useNNAPI):
		TensorflowNNInstance(modelFile, imageProcessSettings, numThreads,
				useNNAPI),
		labelCount(-1)
{

}

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

bool FilterManagementLibrary::TFIntegration::TensorflowNNInstanceClassifier::
		getTopNPredictions(int desiredPredictions, float threshold,
				std::vector<std::string>& outputList)
{


	   std::vector<std::pair<float, int>> top_results;

	   int output = this->tfLiteInterpreter->outputs()[0];

	   switch (this->tfLiteInterpreter->tensor(output)->type)
	   {
	     case kTfLiteFloat32:
	     {
	    	 get_top_n<float>(this->tfLiteInterpreter->
	    	 	    		  	  typed_output_tensor<float>(0),
	    	 	    		  	  this->labelCount, desiredPredictions,
								  threshold, &top_results, true);
	     }

	     break;
	     case kTfLiteUInt8:
	     {
	    	 get_top_n<uint8_t>(this->tfLiteInterpreter->
	    	 	    		   typed_output_tensor<uint8_t>(0),
	    	 	    		   this->labelCount, desiredPredictions,
							   threshold, &top_results, true);
	     }

	      break;
	     default:
	     {
	    	 this->lastError = TensorflowNNInstance::ErrorType::
	    			 ERROR_NN_UNSUPPORTED_OUTPUT_TYPE;
	    	 return false;
	     }
	   }

	   std::stringstream stringstream;
	   outputList.clear();

	   float confidence = 0;
	   int index = 0;

	   for (const auto& result : top_results)
	   {
		   stringstream.str("");
		   confidence = result.first;
		   index = result.second;

		   stringstream << confidence << ": " << index << " "
				   << this->loadedLabels[index];
		   outputList.push_back(stringstream.str());
	   }
	   return true;

}

