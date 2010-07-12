#include "cv.h"

#ifndef __CONCRETE
#include <klee.h>
#endif
#include <assert.h>

#define N1 4


int main(void) {
#ifndef __CONCRETE
	unsigned sse_count_v, sse_count_s;
#endif
	unsigned short mat1data[N1*N1*3];
	float mat2data[12];
	CvMat mat1, mat2;
	CvMat *mat3v = cvCreateMat(N1, N1, CV_16UC3);
	CvMat *mat3s = cvCreateMat(N1, N1, CV_16UC3);

#ifdef __CONCRETE
	srandom(127);
	srand48(127);
	for (int i=0; i < N1*N1*3; i++)
	  mat1data[i] = random();

	for (int i=0; i < 12; i++)
	  mat2data[i] = drand48();
#else
	klee_make_symbolic(mat1data, sizeof(mat1data), "mat1data");
	klee_make_symbolic(mat2data, sizeof(mat2data), "mat2data");
#endif

	mat1 = cvMat(N1, N1, CV_16UC3, mat1data);
	mat2 = cvMat(3, N1, CV_32FC1, mat2data);

	cvUseOptimized(true);
#ifndef __CONCRETE
	klee_sse_count = 0;
#endif
	cvTransform(&mat1, mat3v, &mat2, NULL);
#ifndef __CONCRETE
	sse_count_v = klee_sse_count;
#endif
	cvUseOptimized(false);
#ifndef __CONCRETE
        klee_sse_count = 0;
#endif
	cvTransform(&mat1, mat3s, &mat2, NULL);
#ifndef __CONCRETE
	sse_count_s = klee_sse_count;
	printf("SSE COUNT: V=%d S=%d\n", sse_count_v, sse_count_s);
	assert(sse_count_v > sse_count_s);
#endif

#ifdef __CONCRETE
	for (int i = 0; i < N1*N1*3; i++) {
	  printf("%10d vs. %10d", mat3s->data.s[i], mat3v->data.s[i]);
		 
	  if (mat3s->data.s[i] == mat3v->data.s[i])
	    printf("\n");
	  else printf(" ...NO\n");
	}
#else
	//klee_dump_constraints();
	for (int i = 0; i < N1*N1*3; i++) {
	  printf("Iteration %d\n", i);
	  if (mat3s->data.s[i] != mat3v->data.s[i]) {
	        char buf[256];
		sprintf(buf, "mat3s->data.s[%d]", i);
		klee_print_expr(buf, mat3s->data.s[i]);
		sprintf(buf, "mat3v->data.s[%d]", i);
		klee_print_expr(buf, mat3v->data.s[i]);
		assert(mat3s->data.s[i] == mat3v->data.s[i]);
	  }
	}
	
#endif
}

