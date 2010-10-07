CC = g++
CFLAGS = -Wall -I/usr/local/cuda/include/
LFLAGS =
OBJECTS = OpenCL.o Buffer.o Image2D.o Image3D.o Program.o Timer.o Launcher.o misc.o

AA = ar
AFLAGS = rsv

RELEASE = OCLpp.a

all: $(RELEASE)

$(RELEASE): $(OBJECTS)
	$(AA) $(AFLAGS) $@ $^

%.o: %.cpp %.h
	$(CC) $< -c -o $@ $(CFLAGS)
