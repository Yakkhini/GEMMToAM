#include <gemm.h>

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
