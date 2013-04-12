.PHONY: all

all: test

clean:
	rm -f test.cc test.ast test.ll test.opt.ll test.amp.ll test.amp.opt.ll test.nvvm.ll test.s amp.o test.ptx test

test.cc: test.cpp amp.h ~/obj/llvm/Release+Asserts/bin/clang++ ~/obj/llvm/Release+Asserts/bin/clang ~/obj/llvm/Release+Asserts/bin/opt ~/obj/llvm/Release+Asserts/bin/llc
	~/obj/llvm/Release+Asserts/bin/clang++ -E -fc++amp -std=c++11 -I. test.cpp -o test.cc

test.ast: test.cc
	~/obj/llvm/Release+Asserts/bin/clang++ -cc1 -S -ast-dump -fc++amp -Wall -Werror -std=c++11 -I. test.cc > test.ast

test.ll: test.cc
	~/obj/llvm/Release+Asserts/bin/clang++ -cc1 -g -S -emit-llvm -fc++amp -Wall -Werror -std=c++11 -I. test.cc -o test.ll

test.opt.ll: test.ll test.ptx
	~/obj/llvm/Release+Asserts/bin/opt -amp-create-stubs -amp-kernel-file test.ptx -S test.ll -o test.opt.ll

test.amp.ll: test.cc
	~/obj/llvm/Release+Asserts/bin/clang++ -cc1 -S -emit-llvm -fc++amp -fc++amp-is-kernel -Wall -Werror -std=c++11 -I. test.cc -o test.amp.ll

test.amp.opt.ll: test.amp.ll
	~/obj/llvm/Release+Asserts/bin/opt -O3 -S test.amp.ll -o test.amp.opt.ll

test.nvvm.ll: test.amp.opt.ll
	~/obj/llvm/Release+Asserts/bin/opt -amp-to-opencl -O3 -S test.amp.opt.ll -o test.nvvm.ll

test.s: test.opt.ll
	~/obj/llvm/Release+Asserts/bin/llc test.opt.ll -o test.s

amp.o: amp.c
	~/obj/llvm/Release+Asserts/bin/clang -c amp.c -o amp.o

test.ptx: test.nvvm.ll
	~/obj/llvm/Release+Asserts/bin/llc -march=nvptx test.nvvm.ll -o test.ptx

test: test.s amp.o
	~/obj/llvm/Release+Asserts/bin/clang++ test.s amp.o -lOpenCL -o test
