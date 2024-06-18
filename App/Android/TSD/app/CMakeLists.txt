# For more information about using CMake with Android Studio, read the
# documentation: https://d.android.com/studio/projects/add-native-code.html

# Sets the minimum version of CMake required to build the native library.

cmake_minimum_required(VERSION 3.4.1)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=gnu++11")
# Creates and names a library, sets it as either STATIC
# or SHARED, and provides the relative paths to its source code.
# You can define multiple libraries, and CMake builds them for you.
# Gradle automatically packages shared libraries with your APK.

# Searches for a specified prebuilt library and stores the path as a
# variable. Because CMake includes system libraries in the search path by
# default, you only need to specify the name of the public NDK library
# you want to add. CMake verifies that the library exists before
# completing its build.

find_library( # Sets the name of the path variable.
              log-lib

              # Specifies the name of the NDK library that
              # you want CMake to locate.
              log )

find_library( # Sets the name of the path variable.
              android-lib

              # Specifies the name of the NDK library that
              # you want CMake to locate.
              android )

# Specifies libraries CMake should link to your target library. You
# can link multiple libraries, such as libraries you define in this
# build script, prebuilt third-party libraries, or system libraries.


set(DEPENDENCIES_DIR ${CMAKE_SOURCE_DIR}/dependencies)
set(ROADSIGN_API_DIR ${CMAKE_SOURCE_DIR}/../../../../RoadSignAPI_Standalone_Tensorflow)
set(OPENCV_SDK_DIR ${CMAKE_SOURCE_DIR}/../../../../RoadSignAPI_Standalone_Tensorflow/build_for_android/jni/opencv-android-sdk)


add_library( # Sets the name of the library.
             native-lib

             # Sets the library as a shared library.
             SHARED

             # Provides a relative path to your source file(s).
             src/main/cpp/native-lib.cpp

            ${ROADSIGN_API_DIR}/source_files/FilterManagementLibrary/TensorflowIntegration/TensorflowNNInstance.cpp
            ${ROADSIGN_API_DIR}/source_files/FilterManagementLibrary/TensorflowIntegration/TensorflowNNInstanceClassifier.cpp
            ${ROADSIGN_API_DIR}/source_files/FilterManagementLibrary/TensorflowIntegration/TensorflowResultContainer.cpp
            ${ROADSIGN_API_DIR}/source_files/FilterManagementLibrary/PipeSystem/ProcessingPipeline.cpp
            ${ROADSIGN_API_DIR}/source_files/FilterManagementLibrary/PipeSystem/PipeFilter.cpp
            ${ROADSIGN_API_DIR}/source_files/FilterManagementLibrary/PipeSystem/TFNNBasedPipeFilter.cpp
            ${ROADSIGN_API_DIR}/source_files/FilterManagementLibrary/Utilities.cpp
            ${ROADSIGN_API_DIR}/source_files/FilterManagementLibrary/TensorflowOpenCVUtils.cpp
            ${ROADSIGN_API_DIR}/source_files/FilterManagementLibrary/TensorflowAndroidJNIUtils.cpp
            ${ROADSIGN_API_DIR}/source_files/FilterManagementLibrary/Logger.cpp
            ${ROADSIGN_API_DIR}/source_files/RoadSignAPI/Filters/DetectionBasedImageSlicer.cpp
            ${ROADSIGN_API_DIR}/source_files/RoadSignAPI/Filters/MobilenetV2RoadSignClassificator.cpp
            ${ROADSIGN_API_DIR}/source_files/RoadSignAPI/Filters/SSDLiteRoadSignDetector.cpp
            ${ROADSIGN_API_DIR}/source_files/RoadSignAPI/Filters/ClassifiedSignsGrouper.cpp
            ${ROADSIGN_API_DIR}/source_files/RoadSignAPI/Filters/RoadSignDuplicationDeleter.cpp
            ${ROADSIGN_API_DIR}/source_files/RoadSignAPI/DetectedSignCombination.cpp
            ${ROADSIGN_API_DIR}/source_files/RoadSignAPI/RoadSignAPI.cpp

             )





add_library(libprotobuf STATIC IMPORTED)
set_target_properties(libprotobuf PROPERTIES IMPORTED_LOCATION
    ${DEPENDENCIES_DIR}/lib/tensorflow/${ANDROID_ABI}/libprotobuf.a)

add_library(nsync STATIC IMPORTED)
set_target_properties(nsync PROPERTIES IMPORTED_LOCATION
    ${DEPENDENCIES_DIR}/lib/tensorflow/${ANDROID_ABI}/libnsync.a)

set(CMAKE_VERBOSE_MAKEFILE on)
add_library(lib_opencv SHARED IMPORTED)
set_target_properties(lib_opencv PROPERTIES IMPORTED_LOCATION ${OPENCV_SDK_DIR}/sdk/native/libs/${ANDROID_ABI}/libopencv_java3.so)


target_include_directories(native-lib PRIVATE
"${ROADSIGN_API_DIR}/dependencies/include/tensorflow"
"${ROADSIGN_API_DIR}/dependencies/include/tensorflow/tensorflow/contrib/makefile/gen/host_obj"
"${ROADSIGN_API_DIR}/dependencies/include/tensorflow/tensorflow/contrib/makefile/downloads"
"${ROADSIGN_API_DIR}/dependencies/include/tensorflow/tensorflow/contrib/makefile/downloads/eigen"
"${ROADSIGN_API_DIR}/dependencies/include/tensorflow/tensorflow/contrib/makefile/downloads/gemmlowp"
"${ROADSIGN_API_DIR}/dependencies/include/tensorflow/tensorflow/contrib/makefile/downloads/nsync/public"
"${ROADSIGN_API_DIR}/dependencies/include/tensorflow/tensorflow/contrib/makefile/gen/proto"
"${ROADSIGN_API_DIR}/dependencies/include/tensorflow/tensorflow/contrib/makefile/gen/proto_text"
"${ROADSIGN_API_DIR}/dependencies/include/tensorflow/tensorflow/contrib/makefile/gen/protobuf-host/include"
"${ROADSIGN_API_DIR}/header_files"
${OPENCV_SDK_DIR}/sdk/native/jni/include
)



target_link_libraries( # Specifies the target library.
                       native-lib
                       lib_opencv
                       "-Wl,--allow-multiple-definition"
                       "-Wl,--whole-archive ${DEPENDENCIES_DIR}/lib/tensorflow/${ANDROID_ABI}/libtensorflow-core.a"
                       "-Wl,--no-whole-archive"

                       libprotobuf nsync
                       # Links the target library to the log library
                       # included in the NDK.
                       ${log-lib}
                       ${android-lib})

