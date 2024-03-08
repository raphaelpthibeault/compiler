#include <stack.h>
#include <util.h>
#include <string.h>

void stack_init(Stack *s, int elem_size) {
    s->elem_size = elem_size;
    s->elems = Malloc(INIT_STACK_SIZE * elem_size);
    s->size = INIT_STACK_SIZE;
    s->top = s->elems;
}

void stack_push(Stack *s, void *elem) {
    if ((char *)s->top - (char *)s->elems == s->size * s->elem_size) {
        s->size *= 2;
        s->elems = Realloc(s->elems, s->size * s->elem_size);
    }
    memcpy(s->top, elem, s->elem_size);
    s->top = (char *)s->top + s->elem_size;
}

void stack_pop(Stack *s, void *elem) {
    s->top = (char *)s->top - s->elem_size;
    memcpy(elem, s->top, s->elem_size);
}

void stack_peek(Stack *s, void *elem) {
    memcpy(elem, (char *)s->top - s->elem_size, s->elem_size);
}

