#include "fixedpt.h"
#include <gemm.h>

#define A_row(i, j) a[(i) * lda + (j)]
#define B_row(i, j) b[(i) * ldb + (j)]
#define C_row(i, j) c[(i) * ldc + (j)]

#define A_col(i, j) a[(j) * lda + (i)]
#define B_col(i, j) b[(j) * ldb + (i)]
#define C_col(i, j) c[(j) * ldc + (i)]

void serial_init(int m, int n, fixedpt *a, int lda, int type) {
  int count = 1;
  for (int j = 0; j < n; j++) {
    for (int i = 0; i < m; i++) {
      if (type == 0)
        A_row(i, j) = count++;
      else
        A_col(i, j) = count++;
    }
  }
}

void random_init(int m, int n, fixedpt *a, int lda, int type) {
  for (int j = 0; j < n; j++) {
    for (int i = 0; i < m; i++) {
      if (type == 0)
        A_row(i, j) = 2 * rand() - 1;
      else
        A_col(i, j) = 2 * rand() - 1;
    }
  }
}

void display(fixedpt *matrix, int m, int n, int type) {
  if (type == 0) {
    for (int i = 0; i < m; i++) {
      for (int j = 0; j < n; j++) {
        printf("%f ", matrix[i * m + j]);
      }
      printf("\n");
    }
  } else {
    for (int j = 0; j < n; j++) {
      for (int i = 0; i < m; i++) {
        printf("%f ", matrix[j * m + i]);
      }
      printf("\n");
    }
  }
  return;
}

void matmul_row(int m, int n, int k, fixedpt *a, int lda, fixedpt *b, int ldb,
                fixedpt *c, int ldc) {
  /*
  Computes the matrix multiplication of A and B and stores in C.

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

  Matrix Multiply Condition
  -------------------------
          Number of Columns in first matrix must be equal to the number of rows
  in the second matrix
  */

  if (a == NULL || b == NULL || c == NULL) {
    printf(
        "Argument Error : One of the input arguments to matmul() was NULL\n");
    return;
  }

  for (int i = 0; i < m; i++) {
    for (int j = 0; j < n; j++) {
      for (int p = 0; p < k; p++) {
        C_row(i, j) += A_row(i, p) * B_row(p, j);
      }
    }
  }
  return;
}

void matmul_col(int m, int n, int k, fixedpt *a, int lda, fixedpt *b, int ldb,
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

  Matrix Multiply Condition
  -------------------------
          Number of Columns in first matrix must be equal to the number of rows
  in the second matrix
  */

  if (a == NULL || b == NULL || c == NULL) {
    printf(
        "Argument Error : One of the input arguments to matmul() was NULL\n");
    return;
  }

  for (int i = 0; i < m; i++) {
    for (int j = 0; j < n; j++) {
      for (int p = 0; p < k; p++) {
        // C_col(i, j) = C_col(i, j) + A_col(i, p) * B_col(p, j);

        fixedpt ab_mul = fixedpt_mul(A_col(i, p), B_col(p, j));
        C_col(i, j) = fixedpt_add(C_col(i, j), ab_mul);
      }
    }
  }
  return;
}

int main() {

  int m = 2000;
  int n = 2000;
  int k = 2000;

  fixedpt *A = (fixedpt *)malloc(m * k * sizeof(fixedpt)); // A = (m,k)
  fixedpt *B = (fixedpt *)malloc(k * n * sizeof(fixedpt)); // B = (k,n)
  fixedpt *C = (fixedpt *)malloc(m * n * sizeof(fixedpt)); // C = (m,n)
  fixedpt *D = (fixedpt *)malloc(m * n * sizeof(fixedpt)); // D = (m,n)

  random_init(m, k, A, m, 0);
  random_init(k, n, B, k, 0);

  matmul_row(m, n, k, A, m, B, k, C, m);

  random_init(m, k, A, m, 1);
  random_init(k, n, B, k, 1);

  matmul_col(m, n, k, A, m, B, k, D, m);

  free(A);
  free(B);
  free(C);
  free(D);
  return 0;
}
