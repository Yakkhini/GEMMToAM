#include <gemm.h>

#define A(i, j) a[(j) * lda + (i)]
#define B(i, j) b[(j) * ldb + (i)]
#define C(i, j) c[(j) * ldc + (i)]

void AddDot4x4(int k, fixedpt *a, int lda, fixedpt *b, int ldb, fixedpt *c,
               int ldc) {

  register fixedpt c_00, c_01, c_02, c_03, c_10, c_11, c_12, c_13, c_20, c_21,
      c_22, c_23, c_30, c_31, c_32, c_33, a_0p, a_1p, a_2p, a_3p, b_0p_reg,
      b_1p_reg, b_2p_reg, b_3p_reg;
  fixedpt *b_0p_ptr, *b_1p_ptr, *b_2p_ptr, *b_3p_ptr;

  fixedpt temp;

  c_00 = 0.0;
  c_01 = 0.0;
  c_02 = 0.0;
  c_03 = 0.0;
  c_10 = 0.0;
  c_11 = 0.0;
  c_12 = 0.0;
  c_13 = 0.0;
  c_20 = 0.0;
  c_21 = 0.0;
  c_22 = 0.0;
  c_23 = 0.0;
  c_30 = 0.0;
  c_31 = 0.0;
  c_32 = 0.0;
  c_33 = 0.0;

  b_0p_ptr = &B(0, 0);
  b_1p_ptr = &B(0, 1);
  b_2p_ptr = &B(0, 2);
  b_3p_ptr = &B(0, 3);

  for (int p = 0; p < k; p++) {
    a_0p = A(0, p);
    a_1p = A(1, p);
    a_2p = A(2, p);
    a_3p = A(3, p);

    b_0p_reg = *b_0p_ptr++;
    b_1p_reg = *b_1p_ptr++;
    b_2p_reg = *b_2p_ptr++;
    b_3p_reg = *b_3p_ptr++;

    temp = fixedpt_mul(a_0p, b_0p_reg);
    c_00 = fixedpt_add(c_00, temp);
    temp = fixedpt_mul(a_1p, b_0p_reg);
    c_10 = fixedpt_add(c_10, temp);

    temp = fixedpt_mul(a_0p, b_1p_reg);
    c_01 = fixedpt_add(c_01, temp);
    temp = fixedpt_mul(a_1p, b_1p_reg);
    c_11 = fixedpt_add(c_11, temp);

    temp = fixedpt_mul(a_0p, b_2p_reg);
    c_02 = fixedpt_add(c_02, temp);
    temp = fixedpt_mul(a_1p, b_2p_reg);
    c_12 = fixedpt_add(c_12, temp);

    temp = fixedpt_mul(a_0p, b_3p_reg);
    c_03 = fixedpt_add(c_03, temp);
    temp = fixedpt_mul(a_1p, b_3p_reg);
    c_13 = fixedpt_add(c_13, temp);

    temp = fixedpt_mul(a_2p, b_0p_reg);
    c_20 = fixedpt_add(c_20, temp);
    temp = fixedpt_mul(a_3p, b_0p_reg);
    c_30 = fixedpt_add(c_30, temp);

    temp = fixedpt_mul(a_2p, b_1p_reg);
    c_21 = fixedpt_add(c_21, temp);
    temp = fixedpt_mul(a_3p, b_1p_reg);
    c_31 = fixedpt_add(c_31, temp);

    temp = fixedpt_mul(a_2p, b_2p_reg);
    c_22 = fixedpt_add(c_22, temp);
    temp = fixedpt_mul(a_3p, b_2p_reg);
    c_32 = fixedpt_add(c_32, temp);

    temp = fixedpt_mul(a_2p, b_3p_reg);
    c_23 = fixedpt_add(c_23, temp);
    temp = fixedpt_mul(a_3p, b_3p_reg);
    c_33 = fixedpt_add(c_33, temp);
  }

  C(0, 0) = fixedpt_add(C(0, 0), c_00);
  C(0, 1) = fixedpt_add(C(0, 1), c_01);
  C(0, 2) = fixedpt_add(C(0, 2), c_02);
  C(0, 3) = fixedpt_add(C(0, 3), c_03);

  C(1, 0) = fixedpt_add(C(1, 0), c_10);
  C(1, 1) = fixedpt_add(C(1, 1), c_11);
  C(1, 2) = fixedpt_add(C(1, 2), c_12);
  C(1, 3) = fixedpt_add(C(1, 3), c_13);

  C(2, 0) = fixedpt_add(C(2, 0), c_20);
  C(2, 1) = fixedpt_add(C(2, 1), c_21);
  C(2, 2) = fixedpt_add(C(2, 2), c_22);
  C(2, 3) = fixedpt_add(C(2, 3), c_23);

  C(3, 0) = fixedpt_add(C(3, 0), c_30);
  C(3, 1) = fixedpt_add(C(3, 1), c_31);
  C(3, 2) = fixedpt_add(C(3, 2), c_32);
  C(3, 3) = fixedpt_add(C(3, 3), c_33);
}

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
  // #pragma omp parallel for num_threads(4)
  for (int j = 0; j < n; j += 4) {   // Loop over the columns of C with stride 4
    for (int i = 0; i < m; i += 4) { // Loop over the rows of C
      AddDot4x4(k, &A(i, 0), lda, &B(0, j), ldb, &C(i, j), ldc);
    }
  }
  return;
}

int main() {

  int m = 20;
  int n = 20;
  int k = 20;

  fixedpt *A = (fixedpt *)malloc(m * k * sizeof(fixedpt));
  fixedpt *B = (fixedpt *)malloc(k * n * sizeof(fixedpt));
  fixedpt *C = (fixedpt *)malloc(m * n * sizeof(fixedpt));

  random_init_notype(m, k, A, m);
  random_init_notype(k, n, B, k);

  matmul(m, n, k, A, m, B, k, C, m);

  if (m <= 8) {
    printf("Matrix A : \n");
    display_notype(A, m, k);

    printf("Matrix B : \n");
    display_notype(B, k, n);

    printf("Dot Product Result : \n");
    display_notype(C, m, n);
  }

  return 0;
}
