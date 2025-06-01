#include <am.h>
#include <klib-macros.h>
#include <klib.h>

#include "fixedpt.h"

#define A_row(i, j) a[(i) * lda + (j)]
#define B_row(i, j) b[(i) * ldb + (j)]
#define C_row(i, j) c[(i) * ldc + (j)]

#define A_col(i, j) a[(j) * lda + (i)]
#define B_col(i, j) b[(j) * ldb + (i)]
#define C_col(i, j) c[(j) * ldc + (i)]

void AddDot4x4(int, fixedpt *, int, fixedpt *, int, fixedpt *, int);
void PackMatrixA(int, fixedpt *, int, fixedpt *);
void PackMatrixB(int, fixedpt *, int, fixedpt *);
void InnerKernel(int, int, int, fixedpt *, int, fixedpt *, int, fixedpt *, int,
                 int);
void matmul(int m, int n, int k, fixedpt *a, int lda, fixedpt *b, int ldb,
            fixedpt *c, int ldc);

void serial_init(int m, int n, fixedpt *a, int lda, int type);
void random_init(int m, int n, fixedpt *a, int lda, int type);
void display(fixedpt *matrix, int m, int n, int type);
