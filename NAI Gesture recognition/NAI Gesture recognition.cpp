
#include "pch.h"
#include <opencv/cv.hpp>
#include <opencv/highgui.h>
#include <iostream>
#include <fstream>
#include <list>
using namespace cv;
using namespace std;

bool check(int x, int y, double factor)
{
	return ((x <= factor) && (y >= factor)) ? true : false;
}

int main(int argc, char** argv) {

	VideoCapture cam(0);

	if (!cam.isOpened())
	{
		cout << "Nie moge uruchomic kamery!" << endl;
		return 0;
	}

	list < Point2f > path;

	//pokazanie okna o nazwie HSV
	namedWindow("HSV", CV_WINDOW_AUTOSIZE);

	//ustalanie wartosci poczatkowych
	int lowHSV[3] = { 30,76,0 };
	int highHSV[3] = { 84,255,255 };

	//ustawianie suwaczkow dla progow HSV
	cvCreateTrackbar("lowH", "HSV", &lowHSV[0], 179);
	cvCreateTrackbar("highH", "HSV", &highHSV[0], 179);
	cvCreateTrackbar("lowS", "HSV", &lowHSV[1], 255);
	cvCreateTrackbar("highS", "HSV", &highHSV[1], 255);
	cvCreateTrackbar("lowV", "HSV", &lowHSV[2], 255);
	cvCreateTrackbar("highV", "HSV", &highHSV[2], 255);
	fstream L, Z, W;
	int Ltab[4], Ztab[6], Wtab[8], x, y, pom = 0;
	L.open("L.txt"); Z.open("Z.txt"); W.open("w.txt");
	while (L >> x >> y)
	{
		Ltab[pom] = x; Ltab[pom + 1] = y;
		pom = pom + 2;
	}
	pom = 0;
	while (Z >> x >> y)
	{
		Ztab[pom] = x; Ztab[pom + 1] = y;
		pom = pom + 2;
	}
	pom = 0;
	while (W >> x >> y)
	{
		Wtab[pom] = x; Wtab[pom + 1] = y;
		pom = pom + 2;
	}
	pom = 0;
	L.close(); Z.close(); W.close();
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

		//odwrocenie kamery
		flip(imgBGR, imgBGR, 1);

		//konwersja z bgr na hsv
		cvtColor(imgBGR, imgHSV, COLOR_BGR2HSV);
		
		//wykrywanie obiektu ktory bazuje na wartosciach HSV z zakresu wybranego na suwaczkach
		inRange(imgHSV, Scalar(lowHSV[0], lowHSV[1], lowHSV[2]), Scalar(highHSV[0], highHSV[1], highHSV[2]), imgThresholded);

		//dilate powiększa obramówkę obiektu
		//erode zmniejsza obramówkę, wygładza
		//blur rozmywa
		Mat erodeElement = getStructuringElement(MORPH_RECT, Size(4, 4));
		Mat dilateElement = getStructuringElement(MORPH_RECT, Size(8, 8));
		erode(imgThresholded, eroded, erodeElement);
		dilate(eroded, dilated, dilateElement);
		blur(dilated, dilated, Size(7, 7));

		//kontury
		vector <vector<Point>> contours;
		findContours(dilated, contours, CV_RETR_CCOMP, CV_CHAIN_APPROX_SIMPLE);
		vector <pair <double, vector<Point>>> sortedContours;
		for (int i = 0; i < contours.size(); i++)
		{
			drawContours(imgBGR, contours, i, Scalar(0, 255, 0), 2);
			sortedContours.push_back({ contourArea(contours[i],false) ,contours[i] });
		}

		if (sortedContours.size() > 0) {
			Point2f center;
			float radius;
			sort(sortedContours.begin(), sortedContours.end(), [](auto a, auto b) {
				return a.first > b.first;
			});
			minEnclosingCircle(sortedContours[0].second, center, radius);
			if (radius > 8) {
				if (path.size() < 70) {
					path.push_back(center); // dopisujemy srodek okregu
				}
				else {
					path.pop_front();
					path.push_back(center);
				}
				vector <Point> pathV;
				vector <Point2f> temp;
				approxPolyDP(vector<Point2f>(path.begin(), path.end()), temp, 30, false);
				circle(imgBGR, center, radius, Scalar(255,0,0), 2);
				for (auto &p : temp) pathV.push_back({ (int)p.x,(int)p.y });
				polylines(imgBGR, { pathV }, false, Scalar(0, 255, 0), 2);
				if (pathV.size() >= 6) {
					vector <Point> itr(pathV.end() - 6, pathV.end());
					int z = 0, w = 0, l = 0;
					double factor = (::abs(itr[0].x - itr[1].x) + ::abs(itr[0].y - itr[1].y)) * 2 / 3;
					for (int i = 0; i <= 2; i++)
					{
						if (check((::abs(itr[i].x - itr[i+1].x))*Ltab[i*2], (::abs(itr[i].y - itr[i+1].y))*Ltab[i*2+1], factor))
						{
							l++;
						}
					}
					/*for (int i = 0; i <= 3; i++)
					{
						if (check((::abs(itr[i].x - itr[i + 1].x))*Ztab[i * 2], (::abs(itr[i].y - itr[i + 1].y))*Ztab[i * 2 + 1], factor))
						{
							z++;
							cout << "Z= " << z << endl;
						}
					}
					*/
					for (int i = 0; i < 4; i++)
					{
						if (check((::abs(itr[i].x - itr[i + 1].x))*Wtab[i * 2], (::abs(itr[i].y - itr[i + 1].y))*Wtab[i * 2 + 1], factor))
						{
							w++;
						}
					}
					/*
					if (z == 3) {
						cout << "Rozpoznano Z!" << endl;
						path.clear();
					}*/
					if (w == 4) {
						cout << "Rozpoznano W!" << endl;
						path.clear();
					}
					if (l == 2) {
						cout << "Rozpoznano L!" << endl;
						path.clear();
					}
				}
			}
		}
		//wypisywanie klatek na ekran
		if ((string)argv[1] == "window")
		{
			imshow("HSV", imgHSV);
			imshow("Thresholded Image", imgThresholded);
			imshow("Dilated", dilated);
			imshow("Eroded", eroded);
		}
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

