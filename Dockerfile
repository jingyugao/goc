FROM gcc:4.9
COPY . /usr/src/goc
WORKDIR /usr/src/goc
RUN gcc -o myapp main.c
CMD ["/bin/sh"]