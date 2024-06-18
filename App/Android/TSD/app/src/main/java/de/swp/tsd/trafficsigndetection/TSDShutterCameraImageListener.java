/*
 * Code work or ensemble by Florian Köhler based on https://github.com/miyosuda/
 * TensorFlowAndroidDemo/blob/master/app/src/main/java/org/tensorflow/demo/
 * TensorFlowImageListener.java
 * Special thanks to Stackoverflow and Google
 */

package de.swp.tsd.trafficsigndetection;

import android.app.Activity;
import android.graphics.Bitmap;
import android.graphics.Rect;
import android.media.Image;
import android.media.ImageReader;
import android.support.annotation.NonNull;
import android.util.Log;
import android.widget.ImageView;

import java.nio.ByteBuffer;
import java.util.ArrayList;
import java.util.List;

/**
 * This class is used as image listener for {@link TSDShutterCamera} instances. It reads on every
 * signal to {@link TSDShutterCameraImageListener#onImageAvailable(ImageReader)} the latest image
 * in, extracts the yuv bytes and passes the data to the RoadSignAPI. The API returns the rgb image
 * data and information about found signs, which get processed to
 * {@link TSDSignCombination sign combinations} and are added to the given {@link TSDSignHistory}.
 * Currently only signs will be extracted from the image, when the yuv image delivered by the
 * camera has a pixel stride of two.
 */
public class TSDShutterCameraImageListener implements ImageReader.OnImageAvailableListener
{
    private final static int[] DEVICE_ROTATION = new int[] { 0, 90, 180, 270 };

    private final static int DECT_SIGN_STRIDE = 5;

    private Activity mActivity;
    private ImageView mPostView;
    private TSDSignHistory mSignHistory;
    private TSDDrawableDatabase mDatabase;
    private FrameEventListener mFrameEventListener;

    private long mSignCombiId;
    private long mFrameId;
    private int mImageWidth;
    private int mImageHeight;
    private Bitmap mBitmap;
    private byte[] mYuvData;
    private int[] mArgbData;
    private long mLastFrameFinished;
    private int mSensorRotation;

    private int mFramesSkipped;


    /**
     * This method converts the given yuv image data with a pixel stride of two - which is most
     * common, to rgb image data in native code. Then it feeds the image data into the RoadSign
     * pipeline and extracts the found traffic signs.
     * @param width Width of the image in pixels.
     * @param height Height of the image in pixels.
     * @param yuv Byte array containing the yuv data with pixel stride of two.
     * @param rgb Byte array that will contain the rgb data after the method was called.
     * @param stride Uv pixel stride of the yuv data to determine in which format the yuv byte array
     *               is written. If the stride is 1, then the yuv byte array is in I420 format, if
     *               the pixel stride is 2, then it is in NV21 format.
     * @param rotate Integer to describe the necessary image rotation to get straight up.
     * @return 2D array of integers containing information about the found signs grouped in sign
     * combinations. Each array in this array defines one combination of signs. The first value
     * in each array contains the number of signs in the combination. Then following for each sign
     * in this order there is the classification id of the detected sign, the x and y axis of the
     * upper left corner of the rect where the sign was detected in the image and the x and y axis
     * of the lower right corner of the detected rect.
     */
    public native int[][] roadSignAPIfeedImage(int width, int height,  byte yuv[], int[] rgb,
                                               int stride, int rotate);


    /**
     * This constructor creates a new instance of TSDShutterCameraImageListener. If the history or
     * the database is null, then the sign combinations can not be added to some history.
     * @param history Sign history the image listener should send the new recognized sign combis to.
     * @param database Drawable database to be used for loading sing drawables.
     * @param postView If not null, results from native are displayed in this image view.
     * @param listener Listener which gets notified every time a frame is finished delivering
     *                 information about the time it took to process it.
     * @param activity Parent activity
     */
    TSDShutterCameraImageListener(TSDSignHistory history, TSDDrawableDatabase database,
                                  ImageView postView, FrameEventListener listener,
                                  @NonNull Activity activity)
    {
        mSignHistory = history;
        mActivity = activity;
        mDatabase = database;
        mPostView = postView;
        mFrameEventListener = listener;
        mSensorRotation = -90;

        // Find the latest ids to start off from and add some margin to to avoid updates of older
        // sign combinations.
        if (history != null)
        {
            mSignCombiId = history.getLatestId() + 10;
            mFrameId = history.getLatestFrameId() + 10;
        }
    }


