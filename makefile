.PONY:force



force:
	mkdir -p bin

WNO=-Wno-unused-function  -Wno-unused-variable
CC=gcc
CF= -g -Wall -Werror -std=c11 $(WNO) 
# runtime=-e _rt0_go runtime.c runtime2.c time.c context.c proc.c os.c
runtime=-e _rt0_go -finstrument-functions *.o

fmt:force
	clang-format -i *.[hc]
	clang-format -i ./base/*.[hc]
	clang-format -i ./test/*.[hc]

check_fmt:fmt
	git diff --exit-code

clean:force
	rm -rf *dSYM *.out ./bin/*

runtime:force
	$(CC) $(CF) -c *.c

test_sema:runtime
	$(CC) $(CF) -o ./bin/$@ ./test/sema.c $(runtime)
	./bin/$@

test_retake:force
	$(CC) $(CF) -DMAXPROC=1 -c *.c
	$(CC) $(CF) -o ./bin/$@ ./test/retake.c $(runtime)
	./bin/$@ || ([ $$? -eq 6 ])
	$(CC) $(CF) -DCALL_FF -o ./bin/$@ ./test/retake.c $(runtime)
	./bin/$@

test_netpoll:force
	$(CC) $(CF) -o ./bin/$@ ./test/netpoll.c netpoll.c
	./bin/$@

test_heap:force
	$(CC) $(CF) -o ./bin/$@ ./test/heap.c
	./bin/$@

test_vector:force
	$(CC) $(CF) -o ./bin/$@ ./test/vector.c  
	./bin/$@
	
test_slice:force
	$(CC) $(CF) -o ./bin/$@ ./test/slice.c  
	./bin/$@

test_time:force
	$(CC) $(CF) -o ./bin/$@ ./test/time.c time.c
	./bin/$@

test:clean test_vector test_slice test_time test_netpoll test_sema test_retake
