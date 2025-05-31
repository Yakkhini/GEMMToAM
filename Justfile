set dotenv-load

run:
  make ARCH=riscv32e-npc run

fmt:
    find $GEMM_HOME -iname *.h -o -iname *.c | xargs clang-format -i

clean:
    rm -rf build
