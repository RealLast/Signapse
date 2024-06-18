OpenCV mat to tensorflow with CV FakeMat

cv::Mat image = cv::imread("grace_hopper.bmp");
gettimeofday(&start_time, NULL);
// get pointer to memory for that Tensor

Mat scaledImage(settings.inputHeigth, settings.inputWidth, CV_32FC3);
cv::resize(image, scaledImage, cv::Size(settings.inputHeigth, settings.inputWidth), 0.0, 0.0, INTER_CUBIC );
	int i = 0;

cv::cvtColor(scaledImage, scaledImage, cv::COLOR_BGR2RGB);

// We need a third map because if we would only do
// Mat scaledImage(settings.inputHeigth, settings.inputWidth, CV_32FC3, p);
// this would fail (probably OpenCV allocates a new temporary array when using resize)

// get pointer to memory for that Tensor
float *p = tfInstance.getInputTensor()->flat<float>().data();
// create a "fake" cv::Mat from it
cv::Mat cameraImg(settings.inputHeigth, settings.inputWidth, CV_32FC3, p);
//cv::cvtColor(cameraImg, cameraImg, cv::COLOR_BGR2RGB);
scaledImage.convertTo(cameraImg, CV_32FC3);
