/*--------------------------------------------------------------------*/
// 
// OnlineStar_Client.cpp
// 별 움직이기 클라이언트
//
/*--------------------------------------------------------------------*/
#include <WinSock2.h>
#include <Ws2tcpip.h>
#include <stdlib.h>
#include <stdio.h>
#include "OnlineStart_Client.h"
#include "console.h"

#pragma comment(lib, "Ws2_32.lib")

/*--------------------------------------------------------------------*/
// 플레이어
/*--------------------------------------------------------------------*/
Star g_Players[USER_MAX];
Star *g_pStar;
SOCKET sock;

/*--------------------------------------------------------------------*/
// 함수
/*--------------------------------------------------------------------*/
void KeyProcess();
void Init();
void Network();
void Draw();
void Disconnect();

/*--------------------------------------------------------------------*/
// 에러 표시
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
		KeyProcess();
		Draw();
		Sleep(50);
	}

	return 0;
}

/*--------------------------------------------------------------------*/
// 초기화
// 네트워크, 콘솔 화면을 초기화 함
/*--------------------------------------------------------------------*/
void Init()
{
	int retval;

	/*--------------------------------------------------------------------*/
	// 윈속 초기화
	/*--------------------------------------------------------------------*/
	WSADATA wsa;
	if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0)
		return;

	/*--------------------------------------------------------------------*/
	// 소켓 생성
	/*--------------------------------------------------------------------*/
	sock = socket(AF_INET, SOCK_STREAM, 0);
	if (sock == INVALID_SOCKET)
	{
		err_display("socket()");
		exit(1);
	}

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
	if (retval == SOCKET_ERROR)
	{
		err_display("connect()");
		exit(1);
	}

	/*--------------------------------------------------------------------*/
	// 콘솔 화면 초기화
	/*--------------------------------------------------------------------*/
	cs_Initial();
	cs_ClearScreen();
}

/*--------------------------------------------------------------------*/
// 네트워크 처리
/*--------------------------------------------------------------------*/
void Network()
{
	int retval;

	while (1)
	{
		FD_SET Readset;
		FD_ZERO(&Readset);
		FD_SET(sock, &Readset);

		TIMEVAL Timeval;
		Timeval.tv_sec = 0;
		Timeval.tv_usec = 0;

		retval = select(0, &Readset, NULL, NULL, &Timeval);

		if (retval == 0 || retval == SOCKET_ERROR)	break;

		else if (retval > 0)
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
				// 패킷 처리
				// 0 : ID할당		 (수신)
				// 1 : 신규접속		 (수신)
				// 2 : 연결해제
				// 3 : 이동처리		 (수,발신)
				/*--------------------------------------------------------------------*/
				switch (packet.type)
				{
				case 0:
					for (int iCnt = 0; iCnt < USER_MAX; iCnt++)
					{
						if (g_Players[iCnt].ID == 0)
						{
							g_Players[iCnt].ID = packet.ID;
							g_pStar = &g_Players[iCnt];
							break;
						}
					}
					break;

				case 1:
					///////////////////////////////////////////////////////////////////
					// 자신의 정보 설정
					///////////////////////////////////////////////////////////////////
					if (packet.ID == g_pStar->ID)
					{
						g_pStar->x = packet.x;
						g_pStar->y = packet.y;
					}

					///////////////////////////////////////////////////////////////////
					// 다른 플레이어 설정
					///////////////////////////////////////////////////////////////////
					else
					{
						for (int iCnt = 0; iCnt < USER_MAX; iCnt++)
						{
							if (g_Players[iCnt].ID == 0 && &g_Players[iCnt] != g_pStar)
							{
								g_Players[iCnt].ID = packet.ID;
								g_Players[iCnt].x = packet.x;
								g_Players[iCnt].y = packet.y;
								break;
							}
						}
					}
					break;

				case 2:
					Disconnect();
					break;

				case 3:
					for (int iCnt = 0; iCnt < USER_MAX; iCnt++)
					{
						if (g_Players[iCnt].ID != 0 && g_Players[iCnt].ID == packet.ID &&
							&g_Players[iCnt] != g_pStar)
						{
							g_Players[iCnt].x = packet.x;
							g_Players[iCnt].y = packet.y;
						}
					}
					break;

				default:
					break;
				}
			}
		}
	}
}

/*--------------------------------------------------------------------*/
// 화면 출력
/*--------------------------------------------------------------------*/
void Draw()
{
	char cCount = '0';

	Buffer_Clear();

	Sprite_Draw(0, 0, 'C');
	Sprite_Draw(1, 0, 'l');
	Sprite_Draw(2, 0, 'i');
	Sprite_Draw(3, 0, 'e');
	Sprite_Draw(4, 0, 'n');
	Sprite_Draw(5, 0, 't');
	Sprite_Draw(6, 0, ' ');
	Sprite_Draw(7, 0, 'C');
	Sprite_Draw(8, 0, 'o');
	Sprite_Draw(9, 0, 'n');
	Sprite_Draw(10, 0, 'n');
	Sprite_Draw(11, 0, 'e');
	Sprite_Draw(12, 0, 'c');
	Sprite_Draw(13, 0, 't');
	Sprite_Draw(14, 0, ' ');
	Sprite_Draw(15, 0, ':');
	Sprite_Draw(16, 0, ' ');
	for (int iCnt = 0; iCnt < USER_MAX; iCnt++)
	{
		if (g_Players[iCnt].ID != 0)
		{
			Sprite_Draw(g_Players[iCnt].x, g_Players[iCnt].y, '*');
			cCount++;
		}
	}
	
	Sprite_Draw(17, 0, cCount);
	Buffer_Flip();
}

/*--------------------------------------------------------------------*/
// 키입력 처리
/*--------------------------------------------------------------------*/
void KeyProcess()
{
	int retval;
	stPacket packet;

	if (g_pStar == NULL) return;

	int x = g_pStar->x;
	int y = g_pStar->y;

	if (GetAsyncKeyState(VK_LEFT) & 0x8001)		x--;
	if (GetAsyncKeyState(VK_RIGHT) & 0x8001)	x++;
	if (GetAsyncKeyState(VK_UP) & 0x8001)		y--;
	if (GetAsyncKeyState(VK_DOWN) & 0x8001)		y++;

	if (g_pStar->x != x || g_pStar->y != y){
		g_pStar->x = x;
		g_pStar->y = y;

		if (g_pStar->y < 1)
			g_pStar->y = 1;
		if (g_pStar->x < 0)
			g_pStar->x = 0;
		if (g_pStar->x > dfSCREEN_WIDTH - 2)
			g_pStar->x = dfSCREEN_WIDTH - 2;
		if (g_pStar->y > dfSCREEN_HEIGHT - 1)
			g_pStar->y = dfSCREEN_HEIGHT - 1;

		packet.type = 3;
		packet.ID = g_pStar->ID;
		packet.x = g_pStar->x;
		packet.y = g_pStar->y;

		retval = send(sock, (char *)&packet, sizeof(packet), 0);
		if (retval == SOCKET_ERROR){
			err_display("send()");
			Disconnect();
		}
	}
}

/*--------------------------------------------------------------------*/
// 연결 해제
/*--------------------------------------------------------------------*/
void Disconnect()
{
	closesocket(sock);
	WSACleanup();
	exit(1);
}