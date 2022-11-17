#include "network.h"
#include "protocol.h"

struct PlayerInfo{
    float x, y, z;
    bool is_active;
    short id;
};

struct BulletInfo {
    bool active = false;
    float x, y, z;
    short id;
};

struct EnemyInfo {
    bool active = false;
    float x, y, z;
    short type;
    short hp;
    short id;
};

const char* SERVERIP = "127.0.0.1"; // 임시

WSADATA wsa;
SOCKET sock;
SOCKADDR_IN serveraddr;
char recvBuf[BUF_SIZE];
char sendBuf[BUF_SIZE];

PlayerInfo me;
PlayerInfo other[2];

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

int send_move_packet(int direction) 
{
    int retval = 0;

    CS_MOVE_PACKET* mp = new CS_MOVE_PACKET{ sizeof(CS_MOVE_PACKET), CS_MOVE , me.id };

    memcpy(sendBuf, reinterpret_cast<char*>(mp), sizeof(CS_MOVE_PACKET));

    send(sock, sendBuf, BUF_SIZE, 0);
    
    return retval;
}

DWORD WINAPI do_recv()
{
    int retval;

    while (true) {
        retval = recv(sock, recvBuf, BUF_SIZE, 0);
        if (retval == SOCKET_ERROR) err_display("RECV()");
        char* ptr = recvBuf;
        unsigned char size = *ptr;
        if (size <= 0) {
            printf("잘못된 데이터!");
            continue;
        }
        char type = *(ptr + 1);

        switch (type) {
        case SC_LOGININFO:{
            SC_LOGININFO_PACKET* packet = reinterpret_cast<SC_LOGININFO_PACKET*>(ptr);
            printf("%c", packet->type);
            break;
        }
        case SC_ADD_PLAYER: {
            SC_ADD_PLAYER_PACKET* packet = reinterpret_cast<SC_ADD_PLAYER_PACKET*>(ptr);
            printf("%c", packet->type);
            break;
        }
        case SC_START: {
            SC_START_PACKET* packet = reinterpret_cast<SC_START_PACKET*>(ptr);
            printf("%c", packet->type);
            break;
        }
        case SC_REMOVE_PLAYER: {
            SC_REMOVE_PLAYER_PACKET* packet = reinterpret_cast<SC_REMOVE_PLAYER_PACKET*>(ptr);
            break;
        }
        case SC_MOVE_PLAYER: {
            SC_MOVE_PLAYER_PACKET* packet = reinterpret_cast<SC_MOVE_PLAYER_PACKET*>(ptr);
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
        };
    }
}