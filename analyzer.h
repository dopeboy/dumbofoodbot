#ifndef ANALYZER_H_INCLUDED
#define ANALYZER_H_INCLUDED

IplImage* convertRGBtoHSV(const IplImage *imageRGB);
void my_mouse_callback( int event, int x, int y, int flags, void* param );
unsigned int getMinFromArray(int arr[], int length);
unsigned int getMaxFromArray(int arr[], int length);

#endif
