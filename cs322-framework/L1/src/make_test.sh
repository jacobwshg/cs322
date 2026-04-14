
#!/bin/bash

rm -rf ./*.out

clang++ -std=c++2a -ferror-limit=2 ./parser.cpp ./codegen.cpp ./test.cpp -o test.out \
	&> log

