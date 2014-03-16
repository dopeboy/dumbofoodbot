#include <stdio.h>
#include "cv.h"
#include "analyzer.h"
#include <time.h>
#include <opencv2/highgui/highgui.hpp>
#include <sys/types.h>
#include <sys/stat.h>
#include <limits.h>

uchar *hsv_data = NULL;
IplImage *img_hsv = NULL;
int height,width,step,channels, i, j, k;
char key = 0;
int hues[100] = {};
int sats[100] = {};
int vals[100] = {};

unsigned int h_min, s_min, v_min, h_max, s_max, v_max=0;
unsigned int recorded_count = 0;

int main(int argc, char *argv[])
{
	if (argc != 2)
	{
	    printf("usage: ./analyzer <cropped_image>\n");
	    exit(0);
	}

	IplImage* region = cvLoadImage(argv[1],CV_LOAD_IMAGE_COLOR);
	height    = region->height;
	width     = region->width;
	step      = region->widthStep;
	channels  = region->nChannels;

	img_hsv = convertRGBtoHSV(region);
	hsv_data = (uchar *)img_hsv->imageData;

	cvNamedWindow("window",CV_GUI_EXPANDED);
	cvSetMouseCallback("window", my_mouse_callback, region);
    cvShowImage("window",region);

	while (key != 'q')
	{
		key = cvWaitKey(0);
	}

printf("%d %d %d | %d %d %d\n", h_min,s_min,v_min, h_max, s_max, v_max);

	cvDestroyAllWindows();
	cvReleaseImage(&region);
	cvReleaseImage(&img_hsv);

    return 0;
}

void my_mouse_callback( int event, int x, int y, int flags, void* param )
{
	unsigned int hsv[3] = {};

	for(k=0;k<channels;k++)
	{
		char c = 0;

		if (k==0)
			c = 'H';

		else if (k==1)
			c='S';

		else
			c='V';

		hsv[k] = hsv_data[y*step+x*channels+k];
		//printf("%c: %d\t", c, hsv[k]);
	}

	// Save <H,S,V>
	if (event == CV_EVENT_LBUTTONDOWN)
	{
		hues[recorded_count] = hsv[0];
		sats[recorded_count] = hsv[1];
		vals[recorded_count] = hsv[2];
		++recorded_count;
	}

	// Draw new image with whites in places that match
	else if (event == CV_EVENT_RBUTTONDOWN)
	{
		h_min = getMinFromArray(hues,recorded_count);
		s_min = getMinFromArray(sats,recorded_count);
		v_min = getMinFromArray(vals,recorded_count);

		h_max = getMaxFromArray(hues,recorded_count);
		s_max = getMaxFromArray(sats,recorded_count);
		v_max = getMaxFromArray(vals,recorded_count);
		
		IplImage* img = cvCreateImage(cvSize(width,height), IPL_DEPTH_8U, 1);
		cvInRangeS(img_hsv,cvScalar(h_min,s_min,v_min,0),cvScalar(h_max,s_max,v_max,0),img);

		int count = cvCountNonZero(img);
		printf("count: %d\n", count);

		cvNamedWindow("secondwindow",CV_GUI_EXPANDED);
		cvShowImage("secondwindow",img);
		key = 0;

		while (key != 's')
		{
			key = cvWaitKey(0);			
		}

		memset(hues,0,recorded_count);
		memset(sats,0,recorded_count);
		memset(vals,0,recorded_count);
		recorded_count=0;

		cvReleaseImage(&img);
		cvDestroyWindow("secondwindow");
	}
	
	//printf("\n");	
}

unsigned int getMinFromArray(int arr[], int length)
{
	int i=0;
	unsigned int min=UINT_MAX;

	for (i=0; i<length; ++i)
	{
		if (arr[i] < min)
			min = arr[i];	
	}

	return min;
}

unsigned int getMaxFromArray(int arr[], int length)
{
	int i=0;
	unsigned int max=0;

	for (i=0; i<length; ++i)
	{
		if (arr[i] > max)
			max = arr[i];	
	}

	return max;
}

