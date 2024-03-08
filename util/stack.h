#ifndef COMPILER_STACK_H
#define COMPILER_STACK_H

#define INIT_STACK_SIZE 100

typedef struct {
    void **elems;
    int elem_size;
    int size;
    void *top;
} Stack;

void stack_init(Stack *s, int elem_size);
void stack_push(Stack *s, void *elem);
void stack_pop(Stack *s, void *elem);
void stack_peek(Stack *s, void *elem);


#endif //COMPILER_STACK_H
