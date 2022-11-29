#include "header.h"
#include "Player.h"

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

struct glmvec3 { 
	float x, y, z;
};

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
void gameStart();
void Disconnect(SOCKET*, short);

unordered_map<short, Player>clients;
Bullet* bullets[60];

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
		// ������ ����� �ּ� ����
		hThread = CreateThread(NULL, 0, Receive_Client_Packet, (LPVOID)player, 0, NULL);
		if (hThread == NULL) { closesocket(clients[thread_count]._c_socket); }
		else { CloseHandle(hThread); }
		thread_count++;
	}

	// �־����� ����
	
	
	// �ʱ�ȭ �� ���� ����
	gameStart();
	
	// Send All ������ ����
	hThread = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)SendAll, 0, 0, NULL);
	if (hThread == NULL) { cout << "Send All ������ ���� ����" << endl; }

	// SetEvent
	//SetEvent(_hCalculateEvent);

	// ���� ����
	while (true) {
		
	}
	
	closesocket(c_socket);
	WSACleanup();
}

DWORD WINAPI Receive_Client_Packet(LPVOID player)
{
	Player* plclient = (Player*)player;
	char buf[BUF_SIZE];
	while (true) {
		// �����͸� �޴´�
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
			case CS_LOGIN:
			{
				// �α��� ���� �� ���� ó��
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
				// ���콺 Ŭ������ �� ���� ó��
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
	//DWORD retval = WaitForSingleObject(_hSendEvent, INFINITE);
	while (true) {
		for (int i = 0; i < thread_count; ++i) {
			// Ŭ���̾�Ʈ�� ��ġ����
			for (int j = 0; j < thread_count; ++j) {
				if(clients[i]._id != j)
					send_move_packet(&clients[i]._c_socket, j);
				//cout << "id = " << clients[i]._id << ", x = " << clients[i].x << ", z = " << clients[i].z << endl;
			}
		}
		Sleep(10);
	}

	//SetEvent(_hCalculateEvent);
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
	p.x = bullets[b_id]->x;
	p.y = bullets[b_id]->y;
	p.z = bullets[b_id]->z;

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
	cout << "login packet ����" << endl;
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

// ���� ����
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