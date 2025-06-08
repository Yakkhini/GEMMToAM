NAME = GEMM
# SRCS = src/naive_gemm.c src/common.c
# SRCS = src/baseline_gemm.c src/common.c
SRCS = src/gemm.c src/matmul.c src/common.c
include $(AM_HOME)/Makefile
