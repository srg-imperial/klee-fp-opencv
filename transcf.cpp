#include "cv.h"

#ifndef __CONCRETE
#include <klee.h>
#endif
#include <assert.h>
#include <stdio.h>

#ifndef TRANS_N
#define TRANS_N 4
#endif
#ifndef TRANS_NC
#define TRANS_NC 3
#endif

#include "get_seed.cpp"

int main(int argc, char** argv) {
#ifndef __CONCRETE
	unsigned sse_count_v, sse_count_s;
#endif
	unsigned char mat1data[TRANS_N*TRANS_N*TRANS_NC];
	float mat2data[TRANS_NC*TRANS_N];
	CvMat mat1, mat2;
	CvMat *mat3v = cvCreateMat(TRANS_N, TRANS_N, CV_8UC(TRANS_NC));
	CvMat *mat3s = cvCreateMat(TRANS_N, TRANS_N, CV_8UC(TRANS_NC));

#ifdef __CONCRETE
	int seed = get_seed(argc, argv);
	srandom(seed);
	srand48(seed);

	for (int i=0; i < TRANS_N*TRANS_N*TRANS_NC; i++)
	  mat1data[i] = random();

	for (int i=0; i < TRANS_NC*TRANS_N; i++)
	  mat2data[i] = drand48();
#else
	klee_make_symbolic(mat1data, sizeof(mat1data), "mat1data");
	klee_make_symbolic(mat2data, sizeof(mat2data), "mat2data");
#endif

	mat1 = cvMat(TRANS_N, TRANS_N, CV_8UC(TRANS_NC), mat1data);
	mat2 = cvMat(TRANS_NC, TRANS_N, CV_32FC1, mat2data);

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
	int diffs = 0;
	for (int i = 0; i < TRANS_N*TRANS_N*TRANS_NC; i++) {
	  printf("%8d vs. %8d", mat3s->data.ptr[i], mat3v->data.ptr[i]);
		 
	  if (mat3s->data.ptr[i] == mat3v->data.ptr[i])
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
	//klee_dump_constraints();
	bool same = true;
	for (int i = 0; i < TRANS_N*TRANS_N*TRANS_NC; i++) {
	  printf("Iteration %d\n", i);
	  char buf[256];
	  sprintf(buf, "mat3s->data.ptr[%d]", i);
	  klee_print_expr(buf, mat3s->data.ptr[i]);
	  sprintf(buf, "mat3v->data.ptr[%d]", i);
	  klee_print_expr(buf, mat3v->data.ptr[i]);
	  same &= (mat3s->data.ptr[i] == mat3v->data.ptr[i]);
	}
	assert(same);
#endif
}

