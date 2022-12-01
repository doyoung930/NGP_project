#pragma once
#include <ws2tcpip.h> 

void err_display(int );
void err_display(const char* );
void err_quit(const char* );
int NetInit();
void NetCleanup();
int send_move_packet(float x, float z);
int send_attack_packet(float, float, float);
void err_quit(const char* msg);
void err_display(const char* msg);
void err_display(int errcode);
short GetMyPlayerID();
float GetPlayerX(short id);
float GetPlayerZ(short id);
bool GetBulletState(int);
float GetBulletX(int);
float GetBulletY(int);
float GetBulletZ(int);
bool GetGameState();
DWORD WINAPI do_recv();
