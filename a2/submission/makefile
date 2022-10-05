.PHONY : clean test


cell_distances : cell_distances.c makefile
	gcc -o cell_distances cell_distances.c -lm -fopenmp -O2 -g

cellunpt: cell_distances.c makefile
	gcc -o cellunpt cell_distances.c -lm -fopenmp -O0 -g

# will compare date of left side to right side. will run if right side is more recent
# except for phony targets

test : cell_distances.c makefile
	tar -czvf cell_distances.tar.gz cell_distances.c makefile
	echo "built archive...\n\n"
	./check_submission.py cell_distances.tar.gz

clean:
	rm -rf cell_distances cell_distances.tar.gz distances/ extracted/
