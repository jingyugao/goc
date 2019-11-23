.PONY:force
force:

enter:
	docker-compose exec linux /bin/bash
test:force
	gcc runtime.c -g	-Wall -std=c11
	./a.out