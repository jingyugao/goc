#include "../netpoll.h"

int main() {
  pollInit();
  int ppfd[2];
  if (pipe(ppfd) == -1) {
    panicf("pipe err");
  }
  pollAdd(0,'r');
  listhead *list = netpoll(true);
  list_for_each(pos, list) {
    pollDesc *pd = container_of(pos, pollDesc, list);
    printf("pd:%d\n", pd->fd);
  }
}