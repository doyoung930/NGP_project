#pragma once
#define _CRT_SECURE_NO_WARNINGS 
#define _WINSOCK_DEPRECATED_NO_WARNINGS 

#include "network.h"
#include "../../PerfectShape_Server/protocol.h"
#include "bullet.h"
#include <iostream>
#include <tchar.h>
#include <stdio.h>
#include <stdlib.h> 
#include <string.h> 
#include <ws2tcpip.h> 
#include <winsock2.h> 
#pragma comment(lib, "ws2_32") 

WSADATA wsa;
SOCKET sock;
SOCKADDR_IN serveraddr;
char recvBuf[BUF_SIZE];


struct PlayerInfo {
    int id;
    int hp;
    float x, y, z;
    float dx, dy, dz;

    bool is_active;
    bool is_hit;

    PlayerInfo() : hp{ 3 }, id{ 0 }, x{ 0.f }, y{ 0.f }, z{ 0.f } {
        dx = 0;
        dy = 0;
        dz = 0;
        is_active = true;
    };
};

struct Enemy {
    int hp;
    int kind;
    float x, y, z;
    float s;
    bool pop[3];
    float px, py, pz; // 파티클 원점 위치
    bool is_active;
};


short MyID;
bool gameStart = false;
bool EnemySet = false;
int stage_state = 0;
int login_client_num = 1;
float pillar_y = 2.3f;
float pillarID = -1;

PlayerInfo player[3];


Bullet bullets[MAX_BULLET_NUM];

Enemy enemy[MAX_ENEMY_NUM];

std::string SERVERIP;

int NetInit() 
{
    int retval;
    
    std::cout << "IP 주소를 입력하세요 : ";
    std::cin >> SERVERIP;

    if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0)
        return 1;

    sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock == INVALID_SOCKET) err_quit("socket()");

    memset(&serveraddr, 0, sizeof(serveraddr));
    serveraddr.sin_family = AF_INET;
    serveraddr.sin_port = htons(PORT_NUM);
    inet_pton(AF_INET, SERVERIP.data(), &serveraddr.sin_addr);

    retval = connect(sock, (struct sockaddr*)&serveraddr, sizeof(serveraddr));

    return retval;
}

void NetCleanup()
{
    std::cout << " 소켓  close " << std::endl;
    closesocket(sock);

    WSACleanup();
}

int send_keyboard_packet(int direction)
{
    int retval = 0;

    CS_KEYBOARD_PACKET p{};
    p.size = sizeof(CS_KEYBOARD_PACKET);
    p.id = MyID;
    p.type = CS_KEYBOARD;
    p.direction = direction;
    send(sock, reinterpret_cast<const char*>(&p), sizeof(p), 0);

    return retval;
}

int send_direction_packet(float dx, float dz,float degree)
{
    int retval = 0;

    CS_DIRECTION_PACKET p{};
    p.size = sizeof(CS_DIRECTION_PACKET);
    p.id = MyID;
    p.type = CS_DIRECTION;
    p.dx = dx;
    p.dz = dz;
    p.degree = degree;
    send(sock, reinterpret_cast<const char*>(&p), sizeof(p), 0);

    return retval;
}

int send_attack_packet(float dx, float dy, float dz)
{
    int retval = 0;

    CS_MOUSECLICK_PACKET p;

    p.size = sizeof(CS_MOUSECLICK_PACKET);
    p.type = CS_MOUSECLICK;
    p.id = MyID;
    p.dx = dx;
    p.dy = dy;
    p.dz = dz;

    retval = send(sock, reinterpret_cast<const char*>(&p), sizeof(CS_MOUSECLICK_PACKET), 0);

    return retval;
}

int send_dead_packet()
{
    int retval = 0;

    CS_DEAD_PACKET p;

    p.size = sizeof(CS_DEAD_PACKET);
    p.type = CS_PLAYER_DEAD;
    p.state = false;

    return retval;
}


