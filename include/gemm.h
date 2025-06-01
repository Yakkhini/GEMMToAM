#include <am.h>
#include <klib-macros.h>
#include <klib.h>

#include "fixedpt.h"

void AddDot4x4(int, fixedpt *, int, fixedpt *, int, fixedpt *, int);
void PackMatrixA(int, fixedpt *, int, fixedpt *);
void PackMatrixB(int, fixedpt *, int, fixedpt *);
void InnerKernel(int, int, int, fixedpt *, int, fixedpt *, int, fixedpt *, int,
                 int);
void matmul(int m, int n, int k, fixedpt *a, int lda, fixedpt *b, int ldb,
            fixedpt *c, int ldc);
