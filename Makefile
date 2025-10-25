CXX = g++
ISPC = ispc

CXXFLAGS = -Wall -std=c++17 -O2
ISPCFLAGS = -O2

TARGET = newton
SRC = src/newton.cpp
ISPC_SRC = src/newtonApprox.ispc 
ISPC_HDR  = src/newtonApprox.h
ISPC_OBJ  = $(ISPC_SRC:.ispc=.o)

TASKSYS = src/tasksys.cpp

LODEPNG_SRC = src/lodepng.cpp
LODEPNG_HDR = src/lodepng.h

.PHONY: all clean

all: $(TARGET)

%.o %.h: %.ispc
	$(ISPC) $(ISPCFLAGS) $< -o $*.o -h $*.h

$(TARGET): $(SRC) $(ISPC_OBJ) $(ISPC_HDR) $(LODEPNG_SRC) $(TASKSYS)
	$(CXX) $(CXXFLAGS) $(SRC) $(ISPC_OBJ) $(TASKSYS) $(LODEPNG_SRC) -lpthread -o $(TARGET)

clean:
	rm -rf $(TARGET) $(ISPC_OBJ) *.png *.ppm