DWORD WINAPI do_recv()
{
    int retval;

    while (true) {
        ZeroMemory(recvBuf, BUF_SIZE);
        retval = recv(sock, recvBuf, BUF_SIZE, 0);
        if (retval == SOCKET_ERROR) err_display("RECV()");
        char* ptr = recvBuf;

        while (ptr != NULL) {
            unsigned char size = *ptr;
            if (size <= 0) {
                //printf("잘못된 데이터!\n");
                break;
            }
            char type = *(ptr + 1);

            switch (type) {
            case SC_LOGININFO: {
                SC_LOGININFO_PACKET* packet = reinterpret_cast<SC_LOGININFO_PACKET*>(ptr);
                
                MyID = packet->id;
                break;
            }
            case SC_ADD_PLAYER: {
                SC_ADD_PLAYER_PACKET* packet = reinterpret_cast<SC_ADD_PLAYER_PACKET*>(ptr);
                login_client_num++;
                break;
            }
            case SC_START: {
                SC_START_PACKET* packet = reinterpret_cast<SC_START_PACKET*>(ptr);
                gameStart = true;
                break;
            }
            case SC_REMOVE_PLAYER: {
                SC_REMOVE_PLAYER_PACKET* packet = reinterpret_cast<SC_REMOVE_PLAYER_PACKET*>(ptr);
                break;
            }
            case SC_MOVE_PLAYER: {
                SC_MOVE_PLAYER_PACKET* packet = reinterpret_cast<SC_MOVE_PLAYER_PACKET*>(ptr);
                short id = packet->id;
                player[id].x = packet->x;
                player[id].z = packet->z;
                break;
            }
            case SC_GEN_ENEMY: {
                SC_GEN_ENEMY_PACKET* packet = reinterpret_cast<SC_GEN_ENEMY_PACKET*>(ptr);
                short id = packet->id;
                enemy[id].hp = packet->hp;
                enemy[id].kind = packet->kind;
                enemy[id].x = packet->x;
                enemy[id].y = packet->y;
                enemy[id].z = packet->z;
                enemy[id].is_active = true;

                if (enemy[id].kind > 4) enemy[id].s = 0.4f;
                else enemy[id].s = (float)enemy[id].hp * 0.25f;

                break;
            }
            case SC_ENEMY: {
                SC_ENEMY_PACKET* packet = reinterpret_cast<SC_ENEMY_PACKET*>(ptr);
                short id = packet->id;
                enemy[id].x = packet->x;
                enemy[id].z = packet->z;
                break;
            }
            case SC_ENEMYHIT: {
                SC_ENEMYHIT_PACKET* packet = reinterpret_cast<SC_ENEMYHIT_PACKET*>(ptr);
                short id = packet->id;
                enemy[id].hp -= 1;
                enemy[id].pop[enemy[id].hp] = true;
                if (enemy[id].hp == 0) {
                    enemy[id].is_active = false;
                    break;
                }
                enemy[id].s = (float)enemy[id].hp * 0.25f;
                enemy[id].y = enemy[id].s * 0.5f - 0.5f;
                break;
            }
            case SC_BULLET: {
                SC_BULLET_PACKET* packet = reinterpret_cast<SC_BULLET_PACKET*>(ptr);
                int b_id = packet->bullet_id;
                bullets[b_id].is_active = true;
                bullets[b_id].x = packet->x;
                bullets[b_id].y = packet->y;
                bullets[b_id].z = packet->z;
                break;
            }
            case SC_BULLETHIT: {
                SC_BULLETHIT_PACKET* packet = reinterpret_cast<SC_BULLETHIT_PACKET*>(ptr);
                int b_id = packet->bullet_id;
                bullets[b_id].is_active = false;
                break;
            }
            case SC_STAGE: {
                SC_STAGE_PACKET* packet = reinterpret_cast<SC_STAGE_PACKET*>(ptr);
                short state = packet->state;
                stage_state = state;
                if (stage_state == -1) {
                    for (int i = 0; i < MAX_ENEMY_NUM; ++i) {
                        for (int j = 0; j < 3; ++j) {
                            enemy[i].pop[j] = false;
                        }
                    }
                }

                for (int i = 0; i < 3; i++){
                    player[i].hp = 3;
                    player[i].is_active = true;
                }

                break;
            }
            case SC_PLAYERHIT: {
                SC_PLAYERHIT_PACKET* packet = reinterpret_cast<SC_PLAYERHIT_PACKET*>(ptr);
                short id = packet->id;
                player[id].hp -= 1;
                if (id == MyID)
                {
                    player[id].is_hit = true;
                }
                break;
            }
            case SC_HITEND: {
                SC_HITEND_PACKET* packet = reinterpret_cast<SC_HITEND_PACKET*>(ptr);
                short id = packet->id;
                player[id].is_hit = false;
                break;
            }
            case SC_DEAD: {
                SC_PLAYER_DEAD_PACKET* packet = reinterpret_cast<SC_PLAYER_DEAD_PACKET*>(ptr);
                short id = packet->id;
                player[id].is_active = false;
                break;
            }
            case SC_PILLAR: {
                SC_PILLAR_PACKET* packet = reinterpret_cast<SC_PILLAR_PACKET*>(ptr);
                pillarID = packet->id;
                pillar_y = packet->y;
                break;
            }

            }
            ptr += size;
        }
    }
}

