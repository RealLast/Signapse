#include <jni.h>
#include <string>

#include <android/asset_manager.h>
#include <android/asset_manager_jni.h>

#include "RoadSignAPI/RoadSignAPI.h"
#include "FilterManagementLibrary/Logger.h"
#include "cstdint"


typedef cv::Point3_<uint8_t> Pixel;



double get_us(struct timeval t)
{
    return (t.tv_sec * 1000000 + t.tv_usec);
}

bool roadSignAPIStaticInit(const int numThreads, AAssetManager* const assetManager)
{
 
    return RoadSignAPI::RoadSignAPI::staticInit(numThreads, assetManager);
}

jobjectArray roadSignAPIStaticFeedImage(JNIEnv* env,
                                        jobject thiz,
                                        jint width,
                                        jint height,
                                        jbyteArray YUVFrameData,
                                        jintArray bgra,
                                        jint stride)
{
    jbyte * _yuv = env->GetByteArrayElements(YUVFrameData, 0);
    jint* _bgra = env->GetIntArrayElements(bgra, 0);




    struct timeval start_time, stop_time;
    gettimeofday(&start_time, NULL);
    cv::Mat yuvMat = cv::Mat(height + (height / 2), width, CV_8UC1, _yuv);
    cv::Mat rgbMat = cv::Mat(height, width, CV_8UC3);

    if (stride == 1)
    {
        cv::cvtColor(yuvMat, rgbMat, cv::COLOR_YUV2BGR_I420, 3);
    }
    else
    {
        cv::cvtColor(yuvMat, rgbMat, cv::COLOR_YUV2BGR_NV21, 3);
    }

    yuvMat.release();



    RoadSignAPI::RoadSignAPI::staticFeedImage(rgbMat);
    const std::vector<RoadSignAPI::DetectedSignDescriptor>* detections =
            RoadSignAPI::RoadSignAPI::staticGetDetectedSigns();

    std::vector <RoadSignAPI::DetectedSignDescriptor>::const_iterator iterator =
            detections->begin();
    std::stringstream ss;


    int** signsOnPole = new int*[detections->size()];

    for(int i = 0; i < detections->size(); i++)
    {
        const RoadSignAPI::DetectedSignDescriptor* detectedSignDescriptor =
                &(*iterator);

        int numSignsOnPole = 1;
        signsOnPole[i] = new int[6];
        signsOnPole[i][0] = numSignsOnPole;
        signsOnPole[i][1] = detectedSignDescriptor->detectionPredictedClassID;
        signsOnPole[i][2] = detectedSignDescriptor->upperLeft.x;
        signsOnPole[i][3] = detectedSignDescriptor->upperLeft.y;
        signsOnPole[i][4] = detectedSignDescriptor->lowerRight.x;
        signsOnPole[i][5] = detectedSignDescriptor->lowerRight.y;

        cv::rectangle(rgbMat, detectedSignDescriptor->upperLeft,
                detectedSignDescriptor->lowerRight, cv::Scalar(0, 255, 0), 3);
        ss.str("");
        
        /*labels[detectedSignDescriptor->detectionPredictedClassID - 1] << ": "
                <<*/ 
                
        ss << detectedSignDescriptor->detectorConfidence;
        cv::putText(rgbMat,
                    ss.str().c_str(),
                    cv::Point(detectedSignDescriptor->upperLeft.x,
                            detectedSignDescriptor->upperLeft.y - 20),
                            // Coordinates
                    cv::FONT_HERSHEY_COMPLEX_SMALL, // Font
                    1.0, // Scale. 2.0 = 2x bigger
                    cv::Scalar(255,255,255), // BGR Color
                    3);


        *iterator++;
    }

    int numCombiDetections = detections->size();

    // Get the float array class
    jclass intArrayClass = env->FindClass("[I");

    jobjectArray myReturnable2DArray =
            env->NewObjectArray((jsize) numCombiDetections, intArrayClass,
            NULL);
    
    if (intArrayClass == NULL)
    {
        return NULL;
    }


    // Go through the firs dimension and add the second dimension arrays
    for (unsigned int i = 0; i < numCombiDetections; i++)
    {
        int signs = 1;
        jint length2D = 1 + 5 * signs;
        jintArray intArray = env->NewIntArray(length2D);
        env->SetIntArrayRegion(intArray, (jsize) 0, (jsize) length2D,
                (int*) signsOnPole[i]);
        env->SetObjectArrayElement(myReturnable2DArray, (jsize) i, intArray);
        env->DeleteLocalRef(intArray);
    }

    // cleanup array.
    for(int i = 0; i < numCombiDetections; i++)
    {
        delete[] signsOnPole[i];
    }
    delete[] signsOnPole;


    rgbMat.forEach<Pixel>
            (
                    [&](Pixel &p, const int *position) -> void
                    {

                        int color = (int) p.z << 16 | (int) p.y << 8 |
                                (int) p.x | 0xFF000000;
                        _bgra[position[1] + position[0] * width] = color;

                    }
            );

    env->ReleaseByteArrayElements(YUVFrameData, _yuv, 0);
    env->ReleaseIntArrayElements(bgra, _bgra, 0);

    gettimeofday(&stop_time, NULL);

    FilterManagementLibrary::Logger::printfln("Time: %f",
            ((get_us(stop_time) - get_us(start_time)) / (1000.0)));
    return myReturnable2DArray;
}

extern "C" JNIEXPORT jboolean

JNICALL
Java_de_swp_tsd_trafficsigndetection_MainActivity_initRoadSignAPI(
        JNIEnv *env,
        jobject /* this */, jint numThreads, jobject javaAssetManager) {

    AAssetManager* const assetManager =
    AAssetManager_fromJava(env, javaAssetManager);
    return roadSignAPIStaticInit(numThreads, assetManager);
}


extern "C" JNIEXPORT jobjectArray
JNICALL
Java_de_swp_tsd_trafficsigndetection_TSDShutterCameraImageListener_roadSignAPIfeedImage(
        JNIEnv* env, jobject thiz,
        jint width, jint height,
        jbyteArray YUVFrameData, jintArray bgra, jint stride)
{
    return roadSignAPIStaticFeedImage(env, thiz, width, height, YUVFrameData,
            bgra, stride);
}
