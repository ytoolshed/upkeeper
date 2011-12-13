#include <upkeeper.h>

int
main(int argc, char ** argv, char ** envp)
{
    if(argc > 1)
        upk_rm_rf(argv[1]);
    return 0;
}
