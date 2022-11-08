#pragma once
#include "header.h"
#include "Player.h"

unordered_map<int, Player>clients;

class Server
{
private:
	static Server* m_pInst;

public:
	static Server* GetInst()
	{
		if (!m_pInst)
			m_pInst = new Server;
		return m_pInst;
	}
	static void DestroyInst()
	{
		if (m_pInst)
		{
			delete m_pInst;
			m_pInst = NULL;
		}
	}

public:
	Server();
	~Server();
private:

	//오류검사 ret
	int ret;
	// 스레드 카운트
	int thread_count = 0;
	int _id;
	
	// s소켓 연결
	WSADATA _WsaData;
	SOCKET _s_socket;
	SOCKADDR_IN _server_addr;
	INT _addr_size;

	SOCKET _c_socket;
	WSABUF _rWsaBuf;
	WSABUF _sWsaBuf;
	bool _shutdown = false;

	//쓰레드
	HANDLE hThread;

	// 이벤트 핸들
	static HANDLE	_hSendEvent;
	static HANDLE	_hCalculateEvent;


public:
	bool Init();
	bool Run();
	void InitServerAddr();
	// 클라이언트가 접속하면 접속확인과 id를 보내는 함수
	void send_login_packet(SOCKET* client_socket, int client_id);
	
	void send_add_packet();
	DWORD WINAPI ProcessClient(LPVOID arg);
	static DWORD WINAPI SendAll(LPVOID msg);

public:
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
};
