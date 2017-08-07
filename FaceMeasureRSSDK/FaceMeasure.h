#ifndef FACEMEASURE
#define FACEMEASURE

#include <algorithm>
#include <iostream>
#include <fstream>
#include <iomanip>
#include <ctime>

#include <windows.h>
#include <wchar.h>
#include <stdlib.h>

#include "pxcsensemanager.h"
#include "RealSense/Face/FaceData.h"
#include "RealSense/Face/FaceConfiguration.h"
#include "RealSense/Face/FaceModule.h"
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>

#define FRAME_WIDTH				1280
#define BAR_WIDTH				210
#define	FRAME_HEIGHT			720
#define FRAME_RATE				30

#define NUM_POINTS				17
#define NUM_MEASUREMENTS		11
#define NUM_SAMPLES				5
#define HALFSQUARE				1

#define CONFIDENCE_THRESHOLD	100
#define ACCEPTABLE_VARIANCE		1.0
#define MILLIMETERS_IN_METER	1000
#define REPEAT_PRESS_WAIT		5

#define X_DIMENSION				0
#define Y_DIMENSION				1
#define XY_DIMENSION			2

#define MEASUREMENT_SCALE		1.2788

#define ESC_ASCII				27
#define SPACE_ASCII				32
#define R_ASCII					114

#define TEXT_MARGIN				5
#define TEXT_HEIGHT				40

//  used landmarks
#define INNER_LEFT_EYE			10
#define	TOP_LEFT_EYELID			12
#define OUTER_LEFT_EYE			14
#define BOTTOM_LEFT_EYELID		16
#define INNER_RIGHT_EYE			18
#define TOP_RIGHT_EYELID		20
#define OUTER_RIGHT_EYE			22
#define BOTTOM_RIGHT_EYELID		24
#define NASAL_BRIDGE			26
#define	LEFT_NOSE				30
#define BOTTOM_CENTER_NOSE		31
#define RIGHT_NOSE				32
#define STOMION_UPPER			47
#define STOMION_LOWER			51
#define CHIN					61
#define	LEFT_IRIS				76
#define RIGHT_IRIS				77


using namespace Intel::RealSense;

static const int LANDMARK_ALIGNMENT = 0;

// a functor that displays a distance value at the next appropriate location
class Print {
public:
	int displayDistance(cv::Mat image, double* averages, uint index, int num_lines);
private:
	int current_y = 0;
};

// creates image file and csv
void outputFiles(cv::Mat image, double* averages);

// makes sure confidence is 100
int checkConfidence(Face::FaceData::LandmarkPoint* landmarkPoints, pxcI32 numPoints);

// checks to see if the variance of all samples is within a defined threshold
int checkVariance(double* samples, int sample_number);

// adds a new sample to each measurement's sample set, replacing the oldest piece of data
double addData(double* samples, uint time, Intel::RealSense::Point3DF32 p1, Intel::RealSense::Point3DF32 p2, int dimension);

// measures distance between two points based on a specified dimension
double calcDistance(Intel::RealSense::Point3DF32 p1, Intel::RealSense::Point3DF32 p2, int dimension);
double calcDistance_xy(Intel::RealSense::Point3DF32 p1, Intel::RealSense::Point3DF32 p2);
double calcDistance_x(Intel::RealSense::Point3DF32 p1, Intel::RealSense::Point3DF32 p2);
double calcDistance_y(Intel::RealSense::Point3DF32 p1, Intel::RealSense::Point3DF32 p2);

// converts a pxc image to a cv matrix
cv::Mat PXCImage2CVMat(PXCImage *pxcImage, PXCImage::PixelFormat format);

#endif
