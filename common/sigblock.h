#ifndef SIGBLOCK_H
#define SIGBLOCK_H


void upk_block_signal(int signal); 
void upk_unblock_signal(int signal);
void upk_catch_signal(int signal, void (*f)(int));
void upk_uncatch_signal(int signal);
#endif
