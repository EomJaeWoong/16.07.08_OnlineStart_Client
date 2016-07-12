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
Star *g_Players[USER_MAX];
int g_playerID;
SOCKET sock;

/*--------------------------------------------------------------------*/
// 함수
/*--------------------------------------------------------------------*/
BOOL KeyProcess();
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
		Draw();
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
			// 패킷 처리
			// 0 : ID할당		 (수신)
			// 1 : 신규접속_자신 (수신)
			// 2 : 신규접속_타인 (수신)
			// 3 : 이동처리		 (수,발신)
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
	// 키입력 패킷처리
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
// 화면 출력
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
// 키입력 처리
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
// 연결 해제
/*--------------------------------------------------------------------*/
void Disconnect()
{
	closesocket(sock);
	WSACleanup();
	exit(1);
}