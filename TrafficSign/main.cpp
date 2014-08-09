#pragma once
#include "utils.h"

/************************************************************************/
/* Calculates distance between two points                               */
/************************************************************************/
float ptDist(const Point2f & a, const Point2f & b)
{
	return sqrt( pow(a.x - b.x, 2.0f) + pow(a.y - b.y, 2.0f) );
}

/************************************************************************/
/* Given a crop image, outputs a best circle                            */
/************************************************************************/
bool segmentation(const Mat & input, RotatedRect & candidate, const float & maxDistortion=0.25f, const float & minSize=0.40f, const float & maxSize=0.90f)
{
	Mat tmp;
	if (input.channels() == 1)
		input.copyTo(tmp);
	else
		cvtColor(input, tmp, CV_BGR2GRAY);

	Mat thresh;
	vector<vector<Point> > contours;
 	blur(input, thresh, Size(3, 3));
	Canny(thresh, thresh, 50, 100);
	Mat kernel = getStructuringElement(MORPH_ELLIPSE , Size(3, 3));
	morphologyEx(thresh, thresh, CV_MOP_CLOSE, kernel);
	findContours(thresh, contours, CV_RETR_LIST, CV_CHAIN_APPROX_NONE);
	vector<RotatedRect> potentialBoxes;
	int sz = (int)contours.size();
	for (int i=1; i < sz; i++)
	{
		for (int j=0; j <= i; j++)
		{
			vector<Point> tmpContours;
			tmpContours.insert(tmpContours.end(), contours[i].begin(), contours[i].end());
			tmpContours.insert(tmpContours.end(), contours[j].begin(), contours[j].end());
			if ((int)tmpContours.size() >= 5)
			{
				Mat pointsf;
				Mat(tmpContours).convertTo(pointsf, CV_32F);
				RotatedRect box = fitEllipse(pointsf);

				//add restrains to aspect ratio
				float aspectRatio = 1.0f - (box.size.width / box.size.height);
				//add restrain to ellipse size in comparison to image
				float circleSz = __min(box.size.width, box.size.height) / (float)__min(input.cols, input.rows);

				//admit 25% distortion on circle's aspect ratio && circle size between 40% and 90% of image crop itself
				if (abs(aspectRatio) < maxDistortion && circleSz > minSize && circleSz < maxSize)
				{
					potentialBoxes.push_back(box);
				}
			}
		}
	}

	int pSz = (int)potentialBoxes.size();
	if (!pSz)
		return false;

	float minDist = FLT_MAX;
	Point2f ic((float)input.cols/2.0f, (float)input.rows/2.0f);
	for (int i=0; i < pSz; i++)
	{
		float dist = ptDist(ic, potentialBoxes[i].center);
		if (dist < minDist)
		{
			minDist = dist;
			candidate = potentialBoxes[i];
		}
	}
	//fix best candidate
	float fixedSz = __max(candidate.size.width, candidate.size.height);
	candidate.size = Size2f(fixedSz, fixedSz);
	return true;
}

int main(int argc, char** argv)
{
	//get directory contents
	const string path = "Archiv/";
	const string ext = "ppm";
	vector<string> contents;
	get_dir_contents(path, ext, contents);

	//load each image
	int sz = (int)contents.size();
	for (int i=0; i < sz; i++)
	{
		string fn = path + contents[i];
		Mat src = imread(fn);
		Mat dst;
		src.copyTo(dst);
		RotatedRect candidate;
		if (segmentation(src, candidate))
		{
			ellipse(dst, candidate, Scalar(0, 0, 255));
			circle(dst, Point(candidate.center), 1, Scalar(0, 0, 255));
		}
		imwrite(contents[i], dst);
		imshow("dst", dst);
		imshow("src", src);
		waitKey(10);
	}

	return EXIT_SUCCESS;
}