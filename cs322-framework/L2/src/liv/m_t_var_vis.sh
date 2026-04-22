
#!/bin/bash

rm -rf ./*.out

clang++ \
	-std=c++2a \
	-ferror-limit=2 \
	-o test.out \
	./var_vis.cpp ./t_var_vis.cc #\
	#&> log

