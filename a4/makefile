.PHONY : clean test

newton : diffusion.c makefile
	gcc -o diffusion diffusion.c -lm -lOpenCL -O2 -g

test : newton makefile
	tar -czvf diffusion.tar.gz diffusion.c makefile
	echo "built archive...\n\n"
	./check_submission.py diffusion.tar.gz

clean:
	rm -rf extracted/ diffusion.tar.gz diffusion
	