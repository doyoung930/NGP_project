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


//���� ����
HANDLE hThread;
HANDLE _hSendEvent;
HANDLE _hCalculateEvent;
// ������ ī��Ʈ
int thread_count = 0;


// �Լ� ����
DWORD WINAPI ProcessClient(LPVOID arg);		// Ŭ�� ������
DWORD WINAPI SendAll(LPVOID msg);			// Send ������
void send_login_packet(SOCKET* , int);	// Ŭ���̾�Ʈ�� �����ϸ� ����Ȯ�ΰ� id�� ������ �Լ�
void send_add_packet(SOCKET* , int);

unordered_map<int, Player>clients;

//--------
int main()
//--------
{

	int _id;

	// s���� ����
	WSADATA _WsaData;
	SOCKET _s_socket;
	SOCKADDR_IN _server_addr;
	INT _addr_size;

	SOCKET _c_socket;
	WSABUF _rWsaBuf;
	WSABUF _sWsaBuf;
	bool _shutdown = false;




	ZeroMemory(&_server_addr, sizeof(_server_addr));
	_server_addr.sin_family = AF_INET;
	_server_addr.sin_port = htons(PORT_NUM);
	_server_addr.sin_addr.s_addr = htonl(INADDR_ANY);

	if (WSAStartup(MAKEWORD(2, 2), &_WsaData) != 0)
		return false;
	_s_socket = WSASocket(AF_INET, SOCK_STREAM, IPPROTO_TCP, 0, 0, 0);
	ret = bind(_s_socket, reinterpret_cast<sockaddr*>(&_server_addr), sizeof(_server_addr));
	if (ret == SOCKET_ERROR)
	{
		err_display("bind()");
		return false;
	}

	ret = listen(_s_socket, SOMAXCONN);
	if (ret == SOCKET_ERROR)
	{
		err_display("listen()");
		return false;
	}

	_addr_size = sizeof(_server_addr);

	// �̺�Ʈ ��� �غ�
	_hSendEvent = CreateEvent(NULL, FALSE, TRUE, NULL);
	if (_hSendEvent == NULL) return 1;
	_hCalculateEvent = CreateEvent(NULL, FALSE, TRUE, NULL);
	if (_hCalculateEvent == NULL) return 1;

	// accept 
	// 3�� ���� Ȯ��
	while (thread_count < 3)
	{
		SOCKET c_socket = accept(_s_socket, reinterpret_cast<sockaddr*>(&_server_addr), &_addr_size);
		if (c_socket == INVALID_SOCKET) {
			err_display("accept()");
		}
		else {
			_id = thread_count + 1;
			cout << "���Ἲ�� ID: " << _id << endl;
		}
		// �÷��̾� ����
		Player* player = new Player(c_socket, _id);

		// ������ ����� �ּ� ����
		//hThread = CreateThread(NULL, 0,  ProcessClient, (LPVOID)player, 0, NULL);
		if (hThread == NULL) { closesocket(player->_c_socket); }
		else { CloseHandle(hThread); }
		thread_count++;
	}

	// �־����� ����

	// Send All ������ ����
	hThread = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)SendAll, 0, 0, NULL);

	WSACleanup();
}


DWORD WINAPI  ProcessClient(LPVOID arg)
{

	return 0;
}

DWORD WINAPI  SendAll(LPVOID msg)
{


	// �־����尡 ��ġ����� ��ٸ�
	DWORD retval = WaitForSingleObject(_hSendEvent, INFINITE);

	// ��� Ŭ���̾�Ʈ���� ��ġ ������

	// ��� ���� ��ġ ������

	// ���� ǥ�õǴ� �Ѿ˵��� ��ġ ������

	// �� ������ �˸�
	SetEvent(_hCalculateEvent);

	return 0;
}

void  send_login_packet(SOCKET* c_socket, int c_id)
{
	SC_LOGININFO_PACKET p;
	p.size = sizeof(p);
	p.type = SC_LOGININFO;
	p.id = c_id;
	send(*c_socket, reinterpret_cast<char*>(&p), sizeof(p), 0);
}

void send_add_packet(SOCKET* c_socket, int c_id)
{
	SC_ADD_PLAYER_PACKET p;
	p.size = sizeof(p);
	p.type = SC_ADD_PLAYER;
	p.id = c_id;
	send(*c_socket, reinterpret_cast<char*>(&p), sizeof(p), 0);
}