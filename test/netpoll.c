#include "../netpoll.h"
#include "test.h"
int main() {
  pollInit();
  int ppfd[2];
  if (pipe(ppfd) == -1) {
    panicf("pipe err");
  }
  pollAdd(ppfd[0], 'r');

  pid_t pid = fork();
  if (pid == 0) {
    sleep(1);
    write(ppfd[1], "x", 1);
  } else {
    listhead *list = netpoll(true);
    list_for_each(pos, list) {
      pollDesc *pd = container_of(pos, pollDesc, list);
      if (pd->fd != ppfd[0]) {
        assert(pd->fd == ppfd[0]);
        assert(pd->r == true);
        assert(pd->w == false);
        char buf[10];
        read(pd->fd, buf, 1);
        assert(buf[0] = 'x');
      }
    }
  }
  test_ok;
}