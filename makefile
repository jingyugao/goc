.PONY:force
force:

enter:
	docker-compose exec linux /bin/bash
test:force
	gcc -g	-Wall -std=c11 -e _rt0_go runtime.c main.c
	./a.out

test_slice:force
	gcc -g -Wall -std=c11 slice_test.c 
	./a.out