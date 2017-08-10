#include "FaceMeasure.h"

int WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, char*, int nShowCmd)
{
	// error check
	pxcStatus sts;

	// creating PXCSenseManager
	PXCSenseManager *psm = 0;
	psm = PXCSenseManager::CreateInstance();
	if (!psm)
	{
		//wprintf_s(L"Unable to create the PXCSenseManager\n");
		return 1;
	}

	// enable face analysis
	sts = psm->EnableFace();
	if (sts < PXC_STATUS_NO_ERROR)
	{
		//wprintf_s(L"Error enabling Face Analysis\n");
		return 2;
	}
	// retrieve face module
	Face::FaceModule* faceAnalyzer = psm->QueryFace();
	if (!faceAnalyzer)
	{
		//wprintf_s(L"Error retrieving face results\n");
		return 3;
	}

	// creating opencv window and image
	cv::namedWindow("Face Measuring Utility", cv::WINDOW_AUTOSIZE);
	cv::Mat video_frame = cv::Mat::zeros(cv::Size(FRAME_WIDTH, FRAME_HEIGHT), CV_8UC3);

	// enabling color stream
	psm->EnableStream(PXCCapture::STREAM_TYPE_COLOR, FRAME_WIDTH, FRAME_HEIGHT, FRAME_RATE);

	// initialize the PXCSenseManager
	if(psm->Init() < PXC_STATUS_NO_ERROR)
		return 4;

	Face::FaceData* outputData = faceAnalyzer->CreateOutput();
	Face::FaceConfiguration* config = faceAnalyzer->CreateActiveConfiguration();
	config->SetTrackingMode(Face::FaceConfiguration::TrackingModeType::FACE_MODE_COLOR_PLUS_DEPTH);

	// making an array with all the relevant landmarks
	int measurement_locs[NUM_POINTS] = {
		INNER_LEFT_EYE, 
		TOP_LEFT_EYELID, 
		OUTER_LEFT_EYE, 
		BOTTOM_LEFT_EYELID, 
		INNER_RIGHT_EYE,
		TOP_RIGHT_EYELID, 
		OUTER_RIGHT_EYE, 
		BOTTOM_RIGHT_EYELID,
		NASAL_BRIDGE,
		LEFT_NOSE,
		BOTTOM_CENTER_NOSE, 
		RIGHT_NOSE,
		STOMION_UPPER,
		STOMION_LOWER,
		CHIN,
		LEFT_IRIS,
		RIGHT_IRIS
	};

	// making an array to track the last couple of measurements
	double measurements_total[NUM_MEASUREMENTS][NUM_SAMPLES] = { 0 };

	// an array to hold measurement averages
	double measurement_averages[NUM_MEASUREMENTS] = { 0 };

	// setting up landmarks
	config->landmarks.isEnabled = true;
	// enabling alerts
	config->EnableAllAlerts();
	config->ApplyChanges();

	// a toggled flag for when esc is hit
	bool esc_pressed = false;

	// a tracker to prevent errors from hitting the spacebar too rapidly
	uint time_since_keypress = REPEAT_PRESS_WAIT;
	uint time_elapsed = 0;
	while (psm->AcquireFrame(true) >= PXC_STATUS_NO_ERROR && !esc_pressed)
	{
		outputData->Update();

		/* Landmark structs */
		Face::FaceData::LandmarksData* landmarkData;
		Face::FaceData::LandmarkPoint* landmarkPoints;
		pxcI32 numPoints;

		// iterate through faces
		pxcU16 numFaces = outputData->QueryNumberOfDetectedFaces();

		psm->AcquireFrame();
		PXCCapture::Sample *sample = psm->QuerySample();

		video_frame = PXCImage2CVMat(sample->color, PXCImage::PIXEL_FORMAT_RGB24);
		cv::Mat sidebar = cv::Mat::zeros(cv::Size(BAR_WIDTH, FRAME_HEIGHT), CV_8UC3);


		// get face data by index
		Face::FaceData::Face* trackedFace = outputData->QueryFaceByIndex(0);
		if (trackedFace != NULL)
		{
			landmarkData = trackedFace->QueryLandmarks();

			if (landmarkData != NULL)
			{
				// get num points
				numPoints = landmarkData->QueryNumPoints();
				// create array with number of points
				landmarkPoints = new Face::FaceData::LandmarkPoint[numPoints];
				// query points from landmark data and render

				if (landmarkData->QueryPoints(landmarkPoints))
				{
					cv::Scalar marker_color(255, 255, 255);

					if (checkConfidence(landmarkPoints, numPoints))
					{
						marker_color = cv::Scalar(255, 255, 255);
					}
					else
					{
						// print error text and color markers red
						cv::putText(sidebar, "! Confidence Not 100 !", cv::Point(TEXT_MARGIN, 500), CV_FONT_HERSHEY_PLAIN, 1, cv::Scalar(0, 0, 255), 2);
						marker_color = cv::Scalar(0, 0, 255);
					}
					double cur_avg;

					/* palpebral apeture */
					cur_avg = addData(measurements_total[0], time_elapsed, landmarkPoints[TOP_LEFT_EYELID].world, landmarkPoints[BOTTOM_LEFT_EYELID].world, Y_DIMENSION);		// L
					if (cur_avg > 0) { measurement_averages[0] = cur_avg; }

					cur_avg = addData(measurements_total[1], time_elapsed, landmarkPoints[TOP_RIGHT_EYELID].world, landmarkPoints[BOTTOM_RIGHT_EYELID].world, Y_DIMENSION);		// R
					if (cur_avg > 0) { measurement_averages[1] = cur_avg; }

					/* eye width */
					cur_avg = addData(measurements_total[2], time_elapsed, landmarkPoints[INNER_LEFT_EYE].world, landmarkPoints[OUTER_LEFT_EYE].world, X_DIMENSION);			// L
					if (cur_avg > 0) { measurement_averages[2] = cur_avg; }

					cur_avg = addData(measurements_total[3], time_elapsed, landmarkPoints[INNER_RIGHT_EYE].world, landmarkPoints[OUTER_RIGHT_EYE].world, X_DIMENSION);			// R
					if (cur_avg > 0) { measurement_averages[3] = cur_avg; }

					/* pupiliary dist to center */
					cur_avg = addData(measurements_total[4], time_elapsed, landmarkPoints[LEFT_IRIS].world, landmarkPoints[NASAL_BRIDGE].world, X_DIMENSION);
					if (cur_avg > 0) { measurement_averages[4] = cur_avg; }

					/* outer canthal distance */
					cur_avg = addData(measurements_total[5], time_elapsed, landmarkPoints[OUTER_LEFT_EYE].world, landmarkPoints[OUTER_RIGHT_EYE].world, X_DIMENSION);
					if (cur_avg > 0) { measurement_averages[5] = cur_avg; }

					/* inter pupil distance */
					cur_avg = addData(measurements_total[6], time_elapsed, landmarkPoints[LEFT_IRIS].world, landmarkPoints[RIGHT_IRIS].world, X_DIMENSION);
					if (cur_avg > 0) { measurement_averages[6] = cur_avg; }

					/* inter canthal distance */
					cur_avg = addData(measurements_total[7], time_elapsed, landmarkPoints[INNER_LEFT_EYE].world, landmarkPoints[INNER_RIGHT_EYE].world, X_DIMENSION);
					if (cur_avg > 0) { measurement_averages[7] = cur_avg; }

					/* nasal width */
					cur_avg = addData(measurements_total[8], time_elapsed, landmarkPoints[LEFT_NOSE].world, landmarkPoints[RIGHT_NOSE].world, X_DIMENSION);
					if (cur_avg > 0) { measurement_averages[8] = cur_avg; }

					/* Upper lip - lower third ratio */
					cur_avg = addData(measurements_total[9], time_elapsed, landmarkPoints[BOTTOM_CENTER_NOSE].world, landmarkPoints[STOMION_UPPER].world, Y_DIMENSION);		// subnasion to stoma
					if (cur_avg > 0) { measurement_averages[9] = cur_avg; }

					cur_avg = addData(measurements_total[10], time_elapsed, landmarkPoints[STOMION_LOWER].world, landmarkPoints[CHIN].world, Y_DIMENSION);					// menton to stoma
					if (cur_avg > 0) { measurement_averages[10] = cur_avg; }

					for (int k = 0; k < NUM_POINTS; k++)
					{
						int cur_point = measurement_locs[k];
						cv::circle(video_frame, cv::Point((int)landmarkPoints[cur_point].image.x, (int)landmarkPoints[cur_point].image.y), HALFSQUARE, marker_color, -1, 8, 0);
					}

				}
			}

		}
		// if it cant find a face
		else
		{
			cv::putText(sidebar, "! No Face Detected !", cv::Point(TEXT_MARGIN, 500), CV_FONT_HERSHEY_PLAIN, 1, cv::Scalar(0, 0, 255), 2);

			// reset displayed averages for swapping to a new face
			for (int k = 0; k < NUM_MEASUREMENTS; k++)
			{
				measurement_averages[k] = 0;
			}
		}


		// printing the label text for each measurement on sidebar
		cv::putText(sidebar, "Palpebral Apeture (L)", cv::Point(TEXT_MARGIN, 25), CV_FONT_HERSHEY_PLAIN, 1, cv::Scalar(255, 255, 255));
		cv::putText(sidebar, "Palpebral Apeture (R)", cv::Point(TEXT_MARGIN, 65), CV_FONT_HERSHEY_PLAIN, 1, cv::Scalar(255, 255, 255));

		cv::putText(sidebar, "Eye Width (L)", cv::Point(TEXT_MARGIN, 105), CV_FONT_HERSHEY_PLAIN, 1, cv::Scalar(255, 255, 255));
		cv::putText(sidebar, "Eye Width (R)", cv::Point(TEXT_MARGIN, 145), CV_FONT_HERSHEY_PLAIN, 1, cv::Scalar(255, 255, 255));

		cv::putText(sidebar, "Pupiliary Distance to ", cv::Point(TEXT_MARGIN, 185), CV_FONT_HERSHEY_PLAIN, 1, cv::Scalar(255, 255, 255));
		cv::putText(sidebar, "Center", cv::Point(TEXT_MARGIN, 197), CV_FONT_HERSHEY_PLAIN, 1, cv::Scalar(255, 255, 255));

		cv::putText(sidebar, "Outer Canthal Distance", cv::Point(TEXT_MARGIN, 235), CV_FONT_HERSHEY_PLAIN, 1, cv::Scalar(255, 255, 255));

		cv::putText(sidebar, "Inter Pupil Distance", cv::Point(TEXT_MARGIN, 275), CV_FONT_HERSHEY_PLAIN, 1, cv::Scalar(255, 255, 255));

		cv::putText(sidebar, "Inter Canthal Distance", cv::Point(TEXT_MARGIN, 315), CV_FONT_HERSHEY_PLAIN, 1, cv::Scalar(255, 255, 255));

		cv::putText(sidebar, "Nasal Width", cv::Point(TEXT_MARGIN, 355), CV_FONT_HERSHEY_PLAIN, 1, cv::Scalar(255, 255, 255));

		cv::putText(sidebar, "Subnasion-Stoma", cv::Point(TEXT_MARGIN, 395), CV_FONT_HERSHEY_PLAIN, 1, cv::Scalar(255, 255, 255));
		cv::putText(sidebar, "Distance", cv::Point(TEXT_MARGIN, 407), CV_FONT_HERSHEY_PLAIN, 1, cv::Scalar(255, 255, 255));
		cv::putText(sidebar, "Menton-Stoma Distance", cv::Point(TEXT_MARGIN, 445), CV_FONT_HERSHEY_PLAIN, 1, cv::Scalar(255, 255, 255));

		// printing the measurement averages
		Print sidebarText;

		// displaying all of the calculated distance averages
		sidebarText.displayDistance(sidebar, measurement_averages, 0, 1);
		sidebarText.displayDistance(sidebar, measurement_averages, 1, 1);
		sidebarText.displayDistance(sidebar, measurement_averages, 2, 1);
		sidebarText.displayDistance(sidebar, measurement_averages, 3, 1);
		sidebarText.displayDistance(sidebar, measurement_averages, 4, 2);
		sidebarText.displayDistance(sidebar, measurement_averages, 5, 1);
		sidebarText.displayDistance(sidebar, measurement_averages, 6, 1);
		sidebarText.displayDistance(sidebar, measurement_averages, 7, 1);
		sidebarText.displayDistance(sidebar, measurement_averages, 8, 1);
		sidebarText.displayDistance(sidebar, measurement_averages, 9, 2);
		sidebarText.displayDistance(sidebar, measurement_averages, 10, 1);

		// printing some information text
		cv::putText(sidebar, "(Esc) to quit the", cv::Point(TEXT_MARGIN, 650), CV_FONT_HERSHEY_PLAIN, 1, cv::Scalar(255, 0, 0));
		cv::putText(sidebar, "program", cv::Point(TEXT_MARGIN, 662), CV_FONT_HERSHEY_PLAIN, 1, cv::Scalar(255, 0, 0));

		cv::putText(sidebar, "(Spacebar) to pause", cv::Point(TEXT_MARGIN, 680), CV_FONT_HERSHEY_PLAIN, 1, cv::Scalar(255, 0, 0));
		cv::putText(sidebar, "and save data", cv::Point(TEXT_MARGIN, 692), CV_FONT_HERSHEY_PLAIN, 1, cv::Scalar(255, 0, 0));

		cv::putText(sidebar, "(R) to reset data", cv::Point(TEXT_MARGIN, 710), CV_FONT_HERSHEY_PLAIN, 1, cv::Scalar(255, 0, 0));


		// displaying the image in the correct window
		cv::Mat combined_frame = cv::Mat::zeros(cv::Size(FRAME_WIDTH + BAR_WIDTH, FRAME_HEIGHT), CV_8UC3);
		cv::Mat video(combined_frame, cv::Rect(0, 0, FRAME_WIDTH, FRAME_HEIGHT));
		video_frame.copyTo(video);
		cv::Mat bar(combined_frame, cv::Rect(FRAME_WIDTH, 0, BAR_WIDTH, FRAME_HEIGHT));
		sidebar.copyTo(bar);
		
		// rendering the window
		cv::imshow("Face Measuring Utility", combined_frame);

		int keypress = cv::waitKey(1);
		if (keypress == ESC_ASCII)
		{
			esc_pressed = true;
		}
		else if (keypress == SPACE_ASCII && time_since_keypress >= REPEAT_PRESS_WAIT)
		{
			outputFiles(combined_frame, measurement_averages);

			keypress = cv::waitKey(0);
			if (keypress == ESC_ASCII)
				esc_pressed = true;
			time_since_keypress = 0;
		}
		else if (keypress == R_ASCII && time_since_keypress >= REPEAT_PRESS_WAIT)
		{
			// reset averages
			for (int k = 0; k < NUM_MEASUREMENTS; k++)
			{
				measurement_averages[k] = 0;
			}

			time_since_keypress = 0;
		}

		// increment elapsed 'time'
		time_since_keypress++;
		time_elapsed++;

		psm->ReleaseFrame();
	}


	// close face config interface
	config->Release();

	// close streams and release any session and processing module instances
	psm->Release();

	return 0;
}

