/*
 * image_helpers_impl.h
 *
 *  Created on: 14.05.2018
 *      Author: oracion
 *      Code taken and adapted from tensorflowlite label_image example.
 */

#ifndef IMAGE_HELPERS_IMAGE_HELPERS_IMPL_H_
#define IMAGE_HELPERS_IMAGE_HELPERS_IMPL_H_
#include <stdio.h>

#include "FilterManagementLibrary/TensorflowIntegration/NeuronalNetworkImageProcessSettings.h"
#include "tensorflow/contrib/lite/builtin_op_data.h"
#include "tensorflow/contrib/lite/interpreter.h"
#include "tensorflow/contrib/lite/kernels/register.h"
#include "tensorflow/contrib/lite/string_util.h"
#include "tensorflow/contrib/lite/version.h"

#include "tensorflow/contrib/lite/builtin_op_data.h"
#include "tensorflow/contrib/lite/interpreter.h"
#include "tensorflow/contrib/lite/kernels/register.h"
#include "tensorflow/contrib/lite/string_util.h"
#include "tensorflow/contrib/lite/version.h"

namespace FilterManagementLibrary
{
	namespace TFIntegration
	{
		using namespace tflite;
		template<class T>
		void resize(T* out, uint8_t* in, int image_height, int image_width,
				int image_channels, int wanted_height, int wanted_width,
				int wanted_channels, NeuronalNetworkImageProcessSettings* s)
		{
			int number_of_pixels = image_height * image_width * image_channels;
			std::unique_ptr<Interpreter> interpreter(new Interpreter);

			int base_index = 0;

			// two inputs: input and new_sizes
			interpreter->AddTensors(2, &base_index);
			// one output
			interpreter->AddTensors(1, &base_index);
			// set input and output tensors
			interpreter->SetInputs(
			{ 0, 1 });
			interpreter->SetOutputs(
			{ 2 });

			// set parameters of tensors
			TfLiteQuantizationParams quant;
			interpreter->SetTensorParametersReadWrite(0, kTfLiteFloat32,
					"input",
					{ 1, image_height, image_width, image_channels }, quant);
			interpreter->SetTensorParametersReadWrite(1, kTfLiteInt32,
					"new_size",
					{ 2 }, quant);
			interpreter->SetTensorParametersReadWrite(2, kTfLiteFloat32,
					"output",
					{ 1, wanted_height, wanted_width, wanted_channels }, quant);

			ops::builtin::BuiltinOpResolver resolver;
			TfLiteRegistration* resize_op = resolver.FindOp(
					BuiltinOperator_RESIZE_BILINEAR);
			auto* params = reinterpret_cast<TfLiteResizeBilinearParams*>(malloc(
					sizeof(TfLiteResizeBilinearParams)));
			params->align_corners = false;
			interpreter->AddNodeWithParameters(
			{ 0, 1 },
			{ 2 }, nullptr, 0, params, resize_op, nullptr);

			interpreter->AllocateTensors();

			// fill input image
			// in[] are integers, cannot do memcpy() directly
			auto input = interpreter->typed_tensor<float>(0);
			for (int i = 0; i < number_of_pixels; i++)
			{
				input[i] = in[i];
			}

			// fill new_sizes
			interpreter->typed_tensor<int>(1)[0] = wanted_height;
			interpreter->typed_tensor<int>(1)[1] = wanted_width;

			interpreter->Invoke();

			auto output = interpreter->typed_tensor<float>(2);
			auto output_number_of_pixels = wanted_height * wanted_height
					* wanted_channels;

			for (int i = 0; i < output_number_of_pixels; i++)
			{
				if (s->input_floating)
					out[i] = (output[i] - s->input_mean) / s->input_std;
				else
					out[i] = (uint8_t) output[i];
			}
		}
	}
}

#endif /* IMAGE_HELPERS_IMAGE_HELPERS_IMPL_H_ */
