#include "Server.h"

int main()
{
	if (!Server::GetInst()->Init())
	{
		Server::DestroyInst();
		return 0;
	}
	Server::GetInst()->Run();
	return 0;
}