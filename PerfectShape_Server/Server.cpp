#include "Server.h"
#include"Player.h"

Server* Server::m_pInst = NULL;
Server::Server()
{
}

Server::~Server()
{
	WSACleanup();
}

// 서버 주소
void Server::InitServerAddr()
{
	ZeroMemory(&_server_addr, sizeof(_server_addr));
	_server_addr.sin_family = AF_INET;
	_server_addr.sin_port = htons(PORT_NUM);
	_server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
}

// 서버 Init
bool Server::Init()
{
	if (WSAStartup(MAKEWORD(2, 2), &_WsaData) != 0)
		return false;
	_s_socket = WSASocket(AF_INET, SOCK_STREAM, IPPROTO_TCP, 0, 0, 0);
	
	InitServerAddr();
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

	return true;
}

bool Server::Run()
{
	// 이벤트 사용 준비
	_hSendEvent = CreateEvent(NULL, FALSE, TRUE, NULL);		
	if (_hSendEvent == NULL) return 1;
	_hCalculateEvent = CreateEvent(NULL, FALSE, TRUE, NULL);		
	if (_hCalculateEvent == NULL) return 1;

	// accept 
	// 3명 접속 확인
	while (thread_count < 3 )
	{
		SOCKET c_socket = accept(_s_socket, reinterpret_cast<sockaddr*>(&_server_addr), &_addr_size);
		if (_c_socket == INVALID_SOCKET) {
			err_display("accept()");
		}
		else {
			_id = thread_count + 1;
			cout << "연결성공 ID: " << _id << endl;
		}
		// 플레이어 생성
		Player* player = new Player(c_socket, _id);

		// 쓰레드 만들면 주석 해제
		//hThread = CreateThread(NULL, 0, Server::ProcessClient, (LPVOID)player, 0, NULL);
		if (hThread == NULL) { closesocket(player->_c_socket); }
		else { CloseHandle(hThread); }
		thread_count++;
	}

	// 주쓰레드 생성

	// Send All 쓰레드 생성
	hThread = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)Server::SendAll, 0, 0, NULL);

	return false;
}

void Server::send_login_packet(SOCKET* client_socket, int client_id)
{
	SC_LOGININFO_PACKET packet;
	packet.size = sizeof(packet);
	packet.type = SC_LOGININFO;
	packet.id = client_id;
	send(*client_socket, reinterpret_cast<char*>(&packet), sizeof(packet), 0);
}

DWORD __stdcall Server::ProcessClient(LPVOID arg)
{

	return 0;
}

DWORD WINAPI Server::SendAll(LPVOID msg)
{
	// 주쓰레드가 마치기까지 기다림
	DWORD retval = WaitForSingleObject(_hSendEvent, INFINITE);

	// 모든 클라이언트들의 위치 보내기

	// 모든 적의 위치 보내기

	// 현재 표시되는 총알들의 위치 보내기

	// 다 보냄을 알림
	SetEvent(_hCalculateEvent);
}