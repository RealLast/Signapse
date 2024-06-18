cmake_minimum_required(VERSION 3.4.1)
project( DisplayImage )
find_package( OpenCV REQUIRED )
add_executable( rsapi 
main.cpp 
flatbuffers/code_generators.cpp
flatbuffers/idl_gen_fbs.cpp
flatbuffers/idl_gen_general.cpp
flatbuffers/idl_gen_text.cpp
flatbuffers/idl_parser.cpp
flatbuffers/reflection.cpp
flatbuffers/util.cpp

source_files/FilterManagementLibrary/TensorflowIntegration/image_helpers/image_helpers.cc
source_files/FilterManagementLibrary/TensorflowIntegration/TensorflowNNInstance.cpp
source_files/FilterManagementLibrary/TensorflowIntegration/TensorflowNNInstanceClassifier.cpp
source_files/FilterManagementLibrary/Utilities.cpp
)

target_include_directories(rsapi PRIVATE tensorflow/include header_files)
add_library(libtensorflowLite SHARED IMPORTED)
set_target_properties(libtensorflowLite PROPERTIES IMPORTED_LOCATION
    tensorflow/lib/libtensorflowLite.so)

target_link_libraries( rsapi ${OpenCV_LIBS} libtensorflowLite  )

