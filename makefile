.PONY:force
force:

enter:
	docker-compose exec linux /bin/bash
ctx:force
	gcc context.c -g	-Wall -std=c11
	./a.out