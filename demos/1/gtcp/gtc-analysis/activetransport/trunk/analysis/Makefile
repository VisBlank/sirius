CC= gcc
CXX= g++

all: staging

inline: Analyp.c gts_particle.h Scanfilter.c histogram.c inline.c transpose.c
	mpicc -g -I ${HOME}/work/smoky/include/  -c ./Analyp.c ./Scanfilter.c histogram.c inline.c transpose.c
	ar crvs libhistogram.a ./Analyp.o ./Scanfilter.o histogram.o inline.o transpose.o
	ranlib libhistogram.a

helper: Analyp.c gts_particle.h Scanfilter.c histogram.c transpose.c helper.c
	mpicc -g -I ${HOME}/work/smoky/include/ -I ${HOME}/gts/adios.staging -I ${HOME}/gts/adios.staging/src  -o helper ./helper.c ./Analyp.c ./Scanfilter.c histogram.c transpose.c -L ${HOME}/work/smoky/lib  -ladios -libclient -ladios_read_timing -ladios_sim_timing -L/ccs/proj/e2e/pnorbert/mxml/lib64 -L/ccs/proj/e2e/pnorbert/mxml/lib -lmxml
     
staging: Analyp.c gts_particle.h Scanfilter.c histogram.c transpose.c staging.c
	${CC} -g -O0 \
	-I ${HOME}/work/curie/include/ \
	-I ../adios_timing \
	-o gtc_read \
	./staging.c ./Analyp.c ./Scanfilter.c histogram.c transpose.c \
	-lmpi_stubs -L ${HOME}/work/curie/lib  -L../adios_timing -ladios_nompi \
	-ladios_read_timing -ladios_sim_timing \
	-lmxml \
	-lxpmem -static -static-libgcc -static-libstdc++ -lm
#	-L/opt/cray/xpmem/0.1-2.0402.45248.1.5.gem/lib64 -lxpmem -static -static-libgcc -static-libstdc++ -lm

#	-lpthread -L ${HOME}/work/curie/lib  -L../adios_timing -ladios_nompi \

#	-I ${HOME}/gts/adios.staging \
#	-I ${HOME}/gts/adios.staging/src  \
#	-L/ccs/proj/e2e/pnorbert/mxml/lib64 -L/ccs/proj/e2e/pnorbert/mxml/lib \

install:
	cp libhistogram.a ${HOME}/work/smoky/lib

clean:
#	rm Compsnd Comprcv *.o 
	rm *.o staging helper 
