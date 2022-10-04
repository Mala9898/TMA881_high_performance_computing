.PHONY : build clean test

build : cell_distances.c 
	gcc -o cell cell_distances.c -lm -fopenmp -O2


test :
	tar -czvf cell_distances.tar.gz cell_distances.c makefile
	python3 /home/hpc2022/cell_distances/check_submission.py cell_distances.tar.gz

clean:
	ls -l
	rm -rf cell cell_distances.tar.gz
	ls -l
