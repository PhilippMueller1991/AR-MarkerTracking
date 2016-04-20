#include <iostream>
#include <opencv2\opencv.hpp>

#define KEY_ESC 27

int main(int arc, char** argv)
{
	std::cout << "OpenCV version: " << CV_VERSION << std::endl;

	cv::VideoCapture cap(0);

	if (!cap.isOpened())
	{
		printf("Couldn't open camera, exiting program.");
		return EXIT_SUCCESS;
	}

	// open window and attach trackbar
	cv::namedWindow("camera", 1);
	cv::namedWindow("debug", 1);
	cv::namedWindow("result", 1);
	int threshold = 50;
	cv::createTrackbar("threshold", "camera", &threshold, 255, nullptr);

	// main loop
	int lastKey = -1;
	cv::Mat image;
	while (lastKey != KEY_ESC)
	{
		// get camera frame
		cv::Mat frame;
		cap >> frame;

		// convert image to black and white and threshold
		cv::cvtColor(frame, image, CV_RGB2GRAY);
		cv::threshold(image, image, threshold, 255, cv::THRESH_BINARY);

		// find contours and restrict number of endpoints
		cv::Mat debugImg = image.clone();
		std::vector<std::vector<cv::Point>> contours;
		cv::findContours(image, contours, CV_RETR_TREE, CV_CHAIN_APPROX_SIMPLE);

		// convert debug image back to color space and draw debug lines
		cv::cvtColor(image, image, CV_GRAY2BGR);
		cv::drawContours(image, contours, 0, cv::Scalar(0, 0, 250, 1), 2);

		// show images
		cv::imshow("camera", frame);
		cv::imshow("debug", debugImg);
		cv::imshow("result", image);
		
		// pull key
		lastKey = cv::waitKey(1);
	}

	cap.release();	// Release VideoCapture
	
	return EXIT_SUCCESS;
}
