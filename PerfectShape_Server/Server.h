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

	//�����˻� ret
	int ret;
	// ������ ī��Ʈ
	int thread_count = 0;
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

	//������
	HANDLE hThread;

	// �̺�Ʈ �ڵ�
	HANDLE	_hSendEvent;
	HANDLE	_hCalculateEvent;


public:
	bool Init();
	bool Run();
	void InitServerAddr();
	// Ŭ���̾�Ʈ�� �����ϸ� ����Ȯ�ΰ� id�� ������ �Լ�
	void send_login_packet(SOCKET* client_socket, int client_id);
	
	void send_add_packet();
	DWORD WINAPI ProcessClient(LPVOID arg);

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