void outputFiles(cv::Mat image, double* averages)
{
	// initializing file and string streams
	std::ostringstream strs;
	std::ofstream output_file;

	// getting the current time
	struct tm timeinfo;
	time_t t = time(0);
	localtime_s(&timeinfo, &t);

	// saving the video frame
	strs << "./recordings/";
	strs << std::put_time(&timeinfo, "%Y-%m-%d-%H-%M-%S");
	strs << ".jpg";
	cv::imwrite(strs.str(), image);
	strs.str("");

	// saving the data
	strs << "./recordings/";
	strs << std::put_time(&timeinfo, "%Y-%m-%d-%H-%M-%S");
	strs << ".csv";
	output_file.open(strs.str());
	strs.clear();

	// formatting the csv properly
	output_file << "Measurement,Distance (mm)\n";
	output_file << "\nPalpebral Apeture (L)," << averages[0];
	output_file << "\nPalpebral Apeture (R)," << averages[1];
	output_file << "\nEye Width (L)," << averages[2];
	output_file << "\nEye Width (R)," << averages[3];
	output_file << "\nPupiliary Distance to Center," << averages[4];
	output_file << "\nOuter Canthal Distance," << averages[5];
	output_file << "\nInter Pupil Distance," << averages[6];
	output_file << "\nInter Canthal Distance," << averages[7];
	output_file << "\nNasal Width," << averages[8];
	output_file << "\nSubnasion-Stoma Distance," << averages[9];
	output_file << "\nMenton-Stoma Distance," << averages[10];
	output_file.close();
}

