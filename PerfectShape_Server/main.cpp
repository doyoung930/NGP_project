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

// �����˻�
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

//���� ����
HANDLE hThread;
HANDLE _hSendEvent;
HANDLE _hCalculateEvent;
// ������ ī��Ʈ
short thread_count = 0;
// ���� ���� (false ���� ture ����)
bool game_state = false;

// �Լ� ����
DWORD WINAPI Receive_Client_Packet(LPVOID msg);	// Ŭ�� Recv ������
DWORD WINAPI SendAll(LPVOID msg);				// Send ������
void send_login_packet(SOCKET* , short);			// Ŭ���̾�Ʈ�� �����ϸ� ����Ȯ�ΰ� id�� ������ �Լ�
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
void send_stage_packet(SOCKET* c_socket, short state);
void GenRandEnemy(int clear_num);
void SetEnemies();
void CalculateEnemyDirection(int id);
void InitEnemyBullet();

void gameStart();
void Disconnect();

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

	// s���� ����
	WSADATA WSAData;
	SOCKET s_socket;
	SOCKADDR_IN server_addr;
	INT addr_size;

	SOCKET c_socket;

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

	// �̺�Ʈ ��� �غ�
	_hSendEvent = CreateEvent(NULL, FALSE, TRUE, NULL);
	if (_hSendEvent == NULL) return 1;
	_hCalculateEvent = CreateEvent(NULL, FALSE, TRUE, NULL);
	if (_hCalculateEvent == NULL) return 1;

	// accept 
	// 3�� ���� Ȯ��
	while (thread_count < 3)
	{
		SOCKET c_socket = accept(s_socket, reinterpret_cast<sockaddr*>(&server_addr), &addr_size);
		if (c_socket == INVALID_SOCKET) {
			err_display("accept()");
		}
		else {
			_id = thread_count;
			cout << "���Ἲ�� ID: " << _id << endl;
		}
		// �÷��̾� ����
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

		hThread = CreateThread(NULL, 0, Receive_Client_Packet, (LPVOID)player, 0, NULL);
		if (hThread == NULL) { closesocket(clients[thread_count]._c_socket); }
		else { CloseHandle(hThread); }
		thread_count++;
	}

	for (int i = 0; i < thread_count; ++i) {
		for (int j = 0; j < thread_count; ++j) {
			if(i != j)
				send_add_packet(&clients[i]._c_socket, j);
		}
	}

	// �ʱ�ȭ �� ���� ����
	SetEnemies();
	gameStart();
	for (auto& pl : clients)
		send_stage_packet(&pl.second._c_socket, 0);

	// Send All ������ ����
	hThread = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)SendAll, 0, 0, NULL);
	if (hThread == NULL) { cout << "Send All ������ ���� ����" << endl; }

	// ���� ����
	while (true) {
		// SetEvent
		DWORD retval = WaitForSingleObject(_hCalculateEvent, INFINITE);
		//Ŭ�� ��ġ ������Ʈ
		for (auto& pl : clients) {
			if (pl.second.room == -1) {
				pl.second.update();
			}
			//���� ��ҳ� üũ
			if (Player_Check_Touch_Wall(pl.second._id)) {
				// ������� �̵� ���
				pl.second.undo();
			}
		}
		int cnt = 0;
		for (int i = 0; i < MAX_ENEMY_NUM; ++i) {
			if (enemy[i].is_active) {
				CalculateEnemyDirection(i);
				enemy[i].update();
			}
			else if(map.wave == true){
				cnt++;
				if (cnt == 20) {
					map.wave = false;
					map.random_door = abs(dist(gen)) % 4;
					map.open[map.random_door] = 1;
					if (map.clear_num < 19) {
						map.clear_num++;
					}
					for (auto& pl : clients) {
						pl.second.hp = 3;
						send_stage_packet(&pl.second._c_socket, map.open[map.random_door]);
					}
				}
			}
		}
		InitEnemyBullet();

		if (map.wave == false) {
			int cnt = 0;
			for (auto& pl : clients) {
				if (pl.second.x > 6.0f)
				{
					pl.second.room = 1;
				}
				else if (pl.second.x < -6.0f)
				{
					pl.second.room = 0;
				}
				else if (pl.second.z > 6.0f)
				{
					pl.second.room = 3;
				}
				else if (pl.second.z < -6.0f)
				{
					pl.second.room = 2;
				}
				else if (-5.0f < pl.second.x && pl.second.x < 5.0f)
				{
					pl.second.room = -1;
				}

				if (pl.second.room != -1){
					cnt++;
				}
			}

			if (cnt == thread_count && map.open[map.random_door] != -1) {
				map.open[map.random_door] = -1;
				
				for (auto& pl : clients)
					send_stage_packet(&pl.second._c_socket, map.open[map.random_door]);
			}

			for (int j = 0; j < 2; j++)
			{	
				if (map.open[map.random_door] == 1) {
					if (map.pillar_t[map.random_door][j + 4].y < 5.3) {
						map.pillar_t[map.random_door][j + 4].y += 0.04f;
						//cout << "�ö󰡴� ��!" << endl;
					}
				}
				else if(map.open[map.random_door] == -1){
					if (map.pillar_t[map.random_door][j + 4].y > 2.3) {
						map.pillar_t[map.random_door][j + 4].y -= 0.04f;
					}
					else {
						map.open[map.random_door] = 0;
						SetEnemies();
						for (auto& pl : clients) {
							if (pl.second.room == 0) pl.second.x += 10.f;
							else if (pl.second.room == 1) pl.second.x -= 10.f;
							else if (pl.second.room == 2) pl.second.z += 10.f;
							else if (pl.second.room == 3) pl.second.z -= 10.f;
							pl.second.room = -1;
							send_stage_packet(&pl.second._c_socket, map.open[map.random_door]);
						}
						map.random_door = -1;
						map.wave = true;
						break;
					}
				}
			}

			for (auto& pl : clients) {
				send_pillar_packet(&pl.second._c_socket, map.random_door);
			}
		}

		// �Ѿ� ��ġ �̵� �� �浹 üũ
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

			else if (!bullets[i].is_team) { // �� �Ѿ� - �Ʊ� �浹üũ
				for (auto& pl : clients) {
					if (collide_sphere({ bullets[i].x ,bullets[i].y , bullets[i].z  },
						{ pl.second.x ,pl.second.y ,pl.second.z }, 0.4f && !pl.second._is_unbeatable) && (pl.second.hp > 0)) {// Ŭ�� - �� �Ѿ� �浹ó��
						pl.second._unbeatable_time = chrono::system_clock::now();
						pl.second.hp -= 1;
						pl.second._is_hit = true;
						pl.second._is_unbeatable = true;
						pl.second._hx = bullets[i].dx / sqrt((bullets[i].dx * bullets[i].dx) + (bullets[i].dz * bullets[i].dz));
						pl.second._hz = bullets[i].dz / sqrt((bullets[i].dx * bullets[i].dx) + (bullets[i].dz * bullets[i].dz));
						
						enemy[bullets[i].enemyID].shot = false;
						bullets[i].is_active = false;
						bullets[i].enemyID = -1;

						//cout << "�÷��̾�[" << pl.second._id << "] - ��[" << i << "] �浹" << endl;
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
							// �ӽ� ���� -> ���� �۾� �ؾ���
							pl.second._in_use = false;
							continue;
						}

						break;
					}
				}
			}
			else if (result >= 0) { // �Ʊ� �Ѿ� - �� �浹üũ
				bullets[i].is_active = false;
				for (int j = 0; j < thread_count; ++j) {
					send_bulletHit_packet(&clients[j]._c_socket, i);
					send_enemyHit_packet(&clients[j]._c_socket, result);
				}
			}
			bullets[i].update();
		}

		// �÷��̾�� ������ �浹üũ
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
					//cout << "�÷��̾�[" << pl.second._id << "] - ��[" << i << "] �浹" << endl;
					//cout << "x = " << pl.second._hx << ", z = " << pl.second._hz << endl;
					
					for (int i{};i<thread_count;++i)
						send_hit_packet(&clients[i]._c_socket, pl.second._id);

					if (pl.second.hp <= 0)
					{
						// �ӽ� ���� -> ���� �۾� �ؾ���
						pl.second._in_use = false;
						continue;
					}
				}
			}
		}

		for (auto& pl : clients) {
			// �´� ���¿� �ִ� Ŭ���̾�Ʈ ã�Ƽ� ó��
			if (pl.second._is_hit) {
				Player_KnockBack(pl.second._id);
			}
			// ���� ���¿� �ִ� Ŭ���̾�Ʈ�� ó��
			if (pl.second._is_unbeatable) {
				Player_Check_Unbeatable(pl.second._id, pl.second._unbeatable_time);
			}
		}
		SetEvent(_hSendEvent);
	}
	
	closesocket(c_socket);
	closesocket(s_socket);
	WSACleanup();
}

