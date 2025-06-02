# 面向 GEMM 程序的虚拟 SIMD 指令支持规范

## 程序依赖分析

通过阅读源码、查询手册与阅读反汇编，推测源 GEMM 程序有以下依赖：

```c
/* src/matmul.c:
 * In `AddDot4x4` there is several SSE2 & SSE3 SIMD operations that we need to
 * implement the alternative for.
 *
 * Data Type:
 * `__m128d`
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
```

除此之外，发现 GEMM 程序中的 SIMD 具有规律性：

* SIMD 操作通常为 4 个一组，考虑实现 Batch 模式运算；
* 对于先乘后加的操作 `C = C + A * B`，考虑通过一条 CISC 原子指令实现。
