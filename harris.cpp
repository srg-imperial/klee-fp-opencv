#include "cv.h"
#ifndef __CONCRETE
#include <klee.h>
#endif
#include <assert.h>
#include <stdio.h>

#include "get_seed.cpp"

int main(int argc, char** argv) {
#ifndef __CONCRETE
	unsigned sse_count_v, sse_count_s;
#endif
	IplImage *img1 = cvCreateImage(cvSize(4, 4), IPL_DEPTH_32F, 1);
	IplImage *img2 = cvCreateImage(cvSize(4, 4), IPL_DEPTH_32F, 1);
	IplImage *img3 = cvCreateImage(cvSize(4, 4), IPL_DEPTH_32F, 1);

	float *data = (float*) malloc(img1->widthStep*img1->height);

#ifdef __CONCRETE
	int seed = get_seed(argc, argv);
	srand48(seed);
	for (int i=0; i < 16; i++)
	  data[i] = drand48();
#else
	klee_make_symbolic(data, img1->widthStep*img1->height, "img1data");
#endif
	
	memcpy(img1->imageData, data, img1->widthStep*img1->height);
	cvUseOptimized(true);
#ifndef __CONCRETE
	klee_sse_count = 0;
#endif
	cvCornerHarris(img1, img2, 5);
#ifndef __CONCRETE
	sse_count_v = klee_sse_count;
#endif
	cvUseOptimized(false);
#ifndef __CONCRETE
        klee_sse_count = 0;
#endif
	cvCornerHarris(img1, img3, 5);
#ifndef __CONCRETE
	sse_count_s = klee_sse_count;
	printf("SSE COUNT: V=%d S=%d\n", sse_count_v, sse_count_s);
	assert(sse_count_v > sse_count_s);
#endif
	
#ifdef __CONCRETE
	int diffs = 0;
	for (int i=0; i< 16; i++) {
	  printf("%.20f vs. %.20f", (((float*)img2->imageData))[i], 
		 (((float*)img3->imageData))[i]);
	  if ((((float*)img2->imageData))[i] == (((float*)img3->imageData))[i])
	    printf("\n");
	  else {
	    printf(" ...NO\n");
	    diffs++;
	  }
	}
	printf("--\n");
	if (diffs)
	  printf("%d mismatches FOUND!\n", diffs);
	else printf("No mismatches found.\n");
#else
	klee_print_expr("img2->imageData[0]", ((float *)img2->imageData)[0]);
	klee_print_expr("img3->imageData[0]", ((float *)img3->imageData)[0]);
	assert(((float *)img2->imageData)[0] == ((float *)img3->imageData)[0]);
#endif
}

