#include "cv.h"
#include <klee.h>
#include <assert.h>
#include <unistd.h>

int main(int argc, char **argv) {
	void *mat1data;
	size_t mat1size;
	CvMat mat1;
	CvMat *mat2v, *mat2s;

	int algo = CV_INTER_CUBIC;
	int mat1width = 4, mat2width = 8;
	int mat1height = 4, mat2height = 8;
	int format = CV_8UC1;

	int ch;
	while ((ch = getopt(argc, argv, "lcLf:w:h:W:H:")) != -1) {
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
		}
	}

	mat2v = cvCreateMat(mat2width, mat2height, format);
	mat2s = cvCreateMat(mat2width, mat2height, format);

	mat1size = mat1width * mat1height * (1 << (CV_MAT_DEPTH(format) >> 1));
	mat1data = malloc(mat1size);
	klee_make_symbolic(mat1data, mat1size, "mat1data");

	mat1 = cvMat(mat1width, mat1height, format, mat1data);

	cvUseOptimized(true);
	cvResize(&mat1, mat2v, algo);
	cvUseOptimized(false);
	cvResize(&mat1, mat2s, algo);

#define PRINT_AND_CHECK(FLD) \
	for (int i = 0; i < mat2width*mat2height; i++) { \
		char buf[256]; \
		sprintf(buf, "mat2s->data." #FLD "[%d]", i); \
		klee_print_expr(buf, mat2s->data.FLD[i]); \
		sprintf(buf, "mat2v->data." #FLD "[%d]", i); \
		klee_print_expr(buf, mat2v->data.FLD[i]); \
		assert(mat2s->data.FLD[i] == mat2v->data.FLD[i]); \
	}

	switch (format) {
	case CV_8UC1:
		PRINT_AND_CHECK(ptr)
		break;
	case CV_16UC1:
	case CV_16SC1:
		PRINT_AND_CHECK(s)
		break;
	case CV_32FC1:
		PRINT_AND_CHECK(fl)
		break;
	default: puts("Unsupported format"); exit(1);
	}
}

