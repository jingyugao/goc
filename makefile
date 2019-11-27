.PONY:force
force:



fmt:force
	clang-format -i *.c *.h

enter:
	docker-compose exec linux /bin/bash
test:force
	gcc -g 	-Wall -std=c11 -e _rt0_go  main.c runtime.c runtime2.c time.c context.c proc.c os.c
	./a.out
demo:force
	gcc -std=c11 demo.c 
	./a.out
 