#include "cv.h"
#ifndef __CONCRETE
#include <klee.h>
#endif
#include <assert.h>
#include <stdlib.h>

#define N 16

#include "get_seed.cpp"

int main(int argc, char** argv) {
	unsigned char mat1data[N];
	int mat2data[N];
	CvMat mat1, mat2;
	CvMat *mat3v = cvCreateMat(4, 4, CV_8UC1);
	CvMat *mat3s = cvCreateMat(4, 4, CV_8UC1);

#ifdef __CONCRETE
	int seed = get_seed(argc, argv);
	srandom(seed);
	for (int i = 0; i < N; i++) {
	  mat1data[i] = random();
	  mat2data[i] = random();
	}
#else
	klee_make_symbolic(mat1data, sizeof(mat1data), "mat1data");
	klee_make_symbolic(mat2data, sizeof(mat2data), "mat2data");
#endif

	mat1 = cvMat(4, 4, CV_8UC1, mat1data);
	mat2 = cvMat(4, 4, CV_32SC1, mat2data);

	cvUseOptimized(true);
	cvFilter2D(&mat1, mat3v, &mat2, cvPoint(-1, -1));
	cvUseOptimized(false);
	cvFilter2D(&mat1, mat3s, &mat2, cvPoint(-1, -1));

#ifdef __CONCRETE
	for (int i = 0; i < N; i++) {
	  printf("Element %d: %d vs %d", i, 
		 mat3s->data.ptr[i], 
		 mat3v->data.ptr[i]);
	  if (mat3s->data.ptr[i] !=  mat3v->data.ptr[i])
	    printf(" NO!!!");
	  printf("\n");
	}
#else
	klee_dump_constraints();
	for (int i = 0; i < N; i++) {
		char buf[256];
		sprintf(buf, "mat3s->data.ptr[%d]", i);
		klee_print_expr(buf, mat3s->data.ptr[i]);
		sprintf(buf, "mat3v->data.ptr[%d]", i);
		klee_print_expr(buf, mat3v->data.ptr[i]);
		assert(mat3s->data.ptr[0] == mat3v->data.ptr[0]);
	}
#endif	
}

