#include "header.h"
#include "Player.h"
#include "enemy.h"
#include <random>

std::random_device rd;
std::mt19937 gen(rd());
std::uniform_int_distribution<> dist(-200, 200);

// 오류검사
int ret;
void err_display(const char* msg)
{
	LPVOID lpMsgBuf;
	FormatMessage(
		FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
		NULL, WSAGetLastError(),
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
		(LPTSTR)&lpMsgBuf, 0, NULL);
	printf("[%s] %s", msg, (char*)lpMsgBuf);
	LocalFree(lpMsgBuf);
}
void err_quit(const char* msg)
{
	LPVOID lpMsgBuf;

	FormatMessage(
		FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
		NULL,
		WSAGetLastError(),
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
		(LPTSTR)&lpMsgBuf,
		0,
		NULL);

	MessageBox(NULL, (LPTSTR)lpMsgBuf, (LPTSTR)msg, MB_ICONERROR);
	LocalFree(lpMsgBuf);
	exit(1);
}

struct glmvec3 { 
	float x, y, z;
};

//전역 변수
HANDLE hThread;
HANDLE _hSendEvent;
HANDLE _hCalculateEvent;
// 스레드 카운트
short thread_count = 0;
// 게임 상태 (false 종료 ture 시작)
bool game_state = false;

// 함수 선언
DWORD WINAPI Receive_Client_Packet(LPVOID msg);	// 클라 Recv 쓰레드
DWORD WINAPI SendAll(LPVOID msg);				// Send 쓰레드
void send_login_packet(SOCKET* , short);			// 클라이언트가 접속하면 접속확인과 id를 보내는 함수
void send_add_packet(SOCKET* , short);
void send_remove_packet(SOCKET*, short);
void send_move_packet(SOCKET* c_socket, short c_id);
void send_start_packet(SOCKET*);
void send_bullet_packet(SOCKET* c_socket, short b_id);
void send_GenRandEnemy_packet(SOCKET* c_socket, int e_id);
void send_enemy_packet(SOCKET* c_socket, int e_id);
void GenRandEnemy(int clear_num);
void SetEnemies();
void gameStart();
void Disconnect(SOCKET*, short);

unordered_map<short, Player>clients;
Bullet bullets[MAX_BULLET_NUM];
Enemy enemy[MAX_ENEMY_NUM];

