#include <WinSock2.h>
#include <Ws2tcpip.h>
#include <stdlib.h>
#include <stdio.h>
#include "OnlineStart_Client.h"
#include "console.h"

#pragma comment(lib, "Ws2_32.lib")

/*--------------------------------------------------------------------*/
// �÷��̾�
/*--------------------------------------------------------------------*/
Star *g_Players[USER_MAX];
int g_playerID;
SOCKET sock;

/*--------------------------------------------------------------------*/
// �Լ�
/*--------------------------------------------------------------------*/
BOOL KeyProcess();
void Init();
void Network();
void Draw();
void Disconnect();

/*--------------------------------------------------------------------*/
// ���� ǥ��
/*--------------------------------------------------------------------*/
void err_display(char *msg)
{
	LPVOID lpMsgBuf;
	FormatMessage(
		FORMAT_MESSAGE_ALLOCATE_BUFFER |
		FORMAT_MESSAGE_FROM_SYSTEM,
		NULL, WSAGetLastError(),
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
		(LPTSTR)&lpMsgBuf, 0, NULL);
	printf("[%s] %s", msg, (LPCTSTR)lpMsgBuf);
	LocalFree(lpMsgBuf);
}

int main()
{
	Init();

	while (1)
	{
		Network();
		Draw();
	}

	return 0;
}

/*--------------------------------------------------------------------*/
// �ʱ�ȭ
// ��Ʈ��ũ, �ܼ� ȭ���� �ʱ�ȭ ��
/*--------------------------------------------------------------------*/
void Init()
{
	int retval;

	/*--------------------------------------------------------------------*/
	// ���� �ʱ�ȭ
	/*--------------------------------------------------------------------*/
	WSADATA wsa;
	if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0)
		return;

	/*--------------------------------------------------------------------*/
	// ���� ����
	/*--------------------------------------------------------------------*/
	sock = socket(AF_INET, SOCK_STREAM, 0);
	if (sock == INVALID_SOCKET)
	{
		err_display("socket()");
		exit(1);
	}

	/*--------------------------------------------------------------------*/
	// IP�ּ� ����
	/*--------------------------------------------------------------------*/
	char cAddr[16];
	WCHAR wcAddr[16];
	printf("������ IP�ּҸ� �Է��ϼ��� : ");
	gets_s(cAddr, 16);
	mbstowcs_s(NULL, wcAddr, 16, cAddr, 16);

	SOCKADDR_IN sockaddr;
	sockaddr.sin_family = AF_INET;
	InetPton(AF_INET, wcAddr, &sockaddr.sin_addr);
	sockaddr.sin_port = htons(SERVER_PORT);

	/*--------------------------------------------------------------------*/
	// ���� ����
	/*--------------------------------------------------------------------*/
	retval = connect(sock, (SOCKADDR *)&sockaddr, sizeof(sockaddr));
	if (retval == SOCKET_ERROR)
	{
		err_display("connect()");
		exit(1);
	}

	/*--------------------------------------------------------------------*/
	// �ܼ� ȭ�� �ʱ�ȭ
	/*--------------------------------------------------------------------*/
	cs_Initial();
	cs_ClearScreen();
}

