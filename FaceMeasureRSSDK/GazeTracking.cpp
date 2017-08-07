#include "GazeTracking.h"

int wmaint(int argc, WCHAR* argv[])
{
	// error check
	pxcStatus sts;

	// creating PXCSenseManager
	PXCSenseManager *psm = 0;
	psm = PXCSenseManager::CreateInstance();
	if (!psm)
	{
		wprintf_s(L"Unable to create the PXCSenseManager\n");
		return 1;
	}

	// enable face analysis
	sts = psm->EnableFace();
	if (sts < PXC_STATUS_NO_ERROR)
	{
		wprintf_s(L"Error enabling Face Analysis\n");
		return 2;
	}
	// retrieve face module
	Face::FaceModule* faceAnalyzer = psm->QueryFace();
	if (!faceAnalyzer)
	{
		wprintf_s(L"Error retrieving face results\n");
		return 3;
	}
	// creating the openCV window
	namedWindow("Calibration Window", WINDOW_AUTOSIZE);

	// creating a blank background for calibration
	Mat image(FRAME_HEIGHT,FRAME_WIDTH, CV_8UC1, Scalar(0,255,0));

	putText(image, "test", Point(50, 50), CV_FONT_HERSHEY_PLAIN, 2, Scalar(255, 255, 255));
	// facec is a PXCFaceConfiguration instance
	// faced is GazeCalibData instance
	

	/*

	Face::FaceConfiguration* facec = faceAnalyzer->CreateActiveConfiguration();
	facec->SetTrackingMode(Face::FaceConfiguration::TrackingModeType::FACE_MODE_COLOR_PLUS_DEPTH);

	Face::FaceConfiguration::GazeConfiguration *gazec = facec->QueryGaze();

	gazec->isEnabled = true;
	facec->ApplyChanges();
	*/

	// initialize the PXCSenseManager
	if (psm->Init() < PXC_STATUS_NO_ERROR)
		return 5;
	Face::FaceData* outputData = faceAnalyzer->CreateOutput();
	if(outputData->QueryFaceByID(0)!=NULL)
		Face::FaceData::GazeCalibData* faced = outputData->QueryFaceByID(0)->QueryGazeCalibration();
	//if (!faced)
	//{
	//	wprintf_s(L"Error retrieving gaze calibration\n");
	//	return 4;
	//}


	while (psm->AcquireFrame(true) >= PXC_STATUS_NO_ERROR)
	{
		PXCFaceModule *face2 = psm->QueryFace();
	
	/*	if (face2)
		{
			PXCPointI32 calibp = {};
			Face::FaceData::GazeCalibData::CalibrationState state = faced->QueryCalibrationState();

			switch (state)
			{
			case Face::FaceData::GazeCalibData::CALIBRATION_IDLE:
				// visual clue calib process starts, or load calib data
				cvPutText(&image, "test", Point(0, 0), &temp_font, Scalar(255, 255, 255));
				break;
			case Face::FaceData::GazeCalibData::CALIBRATION_NEW_POINT:
				// visual cue that a new calibration point is avaliable
				clearCenteredRectangle(&image, calibp);
				calibp = faced->QueryCalibPoint();
				centeredRectangle(&image, calibp);
				break;
			case Face::FaceData::GazeCalibData::CALIBRATION_SAME_POINT:
				// continue visual cue to user at same location
				break;
			case Face::FaceData::GazeCalibData::CALIBRATION_DONE:
				// save calibration data, signal to the user that the calibration is complete
				break;
			}

			imshow("Calibration Window", image);
		}
	*/	
		waitKey();
		imshow("Calibration Window", image);
		psm->ReleaseFrame();
	}


	// close streams and release any session and processing module instances
	psm->Release();

	return 0;
}

int centeredRectangle(Mat image, PXCPointI32 center)
{
	// calculate the top left corner
	Point pt1 = Point(center.x - HALFSQUARE, center.y - HALFSQUARE);
	// boundary checking
	if (center.x < HALFSQUARE)
		pt1.x = 0;
	if (center.y < HALFSQUARE)
		pt1.y = 0;

	// calculate the bottom right corner
	Point pt2 = Point(center.x + HALFSQUARE, center.y + HALFSQUARE);
	// boundary checking
	if (center.x + HALFSQUARE > FRAME_WIDTH)
		pt1.x = FRAME_WIDTH;
	if (center.y + HALFSQUARE > FRAME_HEIGHT)
		pt1.y = FRAME_HEIGHT;
	
	rectangle(image, pt1, pt2, Scalar(255,255,255));

	return 0;
}

int clearCenteredRectangle(Mat image, PXCPointI32 center)
{
	// calculate the top left corner
	Point pt1 = Point(center.x - HALFSQUARE, center.y - HALFSQUARE);
	// boundary checking
	if (center.x < HALFSQUARE)
		pt1.x = 0;
	if (center.y < HALFSQUARE)
		pt1.y = 0;

	// calculate the bottom right corner
	Point pt2 = Point(center.x + HALFSQUARE, center.y + HALFSQUARE);
	// boundary checking
	if (center.x + HALFSQUARE > FRAME_WIDTH)
		pt1.x = FRAME_WIDTH;
	if (center.y + HALFSQUARE > FRAME_HEIGHT)
		pt1.y = FRAME_HEIGHT;

	rectangle(image, pt1, pt2, Scalar(0, 0, 0));

	return 0;
}