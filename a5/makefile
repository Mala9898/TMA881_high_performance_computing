BINS = ping
LIBRARIES = -L/usr/lib64/openmpi/lib -lm -lmpi -Wl,-rpath,/usr/lib64/openmpi/lib -Wl,--enable-new-dtags
# CFLAGS  = -ffast-math -O3 -I. -I/usr/include/openmpi-x86_64 -pthread $(LIBRARIES)
CFLAGS  = -O2 -I. -I/usr/include/openmpi-x86_64 -pthread $(LIBRARIES)

.PHONY : all 

diffusion : diffusion.c makefile
	gcc $(CFLAGS) -o $@ $<

test : diffusion makefile
	tar -czvf diffusion.tar.gz diffusion.c makefile
	echo "built archive...\n\n"
	./check_submission.py diffusion.tar.gz

clean:
	rm -rf extracted/ diffusion.tar.gz diffusion

ping : ping.c
	gcc $(CFLAGS) -o $@ $<


scrap : scrap.c makefile
	gcc -o scrap scrap.c -lm -O2 -g