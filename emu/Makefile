
SOURCES = $(wildcard src/**.cpp src/**/**.cpp)
OBJ = ${SOURCES:.cpp=.o}
HEADERS = $(wildcard include/**.hpp)
CFLAGS:=-std=c++17 -Og
LDFLAGS=

INCLUDES:= -I./include/ 
LIBS:=


OUTFILE:=smisa-emu
OUTDIR:=./build


OUT:=$(OUTDIR)/$(OUTFILE)
$(info Final Product: '$(OUT)')






#Main Targets


.PHONY: all
all: clean main

main: $(OBJ)
	@echo $(OBJ)
	$(CXX) $(CFLAGS) $(LDFLAGS) $(OBJ) $(LIBS) -o $(OUT)

%.o : %.cpp $(HEADERS)
	@echo $(OBJ)
	@echo "[ $< ]"
	$(CXX) $(CFLAGS) $(INCLUDES)  -c $< -o $@

.PHONY: run
run: all
	./$(OUT)

	
.PHONY: clean
clean:
	rm -rf $(OUTDIR)/$(OUTFILE).*
	cd src
	find . -type f -name '*.o' -delete