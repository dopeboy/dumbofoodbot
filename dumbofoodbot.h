#ifndef DUMBOFOODBOT_H_INCLUDED
#define DUMBOFOODBOT_H_INCLUDED

IplImage* convertRGBtoHSV(const IplImage *imageRGB);
IplImage* chop(CvPoint* point, unsigned int numPoints, const IplImage* capture);
signed int processRegion(const IplImage* region);
void analyzeRegions(signed int region0_status, signed int region1_status, signed int region2_status);

#endif
