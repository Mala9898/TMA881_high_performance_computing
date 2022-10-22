BINS = ping
LIBRARIES = -L/usr/lib64/openmpi/lib -lm -lmpi -Wl,-rpath,/usr/lib64/openmpi/lib -Wl,--enable-new-dtags
CFLAGS  = -O2 -I. -I/usr/include/openmpi-x86_64 -pthread $(LIBRARIES)

.PHONY : all 

diffusion : diffusion.c makefile
	gcc $(CFLAGS) -o $@ $<

ping : ping.c
	gcc $(CFLAGS) -o $@ $<