    /**
     * Setter for the sensor rotation value used to correct image orientation problems. This value
     * should be the one queried from {@link TSDCameraCharacter#getSensorRotation()}.
     * @param sensorRotation {@link TSDCameraCharacter#getSensorRotation()} of the camera used to
     *                       capture the images.
     */
    public void setSensorRotation(int sensorRotation)
    {
        mSensorRotation = sensorRotation;
    }


    /**
     * This method gets signaled every time an image is available from the {@link TSDShutterCamera}
     * instance this is used in. It reads in the latest image and processes it as described in
     * {@link TSDShutterCameraImageListener}. At the end the image is closed and thus the slot
     * in the image reader freed for a new image from the camera.
     * @param reader Image reader  used
     */
    @Override
    public void onImageAvailable(final ImageReader reader)
    {
        Image image = reader.acquireLatestImage();

        if (image != null)
        {
            // Only process every second frame to ensure getting the latest one.
            if (mFramesSkipped > 0)
            {
                // Query device rotation.
                final int deviceRotationIndex = mActivity.getWindowManager()
                        .getDefaultDisplay().getRotation();
                final int deviceRotation = DEVICE_ROTATION[deviceRotationIndex];
                final int rotateImageBy = deviceRotation - mSensorRotation;

                // Initialize buffers and bitmap for new or changed dimensions.
                if (image.getWidth() != mImageWidth && image.getHeight() != mImageHeight) {
                    // Save dimensions.
                    mImageWidth = reader.getWidth();
                    mImageHeight = reader.getHeight();

                    // Initialize YUV buffer.
                    Image.Plane yPlane = image.getPlanes()[0];
                    int ySize = yPlane.getBuffer().remaining();
                    mYuvData = new byte[ySize + (ySize / 2)];

                    // Initialize ARGB Buffer.
                    mArgbData = new int[mImageWidth * mImageHeight];

                    // Initialize bitmap.
                    mBitmap = Bitmap.createBitmap(mImageWidth, mImageHeight, Bitmap.Config.ARGB_8888);

                    // Initialize the timestamp to track the time it takes to process a frame.
                    mLastFrameFinished = System.currentTimeMillis();
                }

                // Read image channels.
                Image.Plane yPlane = image.getPlanes()[0];
                Image.Plane uPlane = image.getPlanes()[1];
                Image.Plane vPlane = image.getPlanes()[2];

                // be aware that this size does not include the padding at the end, if there is any
                // (e.g. if pixel stride is 2 the size is ySize / 2 - 1)
                final int ySize = yPlane.getBuffer().remaining();
                final int uSize = uPlane.getBuffer().remaining();
                final int vSize = vPlane.getBuffer().remaining();

                yPlane.getBuffer().get(mYuvData, 0, ySize);
                ByteBuffer ub = uPlane.getBuffer();
                ByteBuffer vb = vPlane.getBuffer();

                //stride guaranteed to be the same for u and v planes
                int[][] dectCombis;
                int uvPixelStride = uPlane.getPixelStride();
                if (uvPixelStride == 1)
                {
                    uPlane.getBuffer().get(mYuvData, ySize, uSize);
                    vPlane.getBuffer().get(mYuvData, ySize + uSize, vSize);
                    dectCombis = roadSignAPIfeedImage(mImageWidth, mImageHeight, mYuvData, mArgbData,
                            uvPixelStride, rotateImageBy);
                }
                else
                {
                    // If pixel stride is 2 there is padding between each pixel
                    // Converting it to NV21 by filling the gaps of the v plane with the u values
                    vb.get(mYuvData, ySize, vSize);
                    for (int i = 0; i < uSize; i += 2)
                    {
                        mYuvData[ySize + i + 1] = ub.get(i);
                    }
                    dectCombis = roadSignAPIfeedImage(mImageWidth, mImageHeight, mYuvData, mArgbData,
                            uvPixelStride, rotateImageBy);
                }

                if (dectCombis != null)
                {
                    // Create bitmap from returned argb data.
                    mBitmap.setPixels(mArgbData, 0, mImageWidth, 0, 0, mImageWidth,
                            mImageHeight);

                    // If post view is not null, then isRelevant the resulting image.
                    if (mPostView != null)
                    {
                        mPostView.post(() -> mPostView.setImageBitmap(mBitmap));
                    }

                    // Update history if available.
                    if (mSignHistory != null && mDatabase != null)
                    {
                        // Create an extra list for the converted combinations in which they are
                        // inserted in descending order in regards to their distance to the image
                        // center.
                        // This sorting is used to fake the sorting by the distance to the camera.
                        // Signs farthest away should get the latest entry in the history in this frame.
                        // As an approximation can be taken that signs more close to the center of the
                        // image are probably farther away than other closer to the edge of the image.
                        List<TSDSignCombination> convCombis = new ArrayList<>();

                        // Get each detected sign combination which is stored in one array.
                        for (int[] dectCombi : dectCombis)
                        {
                            // Read the number of signs in this combination from the first entry.
                            int signCount = dectCombi[0];

                            // Read sign information.
                            List<TSDSign> signs = new ArrayList<>();
                            List<Rect> rects = new ArrayList<>();
                            for (int j = 0; j < signCount; j++)
                            {
                                // Convert the given sign detection id to an enum.
                                TSDSign sign = TSDSign.fromId(dectCombi[1 + j * DECT_SIGN_STRIDE]);

                                // Skip the signs which should not be displayed.
                                if (sign.isRelevant())
                                {
                                    signs.add(sign);

                                    final int left = dectCombi[1 + j * DECT_SIGN_STRIDE + 1];
                                    final int top = dectCombi[1 + j * DECT_SIGN_STRIDE + 2];
                                    final int right = dectCombi[1 + j * DECT_SIGN_STRIDE + 3];
                                    final int bottom = dectCombi[1 + j * DECT_SIGN_STRIDE + 4];

                                    rects.add(new Rect(left, top, right, bottom));
                                }
                            }

                            // Only add a new sign combination to history if after filtering the
                            // ones not to isRelevant out at least one is left.
                            if (signs.size() > 0)
                            {
                                // Create sign combination instance with detected signs.
                                TSDSignCombination combi = new TSDSignCombination(++mSignCombiId,
                                        mFrameId, signs.toArray(new TSDSign[signs.size()]),
                                        rects.toArray(new Rect[rects.size()]), mBitmap,
                                        System.currentTimeMillis(), null, mDatabase,
                                        mActivity);

                                // Find the index of the combination, that has a smaller distance to
                                // the center than this combination. Then insert the new one at this
                                // index. This will sort the list by descending distance to the
                                // image center.
                                int i = 0;
                                for (; i < convCombis.size(); i++)
                                {
                                    if (convCombis.get(i).getDistanceToImageCenter()
                                            < combi.getDistanceToImageCenter())
                                    {
                                        break;
                                    }
                                }
                                convCombis.add(i, combi);
                            }
                        }

                        // Add the converted sign combinations to the sign history. Because they are
                        // sorted in descending distance from the image center, those closer to the
                        // center will be added last.
                        for (TSDSignCombination combi : convCombis)
                        {
                            mSignHistory.addSignCombination(combi, mActivity);
                        }
                    }
                }

                // Get the new timestamp after having processed the frame.
                final long newFrameFinished = System.currentTimeMillis();

                // Signal the frame time to the listener if available. Do this on ui thread to avoid
                // issues when wanting to update ui stuff from the listener.
                if (mFrameEventListener != null)
                {
                    // Calculate the time before sending it to the ui thread because
                    // mLastFrameFinished gets updated shortly after this and will make the
                    // calculation useless.
                    final long time = newFrameFinished - mLastFrameFinished;
                    mActivity.runOnUiThread(() ->
                            mFrameEventListener.onFrameFinished(time));
                }

                // Update the timestamp.
                mLastFrameFinished = newFrameFinished;

                // Increase the counter for analysed frames.
                mFrameId++;
            }
            else
            {
                mFramesSkipped++;
            }

            // Free the image place to enable the camera to take a new picture.
            image.close();
        }
    }


    /**
     * This interface is used to receive events of the frame processing. Actually it is only for the
     * event when frames are finished to get processed.
     */
    public interface FrameEventListener
    {
        /**
         * This method is called every time one camera frame is finished.
         * @param duration Time it took to process the frame in milliseconds.
         */
        void onFrameFinished(long duration);
    }
}
