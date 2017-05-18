CC=gcc

FLAGS=-O3 -Wno-unused-result
LDFLAGS=-lpthread
NTHREADS=4
#DEBUG=-DDEBUG
RESULT=-DRESULT
INPUT=input-little.in
all: clean_compile_run

gol: gol.c
	$(CC) $(DEBUG) $(RESULT) $(FLAGS) gol.c -o gol $(LDFLAGS)

clean:
	rm -rf gol
	
clean_compile_run:
	rm -rf gol
	$(CC) $(DEBUG) $(RESULT) $(FLAGS) gol.c -o gol $(LDFLAGS)
	cat $(INPUT) | ./gol $(NTHREADS)
	
run:
	cat $(INPUT) | ./gol $(NTHREADS)
