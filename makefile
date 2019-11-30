.PONY:force



force:
	echo "force"

WNO=-Wno-unused-function  -Wno-unused-variable
CC=gcc
CF=-g -Wall -std=c11 $(WNO)
# runtime=-e _rt0_go runtime.c runtime2.c time.c context.c proc.c os.c
runtime=-e _rt0_go *.c

fmt:force
	clang-format -i *.c *.h

build_main:force
	$(CC) $(CF) -o ./bin/$@ ./test/main.c $(runtime)
clean:force
	rm -rf *dSYM *.out ./bin/*

test_main:build_main
	./bin/$@

test_time:force
	$(CC) $(CF) -o ./bin/$@ ./test/time.c time.c
	./bin/$@

test:test_time test_main 
