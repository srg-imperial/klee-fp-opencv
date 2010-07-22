#include "cv.h"
#include <klee.h>
#include <assert.h>

#define N 4
#define NC 4

int main(int argc, char** argv) {
#ifndef __CONCRETE
	unsigned sse_count_v, sse_count_s;
#endif
  
	float mat1data[N*N*NC], mat2data[NC*N];
	CvMat mat1, mat2;
	CvMat *mat3v = cvCreateMat(N, N, CV_32FC(NC));
	CvMat *mat3s = cvCreateMat(N, N, CV_32FC(NC));

	klee_make_symbolic(mat1data, sizeof(mat1data), "mat1data");
	klee_make_symbolic(mat2data, sizeof(mat2data), "mat2data");

	mat1 = cvMat(N, N, CV_32FC(NC), mat1data);
	mat2 = cvMat(NC, N, CV_32FC1, mat2data);

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

	
	for (int i = 0; i < N*N*NC; i++) {
		char buf[256];
		if (mat3s->data.fl[i] != mat3v->data.fl[i]) {
		  sprintf(buf, "mat3s->data.fl[%d]", i);
		  klee_print_expr(buf, mat3s->data.fl[i]);
		  sprintf(buf, "mat3v->data.fl[%d]", i);
		  klee_print_expr(buf, mat3v->data.fl[i]);
		  
		  assert(mat3s->data.fl[i] == mat3v->data.fl[i]);
		}
	}
}