int GetClientNum() {
    return login_client_num;
}

short GetMyPlayerID() {
    return MyID;
}

bool GetGameState() {
    return gameStart;
}
int GetStageState() {
    return stage_state;
}
float GetPlayerX(short id) {
    return player[int(id)].x;
}

float GetPlayerZ(short id) {
    return player[int(id)].z;
}

int GetPlayerHP(short id) {
    return player[int(id)].hp;
}

bool GetBulletState(int id) {
    return bullets[id].is_active;
}

float GetBulletX(int id) {
    return bullets[id].x;
}

float GetBulletY(int id) {
    return bullets[id].y;
}


float GetBulletZ(int id) {
    return bullets[id].z;
}

bool GetEnemySet() {
    return EnemySet;
}

int GetEnemyKind(int id) {
    return enemy[id].kind;
}

int GetEnemyHP(int id) {
    return enemy[id].hp;
}

float GetEnemyX(int id) {
    return enemy[id].x;
}

float GetEnemyY(int id) {
    return enemy[id].y;
}

float GetEnemyZ(int id) {
    return enemy[id].z;
}

float GetEnemyS(int id) {
    return enemy[id].s;
}

float GetPillarY() {
    return pillar_y;
}

int GetPillarID() {
    return pillarID;
}

bool GetEnemyState(int id) {
    return enemy[id].is_active;
}

bool GetEnemyPopState(int e_id, int pop_id) {
    return enemy[e_id].pop[pop_id];
}

bool GetisHit(int p_id)
{
    return player[p_id].is_hit;
}

// 플레이어 사망-d
bool GetPlayerState(int id) {
    return player[id].is_active;
}

// 플레이어 Hp
short GetPlayerHp(int id) {
    return player[id].hp;
}



void err_quit(const char* msg)
{
    LPVOID lpMsgBuf;
    FormatMessageA(
        FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
        NULL, WSAGetLastError(),
        MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
        (char*)&lpMsgBuf, 0, NULL);
    MessageBoxA(NULL, (const char*)lpMsgBuf, msg, MB_ICONERROR);
    LocalFree(lpMsgBuf);
    exit(1);
}

// 소켓 함수 오류 출력
void err_display(const char* msg)
{
    LPVOID lpMsgBuf;
    FormatMessageA(
        FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
        NULL, WSAGetLastError(),
        MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
        (char*)&lpMsgBuf, 0, NULL);
    printf("[%s] %s\n", msg, (char*)lpMsgBuf);
    LocalFree(lpMsgBuf);
}

// 소켓 함수 오류 출력
void err_display(int errcode)
{
    LPVOID lpMsgBuf;
    FormatMessageA(
        FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
        NULL, errcode,
        MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
        (char*)&lpMsgBuf, 0, NULL);
    printf("[오류] %s\n", (char*)lpMsgBuf);
    LocalFree(lpMsgBuf);
}