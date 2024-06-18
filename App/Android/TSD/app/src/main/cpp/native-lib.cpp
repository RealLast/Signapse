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
                                        jint stride,
										jint rotate)
{
    jbyte * _yuv = env->GetByteArrayElements(YUVFrameData, 0);
    jint* _bgra = env->GetIntArrayElements(bgra, 0);



    // Convert YUV data array to cv BGR Mat (BGR, because the RoadSignAPI expects that)
    struct timeval start_time, stop_time;
    gettimeofday(&start_time, NULL);
    cv::Mat yuvMat = cv::Mat(height + (height / 2), width, CV_8UC1, _yuv);
    cv::Mat bgrMat = cv::Mat(height, width, CV_8UC3);

    if (stride == 1)
    {
        cv::cvtColor(yuvMat, bgrMat, cv::COLOR_YUV2BGR_I420, 3);
    }
    else
    {
        cv::cvtColor(yuvMat, bgrMat, cv::COLOR_YUV2BGR_NV21, 3);
    }

    yuvMat.release();

	// If the image has to be rotated by 180° then flip it both horizontally and vertically.
	if (rotate == 180 || rotate == -180)
	{
		cv::flip(bgrMat, bgrMat, -1);
	}

    RoadSignAPI::RoadSignAPI::staticFeedImage(bgrMat);

    const std::vector<RoadSignAPI::DetectedSignCombination>* detectedSignCombinations =
            RoadSignAPI::RoadSignAPI::staticGetDetectedSignCombinations();



    std::stringstream ss;

    int numSignCombinations = detectedSignCombinations->size();


    int** signsOnPole = new int*[numSignCombinations];


    RoadSignAPI::DetectedSignDescriptor* currentSignDescriptor;

    //FilterManagementLibrary::Logger::printfln("numSignCombinations: %d", numSignCombinations);

    int numSignsOnCurrentCombination = 0;
    int arraySizeForCurrentCombination = 0;

    // Get the int array class
    jclass intArrayClass = env->FindClass("[I");

    jobjectArray myReturnable2DArray =
            env->NewObjectArray((jsize) numSignCombinations, intArrayClass,
                                NULL);

    if (intArrayClass == NULL)
    {
        return NULL;
    }




    for(int i = 0; i < numSignCombinations; i++)
    {
        const RoadSignAPI::DetectedSignCombination*
                currentSignCombination = &detectedSignCombinations->at(i);

        const std::vector<RoadSignAPI::DetectedSignDescriptor>* signsInCurrentCombination =
                currentSignCombination->getSignsInCombination();

        int minX = 100000;
        int minY = 100000;
        int maxX = 0;
        int maxY = 0;

        numSignsOnCurrentCombination = currentSignCombination->getDetectedSignsAmount();
        arraySizeForCurrentCombination = 1 + numSignsOnCurrentCombination * 5;
        signsOnPole[i] = new int[arraySizeForCurrentCombination];
        signsOnPole[i][0] = numSignsOnCurrentCombination;

        for(int j = 0; j < currentSignCombination->getDetectedSignsAmount(); j++)
        {
            const RoadSignAPI::DetectedSignDescriptor* currentSignDescriptor =
                   &signsInCurrentCombination->at(j);



            signsOnPole[i][1 + j * 5] = currentSignDescriptor->classifierApprovedClassID;
            signsOnPole[i][2 + j * 5] = currentSignDescriptor->upperLeft.x;
            signsOnPole[i][3 + j * 5] = currentSignDescriptor->upperLeft.y;
            signsOnPole[i][4 + j * 5] = currentSignDescriptor->lowerRight.x;
            signsOnPole[i][5 + j * 5] = currentSignDescriptor->lowerRight.y;


/*

            // Debug (should be removed prior to release)

            if(currentSignDescriptor->upperLeft.x < minX)
                minX = currentSignDescriptor->upperLeft.x;

            if(currentSignDescriptor->upperLeft.y < minY)
                minY = currentSignDescriptor->upperLeft.y;

            if(currentSignDescriptor->lowerRight.x > maxX)
                maxX = currentSignDescriptor->lowerRight.x;

            if(currentSignDescriptor->lowerRight.y > maxY)
                maxY = currentSignDescriptor->lowerRight.y;

            ss.str("");

            /*labels[detectedSignDescriptor->detectionPredictedClassID - 1] << ": "
                    <<

            int poleXLeft = currentSignCombination->getGestimatedPolePositionX() - (bgrMat.cols * 0.025f);
            int poleXRight = currentSignCombination->getGestimatedPolePositionX() + (bgrMat.cols * 0.025f);

            if(poleXLeft < 0)
                poleXLeft = 0;

            if(poleXRight > bgrMat.cols)
                poleXRight = bgrMat.cols;

            cv::rectangle(bgrMat, cv::Point(poleXLeft, 0), cv::Point(poleXRight, 700), cv::Scalar(0, 255, 255), 3);
            cv::rectangle(bgrMat, currentSignDescriptor->upperLeft,
                          currentSignDescriptor->lowerRight, cv::Scalar(0, 255, 0), 3);

            ss << currentSignDescriptor->classifierApprovedClassID << " " <<
               currentSignDescriptor->classifierConfidence;
            cv::putText(bgrMat,
                        ss.str().c_str(),
                        cv::Point(currentSignDescriptor->upperLeft.x,
                                  currentSignDescriptor->upperLeft.y - 20),
                    // Coordinates
                        cv::FONT_HERSHEY_COMPLEX_SMALL, // Font
                        1.0, // Scale. 2.0 = 2x bigger
                        cv::Scalar(255,255,255), // BGR Color
                        3);
                        */

        }
      //  cv::rectangle(bgrMat, cv::Point(minX, minY), cv::Point(maxX, maxY), cv::Scalar(0, 0, 255), 3);

        jint length2D = 1 + 5 * numSignsOnCurrentCombination;
        jintArray intArray = env->NewIntArray(length2D);
        env->SetIntArrayRegion(intArray, (jsize) 0, (jsize) length2D,
                               (int*) signsOnPole[i]);
        env->SetObjectArrayElement(myReturnable2DArray, (jsize) i, intArray);
        env->DeleteLocalRef(intArray);
    }




    // cleanup array.
    for(int i = 0; i < numSignCombinations; i++)
    {
        delete[] signsOnPole[i];
    }
    delete[] signsOnPole;

    // Copy in reverse order, as Java part expects RGB whereas RoadSignAPI uses BGR
    bgrMat.forEach<Pixel>
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

    //FilterManagementLibrary::Logger::printfln("Time: %f",
    //                                          ((get_us(stop_time) - get_us(start_time)) / (1000.0)));
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
        jbyteArray YUVFrameData, jintArray bgra, jint stride, jint rotate)
{
    return roadSignAPIStaticFeedImage(env, thiz, width, height, YUVFrameData,
                                      bgra, stride, rotate);
}