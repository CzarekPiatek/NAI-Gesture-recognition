
#include "pch.h"
#include <opencv/cv.hpp>
#include <opencv/highgui.h>
#include <iostream>

using namespace cv;
using namespace std;
const int imgwidth = 640, imgheight = 480;

int main() {

	VideoCapture cam(0);

	if (!cam.isOpened())
	{
		cout << "Nie moge uruchomic kamery!" << endl;
		return 0;
	}

	//stale dla wielkosci okna
	

	//pokazanie okna o nazwie HSV
	namedWindow("HSV", CV_WINDOW_AUTOSIZE);

	//ustalanie wartosci poczatkowych
	int lowHSV[3] = { 30,76,0 };
	int highHSV[3] = { 84,255,107 };

	//ustawianie suwaczkow dla progow HSV
	cvCreateTrackbar("lowH", "HSV", &lowHSV[0], 179);
	cvCreateTrackbar("highH", "HSV", &highHSV[0], 179);
	cvCreateTrackbar("lowS", "HSV", &lowHSV[1], 255);
	cvCreateTrackbar("highS", "HSV", &highHSV[1], 255);
	cvCreateTrackbar("lowV", "HSV", &lowHSV[2], 255);
	cvCreateTrackbar("highV", "HSV", &highHSV[2], 255);

	while (true)
	{
		Mat imgBGR, imgHSV, imgThresholded, dilated, eroded;

		//wczytanie kolejnej klatki z video
		bool successRead = cam.read(imgBGR);

		if (!successRead)
		{
			cout << "Nie moge odczytac klatki" << endl;
			break;
		}

		//konwersja z bgr na hsv
		cvtColor(imgBGR, imgHSV, COLOR_BGR2HSV);

		//wykrywanie obiektu ktory bazuje na wartosciach HSV
		inRange(imgHSV, Scalar(lowHSV[0], lowHSV[1], lowHSV[2]), Scalar(highHSV[0], highHSV[1], highHSV[2]), imgThresholded);
		
		//dilate powiększa obramówkę obiektu
		//erode zmniejsza obramówkę, wygładza
		Mat erodeElement = getStructuringElement(MORPH_RECT,Size(4, 4));
		Mat dilateElement = getStructuringElement(MORPH_RECT, Size(8, 8));
		erode(imgThresholded, eroded, erodeElement);
		dilate(eroded, dilated, dilateElement);

		//wypisywanie klatek na ekran
		imshow("HSV", imgHSV);
		imshow("Thresholded Image", imgThresholded);
		imshow("Dilated", dilated);
		imshow("Eroded", eroded);
		imshow("Original", imgBGR);

		//oczekiwanie 30ms na ESC ktory przerywa petle
		if (waitKey(30) == 27)
		{
			cout << "Wcisnieto ESC!" << endl;
			break;
		}

	}
	cam.release();
	return 0;
}