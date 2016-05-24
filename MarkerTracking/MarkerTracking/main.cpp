#include <iostream>
// opencv headers
#include <opencv2\opencv.hpp>

#define KEY_ESC 27


void initVideoStream(cv::VideoCapture &cap) {
	if (cap.isOpened())
		cap.release();

	cap.open(0); // open the default camera
	if (cap.isOpened() == false) {
		std::cout << "No webcam found, using a video file" << std::endl;
		cap.open("MarkerMovie.mpg");
		if (cap.isOpened() == false) {
			std::cout << "No video file found. Exiting." << std::endl;
			exit(0);
		}
	}
}

int main(int arc, char** argv)
{
	std::cout << "OpenCV version: " << CV_VERSION << std::endl;

	cv::VideoCapture cap;
	initVideoStream(cap);

	if (!cap.isOpened())
	{
		printf("Couldn't open camera, exiting program.");
		return EXIT_SUCCESS;
	}

	// Create window and attach trackbar
	cv::namedWindow("debug", 1);
	cv::namedWindow("result", 1);
	int sliderValue = 50;
	cv::createTrackbar("threshold", "debug", &sliderValue, 255);

	// MAIN LOOP
	std::vector<std::vector<cv::Point>> contours;
	std::vector<cv::Point> rect;
	CvMemStorage* memStorage = cvCreateMemStorage();
	// Images
	cv::Mat imgColor;
	cv::Mat imgGray;
	cv::Mat imgFiltered;
	while (true)
	{
		// Get camera frame
		cap >> imgColor;

		if (imgColor.empty())
		{
			std::cout << "Could not query frame. Trying to reinitialize." << std::endl;
			initVideoStream(cap);
			cv::waitKey(1000);
			continue;
		}

		// Convert image to black and white and threshold
		cv::cvtColor(imgColor, imgGray, CV_BGR2GRAY);
		cv::threshold(imgGray, imgFiltered, sliderValue, 255, cv::THRESH_BINARY);
		cv::imshow("debug", imgFiltered);

		// Find contours (old OpenCV API)
		CvSeq* contours;
		CvMat imgFilteredCpy(imgFiltered);
		cvFindContours(&imgFilteredCpy, memStorage, &contours, sizeof(CvContour), CV_RETR_LIST, CV_CHAIN_APPROX_SIMPLE);
		
		// For each header in contours we approximate a polygon and count the edges
		for (; contours; contours = contours->h_next)
		{
			// Approximate polygons
			CvSeq* polys = cvApproxPoly(contours, sizeof(CvContour), memStorage, CV_POLY_APPROX_DP, cvContourPerimeter(contours) * 0.02, 0);

			if (polys->total != 4)
				continue;

			cv::Mat result = cv::cvarrToMat(polys);
			cv::Rect r = cv::boundingRect(result);

			// Discard rects that are too small or too big
			if(r.height<20 || r.width<20 || r.height > imgFiltered.rows -10 || r.width > imgFiltered.cols - 10)
				continue;

			const cv::Point* rect = (const cv::Point*) result.data;
			int numPoints = result.rows;

			cv::polylines(imgColor, &rect, &numPoints, 1, true, CV_RGB(255,0,0), 2);

			for (int i = 0; i < 4; i++)
			{
				cv::Point2d dir = cv::Point2d(rect[(i + 1) % 4] - rect[i]) / 7.0;
				for (int j = 1; j < 7; j++)
				{
					cv::Point2d pos = cv::Point2d(rect[i]) + j * dir;
					cv::circle(imgColor, pos, 2, CV_RGB(0, 0, 255), -1);
				}
				cv::circle(imgColor, rect[i], 3, CV_RGB(0, 255, 0), -1);
			}
		}
		
		cv::imshow("result", imgColor);
		// Clear
		cvClearMemStorage(memStorage);
		// Poll keys
		if(cv::waitKey(10) == KEY_ESC) break;
	}

	cvReleaseMemStorage(&memStorage);	// Release contours memory storage
	cap.release();	// Release VideoCapture
	
	cv::destroyAllWindows();

	return EXIT_SUCCESS;
}
