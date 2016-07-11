#include <WinSock2.h>
#include <Ws2tcpip.h>
#include <stdlib.h>
#include <stdio.h>
#include "OnlineStart_Client.h"
#include "console.h"

#pragma comment(lib, "Ws2_32.lib")

Star *g_Players[USER_MAX];
int g_playerID;

void DrawPlayer();

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
	int retval;
	stPacket packet;

	/*--------------------------------------------------------------------*/
	// 윈속 초기화
	/*--------------------------------------------------------------------*/
	WSADATA wsa;
	if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0)
		return -1;

	/*--------------------------------------------------------------------*/
	// 소켓 생성
	/*--------------------------------------------------------------------*/
	SOCKET sock = socket(AF_INET, SOCK_STREAM, 0);
	if (sock == INVALID_SOCKET)	err_display("socket()");

	/*--------------------------------------------------------------------*/
	// IP주소 셋팅
	/*--------------------------------------------------------------------*/
	char cAddr[16];
	WCHAR wcAddr[16];
	printf("접속할 IP주소를 입력하세요 : ");
	gets_s(cAddr, 16);
	mbstowcs_s(NULL, wcAddr, 16, cAddr, 16);

	SOCKADDR_IN sockaddr;
	sockaddr.sin_family = AF_INET;
	InetPton(AF_INET, wcAddr, &sockaddr.sin_addr);
	sockaddr.sin_port = htons(SERVER_PORT);

	/*--------------------------------------------------------------------*/
	// 소켓 연결
	/*--------------------------------------------------------------------*/
	retval = connect(sock, (SOCKADDR *)&sockaddr, sizeof(sockaddr));
	if (retval == SOCKET_ERROR)		err_display("connect()");

	/*--------------------------------------------------------------------*/
	// 콘솔 화면 초기화
	/*--------------------------------------------------------------------*/
	cs_Initial();
	cs_ClearScreen();

	while (1)
	{
		Buffer_Clear();
		retval = recv(sock, (char*)&packet, sizeof(packet), 0);

		switch (packet.type)
		{
		case 0 :
			if (retval == SOCKET_ERROR)	err_display("No received ID\n\n");
			
			else if (retval == 0)	{}
			else
			{
				if (g_Players[0] == NULL)
				{
					g_playerID = packet.ID;
					g_Players[0] = new Star;
					g_Players[0]->ID = g_playerID;
					break;
				}
			}
			break;

		case 1 :
			if (retval == SOCKET_ERROR)	err_display("No connection\n\n");

			if (g_playerID != 0)
			{
				if (g_Players[0]->ID == packet.ID)
				{
					g_Players[0]->x = packet.x;
					g_Players[0]->y = packet.y;
				}

				else{
					for (int iCnt = 1; iCnt < USER_MAX; iCnt++)
					{
						if (g_Players[iCnt] == NULL)
						{
							g_Players[iCnt]->ID = packet.ID;
							g_Players[iCnt]->x  = packet.x;
							g_Players[iCnt]->y  = packet.y;
						}
					}
				}
			}
			DrawPlayer();
			Buffer_Flip();
			break;

		case 2 :
			//접속해제
			break;

		case 3 :
			//액션
			//retval = send(sock, )
			break;

		default :
			break;
		}
	}

	closesocket(sock);
	WSACleanup();

	return 0;
}

void DrawPlayer()
{
	for (int iCnt = 0; iCnt < USER_MAX; iCnt++)
	{
		if (g_Players[iCnt] != NULL && g_Players[iCnt]->ID != 0)
			Sprite_Draw(g_Players[iCnt]->x, g_Players[iCnt]->y, '*');
	}
}