cmake_minimum_required(VERSION 3.4.1)
project( DisplayImage )
find_package( OpenCV REQUIRED )
add_executable( rsapi 

main.cpp

source_files/FilterManagementLibrary/TensorflowIntegration/TensorflowNNInstance.cpp
source_files/FilterManagementLibrary/TensorflowIntegration/TensorflowNNInstanceClassifier.cpp
source_files/FilterManagementLibrary/TensorflowIntegration/TensorflowResultContainer.cpp
source_files/FilterManagementLibrary/PipeSystem/ProcessingPipeline.cpp
source_files/FilterManagementLibrary/PipeSystem/PipeFilter.cpp
source_files/FilterManagementLibrary/PipeSystem/TFNNBasedPipeFilter.cpp
source_files/FilterManagementLibrary/Utilities.cpp
source_files/FilterManagementLibrary/TensorflowOpenCVUtils.cpp
source_files/FilterManagementLibrary/Logger.cpp

source_files/RoadSignAPI/Filters/DetectionBasedImageSlicer.cpp
source_files/RoadSignAPI/Filters/MobilenetV2RoadSignClassificator.cpp
source_files/RoadSignAPI/Filters/SSDLiteRoadSignDetector.cpp
source_files/RoadSignAPI/Filters/ClassifiedSignsGrouper.cpp
source_files/RoadSignAPI/Filters/RoadSignDuplicationDeleter.cpp
source_files/RoadSignAPI/DetectedSignCombination.cpp
source_files/RoadSignAPI/RoadSignAPI.cpp
header_files/FilterManagementLibrary/TensorflowIntegration/TensorflowNNModelDescription.h

)

target_include_directories(rsapi PRIVATE 

"dependencies/include/tensorflow"
"dependencies/include/tensorflow/tensorflow/contrib/makefile/gen/host_obj"
"dependencies/include/tensorflow/tensorflow/contrib/makefile/downloads"
"dependencies/include/tensorflow/tensorflow/contrib/makefile/downloads/eigen"
"dependencies/include/tensorflow/tensorflow/contrib/makefile/downloads/gemmlowp"
"dependencies/include/tensorflow/tensorflow/contrib/makefile/downloads/nsync/public"
"dependencies/include/tensorflow/tensorflow/contrib/makefile/gen/proto"
"dependencies/include/tensorflow/tensorflow/contrib/makefile/gen/proto_text"
"dependencies/include/tensorflow/tensorflow/contrib/makefile/gen/protobuf-host/include"
"header_files"
)




add_library(libprotobuf STATIC IMPORTED)
set_target_properties(libprotobuf PROPERTIES IMPORTED_LOCATION
    dependencies/lib/tensorflow/libprotobuf.a)

add_library(nsync STATIC IMPORTED)
set_target_properties(nsync PROPERTIES IMPORTED_LOCATION
    dependencies/lib/tensorflow/nsync.a)

target_link_libraries( rsapi ${OpenCV_LIBS} 
"-Wl,--allow-multiple-definition"
"-Wl,--whole-archive dependencies/lib/tensorflow/libtensorflow-core.a"
"-Wl,--no-whole-archive"
dl pthread

libprotobuf nsync)

