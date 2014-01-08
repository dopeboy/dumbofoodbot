#include <stdio.h>
#include "cv.h"
#include <opencv2/highgui/highgui.hpp>

#define KORILLA 0
#define MAMU 1
#define MOOSHUGRILL 2
#define MEXICOBVLD 3
#define HIBACHIHEAVEN 4
#define YOUGOTSMOKED 5
#define SHORTYS 6

IplImage* convertRGBtoHSV(const IplImage *imageRGB);
IplImage* chop(CvPoint* point, unsigned int numPoints, const IplImage* capture);
signed int processRegion(const IplImage* region);
void analyzeRegions(signed int region0_status, signed int region1_status, signed int region2_status);

unsigned int debug = 0;

int main(int argc, char *argv[])
{
	if (argc != 3)
	{
	    printf("usage: ./findmesometrucks <image> <debug_mode>\n");
	    exit(0);
	}

	// debug mode?
	debug = atoi(argv[2]);

	// OK cool, we're in. Let's load the input image. 
 	IplImage* capture = cvLoadImage(argv[1],CV_LOAD_IMAGE_COLOR);

	// Let's crop the image to get just the meaningful part of it. These vertices & widths were found manually via GIMP.
	CvRect cr = {252,170,1222-252,620-170};
	cvSetImageROI(capture, cr);
	IplImage* cropped = cvCreateImage( cvSize(cr.width,cr.height), capture->depth, capture->nChannels );
	cvCopy( capture, cropped, 0 );

	//cropped = cvLoadImage("a.png",CV_LOAD_IMAGE_COLOR);
	if (debug == 1)
{
int p[3];
    p[0] = CV_IMWRITE_JPEG_QUALITY;
    p[1] = 100;
    p[2] = 0;
		cvSaveImage( "lot_capture_cropped.jpg", cropped, p );
}
	// Now chop it into three regions. Each region represents a parking spot. These vertices were also found manually via gimp.
	unsigned int numPoints = 0;

	// R0 (Southern most spot)
	numPoints = 4;
	CvPoint points_r1[numPoints];

	points_r1[0].x=25;
	points_r1[0].y=25;

	points_r1[1].x=235;
	points_r1[1].y=25;

	points_r1[2].x=235;
	points_r1[2].y=211;

	points_r1[3].x=9;
	points_r1[3].y=242;

	IplImage* region0 = chop(&points_r1[0], numPoints, cropped);

	// R1 (middle spot)
	numPoints = 5;
	CvPoint points_r2[numPoints];

	points_r2[0].x=519;
	points_r2[0].y=261;

	points_r2[1].x=235;
	points_r2[1].y=219;

	points_r2[2].x=238;
	points_r2[2].y=42;

	points_r2[3].x=756;
	points_r2[3].y=140;

	points_r2[4].x=541;
	points_r2[4].y=199;

	IplImage* region1 = chop(&points_r2[0], numPoints, cropped);

	// R2 (southernmost spot)
	numPoints = 6;
	CvPoint points_r3[numPoints];

	points_r3[0].x=505;
	points_r3[0].y=419;

	points_r3[1].x=891;
	points_r3[1].y=273;

	points_r3[2].x=903;
	points_r3[2].y=171;

	points_r3[3].x=745;
	points_r3[3].y=128;

	points_r3[4].x=545;
	points_r3[4].y=188;

	points_r3[5].x=444;
	points_r3[5].y=385;

	IplImage* region2 = chop(&points_r3[0], numPoints, cropped);

	// OK, we got all the cropped regions now. Now let's run each region through the color detection algorithm to find what truck is in each region
	signed int region0_status = processRegion(region0);
	printf("\n");
	signed int region1_status = processRegion(region1);
	printf("\n");
	signed int region2_status = processRegion(region2);
	printf("\n");
		
	analyzeRegions(region0_status, region1_status, region2_status);

	// Mother always said we should clean up after ourselves.
    cvReleaseImage(&capture);
    cvReleaseImage(&cropped);
    cvReleaseImage(&region0);
    cvReleaseImage(&region1);
    cvReleaseImage(&region2);

    return 0;
}

