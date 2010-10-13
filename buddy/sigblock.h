void upk_block_signal(int signal); { 
  sigset_t set;
  sigemptyset(&set);
  sigaddset(&set,signal);
  sigprocmask(SIG_BLOCK, set,NULL);
}

void upk_unblock_signal(int signal) { 
  sigset_t set;
  sigemptyset(&set);
  sigaddset(&set,signal);
  sigprocmask(SIG_UNBLOCK, set, NULL);
}

void upk_catch_signal(int signal, void (*f)(int)) { 
  struct sigaction sa = { NULL };
  sigemptyset(sa.sa_mask);

  sa.sa_handler = f;
  sigaction(signal,&sa,NULL);
}