int checkConfidence(Face::FaceData::LandmarkPoint* landmarkPoints, pxcI32 numPoints)
{
	for (int k = 0; k < numPoints; k++)
	{
		if (landmarkPoints[k].confidenceWorld < CONFIDENCE_THRESHOLD)
			return 0;
	}
	return 1;
}

int Print::displayDistance(cv::Mat image, double* averages, uint index, int num_lines)
{
	std::ostringstream strs;
	current_y += (TEXT_HEIGHT + (10 * (num_lines - 1)));

	if (averages[index] < 0)
	{
		cv::putText(image, "Not Enough Data", cv::Point(TEXT_MARGIN, current_y), CV_FONT_HERSHEY_PLAIN, 1, cv::Scalar(255, 255, 255));
	}
	else
	{
		strs << std::fixed;
		strs << std::setprecision(2);
		strs << averages[index];
		cv::putText(image, strs.str(), cv::Point(TEXT_MARGIN, current_y), CV_FONT_HERSHEY_PLAIN, 1, cv::Scalar(255, 255, 255));
		strs.clear();
	}

	return 0;
}

int checkVariance(double* samples, int sample_number)
{
	for (int k = 0; k < NUM_SAMPLES; k++)
	{
		if (samples[sample_number] < samples[k] - (ACCEPTABLE_VARIANCE/2) )
			return FALSE;
		else if (samples[sample_number] > samples[k] + (ACCEPTABLE_VARIANCE/2) )
			return FALSE;
	}

	return TRUE;
}

