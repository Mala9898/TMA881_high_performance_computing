.PHONY : build clean test

build : cell_distances.c 
	gcc -o cell_distances cell_distances.c -lm -fopenmp -O2


test :
	tar -czvf cell_distances.tar.gz cell_distances.c makefile
	echo "built archive...\n\n"
	./check_submission.py cell_distances.tar.gz

clean:
	ls -l
	rm -rf cell_distances cell_distances.tar.gz
	ls -l
