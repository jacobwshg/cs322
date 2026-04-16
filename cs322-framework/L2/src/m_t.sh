
#!/bin/bash

rm -rf ./*.out

LIV_DIR="./liv"

clang++ \
	-std=c++2a \
	-ferror-limit=2 \
	-o test.out \
	./parser.cpp $LIV_DIR/var_id_set.cpp $LIV_DIR/var_vis.cpp $LIV_DIR/instr_vis.cpp ./main.cpp  #\
	#&> log

