.PHONY : clean test

newton : newton.c makefile
	gcc -o newton newton.c -lm -lpthread -O2 -g

test : newton makefile
	echo "TODO: fix makefile"

clean:
	rm -rf newton *.ppm extracted/ newton.tar.gz newton
	