.PHONY : clean test

diffusion : diffusion.c cl_functions.cl makefile
	gcc -o diffusion diffusion.c -lm -lOpenCL -O2 -g

test : diffusion cl_functions.cl makefile
	tar -czvf diffusion.tar.gz diffusion.c cl_functions.cl makefile
	echo "built archive...\n\n"
	./check_submission.py diffusion.tar.gz

clean:
	rm -rf extracted/ diffusion.tar.gz diffusion
	