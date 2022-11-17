#pragma once
#include "header.h"

void err_display(int );
void err_display(const char* );
void err_quit(const char* );
int NetInit();
void NetCleanup();
int send_move_packet(int);
DWORD WINAPI do_recv();