double addData(double* samples, uint time, Intel::RealSense::Point3DF32 p1, Intel::RealSense::Point3DF32 p2, int dimension)
{
	int index = time % NUM_SAMPLES;
	samples[index] = calcDistance(p1, p2, dimension);

	if (checkVariance(samples, index))
	{
		double total = 0;
		for (int k = 0; k < NUM_SAMPLES; k++)
		{
			total += samples[k];
		}

		double average = total / NUM_SAMPLES;
		return average;
	}

	return -1;
}

double calcDistance(Intel::RealSense::Point3DF32 p1, Intel::RealSense::Point3DF32 p2, int dimension)
{
	double dist;

	switch (dimension)
	{
	case X_DIMENSION:
		dist = calcDistance_x(p1, p2);
		break;
	case Y_DIMENSION:
		dist = calcDistance_y(p1, p2);
		break;
	case XY_DIMENSION:
		dist = calcDistance_xy(p1, p2);
		break;
	}
	
	return dist;
}

// returns distance between two points in mm
double calcDistance_xy(Intel::RealSense::Point3DF32 p1, Intel::RealSense::Point3DF32 p2)
{
	float xd = p1.x - p2.x;
	float yd = p1.y - p2.y;
	return sqrt(xd*xd + yd*yd) * MILLIMETERS_IN_METER * MEASUREMENT_SCALE;
}

