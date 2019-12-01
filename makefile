.PONY:force



force:
	mkdir -p bin

WNO=-Wno-unused-function  -Wno-unused-variable
CC=gcc
CF=-pthread -g -Werror -Wall -std=gnu11 $(WNO)
# runtime=-e _rt0_go runtime.c runtime2.c time.c context.c proc.c os.c
runtime=-e _rt0_go *.c

fmt:force
	clang-format -i *.c *.h


clean:force
	rm -rf *dSYM *.out ./bin/*

test_main:force
	$(CC) $(CF) -o ./bin/$@ ./test/main.c $(runtime)
	./bin/$@ 1>/dev/null

test_mess:force
	$(CC) $(CF) -o ./bin/$@ ./test/mess.c 
	./bin/$@  && false || true
	

test_netpoll:force
	$(CC) $(CF) -o ./bin/$@ ./test/netpoll.c netpoll.c
	./bin/$@

test_time:force
	$(CC) $(CF) -o ./bin/$@ ./test/time.c time.c
	./bin/$@  

test:clean test_mess test_time test_netpoll test_main 
