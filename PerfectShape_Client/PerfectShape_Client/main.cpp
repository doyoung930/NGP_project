#include "network.h"

int main() 
{
	NetInit();

	HANDLE hThread = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)do_recv, (LPVOID)NULL, 0, NULL);

	NetCleanup();
}