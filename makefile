.PONY:force



force:
	mkdir -p bin

WNO=-Wno-unused-function  -Wno-unused-variable
CC=gcc
CF=-g -Wall -std=c11 $(WNO)
# runtime=-e _rt0_go runtime.c runtime2.c time.c context.c proc.c os.c
runtime=-e _rt0_go *.c

fmt:force
	clang-format -i *.[hc]
	clang-format -i ./base/*.[hc]
	clang-format -i ./test/*.[hc]

check_fmt:fmt
	git diff --exit-code

clean:force
	rm -rf *dSYM *.out ./bin/*

test_main:force
	$(CC) $(CF) -o ./bin/$@ ./test/main.c $(runtime)
	./bin/$@


test_netpoll:force
	$(CC) $(CF) -o ./bin/$@ ./test/netpoll.c netpoll.c
	./bin/$@

test_heap:force
	$(CC) $(CF) -o ./bin/$@ ./test/heap.c
	./bin/$@

test_slice:force
	$(CC) $(CF) -o ./bin/$@ ./test/slice.c  
	./bin/$@

test_time:force
	$(CC) $(CF) -o ./bin/$@ ./test/time.c time.c
	./bin/$@

test:clean test_slice test_time test_netpoll test_main 
