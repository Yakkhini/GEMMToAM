#include "klib.h"
#include <gemm.h>

/* Create macros so that the matrices are stored in column-major order */

#define A(i, j) a[(j) * lda + (i)]
#define B(i, j) b[(j) * ldb + (i)]
#define C(i, j) c[(j) * ldc + (i)]

/* Block sizes */
#define mc 256
#define kc 128
#define nb 1000

#define min(i, j) ((i) < (j) ? (i) : (j))

/* Routine for computing C = A * B + C */

void matmul(int m, int n, int k, fixedpt *a, int lda, fixedpt *b, int ldb,
            fixedpt *c, int ldc) {
  /*
  Computes the matrix multiplication of A and B and stores in C.
  C = A*B + C
  Arguments
  ---------
          m,n,k : Specifies matrix dimensions
          a : pointer to first matrix
          b : pointer to second matrix
          c : pointer to the resultant matrix
          lda : leading dimension of matrix a
          ldb : leading dimension of matrix b
          ldc : leading dimension of matrix c

  Return
  ------
          None
  */

  if (a == NULL || b == NULL || c == NULL) {
    printf(
        "Argument Error : One of the input arguments to matmul() was NULL\n");
    return;
  }

  int i, p, pb, ib;

  /* This time, we compute a mc x n block of C by a call to the InnerKernel */

  for (p = 0; p < k; p += kc) {
    pb = min(k - p, kc);
    for (i = 0; i < m; i += mc) {
      ib = min(m - i, mc);
      InnerKernel(ib, n, pb, &A(i, p), lda, &B(p, 0), ldb, &C(i, 0), ldc,
                  i == 0);
    }
  }
  return;
}

void InnerKernel(int m, int n, int k, fixedpt *a, int lda, fixedpt *b, int ldb,
                 fixedpt *c, int ldc, int first_time) {
  int i, j;
  fixedpt *packedA = (fixedpt *)malloc(m * k * sizeof(fixedpt));
  fixedpt *packedB = (fixedpt *)malloc(kc * nb * sizeof(fixedpt));

  for (j = 0; j < n; j += 4) {
    if (first_time)
      PackMatrixB(k, &B(0, j), ldb, &packedB[j * k]);
    for (i = 0; i < m; i += 4) {
      if (j == 0)
        PackMatrixA(k, &A(i, 0), lda, &packedA[i * k]);
      AddDot4x4(k, &packedA[i * k], 4, &packedB[j * k], k, &C(i, j), ldc);
    }
  }
}

void PackMatrixA(int k, fixedpt *a, int lda, fixedpt *a_to) {
  int j;
  for (j = 0; j < k; j++) { /* loop over columns of A */
    fixedpt *a_ij_pntr = &A(0, j);
    *a_to = *a_ij_pntr;
    *(a_to + 1) = *(a_ij_pntr + 1);
    *(a_to + 2) = *(a_ij_pntr + 2);
    *(a_to + 3) = *(a_ij_pntr + 3);

    a_to += 4;
  }
}

void PackMatrixB(int k, fixedpt *b, int ldb, fixedpt *b_to) {
  int i;
  fixedpt *b_i0_pntr = &B(0, 0), *b_i1_pntr = &B(0, 1), *b_i2_pntr = &B(0, 2),
          *b_i3_pntr = &B(0, 3);

  for (i = 0; i < k; i++) { /* loop over rows of B */
    *b_to++ = *b_i0_pntr++;
    *b_to++ = *b_i1_pntr++;
    *b_to++ = *b_i2_pntr++;
    *b_to++ = *b_i3_pntr++;
  }
}

/*
 * In Intel SIMD intructions, use 128-bit vectors to store two double-precision
 * numbers. We use a virtual mmio-coprocessor with proper interface.
 *
 * */
typedef union {
  fixedpt d[2];
} v2df_t;

