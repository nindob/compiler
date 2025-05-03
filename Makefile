CC=cc
CXX=c++
COMPILER=build/compiler
INPUT=tests/input01
ASM=$(INPUT).s
OUT=out

all: $(COMPILER)

$(COMPILER):
	cd build && cmake .. && make

run: $(COMPILER)
	./$(COMPILER) $(INPUT)

assemble: run
	$(CC) -no-pie -o $(OUT) $(ASM) -lc

execute: assemble
	./$(OUT)

clean:
	rm -rf build $(ASM) $(OUT)

.PHONY: all run assemble execute clean 