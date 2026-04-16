
#!/bin/bash

rm -rf ./*.out

clang++ \
	-std=c++2a \
	-ferror-limit=2 \
	-o test.out \
	./t_var_id_set.cpp ./var_id_set.cpp #\
	#&> log

