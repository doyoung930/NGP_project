#pragma once
#include "header.h"

int NetInit();
void NetCleanup();
DWORD WINAPI do_recv();