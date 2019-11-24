.PONY:force
force:



fmt:force
	clang-format -i *.c *.h

enter:
	docker-compose exec linux /bin/bash
test:force
	gcc -g 	-Wall -std=c11 -e _rt0_go runtime.c main.c
	./a.out
 