#include <gemm.h>

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
