
#!/bin/bash

clang++ -std=c++2a -ferror-limit=2 ./parser.cpp ./parser_test.cpp -o parser_test.out \
	&> log

