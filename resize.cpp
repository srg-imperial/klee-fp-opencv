#include "cv.h"
#ifndef __CONCRETE
#include <klee.h>
#endif
#include <assert.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>

int main(int argc, char **argv) {
	void *mat1data;
	size_t mat1size;
	CvMat mat1;
	CvMat *mat2v, *mat2s;

	int algo = CV_INTER_CUBIC;
	int mat1width = 4, mat2width = 8;
	int mat1height = 4, mat2height = 8;
	int format = CV_8UC1;
	int do_random = 0;

	int ch;
	while ((ch = getopt(argc, argv, "lcLf:w:h:W:H:r:")) != -1) {
		switch (ch) {
		case 'l': algo = CV_INTER_LINEAR; break;
		case 'c': algo = CV_INTER_CUBIC; break;
		case 'L': algo = 4 /* INTER_LANCZOS4 */; break;
		case 'f': {
#define ID(depth, type) (((depth) << 8) | (type))
			int id = ID(atoi(optarg+1), *optarg);
			switch (id) {
				case ID(8,'u'): format = CV_8UC1; break;
				case ID(16,'u'): format = CV_16UC1; break;
				case ID(16,'s'): format = CV_16SC1; break;
				case ID(32,'f'): format = CV_32FC1; break;
				default: puts("Unsupported format"); exit(1);
			}
			break;
#undef ID
		}
		case 'w': mat1width = atoi(optarg); break;
		case 'h': mat1height = atoi(optarg); break;
		case 'W': mat2width = atoi(optarg); break;
		case 'H': mat2height = atoi(optarg); break;
#ifdef __CONCRETE
		case 'r': do_random = 1; srandom(atoi(optarg)); break;
#endif
		}
	}

	mat2v = cvCreateMat(mat2width, mat2height, format);
	mat2s = cvCreateMat(mat2width, mat2height, format);

	mat1size = mat1width * mat1height * (1 << (CV_MAT_DEPTH(format) >> 1));
	mat1data = malloc(mat1size);
#ifdef __CONCRETE
	if (do_random) {
		char *mat1cdata = (char *) mat1data;
		int left = mat1size;
		while (left--)
			*(mat1cdata++) = random();
	} else {
		memcpy(mat1data, "\x7f\x7f\xba\x00\xb8\x9b/\x00\xde\xa4\x11\x00UH\xfe\x00", mat1size);
	}
#else
	klee_make_symbolic(mat1data, mat1size, "mat1data");
#endif

	mat1 = cvMat(mat1width, mat1height, format, mat1data);

	cvUseOptimized(true);
	cvResize(&mat1, mat2v, algo);
	cvUseOptimized(false);
	cvResize(&mat1, mat2s, algo);

#ifdef __CONCRETE
#define PRINT_AND_CHECK(FLD, S) \
	for (int i = 0; i < mat2width*mat2height; i++) { \
		printf("mat2s->data." #FLD "[%d] = " S "\n", i, mat2s->data.FLD[i]); \
		printf("mat2v->data." #FLD "[%d] = " S "\n", i, mat2v->data.FLD[i]); \
		if (mat2s->data.FLD[i] != mat2v->data.FLD[i]) { \
			puts("differ!"); \
		} \
	}
#else
#define PRINT_AND_CHECK(FLD, S) \
	for (int i = 0; i < mat2width*mat2height; i++) { \
		char buf[256]; \
		sprintf(buf, "mat2s->data." #FLD "[%d] == mat2v->data." #FLD "[%d]", i, i); \
		klee_print_expr(buf, mat2s->data.FLD[i] == mat2v->data.FLD[i]); \
		assert(mat2s->data.FLD[i] == mat2v->data.FLD[i]); \
	}
#endif

	switch (format) {
	case CV_8UC1:
		PRINT_AND_CHECK(ptr, "%d")
		break;
	case CV_16UC1:
	case CV_16SC1:
		PRINT_AND_CHECK(s, "%d")
		break;
	case CV_32FC1:
		PRINT_AND_CHECK(fl, "%f")
		break;
	default: puts("Unsupported format"); exit(1);
	}
}