//--------
int main()
//--------
{
	short _id = 0;

	// s소켓 연결
	WSADATA WSAData;
	SOCKET s_socket;
	SOCKADDR_IN server_addr;
	INT addr_size;

	SOCKET c_socket;
	WSABUF rWsaBuf;
	WSABUF sWsaBuf;
	bool shutdown = false;

	ZeroMemory(&server_addr, sizeof(server_addr));
	server_addr.sin_family = AF_INET;
	server_addr.sin_port = htons(PORT_NUM);
	server_addr.sin_addr.s_addr = htonl(INADDR_ANY);

	if (WSAStartup(MAKEWORD(2, 2), &WSAData) != 0)
		return false;
	s_socket = socket(AF_INET, SOCK_STREAM, 0);
	if (s_socket == INVALID_SOCKET) err_quit("socket()");

	ret = bind(s_socket, reinterpret_cast<sockaddr*>(&server_addr), sizeof(server_addr));
	if (ret == SOCKET_ERROR)
	{
		err_display("bind()");
		return false;
	}

	ret = listen(s_socket, SOMAXCONN);
	if (ret == SOCKET_ERROR)
	{
		err_display("listen()");
		return false;
	}

	addr_size = sizeof(server_addr);

	// 이벤트 사용 준비
	_hSendEvent = CreateEvent(NULL, FALSE, TRUE, NULL);
	if (_hSendEvent == NULL) return 1;
	_hCalculateEvent = CreateEvent(NULL, FALSE, TRUE, NULL);
	if (_hCalculateEvent == NULL) return 1;

	// accept 
	// 3명 접속 확인
	while (thread_count < 2)
	{
		SOCKET c_socket = accept(s_socket, reinterpret_cast<sockaddr*>(&server_addr), &addr_size);
		if (c_socket == INVALID_SOCKET) {
			err_display("accept()");
		}
		else {
			_id = thread_count;
			cout << "연결성공 ID: " << _id << endl;
		}
		// 플레이어 생성
		Player* player = new Player(c_socket, _id);
		clients.try_emplace(thread_count, *player);
		clients[thread_count]._id = _id;

		if (_id == 0) {
			clients[thread_count].x = 3.0f;
			clients[thread_count].z = 3.0f;
		}
		else if (_id == 1) {
			clients[thread_count].x = -3.0f;
			clients[thread_count].z = 3.0f;
		}
		
		else if (_id == 2) {
			clients[thread_count].x = -3.0f;
			clients[thread_count].z = -3.0f;
		}

		send_login_packet(&clients[thread_count]._c_socket, clients[thread_count]._id);
		// 쓰레드 만들면 주석 해제
		hThread = CreateThread(NULL, 0, Receive_Client_Packet, (LPVOID)player, 0, NULL);
		if (hThread == NULL) { closesocket(clients[thread_count]._c_socket); }
		else { CloseHandle(hThread); }
		thread_count++;
	}

	// 주쓰레드 생성
	SetEnemies();
	
	// 초기화 및 게임 시작
	gameStart();
	
	// Send All 쓰레드 생성
	hThread = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)SendAll, 0, 0, NULL);
	if (hThread == NULL) { cout << "Send All 쓰레드 생성 에러" << endl; }

	// SetEvent
	// 메인 루프
	while (true) {
		DWORD retval = WaitForSingleObject(_hCalculateEvent, INFINITE);
		// 총알 위치 이동 및 충돌 체크
		for (int i = 0; i < MAX_BULLET_NUM; ++i) {
			if (bullets[i].is_active) {
				if (bullets[i].IsOut()) {
					bullets[i].is_active = false;
				}
				else {
					bullets[i].update();
				}
			}
		}
		SetEvent(_hSendEvent);
	}
	
	closesocket(c_socket);
	WSACleanup();
}

DWORD WINAPI Receive_Client_Packet(LPVOID player)
{
	Player* plclient = (Player*)player;
	
	char buf[BUF_SIZE];
	while (true) {
		// 데이터를 받는다
		ZeroMemory(buf, sizeof(buf));
		ret = recv(plclient->_c_socket, buf, sizeof(buf), 0);
		if (ret == SOCKET_ERROR) {
			err_display("recv()");
			return 0;
		}

		char* p = buf;

		while (p != NULL) {
			char type = *(p + 1);
			char size = *p;
			if (size <= 0) {
				break;
			}
			// 데이터를 분석한다
			switch (type)
			{
			case CS_LOGIN:
			{
				// 로그인 됐을 때 할일 처리
				break;
			}
			case CS_MOVE:
			{
				CS_MOVE_PACKET* packet = reinterpret_cast<CS_MOVE_PACKET*>(p);
				int pid = packet->id;

				float px = packet->x;
				float pz = packet->z;
				clients[pid].x = packet->x;
				clients[pid].z = packet->z;
				//cout << packet->id << "|" << packet->x << " | " << packet->z << endl;
				break;
			}
			case CS_MOUSECLICK:
			{
				// 마우스 클릭했을 때 할일 처리
				CS_MOUSECLICK_PACKET* packet = reinterpret_cast<CS_MOUSECLICK_PACKET*>(p);
				int c_id = packet->id;

				for (int i = 0; i < MAX_BULLET_NUM; ++i) {
					if (!bullets[i].is_active) {
						bullets[i].is_active = true;
						bullets[i].is_team = true;

						bullets[i].x = clients[c_id].x;
						bullets[i].y = clients[c_id].y;
						bullets[i].z = clients[c_id].z;

						bullets[i].dx = packet->dx;
						bullets[i].dy = packet->dy;
						bullets[i].dz = packet->dz; 
						break;
					}
				}

				break;
			}
			}
			p += size;
		}
	}
}

