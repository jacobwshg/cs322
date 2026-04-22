
#!/bin/bash

rm -rf ./*.out

clang++ \
	-std=c++2a \
	-ferror-limit=2 \
	./unparser.cpp ./spiller.cpp ../parser.cpp ./main.cc  \
	-o test.out \
	#&> log

