/*
 * image_helpers.cc
 *
 *  Created on: 16.05.2018
 *      Author: oracion
 */




#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <unistd.h>  // NOLINT(build/include_order)

#include "FilterManagementLibrary/TensorflowIntegration/image_helpers/image_helpers.h"

#define LOG(x) std::cerr

namespace tflite {


uint8_t* decode_bmp(const uint8_t* input, int row_size, uint8_t* const output,
                    int width, int height, int channels, bool top_down) {
	 printf("Lines: %d\n", width*height);
	for (int i = 0; i < height; i++) {
    int src_pos;
    int dst_pos;

    for (int j = 0; j < width; j++) {
      if (!top_down) {
        src_pos = ((height - 1 - i) * row_size) + j * channels;
      } else {
        src_pos = i * row_size + j * channels;
      }


      dst_pos = (i * width + j) * channels;

      switch (channels) {
        case 1:
          output[dst_pos] = input[src_pos];
          printf("case 1\n");
          break;
        case 3:
          // BGR -> RGB

          output[dst_pos] = input[src_pos + 2];
          output[dst_pos + 1] = input[src_pos + 1];
          output[dst_pos + 2] = input[src_pos];
          printf("%d dst\n", dst_pos + 2);
      //    printf("%d %d %d\n", output[dst_pos], output[dst_pos + 1], output[dst_pos + 2]);
          break;
        case 4:
          // BGRA -> RGBA
          printf("case 4\n");
          output[dst_pos] = input[src_pos + 2];
          output[dst_pos + 1] = input[src_pos + 1];
          output[dst_pos + 2] = input[src_pos];
          output[dst_pos + 3] = input[src_pos + 3];
          break;
        default:
          LOG(FATAL) << "Unexpected number of channels: " << channels;
          break;
      }
    }
  }

  return output;
}



}
