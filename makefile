.PONY:force
force:
	

test:force
	gcc main.c call.c
	./a.out
enter:
	docker-compose exec linux /bin/bash

demo:force
	gcc demo/ctx.S demo/demo.c -o bin/demo

ctx:force
	gcc context.c
	./a.out