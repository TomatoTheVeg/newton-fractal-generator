CXX = g++
ISPC = ispc

CXXFLAGS = -Wall -std=c++17 -O2
ISPCFLAGS = -O2

TARGET = newton
SRC = newton.cpp
ISPC_SRC = newtonApprox.ispc complexISPC.ispc
ISPC_HDR  = newtonApprox.h
ISPC_OBJ  = $(ISPC_SRC:.ispc=.o)

TASKSYS = tasksys.cpp

.PHONY: all clean

all: $(TARGET)

# complexISPC.isph: complexISPC.ispc
# 	@echo "// Auto-generated from $< — do not edit" > $@
# 	@echo "#pragma once" >> $@
# 	@awk '\
# /^[[:space:]]*export[[:space:]]/ { \
#   sig=$$0; \
#   while (sig !~ /\)\s*\{/ && (getline l)>0) sig = sig "\n" l; \
#   sub(/^[[:space:]]*export[[:space:]]+/, "extern ", sig); \
#   sub(/\)\s*\{.*/, ");", sig); \
#   print sig; \
# }' $< >> $@

%.o %.h: %.ispc
	$(ISPC) $(ISPCFLAGS) $< -o $*.o -h $*.h

$(TARGET): $(SRC) $(ISPC_OBJ) $(ISPC_HDR)
	$(CXX) $(CXXFLAGS) $(SRC) $(ISPC_OBJ) $(TASKSYS) -lpthread -o $(TARGET)

newtonApprox.o newtonApprox.h: complexISPC.isph complexISPC.o
clean:
	rm -f $(TARGET) $(ISPC_HDR) $(ISPC_OBJ) complexISPC.isph