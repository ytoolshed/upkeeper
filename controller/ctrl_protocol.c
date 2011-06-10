#include "controller.h"
#include <stdio.h>

upk_clientstatus_t
handle_client(int32_t client_fd)
{
	printf("We have a client named: %d\n", client_fd);
	return true;
}