DWORD WINAPI Receive_Client_Packet(LPVOID player)
{
	Player* plclient = (Player*)player;
	
	char buf[BUF_SIZE];
	while (true) {
		// �����͸� �޴´�
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
			// �����͸� �м��Ѵ�
			switch (type)
			{
			case CS_KEYBOARD:
			{
				// Ŭ�� Ű���� �Է�
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
				//Ŭ�� �ٶ󺸴� ����
				CS_DIRECTION_PACKET* packet = reinterpret_cast<CS_DIRECTION_PACKET*>(p);
				int pid = packet->id;
				clients[pid].dx = packet->dx;
				clients[pid].dz = packet->dz;
				clients[pid].view_degree = packet->degree;
				//cout << packet->id << "|" << packet->dx << " | " << packet->dz << " | " 
				//	<< clients[pid].view_degree << endl;
				break;
			}
			case CS_MOUSECLICK:
			{
				// ���콺 Ŭ������ �� ���� ó��
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

			case CS_PLAYER_DEAD:
			{
				
				Disconnect();
				break;
			}

			}
			p += size;
		}
	}
}

DWORD WINAPI SendAll(LPVOID msg)
{
	// �־����尡 ��ġ����� ��ٸ�
	while (true) {
		DWORD retval = WaitForSingleObject(_hSendEvent, INFINITE);
		for (int i = 0; i < thread_count; ++i) {
			// Ŭ���̾�Ʈ�� ��ġ����
			for (int j = 0; j < thread_count; ++j) {
				send_move_packet(&clients[i]._c_socket, j);
				//cout << "id = " << clients[i]._id << ", x = " << clients[i].x << ", z = " << clients[i].z << endl;
			}
			// �Ѿ� ��ġ ����
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
	cout << "login packet ����" << endl;
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

// ���� �÷��̾� �����Լ�
void send_remove_packet(SOCKET* c_socket, short c_id)
{
	SC_REMOVE_PLAYER_PACKET p;
	p.size = sizeof(p);
	p.type = SC_REMOVE_PLAYER;
	p.id = c_id;
	send(*c_socket, reinterpret_cast<char*>(&p), sizeof(p), 0);
	//cout << "packet ����" << endl;
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


// ��� HP�� ���� �÷��̾� id ���� SEND �Լ�
void send_dead_packet(SOCKET* c_socket, short c_id)
{
	SC_PLAYER_DEAD_PACKET p;
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

// ���� ���� �Լ�
void send_start_packet(SOCKET* c_socket)
{
	SC_START_PACKET p;
	p.size = sizeof(p);
	p.type = SC_START;
	send(*c_socket, reinterpret_cast<char*>(&p), sizeof(p), 0);
	cout << "start ��Ŷ ����" << endl;
}

void send_stage_packet(SOCKET* c_socket, short state) {
	SC_STAGE_PACKET p;
	p.size = sizeof(p);
	p.type = SC_STAGE;
	p.state = state;
	send(*c_socket, reinterpret_cast<char*>(&p), sizeof(p), 0);
}

void send_pillar_packet(SOCKET* c_socket,short pillar_id)
{
	SC_PILLAR_PACKET p;
	p.size = sizeof(p);
	p.type = SC_PILLAR;
	p.id = pillar_id;
	p.y = map.pillar_t[pillar_id][4].y;
	send(*c_socket, reinterpret_cast<char*>(&p), sizeof(p), 0);
}


void gameStart()
{
	cout << "GameStart!" << endl;
	// �ʱ�ȭ

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
		for (int i = 0; i < 1 + map.clear_num; ++i) {
			send_GenRandEnemy_packet(&clients[j]._c_socket, i);
		}
	}
}

void GenRandEnemy(int clear_num)
{
	for (int i = 0; i < 1 + clear_num; i++)
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
// ���� ����
void Disconnect()
{
	for (int i = 0; i < MAX_USER; ++i) {
		cout << " ID : " << i << " Delete ! " << endl;
		closesocket(clients[i]._c_socket);
	}	
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

	// ���̶� �浹
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