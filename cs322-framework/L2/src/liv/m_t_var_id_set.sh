
#!/bin/bash

rm -rf ./*.out

clang++ \
	-std=c++2a \
	-ferror-limit=2 \
	-o test.out \
	./var_id_set.cpp ./t_var_id_set.cc #\
	#&> log