IplImage* chop(CvPoint* point, unsigned int numPoints, const IplImage* capture)
{
	
	IplImage* mask = cvCreateImage(cvSize(capture->width,capture->height), capture->depth, 1);
	cvFillPoly(mask, &point, &numPoints, 1, cvScalar(255,255,255,255), 8, 0);   

	IplImage* invmask = cvCreateImage(cvSize(capture->width,capture->height), capture->depth, 1);
	unsigned int step = invmask->widthStep;

	uchar *data1 = (uchar *)mask->imageData;;
	uchar *data2 = (uchar *)invmask->imageData;

	unsigned int i, j;

	for(i=0;i<capture->height;i++)
 		for(j=0;j<capture->width;j++)
		   data2[i*step+j]=255-data1[i*step+j];    //inverting the image
	
	IplImage* region_uncropped = cvCreateImage(cvSize(capture->width,capture->height), capture->depth, capture->nChannels);
	IplImage* purple_bg = cvCreateImage(cvSize(capture->width,capture->height), capture->depth, capture->nChannels);
	cvSet(purple_bg, cvScalar(255,0,255, 0), 0);

	cvCopy(capture, region_uncropped, mask);
	cvCopy(purple_bg, region_uncropped, invmask);

	// Note: something screwy goes on if we does this at the end of the function. Do it now.
	cvReleaseImage(&mask);
	cvReleaseImage(&invmask);

	// Smart crop the region
	unsigned int minx = UINT_MAX;
	unsigned int miny = UINT_MAX;
	unsigned int maxx = 0;
	unsigned int maxy = 0;

	unsigned int width, height = 0;

	for (i=0; i<numPoints; ++i)
	{
		if (point[i].x < minx)
			minx = point[i].x;

		if (point[i].y < miny)
			miny = point[i].y;

		if (point[i].x > maxx)
			maxx = point[i].x;

		if (point[i].y > maxy)
			maxy = point[i].y;		
	}

	CvRect cr = {minx,miny,maxx-minx,maxy-miny};
	cvSetImageROI(region_uncropped, cr);
	IplImage* region_cropped = cvCreateImage( cvSize(cr.width,cr.height), capture->depth, capture->nChannels );
	cvCopy( region_uncropped, region_cropped, 0 );

    cvReleaseImage(&region_uncropped);
    cvReleaseImage(&purple_bg);

	return region_cropped;
}
	
signed int processRegion(const IplImage* region)
{
	// Convert the image from BGR to HSV
	IplImage *imgHSV = convertRGBtoHSV(region);

	// Korilla
	IplImage* korilla1 = cvCreateImage(cvSize(region->width,region->height), IPL_DEPTH_8U, 1);
	unsigned int korilla_count = 0;

  	cvInRangeS(imgHSV,cvScalar(8,132,106,0),cvScalar(16,192,255,0),korilla1);
	korilla_count = cvCountNonZero(korilla1);
	printf("Korilla count: %d\n", korilla_count);

	// Mamu	
	IplImage* mamu1 = cvCreateImage(cvSize(region->width,region->height), IPL_DEPTH_8U, 1);
	IplImage* mamu2 = cvCreateImage(cvSize(region->width,region->height), IPL_DEPTH_8U, 1);
	unsigned int mamu_count_blue = 0;
	unsigned int mamu_count_yellow = 0;
	
  	cvInRangeS(imgHSV,cvScalar(153,30,110,0),cvScalar(192,77,234,0),mamu1);
	cvInRangeS(imgHSV,cvScalar(18,16,136,0),cvScalar(38,52,154,0),mamu2);

	mamu_count_blue = cvCountNonZero(mamu1);
	mamu_count_yellow = cvCountNonZero(mamu2);

	printf("Mamu count (Blue, Yellow): %d, %d\n", mamu_count_blue, mamu_count_yellow);

	// Moo shu Grill	
	IplImage* msg = cvCreateImage(cvSize(region->width,region->height), IPL_DEPTH_8U, 1);

	unsigned int msg_count = 0;

  	cvInRangeS(imgHSV,cvScalar(27,56,105,0),cvScalar(35,97,237,0),msg);
	msg_count = cvCountNonZero(msg);
	
	printf("Moo Shuu Grill: %d\n", msg_count);

	// Mexico BVLD
	IplImage* mexico1 = cvCreateImage(cvSize(region->width,region->height), IPL_DEPTH_8U, 1);
	IplImage* mexico2 = cvCreateImage(cvSize(region->width,region->height), IPL_DEPTH_8U, 1);

	unsigned int mexico_yellow_count = 0;
	unsigned int mexico_black_count = 0;

  	cvInRangeS(imgHSV,cvScalar(19,45,170,0),cvScalar(27,103,233,0),mexico1);
	mexico_yellow_count = cvCountNonZero(mexico1);

  	cvInRangeS(imgHSV,cvScalar(0,0,51,0),cvScalar(234,30,127,0),mexico2);
	mexico_black_count = cvCountNonZero(mexico2);
	
	printf("Mexico Bvld Count (black, yellow): %d %d\n", mexico_black_count, mexico_yellow_count);

	// Hibachi Heaven
	IplImage* hibachi1 = cvCreateImage(cvSize(region->width,region->height), IPL_DEPTH_8U, 1);
	IplImage* hibachi2 = cvCreateImage(cvSize(region->width,region->height), IPL_DEPTH_8U, 1);
	IplImage* hibachi3 = cvCreateImage(cvSize(region->width,region->height), IPL_DEPTH_8U, 1);

	unsigned int hibachi_red_count = 0;
	unsigned int hibachi_gray_count = 0;
	unsigned int hibachi_yellow_count = 0;

  	cvInRangeS(imgHSV,cvScalar(1,81,166,0),cvScalar(253,167,213,0),hibachi1);
	hibachi_red_count = cvCountNonZero(hibachi1);

  	cvInRangeS(imgHSV,cvScalar(177,46,40,0),cvScalar(224,79,154,0),hibachi2);
	hibachi_gray_count = cvCountNonZero(hibachi2);

  	cvInRangeS(imgHSV,cvScalar(12,85,140,0),cvScalar(24,132,200,0),hibachi3);
	hibachi_yellow_count = cvCountNonZero(hibachi3);

	printf("Hibachi Count (red, gray, yellow): %d %d %d\n", hibachi_red_count, hibachi_gray_count, hibachi_yellow_count);

	// You got smoked
	IplImage* ygs = cvCreateImage(cvSize(region->width,region->height), IPL_DEPTH_8U, 1);
	unsigned int ygs_count = 0;

  	cvInRangeS(imgHSV,cvScalar(26,56,50,0),cvScalar(57,140,200,0),ygs);
	ygs_count = cvCountNonZero(ygs);
	printf("You got smoked count: %d\n", ygs_count);

	// Shorty's
	IplImage* shty1 = cvCreateImage(cvSize(region->width,region->height), IPL_DEPTH_8U, 1);
	IplImage* shty2 = cvCreateImage(cvSize(region->width,region->height), IPL_DEPTH_8U, 1);
	
	unsigned int shty_red_count = 0;
	unsigned int shty_white_count = 0;

  	cvInRangeS(imgHSV,cvScalar(4,7,80,0),cvScalar(234,40,95,0),shty1);
	shty_red_count = cvCountNonZero(shty1);

  	cvInRangeS(imgHSV,cvScalar(38,20,126,0),cvScalar(161,91,152,0),shty2);
	shty_white_count = cvCountNonZero(shty2);

	printf("Shorty's count (red, white): %d %d\n", shty_red_count, shty_white_count);

	signed int match = -1;

	// Now make determinations on whether to match or not

	// Korilla
	if (korilla_count > 5000)
	{
		printf("Korilla matched.\n");
		match = KORILLA;
	}

	// Mamu	
	else if (mamu_count_blue > 3500 && mamu_count_yellow > 2000)
	{
		printf("Mamu matched.\n");
		match = MAMU;
	}

	// Mooshu Grill	
	else if (msg_count > 7000)
	{
		printf("Moo Shuu matched.\n");
		match = MOOSHUGRILL;
	}

	// Mexico	
	else if (mexico_black_count > 20000 && mexico_yellow_count > 1000)
	{
		printf("Mexico Bvld matched.\n");
		match = MEXICOBVLD;
	}

	// Hibachi Heaven
	else if (hibachi_red_count > 1500 && hibachi_gray_count > 6000 && hibachi_yellow_count > 300)
	{
		printf("Hibachi Heaven matched.\n");
		match = HIBACHIHEAVEN;
	}

	// You got smoked
	else if (ygs_count > 12000)
	{
		printf("You got smoked matched.\n");
		match = YOUGOTSMOKED;
	}

	// Shorty's
	else if (shty_red_count > 6000 && shty_white_count > 9000)
	{
		printf("Shorty's matched.\n");
		match = SHORTYS;
	}

    cvReleaseImage(&imgHSV);
    cvReleaseImage(&korilla1);
	cvReleaseImage(&mamu1);
	cvReleaseImage(&mamu2);
	cvReleaseImage(&msg);
	cvReleaseImage(&mexico1);
	cvReleaseImage(&mexico2);
	cvReleaseImage(&hibachi1);
	cvReleaseImage(&hibachi2);
	cvReleaseImage(&hibachi3);
	cvReleaseImage(&ygs);
	cvReleaseImage(&shty1);
	cvReleaseImage(&shty2);

	return match;
}