DWORD WINAPI SendAll(LPVOID msg)
{
	// 주쓰레드가 마치기까지 기다림
	while (true) {
		DWORD retval = WaitForSingleObject(_hSendEvent, INFINITE);
		for (int i = 0; i < thread_count; ++i) {
			// 클라이언트의 위치전송
			for (int j = 0; j < thread_count; ++j) {
				if(clients[i]._id != j)
					send_move_packet(&clients[i]._c_socket, j);
				//cout << "id = " << clients[i]._id << ", x = " << clients[i].x << ", z = " << clients[i].z << endl;
			}
			// 총알 위치 전송
			for (int k = 0; k < MAX_BULLET_NUM; ++k) {
				if (bullets[k].is_active) {
					send_bullet_packet(&clients[i]._c_socket, k);
				}
			}

			for (int q = 0; q < MAX_ENEMY_NUM; ++q) {
				//cout << q << " | " << enemy[q].x << " | " << enemy[q].z << endl;
				if (enemy[q].kind != 4) {
					send_enemy_packet(&clients[i]._c_socket, q);
				}
			}
		}
		SetEvent(_hCalculateEvent);
		Sleep(10);
	}
	return 0;
}

void send_login_packet(SOCKET* c_socket, short c_id)
{
	SC_LOGININFO_PACKET p;
	p.size = sizeof(p);
	p.type = SC_LOGININFO;
	p.id = c_id;
	send(*c_socket, reinterpret_cast<char*>(&p), sizeof(p), 0);
	cout << "login packet 보냄" << endl;
}

void send_add_packet(SOCKET* c_socket, short c_id)
{
	SC_ADD_PLAYER_PACKET p;
	p.size = sizeof(p);
	p.type = SC_ADD_PLAYER;
	p.id = c_id;
	send(*c_socket, reinterpret_cast<char*>(&p), sizeof(p), 0);
}

void send_move_packet(SOCKET* c_socket, short c_id)
{
	SC_MOVE_PLAYER_PACKET p;
	p.size = sizeof(p);
	p.type = SC_MOVE_PLAYER;
	p.id = c_id;
	p.x = clients[c_id].x;
	p.z = clients[c_id].z;

	send(*c_socket, reinterpret_cast<char*>(&p), sizeof(p), 0);
}

void send_bullet_packet(SOCKET* c_socket, short b_id)
{
	SC_BULLET_PACKET p;
	p.size = sizeof(p);
	p.type = SC_BULLET;
	p.bullet_id = b_id;
	p.x = bullets[b_id].x;
	p.y = bullets[b_id].y;
	p.z = bullets[b_id].z;

	send(*c_socket, reinterpret_cast<char*>(&p), sizeof(p), 0);
}

// 죽은 플레이어 삭제함수
void send_remove_packet(SOCKET* c_socket, short c_id)
{
	SC_REMOVE_PLAYER_PACKET p;
	p.size = sizeof(p);
	p.type = SC_REMOVE_PLAYER;
	p.id = c_id;
	send(*c_socket, reinterpret_cast<char*>(&p), sizeof(p), 0);
	cout << "login packet 보냄" << endl;
}

void send_enemy_packet(SOCKET* c_socket, int e_id)
{
	SC_ENEMY_PACKET p;
	p.size = sizeof(p);
	p.type = SC_ENEMY;
	p.id = e_id;
	p.x = enemy[e_id].x;
	p.z = enemy[e_id].z;

	send(*c_socket, reinterpret_cast<char*>(&p), sizeof(p), 0);
}

void send_GenRandEnemy_packet(SOCKET* c_socket, int e_id)
{
	SC_GEN_ENEMY_PACKET p;
	p.size = sizeof(p);
	p.type = SC_GEN_ENEMY;
	p.id = e_id;
	p.kind = enemy[e_id].kind;
	p.hp = enemy[e_id].hp;
	p.x = enemy[e_id].x;
	p.y = enemy[e_id].y;
	p.z = enemy[e_id].z;

	send(*c_socket, reinterpret_cast<char*>(&p), sizeof(p), 0);
}

