#pragma once
#include "header.h"
#include "Player.h"
#include "enemy.h"
#include "bullet.h"
#include "Map.h"
#include "physics.h"

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
void send_bulletHit_packet(SOCKET* c_socket, short b_id);
void send_enemyHit_packet(SOCKET* c_socket, short id);
void send_hit_packet(SOCKET* c_socket, short c_id);
void send_hitend_packet(SOCKET* c_socket, short c_id);
void send_pillar_packet(SOCKET* c_socket, short c_id);
void GenRandEnemy(int clear_num);
void SetEnemies();
void CalculateEnemyDirection(int id);
void InitEnemyBullet();

void gameStart();
void Disconnect(SOCKET*, short);

bool IsCollision_PE(Enemy en, Player pl);
void Player_KnockBack(short id);
void Player_Check_Unbeatable(short id, chrono::system_clock::time_point start_time);

bool Player_Check_Touch_Wall(short id);

void send_dead_packet(SOCKET* c_socket, short c_id);

unordered_map<short, Player>clients;
Bullet bullets[MAX_BULLET_NUM];
Enemy enemy[MAX_ENEMY_NUM];

Map map;

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
	while (thread_count < 3)
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

		//break;
	}

	for (int i = 0; i < thread_count; ++i) {
		for (int j = 0; j < thread_count; ++j) {
			if(i != j)
				send_add_packet(&clients[i]._c_socket, j);
		}
	}

	// 초기화 및 게임 시작
	SetEnemies();
	gameStart();
	
	// Send All 쓰레드 생성
	hThread = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)SendAll, 0, 0, NULL);
	if (hThread == NULL) { cout << "Send All 쓰레드 생성 에러" << endl; }

	// SetEvent
	// 메인 루프
	while (true) {
		DWORD retval = WaitForSingleObject(_hCalculateEvent, INFINITE);
		//클라 위치 업데이트
		for (auto& pl : clients) {
			pl.second.update();
			//벽에 닿았나 체크
			if (Player_Check_Touch_Wall(pl.second._id)) {
				// 닿았으면 이동 취소
				pl.second.undo();
			}
		}
		int cnt = 0;
		for (int i = 0; i < MAX_ENEMY_NUM; ++i) {
			if (enemy[i].is_active) {
				CalculateEnemyDirection(i);
				enemy[i].update();
			}
			else {
				cnt++;
			}
		}
		InitEnemyBullet();

		if (cnt == 20) {
			for (auto& pl : clients) {
				for (int i = 0; i < thread_count; ++i)
					send_pillar_packet(&pl.second._c_socket, i);
			}
		}

		// 총알 위치 이동 및 충돌 체크
		for (int i = 0; i < MAX_BULLET_NUM; ++i) {
			if (!bullets[i].is_active) continue;

			int result = bullets[i].ColisionCheckEnemy(enemy);
			if (bullets[i].isOut()) {
				bullets[i].is_active = false;
				if (!bullets[i].is_team) {
					enemy[bullets[i].enemyID].shot = false;
				}
				for (int j = 0; j < thread_count; ++j) {
					send_bulletHit_packet(&clients[j]._c_socket, i);
				}
			}

			else if (!bullets[i].is_team) { // 적 총알 - 아군 충돌체크
				for (auto& pl : clients) {
					if (collide_sphere({ bullets[i].x ,bullets[i].y , bullets[i].z  },
						{ pl.second.x ,pl.second.y ,pl.second.z }, 0.4f && !pl.second._is_unbeatable) && (pl.second.hp > 0)) {// 클라 - 적 총알 충돌처리
						pl.second._unbeatable_time = chrono::system_clock::now();
						pl.second.hp -= 1;
						pl.second._is_hit = true;
						pl.second._is_unbeatable = true;
						pl.second._hx = bullets[i].dx / sqrt((bullets[i].dx * bullets[i].dx) + (bullets[i].dz * bullets[i].dz));
						pl.second._hz = bullets[i].dz / sqrt((bullets[i].dx * bullets[i].dx) + (bullets[i].dz * bullets[i].dz));
						
						enemy[bullets[i].enemyID].shot = false;
						bullets[i].is_active = false;
						bullets[i].enemyID = -1;

						//cout << "플레이어[" << pl.second._id << "] - 적[" << i << "] 충돌" << endl;
						//cout << "x = " << pl.second._hx << ", z = " << pl.second._hz << endl;
						for (int i{}; i < thread_count; ++i) {
							send_hit_packet(&clients[i]._c_socket, pl.second._id);
							send_bulletHit_packet(&clients[i]._c_socket, i);
						}                     

						if (pl.second.hp <= 0)
						{
							for (int j = 0; j < thread_count; ++j) {
								send_dead_packet(&clients[j]._c_socket, pl.second._id);
							}
							// 임시 방편 -> 삭제 작업 해야함
							pl.second._in_use = false;
							continue;
						}

						break;
					}
				}
			}
			else if (result >= 0) { // 아군 총알 - 적 충돌체크
				bullets[i].is_active = false;
				for (int j = 0; j < thread_count; ++j) {
					send_bulletHit_packet(&clients[j]._c_socket, i);
					send_enemyHit_packet(&clients[j]._c_socket, result);
				}
			}
			bullets[i].update();
		}

		// 플레이어와 적들의 충돌체크
		for (int i{}; i < MAX_ENEMY_NUM; ++i) {

			for (auto& pl : clients) {
				if (enemy[i].is_active == false) continue;
				if (IsCollision_PE(enemy[i], pl.second) && !pl.second._is_unbeatable && (pl.second.hp > 0))
				{
					pl.second._unbeatable_time = chrono::system_clock::now();
					pl.second.hp -= 1;
					pl.second._is_hit = true;
					pl.second._is_unbeatable = true;
					pl.second._hx = enemy[i].dx;
					pl.second._hz = enemy[i].dz;
					//cout << "플레이어[" << pl.second._id << "] - 적[" << i << "] 충돌" << endl;
					//cout << "x = " << pl.second._hx << ", z = " << pl.second._hz << endl;
					
					for (int i{};i<thread_count;++i)
						send_hit_packet(&clients[i]._c_socket, pl.second._id);

					if (pl.second.hp <= 0)
					{
						// 임시 방편 -> 삭제 작업 해야함
						pl.second._in_use = false;
						continue;
					}
				}
			}
		}

		for (auto& pl : clients) {
			// 맞는 상태에 있는 클라이언트 찾아서 처리
			if (pl.second._is_hit) {
				Player_KnockBack(pl.second._id);
			}
			// 무적 상태에 있는 클라이언트들 처리
			if (pl.second._is_unbeatable) {
				Player_Check_Unbeatable(pl.second._id, pl.second._unbeatable_time);
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
			case CS_KEYBOARD:
			{
				// 클라 키보드 입력
				CS_KEYBOARD_PACKET* packet = reinterpret_cast<CS_KEYBOARD_PACKET*>(p);
				int pid = packet->id;
				int dir = packet->direction;
				
				switch (dir) {
				// key down
				case 1:
					clients[pid].FB_Dir += 1;
					break;
				case 2:
					clients[pid].LR_Dir -= 1;
					break;
				case 3:
					clients[pid].FB_Dir -= 1;
					break;
				case 4:
					clients[pid].LR_Dir += 1;
					break;
				//key up
				case -1:
					clients[pid].FB_Dir -= 1;
					break;
				case -2:
					clients[pid].LR_Dir += 1;
					break;
				case -3:
					clients[pid].FB_Dir += 1;
					break;
				case -4:
					clients[pid].LR_Dir -= 1;
					break;
				}

				//cout << packet->id << "|" << clients[pid].FB_Dir << " " << clients[pid].LR_Dir <<endl;
				break;
			}
			case CS_DIRECTION:
			{
				//클라 바라보는 방향
				CS_DIRECTION_PACKET* packet = reinterpret_cast<CS_DIRECTION_PACKET*>(p);
				int pid = packet->id;
				clients[pid].dx = packet->dx;
				clients[pid].dz = packet->dz;
				clients[pid].view_degree = packet->degree;
				//cout << packet->id << "|" << packet->dx << " | " << packet->dz << " | " 
				//	<< clients[pid].view_degree << endl;
				break;
			}
			case CS_MOVE:
			{
				//이제 없어도 됨
				CS_MOVE_PACKET* packet = reinterpret_cast<CS_MOVE_PACKET*>(p);
				int pid = packet->id;
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
						bullets[i].speed = 0.3f;
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

void send_bulletHit_packet(SOCKET* c_socket, short b_id)
{
	SC_BULLETHIT_PACKET p;
	p.size = sizeof(p);
	p.type = SC_BULLETHIT;
	p.bullet_id = b_id;

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
	//cout << "packet 보냄" << endl;
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

void send_enemyHit_packet(SOCKET* c_socket, short id)
{
	SC_ENEMYHIT_PACKET p;
	p.size = sizeof(p);
	p.type = SC_ENEMYHIT;
	p.id = id;

	send(*c_socket, reinterpret_cast<char*>(&p), sizeof(p), 0);
}

void send_hit_packet(SOCKET* c_socket, short c_id)
{
	SC_PLAYERHIT_PACKET p;
	p.size = sizeof(p);
	p.type = SC_PLAYERHIT;
	p.id = c_id;

	send(*c_socket, reinterpret_cast<char*>(&p), sizeof(p), 0);
}

void send_hitend_packet(SOCKET* c_socket, short c_id)
{
	SC_HITEND_PACKET p;
	p.size = sizeof(p);
	p.type = SC_HITEND;
	p.id = c_id;

	send(*c_socket, reinterpret_cast<char*>(&p), sizeof(p), 0);
}


// 모든 HP가 달은 플레이어 id 정보 SEND 함수
void send_dead_packet(SOCKET* c_socket, short c_id)
{
	SC_HITEND_PACKET p;
	p.size = sizeof(p);
	p.type = SC_DEAD;
	p.id = c_id;

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

void send_pillar_packet(SOCKET* c_socket,short pillar_id)
{
	SC_PILLAR_PACKET p;
	p.size = sizeof(p);
	p.type = SC_PILLAR;
	p.id = pillar_id;
	p.y = map.pillar_t[pillar_id][4].y;
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
	GenRandEnemy(map.clear_num);

	for (int j = 0; j < thread_count; ++j) {
		for (int i = 0; i < 5 + map.clear_num; ++i) {
			send_GenRandEnemy_packet(&clients[j]._c_socket, i);
		}
	}
}

void GenRandEnemy(int clear_num)
{
	for (int i = 0; i < 5 + clear_num; i++)
	{
		enemy[i].is_active = true;
		enemy[i].shot == false;
		//enemy[i].kind =4;
		enemy[i].kind = abs(dist(gen) % 5);
		if (enemy[i].kind < 4)
		{
			enemy[i].hp = abs(dist(gen) % 2) + 2;
			enemy[i].speed = 0.025f / (float)enemy[i].hp;
			enemy[i].radius = 0.5 * (float)enemy[i].hp * 0.25f * 1.3f;
			enemy[i].x = (float)dist(gen) / 50.f;
			enemy[i].y = (float)enemy[i].hp * 0.25f * 0.5f - 0.5f;
			enemy[i].z = (float)dist(gen) / 50.f;
		}

		else if (enemy[i].kind == 4)
		{
			enemy[i].hp = 1;
			enemy[i].speed = 0.1;
			enemy[i].radius = 0.5 * 0.4f * 1.2;
			enemy[i].x = (float)dist(gen) / 50.f;
			enemy[i].y = abs((float)dist(gen) / 100.f) + 2.0f;
			enemy[i].z = (float)dist(gen) / 50.f;
		}
	}
}

void CalculateEnemyDirection(int id) 
{
	float dist;
	float min = 1000.f;
	int c_id;
	for (int i = 0; i < thread_count; ++i) {
		if (clients[i].hp > 0) {
			if (enemy[id].kind == 4) {
				dist = (clients[i].x - enemy[id].x) * (clients[i].x - enemy[id].x) + (clients[i].y - enemy[id].y) * (clients[i].y - enemy[id].y)
					+ (clients[i].z - enemy[id].z) * (clients[i].z - enemy[id].z);
				if (min > dist) {
					min = dist;
					enemy[id].dx = (clients[i].x - enemy[id].x) / sqrt(min);
					enemy[id].dy = (clients[i].y - enemy[id].y) / sqrt(min);
					enemy[id].dz = (clients[i].z - enemy[id].z) / sqrt(min);
				}
			}
			else {
				dist = (clients[i].x - enemy[id].x) * (clients[i].x - enemy[id].x) +
					(clients[i].z - enemy[id].z) * (clients[i].z - enemy[id].z);
				if (min > dist) {
					min = dist;
					enemy[id].dx = (clients[i].x - enemy[id].x) / sqrt(min);
					enemy[id].dz = (clients[i].z - enemy[id].z) / sqrt(min);
				}
			}
		}
	}
}
void InitEnemyBullet() {
	for (int i = 0; i < MAX_ENEMY_NUM; ++i) {
		if (!enemy[i].is_active || enemy[i].shot) continue;

		for (int j = 0; j < MAX_BULLET_NUM; ++j) {
			if (!bullets[j].is_active && enemy[i].kind == 4) {
				bullets[j].is_active = true;
				bullets[j].is_team = false;
				enemy[i].shot = true;

				bullets[j].x = enemy[i].x;
				bullets[j].y = enemy[i].y;
				bullets[j].z = enemy[i].z;

				bullets[j].dx = enemy[i].dx;
				bullets[j].dy = enemy[i].dy;
				bullets[j].dz = enemy[i].dz;
				bullets[j].enemyID = i;
				bullets[j].speed = 0.05f;
				break;
			}
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

bool IsCollision_PE(Enemy en, Player pl)
{
	glmvec3 en_p = { en.x, en.y, en.z };
	glmvec3 pl_p = { pl.x, pl.y, pl.z };
	glmvec3 en_s = { en.radius, en.radius, en.radius };

	if (collide_box(pl_p, en_p, { 0.8f, 0.8f, 0.8f }, en_s))
	{
		return true;
	}

	return false;
}

void Player_KnockBack(short id)
{
	clients[id].x = clients[id].x + 0.1f * clients[id]._hx;
	clients[id].y = clients[id].y + 0.1f * clients[id]._hy;
	clients[id].z = clients[id].z + 0.1f * clients[id]._hz;

	glmvec3 plp{ clients[id].x, clients[id].y, clients[id].z };

	// 벽이랑 충돌
	for (int i{}; i < 4; ++i)
	{
		for (int j{}; j < 10; ++j)
		{
			if (collide_box(plp, map.pillar_t[i][j], { 1.0f, 1.0f, 1.0f }, map.pillar_s))
			{
				clients[id].x -= clients[id]._hit_speed * clients[id]._hx;
				clients[id].y -= clients[id]._hit_speed * clients[id]._hy;
				clients[id].z -= clients[id]._hit_speed * clients[id]._hz;
				
				glmvec3 hitvec{ clients[id]._hx, clients[id]._hy, clients[id]._hz };
				clients[id]._hx = CC_CalculateRVector(hitvec, map.pillar_normal[i]).x;
				clients[id]._hy = CC_CalculateRVector(hitvec, map.pillar_normal[i]).y;
				clients[id]._hz = CC_CalculateRVector(hitvec, map.pillar_normal[i]).z;

				break;
			}
		}
	}

	clients[id]._hit_speed -= clients[id]._hit_speed * 0.1f;

	clients[id]._hit_cnt += 1;

	//cout << "id = " << id << ", x = " << clients[id].x << ", z = " << clients[id].z << endl;

	if (clients[id]._hit_cnt >= 30)
	{
		clients[id]._is_hit = false;
		clients[id]._hit_cnt = 0;
		clients[id]._hit_speed = 0.2f;
		send_hitend_packet(&clients[id]._c_socket, clients[id]._id);
	}
}

void Player_Check_Unbeatable(short id, chrono::system_clock::time_point start_time)
{
	auto now = chrono::system_clock::now();

	if (now - clients[id]._unbeatable_time >= 5s)
	{
		clients[id]._is_unbeatable = false;
	}
}

bool Player_Check_Touch_Wall(short id)
{
	glmvec3 plp{ clients[id].x, clients[id].y, clients[id].z };

	for (int i{}; i < 4; ++i)
	{
		for (int j{}; j < 10; ++j)
		{
			if (collide_box(plp, map.pillar_t[i][j], { 1.0f, 1.0f, 1.0f }, map.pillar_s))
			{
				return true;
			}
		}
	}
	return false;
}