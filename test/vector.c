#include "../base/vector.h"
#include "test.h"
int main(void) {
  vector v;
  vector_init(&v);

  vector_add(&v, "emil");
  vector_add(&v, "hannes");
  vector_add(&v, "lydia");
  vector_add(&v, "olle");
  vector_add(&v, "erik");

  int i;
  printf("first round:\n");
  for (i = 0; i < vector_count(&v); i++) {
    printf("%s\n", (char *)vector_get(&v, i));
  }

  vector_delete(&v, 1);
  vector_delete(&v, 3);

  printf("second round:\n");
  for (i = 0; i < vector_count(&v); i++) {
    printf("%s\n", (char *)vector_get(&v, i));
  }

  vector_free(&v);

  test_ok;
}