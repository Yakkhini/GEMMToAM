set dotenv-load

fmt:
    find $GEMM_HOME -iname *.h -o -iname *.c | xargs clang-format -i

clean:
    rm -rf build