/*
 * In `AddDot4x4` there is several SSE2 & SSE3 SIMD operations that we need to
 * implement the alternative for.
 *
 * Include Header files:
 * <emmintrin.h> SSE2
 * <pmmintrin.h> SSE3
 *
 * Operations:
 * `__mm_setzero_pd()`: SSE2 `xorpd xmm, xmm` Return vector of type `__m128d`
 * with all elements set to zero
 * `__mm_load_pd()`: SSE2 `movapd xmm, m128` Load two double-precision
 * floating-point values from memory into a pointer of type `__m128d`
 * `__mm_loaddup_pd()`: SSE3 `movddup xmm, m64` Load a double-precision
 * floating-point value from memory and duplicate it to both elements of the
 * vector
 *
 * Additional Instructions from disassembly with `objdump -d` command:
 * `addpd`: SSE2 `__mm_add_pd` Add packed double-precision (64-bit)
 * floating-point elements in a and b, and store the results in dst.
 * `mulpd`: SSE3 `__mm_mul_pd` Multiply packed double-precision (64-bit)
 * floating-point elements in a and b, and store the results in dst.
 *
 * Other not important instructions:
 * Like `*sd` `unpckhpd` compiler will handle them after use `fixedpt` data
 * type.
 *
 * Reference:
 * Intel User Guide:
 * https://www.intel.com/content/www/us/en/docs/intrinsics-guide/index.html
 * LLVM `emmintrin.h` Reference:
 * https://clang.llvm.org/doxygen/emmintrin_8h.html
 * LLVM `pmmintrin.h` Reference:
 * https://clang.llvm.org/doxygen/pmmintrin_8h.html
 *
 * */
