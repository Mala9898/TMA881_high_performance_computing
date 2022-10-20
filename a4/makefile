.PHONY : clean test

diffusion : diffusion.c makefile
	gcc -o diffusion diffusion.c -lm -lOpenCL -O2 -g

test : diffusion makefile
	tar -czvf diffusion.tar.gz diffusion.c makefile
	echo "built archive...\n\n"
	./check_submission.py diffusion.tar.gz

clean:
	rm -rf extracted/ diffusion.tar.gz diffusion
	