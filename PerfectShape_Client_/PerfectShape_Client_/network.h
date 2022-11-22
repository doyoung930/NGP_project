#pragma once
#include <ws2tcpip.h> 

void err_display(int );
void err_display(const char* );
void err_quit(const char* );
int NetInit();
void NetCleanup();
int send_move_packet(float x, float z);
int send_attack_packet();
void err_quit(const char* msg);
void err_display(const char* msg);
void err_display(int errcode);
short GetMyPlayerID();
float GetPlayerX(short id);
float GetPlayerZ(short id);
DWORD WINAPI do_recv();