void AddDot4x4(int k, fixedpt *a, int lda, fixedpt *b, int ldb, fixedpt *c,
               int ldc) {
  /* So, this routine computes a 4x4 block of matrix A
           C( 0, 0 ), C( 0, 1 ), C( 0, 2 ), C( 0, 3 ).
           C( 1, 0 ), C( 1, 1 ), C( 1, 2 ), C( 1, 3 ).
           C( 2, 0 ), C( 2, 1 ), C( 2, 2 ), C( 2, 3 ).
           C( 3, 0 ), C( 3, 1 ), C( 3, 2 ), C( 3, 3 ).
     Notice that this routine is called with c = C( i, j ) in the
     previous routine, so these are actually the elements
           C( i  , j ), C( i  , j+1 ), C( i  , j+2 ), C( i  , j+3 )
           C( i+1, j ), C( i+1, j+1 ), C( i+1, j+2 ), C( i+1, j+3 )
           C( i+2, j ), C( i+2, j+1 ), C( i+2, j+2 ), C( i+2, j+3 )
           C( i+3, j ), C( i+3, j+1 ), C( i+3, j+2 ), C( i+3, j+3 )

     in the original matrix C
     And now we use vector registers and instructions */
  int p;
  v2df_t c_00_c_10_vreg, c_01_c_11_vreg, c_02_c_12_vreg, c_03_c_13_vreg,
      c_20_c_30_vreg, c_21_c_31_vreg, c_22_c_32_vreg, c_23_c_33_vreg,
      a_0p_a_1p_vreg, a_2p_a_3p_vreg, b_p0_vreg, b_p1_vreg, b_p2_vreg,
      b_p3_vreg;

  // c_00_c_10_vreg.v = _mm_setzero_pd();
  // c_01_c_11_vreg.v = _mm_setzero_pd();
  // c_02_c_12_vreg.v = _mm_setzero_pd();
  simd_setzero((uintptr_t)&c_00_c_10_vreg, (uintptr_t)&c_01_c_11_vreg,
               (uintptr_t)&c_02_c_12_vreg);
  // c_03_c_13_vreg.v = _mm_setzero_pd();
  // c_20_c_30_vreg.v = _mm_setzero_pd();
  // c_21_c_31_vreg.v = _mm_setzero_pd();
  simd_setzero((uintptr_t)&c_03_c_13_vreg, (uintptr_t)&c_20_c_30_vreg,
               (uintptr_t)&c_21_c_31_vreg);
  // c_22_c_32_vreg.v = _mm_setzero_pd();
  // c_23_c_33_vreg.v = _mm_setzero_pd();
  simd_setzero((uintptr_t)&c_22_c_32_vreg, (uintptr_t)&c_23_c_33_vreg, 0x00);

  for (p = 0; p < k; p++) {
    // a_0p_a_1p_vreg.v = _mm_load_pd((fixedpt *)a);
    simd_load((uintptr_t)&a_0p_a_1p_vreg, (uintptr_t)(a));
    // a_2p_a_3p_vreg.v = _mm_load_pd((fixedpt *)(a + 2));
    simd_load((uintptr_t)&a_2p_a_3p_vreg, (uintptr_t)(a + 2));
    a += 4;

    // b_p0_vreg.v = _mm_loaddup_pd((fixedpt *)b);
    simd_loaddup((uintptr_t)&b_p0_vreg,
                 (uintptr_t)(b + 0)); /* load and duplicate */
    // b_p1_vreg.v = _mm_loaddup_pd((fixedpt *)(b + 1));
    simd_loaddup((uintptr_t)&b_p1_vreg,
                 (uintptr_t)(b + 1)); /* load and duplicate */
    // b_p2_vreg.v = _mm_loaddup_pd((fixedpt *)(b + 2));
    simd_loaddup((uintptr_t)&b_p2_vreg,
                 (uintptr_t)(b + 2)); /* load and duplicate */
    // b_p3_vreg.v = _mm_loaddup_pd((fixedpt *)(b + 3));
    simd_loaddup((uintptr_t)&b_p3_vreg,
                 (uintptr_t)(b + 3)); /* load and duplicate */
    b += 4;

    /* First row and second rows */
    // c_00_c_10_vreg.v += a_0p_a_1p_vreg.v * b_p0_vreg.v;
    simd_mul_add((uintptr_t)&c_00_c_10_vreg, (uintptr_t)&a_0p_a_1p_vreg,
                 (uintptr_t)&b_p0_vreg);
    // c_01_c_11_vreg.v += a_0p_a_1p_vreg.v * b_p1_vreg.v;
    simd_mul_add((uintptr_t)&c_01_c_11_vreg, (uintptr_t)&a_0p_a_1p_vreg,
                 (uintptr_t)&b_p1_vreg);
    // c_02_c_12_vreg.v += a_0p_a_1p_vreg.v * b_p2_vreg.v;
    simd_mul_add((uintptr_t)&c_02_c_12_vreg, (uintptr_t)&a_0p_a_1p_vreg,
                 (uintptr_t)&b_p2_vreg);
    // c_03_c_13_vreg.v += a_0p_a_1p_vreg.v * b_p3_vreg.v;
    simd_mul_add((uintptr_t)&c_03_c_13_vreg, (uintptr_t)&a_0p_a_1p_vreg,
                 (uintptr_t)&b_p3_vreg);

    /* Third and fourth rows */
    // c_20_c_30_vreg.v += a_2p_a_3p_vreg.v * b_p0_vreg.v;
    simd_mul_add((uintptr_t)&c_20_c_30_vreg, (uintptr_t)&a_2p_a_3p_vreg,
                 (uintptr_t)&b_p0_vreg);
    // c_21_c_31_vreg.v += a_2p_a_3p_vreg.v * b_p1_vreg.v;
    simd_mul_add((uintptr_t)&c_21_c_31_vreg, (uintptr_t)&a_2p_a_3p_vreg,
                 (uintptr_t)&b_p1_vreg);
    // c_22_c_32_vreg.v += a_2p_a_3p_vreg.v * b_p2_vreg.v;
    simd_mul_add((uintptr_t)&c_22_c_32_vreg, (uintptr_t)&a_2p_a_3p_vreg,
                 (uintptr_t)&b_p2_vreg);
    // c_23_c_33_vreg.v += a_2p_a_3p_vreg.v * b_p3_vreg.v;
    simd_mul_add((uintptr_t)&c_23_c_33_vreg, (uintptr_t)&a_2p_a_3p_vreg,
                 (uintptr_t)&b_p3_vreg);
  }

  C(0, 0) += c_00_c_10_vreg.d[0];
  C(0, 1) += c_01_c_11_vreg.d[0];
  C(0, 2) += c_02_c_12_vreg.d[0];
  C(0, 3) += c_03_c_13_vreg.d[0];

  C(1, 0) += c_00_c_10_vreg.d[1];
  C(1, 1) += c_01_c_11_vreg.d[1];
  C(1, 2) += c_02_c_12_vreg.d[1];
  C(1, 3) += c_03_c_13_vreg.d[1];

  C(2, 0) += c_20_c_30_vreg.d[0];
  C(2, 1) += c_21_c_31_vreg.d[0];
  C(2, 2) += c_22_c_32_vreg.d[0];
  C(2, 3) += c_23_c_33_vreg.d[0];

  C(3, 0) += c_20_c_30_vreg.d[1];
  C(3, 1) += c_21_c_31_vreg.d[1];
  C(3, 2) += c_22_c_32_vreg.d[1];
  C(3, 3) += c_23_c_33_vreg.d[1];
}