/*--------------------------------------------------------------------*/
// ��Ʈ��ũ ó��
/*--------------------------------------------------------------------*/
void Network()
{
	int retval;

	FD_SET Readset;
	FD_ZERO(&Readset);
	FD_SET(sock, &Readset);
		
	TIMEVAL Timeval;
	Timeval.tv_sec = 0;
	Timeval.tv_usec = 0;

	retval = select(0, &Readset, NULL, NULL, &Timeval);

	if (retval > 0)
	{
		if (FD_ISSET(sock, &Readset))
		{
			stPacket packet;
			retval = recv(sock, (char *)&packet, sizeof(packet), 0);
			if (retval == SOCKET_ERROR){
				err_display("recv()");
				Disconnect();
			}

			/*--------------------------------------------------------------------*/
			// ��Ŷ ó��
			// 0 : ID�Ҵ�		 (����)
			// 1 : �ű�����_�ڽ� (����)
			// 2 : �ű�����_Ÿ�� (����)
			// 3 : �̵�ó��		 (��,�߽�)
			/*--------------------------------------------------------------------*/
			switch (packet.type)
			{
			case 0 :		
				for (int iCnt = 0; iCnt < USER_MAX; iCnt++)
				{
					if (g_Players[iCnt] == NULL)
					{
						g_playerID = packet.ID;
						g_Players[iCnt] = new Star;
						g_Players[iCnt]->ID = g_playerID;
						break;
					}
				}
				break;

			case 1 :
				for (int iCnt = 0; iCnt < USER_MAX; iCnt++)
				{
					if (g_Players[iCnt] != NULL && g_Players[iCnt]->ID == packet.ID)
					{
						g_Players[iCnt]->x = packet.x;
						g_Players[iCnt]->y = packet.y;
					}
				}
				break;

			case 2 :
				Disconnect();
				break;

			case 3 :
				for (int iCnt = 0; iCnt < USER_MAX; iCnt++)
				{
					if (g_Players[iCnt] != NULL && g_Players[iCnt]->ID == packet.ID)
					{
						g_Players[iCnt]->x = packet.x;
						g_Players[iCnt]->y = packet.y;
					}
				}
				break;

			default:
				break;
			}
		}
	}

	/*--------------------------------------------------------------------*/
	// Ű�Է� ��Ŷó��
	/*--------------------------------------------------------------------*/
	if (KeyProcess())
	{
		Star* pStar = NULL;

		for (int iCnt = 0; iCnt < USER_MAX; iCnt++)
		{
			if (g_Players[iCnt] != NULL && g_Players[iCnt]->ID == g_playerID)
			{
				pStar = g_Players[iCnt];
				break;
			}
		}
		
		stPacket packet;
		packet.type = 3;
		packet.ID = pStar->ID;
		packet.x = pStar->x;
		packet.y = pStar->y;
		
		retval = send(sock, (char *)&packet, sizeof(packet), 0);
		if (retval == SOCKET_ERROR){
			err_display("send()");
			Disconnect();
		}
	}
}

/*--------------------------------------------------------------------*/
// ȭ�� ���
/*--------------------------------------------------------------------*/
void Draw()
{
	Buffer_Clear();
	for (int iCnt = 0; iCnt < USER_MAX; iCnt++)
	{
		if (g_Players[iCnt] != NULL && g_Players[iCnt]->ID != 0)
			Sprite_Draw(g_Players[iCnt]->x, g_Players[iCnt]->y, '*');
	}
	
	Buffer_Flip();
	Sleep(50);
}

/*--------------------------------------------------------------------*/
// Ű�Է� ó��
/*--------------------------------------------------------------------*/
BOOL KeyProcess()
{
	int retval;
	Star *pStar = NULL;

	for (int iCnt = 0; iCnt < USER_MAX; iCnt++)
	{
		if (g_Players[iCnt] != NULL && g_Players[iCnt]->ID == g_playerID)
		{
			pStar = g_Players[iCnt];
			break;
		}
	}
	int x = pStar->x;
	int y = pStar->y;

	if (GetAsyncKeyState(VK_LEFT) & 0x8001)		x--;
	if (GetAsyncKeyState(VK_RIGHT) & 0x8001)	x++;
	if (GetAsyncKeyState(VK_UP) & 0x8001)		y--;
	if (GetAsyncKeyState(VK_DOWN) & 0x8001)		y++;

	if (pStar != NULL && (pStar->x != x || pStar->y != y)){
		pStar->x = x;
		pStar->y = y;
		return TRUE;
	}

	return FALSE;
}

/*--------------------------------------------------------------------*/
// ���� ����
/*--------------------------------------------------------------------*/
void Disconnect()
{
	closesocket(sock);
	WSACleanup();
	exit(1);
}