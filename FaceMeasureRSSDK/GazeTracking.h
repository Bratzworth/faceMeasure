#pragma once
#ifndef GAZETRACKING
#define GAZETRACKING

#include <windows.h>
#include <wchar.h>
#include <iostream>
#include <algorithm>
#include "pxcsensemanager.h"
#include "face_render.h"
#include "RealSense/Face/FaceData.h"
#include "RealSense/Face/FaceConfiguration.h"
#include "RealSense/Face/FaceModule.h"
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>

#define FRAME_WIDTH				640
#define	FRAME_HEIGHT			480

#define HALFSQUARE				5

#define MILLIMETERS_IN_METER	1000

using namespace cv;
using namespace std;
using namespace Intel::RealSense;

int centeredRectangle(Mat image, PXCPointI32 center);
int clearCenteredRectangle(Mat image, PXCPointI32 center);

#endif