// returns distance between two points in mm
double calcDistance_x(Intel::RealSense::Point3DF32 p1, Intel::RealSense::Point3DF32 p2)
{
	float xd = p1.x - p2.x;
	return abs(xd) * MILLIMETERS_IN_METER * MEASUREMENT_SCALE;
}

// returns distance between two points in mm
double calcDistance_y(Intel::RealSense::Point3DF32 p1, Intel::RealSense::Point3DF32 p2)
{
	float yd = p1.y - p2.y;
	return abs(yd) * MILLIMETERS_IN_METER * MEASUREMENT_SCALE;
}

// from intel
cv::Mat PXCImage2CVMat(PXCImage *pxcImage, PXCImage::PixelFormat format)
{
	PXCImage::ImageData data;
	pxcImage->AcquireAccess(PXCImage::ACCESS_READ, format, &data);

	int width = pxcImage->QueryInfo().width;
	int height = pxcImage->QueryInfo().height;
	if (!format)
		format = pxcImage->QueryInfo().format;

	int type;
	if (format == PXCImage::PIXEL_FORMAT_Y8)
		type = CV_8UC1;
	else if (format == PXCImage::PIXEL_FORMAT_RGB24)
		type = CV_8UC3;
	else if (format == PXCImage::PIXEL_FORMAT_DEPTH_F32)
		type = CV_32FC1;
	else if (format == PXCImage::PIXEL_FORMAT_DEPTH)
		type = CV_16UC1;

	cv::Mat ocvImage = cv::Mat(cv::Size(width, height), type, data.planes[0]);


	pxcImage->ReleaseAccess(&data);
	return ocvImage;
}
