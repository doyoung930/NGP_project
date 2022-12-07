#pragma once
#include <ws2tcpip.h> 

void err_display(int );
void err_display(const char* );
void err_quit(const char* );
int NetInit();
void NetCleanup();
int send_move_packet(float x, float z);
int send_attack_packet(float, float, float);
int send_direction_packet(float, float,float);
int send_keyboard_packet(int direction);
void err_quit(const char* msg);
void err_display(const char* msg);
void err_display(int errcode);
int GetClientNum();
short GetMyPlayerID();
float GetPlayerX(short id);
float GetPlayerZ(short id);
int GetPlayerHP(short id);
bool GetBulletState(int);
float GetBulletX(int);
float GetBulletY(int);
float GetBulletZ(int);
bool GetGameState();
bool GetEnemySet();
int GetEnemyKind(int id);
int GetEnemyHP(int id);
float GetEnemyX(int id);
float GetEnemyY(int id);
float GetEnemyZ(int id);
float GetEnemyS(int id);
bool GetEnemyState(int id);
bool GetEnemyPopState(int e_id,int pop_id);
bool GetisHit(int p_id);
bool GetPlayerState(int id);
short GetPlayerHp(int id);
DWORD WINAPI do_recv();
