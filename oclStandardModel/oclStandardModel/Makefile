CC = g++
USR_PATH = /home/administrador
CFLAGS = -Wall -Ianttweakbar/include/ -I../OCLpp -I/usr/local/cuda/include/ -I$(USR_PATH)/NVIDIA_GPU_Computing_SDK/OpenCL/common/inc/ -Icam/ -I$(USR_PATH)/NVIDIA_GPU_Computing_SDK/shared/inc -L../OCLpp/ -I$(USR_PATH)/NVIDIA_GPU_Computing_SDK/OpenCL/common/lib/ -I$(USR_PATH)/NVIDIA_GPU_Computing_SDK/shared/lib/
LFLAGS = -lOpenCL -lglut -lGLEW
OBJECTS = rasterizer.o cpuRasterizer.o ompRasterizer.o gpuRasterizer.o Constraint.o LinearAlgebra.o nchoosek.o ../OCLpp/OCLpp.a $(USR_PATH)/NVIDIA_GPU_Computing_SDK/OpenCL/common/lib/liboclUtil_i386.a $(USR_PATH)/NVIDIA_GPU_Computing_SDK/shared/lib/libshrutil_i386.a

all: main.cpp $(OBJECTS)
	$(CC) $^ -o prog $(CFLAGS) $(LFLAGS)

%.o: %.cpp %.h
	$(CC) $< -c -o $@