void analyzeRegions(signed int region0_status, signed int region1_status, signed int region2_status)
{
	char outputString[140]; 
	char* end = " in the @dumbolot.";
	strcpy(outputString,"I spy with my little eye a");

	char tterCommand[300];
	char* tterEnd = "\"";
	strcpy(tterCommand,"ttytter -status=\"");

	// If all the regions were empty, say so and bail
	if (region0_status == -1 && region1_status == -1 && region2_status == -1)
		strcpy(outputString,"No food trucks today");

	else
	{
		int status[3] = {region0_status, region1_status, region2_status};

		unsigned int i = 0;
		for (i=0; i<3; ++i)
		{
			if (status[i] == KORILLA)
				strcat(outputString," @KorillaBBQ");
			else if (status[i] == MAMU)
				strcat(outputString," @mamuthainoodle");
			else if (status[i] == MOOSHUGRILL)
				strcat(outputString," @mooshugrill");
			else if (status[i] == MEXICOBVLD)
				strcat(outputString," @MexicoBlvd");
			else if (status[i] == HIBACHIHEAVEN)
				strcat(outputString," @hibachiheaven");
			else if (status[i] == YOUGOTSMOKED)
				strcat(outputString," @YouGotSmoked");
			else if (status[i] == SHORTYS)
				strcat(outputString," @shortysnyc");
		}
	}

	strcat(outputString," in the @dumbolot.");

	printf("output: %s\n",outputString);

	if (debug == 0)
	{
		strcat(tterCommand,outputString);
		strcat(tterCommand,tterEnd);
		system(tterCommand);
	}
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


