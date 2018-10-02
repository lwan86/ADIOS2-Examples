ADIOS_DIR=/ccs/home/lwan86/adios2-bp4/install-dir

CXX=CC
CXXFLAGS=-g -std=gnu++11 
LDFLAGS=-g

override ADIOS_INC=-I${ADIOS_DIR}/include
#override ADIOS_LIB=`${ADIOS_DIR}/bin/adios2-config --libs`
override ADIOS_LIB=-L${ADIOS_DIR}/lib -ladios2 -L/usr/lib64 -lbz2

default: help
all: miniapp

INC=${ADIOS_INC}


help:
	@echo "Make targets: "
	@echo " all:         build the examples "
	@echo " clean:       delete files from the build process"
	@echo " clean-files: delete files from running the examples"


%.o : %.cpp
	${CXX} ${CXXFLAGS} -c ${INC} -o $@ $< 

miniapp: main.o MiniApp.o IO_adios2.o Settings.o
	${CXX} ${CXXFLAGS} -o miniapp $^ ${ADIOS_LIB} 

clean:
	-rm *.o $(objects) miniapp
