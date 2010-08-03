#include <klee-opencv.h>
#include "cv.h"
#include <klee.h>
#include <assert.h>

int main(void) {
#ifndef __CONCRETE
	unsigned sse_count_v, sse_count_s;
#endif
	IplImage *img1 = cvCreateImage(cvSize(4, 4), IPL_DEPTH_32F, 1);
	IplImage *img2 = cvCreateImage(cvSize(4, 4), IPL_DEPTH_32F, 1);
	IplImage *img3 = cvCreateImage(cvSize(4, 4), IPL_DEPTH_32F, 1);
	void *data = malloc(img1->widthStep*img1->height);
	klee_make_symbolic(data, img1->widthStep*img1->height, "img1data");
	memcpy(img1->imageData, data, img1->widthStep*img1->height);
	cvUseOptimized(true);
#ifndef __CONCRETE
	klee_sse_count = 0;
#endif
	cvCornerMinEigenVal(img1, img2, 5);
#ifndef __CONCRETE
	sse_count_v = klee_sse_count;
#endif
	cvUseOptimized(false);
#ifndef __CONCRETE
        klee_sse_count = 0;
#endif
	cvCornerMinEigenVal(img1, img3, 5);
#ifndef __CONCRETE
	sse_count_s = klee_sse_count;
	printf("SSE COUNT: V=%d S=%d\n", sse_count_v, sse_count_s);
	assert(sse_count_v > sse_count_s);
#endif
	klee_print_expr("img2->imageData[0]", ((float *)img2->imageData)[0]);
	klee_print_expr("img3->imageData[0]", ((float *)img3->imageData)[0]);
	assert(bitwise_eq(((float *)img2->imageData)[0], ((float *)img3->imageData)[0]));
}

