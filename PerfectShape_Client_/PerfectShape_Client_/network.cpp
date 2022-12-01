#pragma once
#define _CRT_SECURE_NO_WARNINGS 
#define _WINSOCK_DEPRECATED_NO_WARNINGS 

#include "network.h"
#include "protocol.h"
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

short MyID;
bool gameStart = false;
PlayerInfo player[3];
Bullet bullets[MAX_BULLET_NUM];

const char* SERVERIP = "127.0.0.1"; // 임시

int NetInit() 
{
    int retval;

    if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0)
        return 1;

    sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock == INVALID_SOCKET) err_quit("socket()");

    memset(&serveraddr, 0, sizeof(serveraddr));
    serveraddr.sin_family = AF_INET;
    serveraddr.sin_port = htons(PORT_NUM);
    inet_pton(AF_INET, SERVERIP, &serveraddr.sin_addr);

    retval = connect(sock, (struct sockaddr*)&serveraddr, sizeof(serveraddr));

    return retval;
}

void NetCleanup()
{
    closesocket(sock);

    WSACleanup();
}

int send_move_packet(float x, float z)
{
    int retval = 0;

    CS_MOVE_PACKET p{};
    p.size = sizeof(CS_MOVE_PACKET);
    p.id = MyID;
    p.type = CS_MOVE;
    p.x = x;
    p.z = z;
    //std::cout << x << " " << z << " " << std::endl;
    send(sock, reinterpret_cast<const char*>(&p), sizeof(CS_MOVE_PACKET), 0);
    
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
                //printf("%d ", packet->type);
                break;
            }
            case SC_ADD_PLAYER: {
                SC_ADD_PLAYER_PACKET* packet = reinterpret_cast<SC_ADD_PLAYER_PACKET*>(ptr);
                break;
            }
            case SC_START: {
                SC_START_PACKET* packet = reinterpret_cast<SC_START_PACKET*>(ptr);
                gameStart = true;
                //printf("%d ", packet->type);
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
                //std::cout << player[id].id << " | " << player[id].x << " | " << player[id].z << std::endl;
                break;
            }
            case SC_ENEMY: {
                SC_ENEMY_PACKET* packet = reinterpret_cast<SC_ENEMY_PACKET*>(ptr);
                break;
            }
            case SC_ENERMYHIT: {
                SC_ENERMYHIT_PACKET* packet = reinterpret_cast<SC_ENERMYHIT_PACKET*>(ptr);
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
                break;
            }
            case SC_STAGE: {
                SC_STAGE_PACKET* packet = reinterpret_cast<SC_STAGE_PACKET*>(ptr);
                break;
            }
            case SC_PLAYERHIT: {
                SC_PLAYERHIT_PACKET* packet = reinterpret_cast<SC_PLAYERHIT_PACKET*>(ptr);
                break;
            }
            }
            ptr += size;
        }
    }
}

short GetMyPlayerID() {
    return MyID;
}

bool GetGameState() {
    return gameStart;
}

float GetPlayerX(short id) {
    return player[int(id)].x;
}

float GetPlayerZ(short id) {
    return player[int(id)].z;
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