// Create a HSV image from the RGB image using the full 8-bits, since OpenCV only allows Hues up to 180 instead of 255.
// ref: "http://cs.haifa.ac.il/hagit/courses/ist/Lectures/Demos/ColorApplet2/t_convert.html"
// Remember to free the generated HSV image.
IplImage* convertRGBtoHSV(const IplImage *imageRGB)
{
	float fR, fG, fB;
	float fH, fS, fV;
	const float FLOAT_TO_BYTE = 255.0f;
	const float BYTE_TO_FLOAT = 1.0f / FLOAT_TO_BYTE;

	// Create a blank HSV image
	IplImage *imageHSV = cvCreateImage(cvGetSize(imageRGB), 8, 3);
	if (!imageHSV || imageRGB->depth != 8 || imageRGB->nChannels != 3) {
		printf("ERROR in convertImageRGBtoHSV()! Bad input image.\n");
		exit(1);
	}

	int h = imageRGB->height;		// Pixel height.
	int w = imageRGB->width;		// Pixel width.
	int rowSizeRGB = imageRGB->widthStep;	// Size of row in bytes, including extra padding.
	char *imRGB = imageRGB->imageData;	// Pointer to the start of the image pixels.
	int rowSizeHSV = imageHSV->widthStep;	// Size of row in bytes, including extra padding.
	char *imHSV = imageHSV->imageData;	// Pointer to the start of the image pixels.
	int y, x = 0;

	for (y=0; y<h; y++) {
		for (x=0; x<w; x++) {
			// Get the RGB pixel components. NOTE that OpenCV stores RGB pixels in B,G,R order.
			uchar *pRGB = (uchar*)(imRGB + y*rowSizeRGB + x*3);
			int bB = *(uchar*)(pRGB+0);	// Blue component
			int bG = *(uchar*)(pRGB+1);	// Green component
			int bR = *(uchar*)(pRGB+2);	// Red component

			// Convert from 8-bit integers to floats.
			fR = bR * BYTE_TO_FLOAT;
			fG = bG * BYTE_TO_FLOAT;
			fB = bB * BYTE_TO_FLOAT;

			// Convert from RGB to HSV, using float ranges 0.0 to 1.0.
			float fDelta;
			float fMin, fMax;
			int iMax;
			// Get the min and max, but use integer comparisons for slight speedup.
			if (bB < bG) {
				if (bB < bR) {
					fMin = fB;
					if (bR > bG) {
						iMax = bR;
						fMax = fR;
					}
					else {
						iMax = bG;
						fMax = fG;
					}
				}
				else {
					fMin = fR;
					fMax = fG;
					iMax = bG;
				}
			}
			else {
				if (bG < bR) {
					fMin = fG;
					if (bB > bR) {
						fMax = fB;
						iMax = bB;
					}
					else {
						fMax = fR;
						iMax = bR;
					}
				}
				else {
					fMin = fR;
					fMax = fB;
					iMax = bB;
				}
			}
			fDelta = fMax - fMin;
			fV = fMax;				// Value (Brightness).
			if (iMax != 0) {			// Make sure it's not pure black.
				fS = fDelta / fMax;		// Saturation.
				float ANGLE_TO_UNIT = 1.0f / (6.0f * fDelta);	// Make the Hues between 0.0 to 1.0 instead of 6.0
				if (iMax == bR) {		// between yellow and magenta.
					fH = (fG - fB) * ANGLE_TO_UNIT;
				}
				else if (iMax == bG) {		// between cyan and yellow.
					fH = (2.0f/6.0f) + ( fB - fR ) * ANGLE_TO_UNIT;
				}
				else {				// between magenta and cyan.
					fH = (4.0f/6.0f) + ( fR - fG ) * ANGLE_TO_UNIT;
				}
				// Wrap outlier Hues around the circle.
				if (fH < 0.0f)
					fH += 1.0f;
				if (fH >= 1.0f)
					fH -= 1.0f;
			}
			else {
				// color is pure Black.
				fS = 0;
				fH = 0;	// undefined hue
			}

			// Convert from floats to 8-bit integers.
			int bH = (int)(0.5f + fH * 255.0f);
			int bS = (int)(0.5f + fS * 255.0f);
			int bV = (int)(0.5f + fV * 255.0f);

			// Clip the values to make sure it fits within the 8bits.
			if (bH > 255)
				bH = 255;
			if (bH < 0)
				bH = 0;
			if (bS > 255)
				bS = 255;
			if (bS < 0)
				bS = 0;
			if (bV > 255)
				bV = 255;
			if (bV < 0)
				bV = 0;

			// Set the HSV pixel components.
			uchar *pHSV = (uchar*)(imHSV + y*rowSizeHSV + x*3);
			*(pHSV+0) = bH;		// H component
			*(pHSV+1) = bS;		// S component
			*(pHSV+2) = bV;		// V component
		}
	}
	return imageHSV;
}


