ADIOS_DIR=/usr/local

CXX=mpic++
CXXFLAGS=-g -std=gnu++11 
LDFLAGS=-g

override ADIOS_INC=-I${ADIOS_DIR}/include
override ADIOS_LIB=`${ADIOS_DIR}/bin/adios2-config --libs`

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

