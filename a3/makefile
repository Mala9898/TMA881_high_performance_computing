.PHONY : clean test

newton : newton.c makefile
	gcc -o newton newton.c -lm -lpthread -O2 -g

test : newton makefile
	tar -czvf newton.tar.gz newton.c makefile
	echo "built archive...\n\n"
	./check_submission.py newton.tar.gz

clean:
	rm -rf newton *.ppm extracted/ pictures/ newton.tar.gz newton
	