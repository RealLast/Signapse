/*
 * TensorflowAndroidJNIUtils.h
 *
 *  Created on: 18.06.2018
 *      Author: oracion
 *
 *  Code adapted from TensorflowAndroidDemo Project:
 *  https://github.com/miyosuda/TensorFlowAndroidDemo
 * Copyright 2015 The TensorFlow Authors. All Rights Reserved.

 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at

    http://www.apache.org/licenses/LICENSE-2.0

 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.*/

#ifndef HEADER_FILES_FILTERMANAGEMENTLIBRARY_TENSORFLOWANDROIDJNIUTILS_H_
#define HEADER_FILES_FILTERMANAGEMENTLIBRARY_TENSORFLOWANDROIDJNIUTILS_H_



#include <jni.h>
#include <string>
#include <vector>
#include <string>

#include "tensorflow/core/platform/types.h"

namespace google {
namespace protobuf {
class MessageLite;
}  // google
}  // protobuf

class AAssetManager;

namespace FilterManagementLibrary
{
	class TensorflowAndroidJNIUtils
	{
		public:
			bool static PortableReadFileToProto(const std::string& file_name,
										 ::google::protobuf::MessageLite* proto);

			bool static IsAsset(std::string filename);

			void static ReadFileToProto(AAssetManager* const asset_manager,
					std::string filename, google::protobuf::MessageLite* message);

			void static ReadFileToString(AAssetManager* const asset_manager,
					std::string filename, std::string* str);

			void static ReadFileToVector(AAssetManager* const asset_manager,
					std::string filename, std::vector<std::string>* str_vector);

			void static WriteProtoToFile(std::string filename,
								  const google::protobuf::MessageLite& message);
	};

}


#endif /* HEADER_FILES_FILTERMANAGEMENTLIBRARY_TENSORFLOWANDROIDJNIUTILS_H_ */
