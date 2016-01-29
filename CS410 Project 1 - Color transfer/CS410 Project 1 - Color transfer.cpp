/*
 * Max Litvinenko
 * CS410 - Winter 2016
 * Project 1
 * Credits: 
 *		http://www.pyimagesearch.com/2014/06/30/super-fast-color-transfer-images/, 
 *		http://www.cs.tau.ac.il/~turkel/imagepapers/ColorTransfer.pdf
 */

#include "stdafx.h"
#include <opencv2/opencv.hpp>
#include <opencv\highgui.h>
#include <iostream>
#include <string>
using namespace cv;
using namespace std;

// Path to an images that needs to apply the colors  AND where to get color space from
string targetImageName = "data/1a.jpg";		string sourceImageName = "data/1b.jpg";
//string targetImageName = "data/2a.jpg";		string sourceImageName = "data/2b.jpg";
//string targetImageName = "data/3a.jpg";			string sourceImageName = "data/3b.jpg";

// Working color space. NOTE there are BGR and RGBs
int colorSpace = COLOR_BGR2Lab;
//int colorSpace = COLOR_RGB2Lab;
//int colorSpace = COLOR_RGB2XYZ;
//int colorSpace = COLOR_BGR2XYZ;
//int colorSpace = COLOR_BGR2HSV;
//int colorSpace = COLOR_BGR2Luv;

void displayWindow(char * windowName, Mat image);
int getColorSpace();
int getColorSpaceRevert();
Mat getImage(string imageName);
Mat transformColor(Mat source, Mat target);

int main(int argc, char** argv) 
{
	Mat targetImage, sourceImage, outputImage;

	// Just in case if cmd is used, providing ability to change images 
	// and color spaces (NOTE need to supply the correct INTEGER)
	if (argc > 1)
	{
		if (argv[1] != NULL)
			targetImageName = argv[1];
		if (argv[2] != NULL)
			sourceImageName = argv[2];
		if (argv[3] != NULL)
			colorSpace = (int)argv[3];
	}

	try
	{
		// Reading images
		targetImage = getImage(targetImageName);
		sourceImage = getImage(sourceImageName);

		// Transfering color space from one image to another
		outputImage = transformColor(sourceImage, targetImage);
	}
	catch (exception& e)
	{
		cout << e.what() << std::endl;
		return -1;
	}

	// Displaying Windows with names and images
	displayWindow("Target", targetImage);
	displayWindow("Source", sourceImage);
	displayWindow("Result", outputImage);

	// Wait for a keystroke in the window to prefent automatic closure
	waitKey(0);
	
	return 0;
}

/*
 * Displays a window with given name and image
 */
void displayWindow(char * windowName, Mat image)
{
	namedWindow(windowName, WINDOW_AUTOSIZE);	// Create a window for display
	imshow(windowName, image);					// Show our image inside window
}

/*
 * Tries to read an image, if can't throws an exception; otherwise
 * returns a an image (in Mat type)
 */
Mat getImage(string imageName)
{
	Mat image = imread(imageName);
	if (image.empty())							// Check for invalid input
	{
		string message = "Could not open or find the image '" + imageName + "'";
		throw new exception(message.c_str());
	}

	return image;
}

/*
 * returns current working color space
 */
int getColorSpace()
{
	return colorSpace;
}

/*
 * Checks current colors space and 
 * returns reversed one
 */
int getColorSpaceRevert()
{
	int result;
	switch (colorSpace)
	{
		case COLOR_BGR2Lab:
			result = COLOR_Lab2BGR;
			break;
		case COLOR_RGB2Lab:
			result = COLOR_Lab2RGB;
			break;
		case COLOR_RGB2XYZ:
			result = COLOR_XYZ2RGB;
			break;
		case COLOR_BGR2XYZ:
			result = COLOR_XYZ2BGR;
			break;
		case COLOR_BGR2HSV:
			result = COLOR_HSV2BGR;
			break;
		case COLOR_BGR2Luv:
			result = COLOR_Luv2BGR;
			break;
		dafault:
			string message = "Unknown color space '" + to_string(colorSpace) + "'";
			throw new exception(message.c_str());
	}
	return result;
}

/*
 * Transfers colors of one image to another and 
 * returns a new image
 */
Mat transformColor(Mat source, Mat target)
{
	// Temporarty variables to perform mathematical operations
	Mat outputImage, tmpSourceLab, tmpTargetLab;
	Mat sourceChanels[3], targetChanels[3], outputChanels[3];
	Scalar sourceMeanL, sourceMeanA, sourceMeanB, sourceStdL, sourceStdA, sourceStdB;
	Scalar targetMeanL, targetMeanA, targetMeanB, targetStdL, targetStdA, targetStdB;
	
	// Converting an image from one color space to another.
	// NOTE: we do not change the original souce and target images,
	//			so no need to convert them back at the end...
	cvtColor(source, tmpSourceLab, getColorSpace());
	cvtColor(target, tmpTargetLab, getColorSpace());
	
	split(tmpSourceLab, sourceChanels); // Spliting lab SOURCE image into chanels 
	split(tmpTargetLab, targetChanels); // Spliting lab TARGET image into chanels
	split(tmpTargetLab, outputChanels); // Spliting lab TARGET image into chanels again for output results

	meanStdDev(sourceChanels[0], sourceMeanL, sourceStdL);	// L for SOURCE
	meanStdDev(sourceChanels[1], sourceMeanA, sourceStdA);	// A for SOURCE
	meanStdDev(sourceChanels[2], sourceMeanB, sourceStdB);	// B for SOURCE
	
	meanStdDev(targetChanels[0], targetMeanL, targetStdL);	// L for TARGET
	meanStdDev(targetChanels[1], targetMeanA, targetStdA);	// A for TARGET
	meanStdDev(targetChanels[2], targetMeanB, targetStdB);	// B for TARGET
	
	// START: Mathematical computations
	subtract(outputChanels[0], targetMeanL, outputChanels[0]);
	subtract(outputChanels[1], targetMeanA, outputChanels[1]);
	subtract(outputChanels[2], targetMeanB, outputChanels[2]);
	
	Scalar tmpL, tmpA, tmpB; // temp vars for division only
	divide(targetStdL, sourceStdL, tmpL);
	divide(targetStdA, sourceStdA, tmpA);
	divide(targetStdB, sourceStdB, tmpB);

	/*/
	multiply(outputChanels[0], tmpL, outputChanels[0]);
	multiply(outputChanels[1], tmpA, outputChanels[1]);
	multiply(outputChanels[2], tmpB, outputChanels[2]);
	//*/

	add(outputChanels[0], sourceMeanL, outputChanels[0]);
	add(outputChanels[1], sourceMeanA, outputChanels[1]);
	add(outputChanels[2], sourceMeanB, outputChanels[2]);
	// END: Mathematical computations

	/*
	// Need to make sure that our values are in range of RGB values
	inRange(outputChanels[0], Scalar(0), Scalar(255), outputChanels[0]);
	inRange(outputChanels[1], Scalar(0), Scalar(255), outputChanels[1]);
	inRange(outputChanels[2], Scalar(0), Scalar(255), outputChanels[2]);
	//*/

	// Merging created color space into an image
	merge(outputChanels, 3, outputImage);
	
	// transfering back to needed color space
	cvtColor(outputImage, outputImage, getColorSpaceRevert());

	return outputImage;
}