// 게임 시작 함수
void send_start_packet(SOCKET* c_socket)
{
	SC_START_PACKET p;
	p.size = sizeof(p);
	p.type = SC_START;
	send(*c_socket, reinterpret_cast<char*>(&p), sizeof(p), 0);
	cout << "start 패킷 보냄" << endl;
}

void gameStart()
{
	cout << "GameStart!" << endl;
	// 초기화

	game_state = true;

	for (auto& pl : clients)
	{
		send_start_packet(&pl.second._c_socket);
	}
}

void SetEnemies()
{
	GenRandEnemy(0);

	for (int j = 0; j < thread_count; ++j) {
		for (int i = 0; i < MAX_ENEMY_NUM; ++i) {
			send_GenRandEnemy_packet(&clients[j]._c_socket, i);
		}
	}
}

void GenRandEnemy(int clear_num)
{
	for (int i = 0; i < MAX_ENEMY_NUM; i++)
	{
		enemy[i].is_active = true;
		enemy[i].shot == false;
		enemy[i].kind = abs(dist(gen) % 5);
		if (enemy[i].kind < 4)
		{
			enemy[i].hp = abs(dist(gen) % 2) + 2;
			enemy[i].speed = 0.035 / (float)enemy[i].hp;
			enemy[i].radius = 0.5 * (float)enemy[i].hp * 0.25f * 1.3f;
			enemy[i].x = (float)dist(gen) / 50.f;
			enemy[i].y = (float)enemy[i].hp * 0.25f * 0.5f - 0.5f;
			enemy[i].z = (float)dist(gen) / 50.f;
		}

		else if (enemy[i].kind == 4)
		{
			enemy[i].hp = 1;
			enemy[i].speed = 0.1;
			enemy[i].radius = 0.5 * (float)enemy[i].hp * 0.25f * 1.2;
			enemy[i].x = (float)dist(gen) / 50.f;
			enemy[i].y = abs((float)dist(gen) / 100.f) + 2.0f;
			enemy[i].z = (float)dist(gen) / 50.f;
		}
	}
}

// 연결 해제
void Disconnect(SOCKET* c_socket, short c_id)
{
	for (auto& pl : clients) {
		if (pl.second._id == c_id) continue;
		SC_REMOVE_PLAYER_PACKET p;
		p.id = c_id;
		p.size = sizeof(p);
		p.type = SC_REMOVE_PLAYER;
		send_remove_packet(c_socket, c_id);
	}
	closesocket(clients[c_id]._c_socket);
}

bool collide_sphere(glmvec3 a, glmvec3 b, float coll_dist)
{
	glmvec3 c;
	c.x = a.x - b.x;
	c.y = a.y - b.y;
	c.z = a.z - b.z;

	float dist = sqrt(c.x * c.x + c.y * c.y + c.z * c.z);

	if (coll_dist > dist)
	{
		return true;
	}

	return false;
}

bool collide_box(glmvec3 bb, glmvec3 tb, glmvec3 bb_scale, glmvec3 tb_scale)
{
	float size = 0.5;

	glmvec3 bb_min = { bb.x - size * bb_scale.x,bb.y - size * bb_scale.y,bb.z - size * bb_scale.z };
	glmvec3 bb_max = { bb.x + size * bb_scale.x,bb.y + size * bb_scale.y,bb.z + size * bb_scale.z };

	glmvec3 tb_min = { tb.x - size * tb_scale.x ,tb.y - size * tb_scale.y,tb.z - size * tb_scale.z };
	glmvec3 tb_max = { tb.x + size * tb_scale.x,tb.y + size * tb_scale.y,tb.z + size * tb_scale.z };

	if (bb_min.x <= tb_max.x &&
		bb_max.x >= tb_min.x &&
		bb_min.y <= tb_max.y &&
		bb_max.y >= tb_min.y &&
		bb_min.z <= tb_max.z &&
		bb_max.z >= tb_min.z)
	{
		return true;
	}

	return false;
}