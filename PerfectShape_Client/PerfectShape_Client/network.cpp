#include "header.h"
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

       /* switch (type) {
        case SC_LOGININFO:
            SC_LOGININFO_PACKET* packet = (SC_LOGININFO_PACKET*)ptr;
            printf("%c", packet->type);
            break;
        case SC_ADD_PLAYER:
            SC_ADD_PLAYER_PACKET* packet = (SC_ADD_PLAYER_PACKET*)ptr;
            printf("%c", packet->type);
            break;
        case SC_START:
            SC_START_PACKET* packet = (SC_START_PACKET*)ptr;
            printf("%c", packet->type);
            break;
        case SC_REMOVE_PLAYER:
            SC_REMOVE_PLAYER_PACKET* packet = (SC_REMOVE_PLAYER_PACKET*)ptr;
            break;
        case SC_MOVE_PLAYER:
            SC_MOVE_PLAYER_PACKET* packet = (SC_MOVE_PLAYER_PACKET*)ptr;
            break;
        case SC_ENEMY:
            SC_ENEMY_PACKET* packet = (SC_ENEMY_PACKET*)ptr;
            break;
        case SC_ENERMYHIT:
            SC_ENERMYHIT_PACKET* packet = (SC_ENERMYHIT_PACKET*)ptr;
            break;
        case SC_BULLET:
            SC_BULLET_PACKET* packet = (SC_BULLET_PACKET*)ptr;
            break;
        case SC_BULLETHIT:
            SC_BULLETHIT_PACKET* packet = (SC_BULLETHIT_PACKET*)ptr;
            break;
        case SC_STAGE:
            SC_STAGE_PACKET* packet = (SC_STAGE_PACKET*)ptr;
            break;
        case SC_PLAYERHIT:
            SC_PLAYERHIT_PACKET* packet = (SC_PLAYERHIT_PACKET*)ptr;
            break;
        };*/
    }
}