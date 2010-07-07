#include "cv.h"
#ifndef __CONCRETE
#include <klee.h>
#endif
#include <assert.h>
#include <stdio.h>

int main(void) {
	IplImage *img1 = cvCreateImage(cvSize(4, 4), IPL_DEPTH_32F, 1);
	IplImage *img2 = cvCreateImage(cvSize(4, 4), IPL_DEPTH_32F, 1);
	IplImage *img3 = cvCreateImage(cvSize(4, 4), IPL_DEPTH_32F, 1);

	float *data = (float*) malloc(img1->widthStep*img1->height);

#ifdef __CONCRETE
	srand48(123);
	for (int i=0; i < 16; i++)
	  data[i] = drand48();
#else
	klee_make_symbolic(data, img1->widthStep*img1->height, "img1data");
#endif
	
	memcpy(img1->imageData, data, img1->widthStep*img1->height);
	cvCornerHarris(img1, img2, 5);
	cvUseOptimized(false);
	cvCornerHarris(img1, img3, 5);
	
#ifdef __CONCRETE
	for (int i=0; i< 16; i++) {
	  printf("%.20f vs. %.20f", (((float*)img2->imageData))[i], 
		 (((float*)img3->imageData))[i]);
	  if ((((float*)img2->imageData))[i] == (((float*)img3->imageData))[i])
	    printf("\n");
	  else printf(" ...NO\n");
	}
#else
	klee_print_expr("img2->imageData[0]", ((float *)img2->imageData)[0]);
	klee_print_expr("img3->imageData[0]", ((float *)img3->imageData)[0]);
	assert(((float *)img2->imageData)[0] == ((float *)img3->imageData)[0]);
#endif
}

