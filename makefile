.PONY:force
force:

CC=gcc
CF=-g -Wall -std=c11
runtime=-e _rt0_go runtime.c runtime2.c time.c context.c proc.c os.c

fmt:force
	clang-format -i *.c *.h

clean:force
	rm -rf *dSYM *.out

test_main:force
	$(CC) $(CF) -o ./bin/$@. ./test/main.c $(runtime)
	./bin/$@.

test_time:force
	$(CC) $(CF) -o ./bin/$@. ./test/time.c time.c
	./bin/$@.

test:test_time test_main 
