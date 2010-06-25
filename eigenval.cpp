#include "cv.h"
#include <klee.h>
#include <assert.h>

int main(void) {
	IplImage *img1 = cvCreateImage(cvSize(4, 4), IPL_DEPTH_32F, 1);
	IplImage *img2 = cvCreateImage(cvSize(4, 4), IPL_DEPTH_32F, 1);
	IplImage *img3 = cvCreateImage(cvSize(4, 4), IPL_DEPTH_32F, 1);
	void *data = malloc(img1->widthStep*img1->height);
	klee_make_symbolic(data, img1->widthStep*img1->height, "img1data");
	memcpy(img1->imageData, data, img1->widthStep*img1->height);
	cvCornerMinEigenVal(img1, img2, 5);
	cvUseOptimized(false);
	cvCornerMinEigenVal(img1, img3, 5);
	klee_print_expr("img2->imageData[0]", ((float *)img2->imageData)[0]);
	klee_print_expr("img3->imageData[0]", ((float *)img3->imageData)[0]);
	assert(((float *)img2->imageData)[0] == ((float *)img3->imageData)[0]);
}

