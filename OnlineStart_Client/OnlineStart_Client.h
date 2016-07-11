#ifndef __ONLINESTAR_CLIENT__H__
#define __ONLINESTAR_CLIENT__H__

#define SERVER_PORT 3000
#define USER_MAX 100

typedef struct st_Packet
{
	int type;
	int ID;
	int x;
	int y;
} stPacket;

typedef struct st_Star
{
	int ID;
	int x;
	int y;
} Star;
#endif