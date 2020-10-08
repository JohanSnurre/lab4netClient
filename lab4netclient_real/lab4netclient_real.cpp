// lab4netclient_real.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <iostream>
#include <stdio.h> 
#include <stdlib.h> 
#include <string> 
#include <sys/types.h> 
#include <winsock2.h>
#include <Ws2tcpip.h>
#include <thread>
#include <conio.h>
#include "ClientHeader.h""
#pragma comment(lib, "Ws2_32.lib")


#define SERVER "127.0.0.1"	//ip address of udp server
#define BUFLEN 512	//Max length of buffer
#define PORT 5000	//The port on which to listen for incoming data
#define MAPSIZE 201
int field[MAPSIZE][MAPSIZE];





int * getPos(int playerID) {

	for (int i = 0; i < MAPSIZE; i++) {
		for (int j = 0; j < MAPSIZE; j++) {
			if (field[i][j] == playerID) {
				int a[] = {i,j};
				return a;

			}

		}

	}
	int a[] = { 0,0 };
	return a;

}


void sendMoveReq(SOCKET sock, int x, int y, char buf[], int playerID, int seqNum) {


	int *currPos = getPos(playerID);

	




	MoveEvent moveMsg;
	moveMsg.pos.x = currPos[0] + x - 100;
	moveMsg.pos.y = currPos[1] + y - 100;
	moveMsg.event.head.type = Event;
	moveMsg.event.head.id = playerID;
	moveMsg.event.type = Move;
	moveMsg.event.head.length = sizeof(moveMsg);
	moveMsg.event.head.seq_num = seqNum;
	memcpy((void*)buf, (void*)&moveMsg, sizeof(moveMsg));

	int sent = send(sock, buf, sizeof(buf), 0);
	

}


void sendInfo(SOCKET sock, char buf[], int seqNum, int playerID) {
	
	std::fill_n(buf, sizeof(buf), 0);
	

	char action;
	while (true) {
		std::cout << "::>";
		action = _getch();

		switch (action) {

		case 'w':
			std::cout << "w" << std::endl;
			sendMoveReq(sock, 0, -1, buf, playerID, seqNum);
			seqNum++;
			break;

		case 'a':
			std::cout << "a" << std::endl;
			sendMoveReq(sock, -1, 0, buf, playerID, seqNum);
			seqNum++;
			break;

		case 's':
			std::cout << "s" << std::endl;
			sendMoveReq(sock, 0, 1, buf, playerID, seqNum);
			seqNum++;
			break;

		case 'd':
			std::cout << "d" << std::endl;
			sendMoveReq(sock, 1, 0, buf, playerID, seqNum);
			seqNum++;
			break;


		default:
			break;

		}


		std::fill_n(buf, sizeof(buf), 0);


	}
}


void receiveInfo(SOCKET sock, char buf[], int seqNum) {
	int receive;
	std::fill_n(buf, sizeof(buf), 0);



	while (true) {

		receive = recv(sock, buf, sizeof(buf), 0);
		MsgHead* head = (MsgHead*)buf;
		std::cout << head->type << " : " << head->id << " : " << head->length << " : " << head->seq_num << std::endl;

		if (head->type == Change) {
			ChangeMsg* change = (ChangeMsg*)head;
			if (change->type == NewPlayerPosition) {
				NewPlayerPositionMsg* msg = (NewPlayerPositionMsg*)buf;
				int x, y;
				
				x = msg->pos.x + 100;
				y = msg->pos.y + 100;
				field[x][y] = msg->msg.head.id;


				std::cout << field[x][y] << std::endl;
			}

		}
		std::fill_n(buf, sizeof(buf), 0);
	}



}


int main()
{
	std::string GUImsg;
	char sendBuf[BUFLEN];
	char recvBuf[BUFLEN];


	//startup winsock
	WSADATA data;
	WORD version = MAKEWORD(2, 2);
	int wsOK = WSAStartup(version, &data);
	if (wsOK != 0) {

		std::cout << "can't start winsock" << wsOK;
		return 0;

	}


	//create hint structure for GUI
	sockaddr_in GUI;
	GUI.sin_family = AF_INET;
	GUI.sin_port = htons(5000);
	inet_pton(AF_INET, "127.0.0.1", &GUI.sin_addr);

	//create hint structure for the server
	sockaddr_in server;
	server.sin_family = AF_INET;
	server.sin_port = htons(49152);
	inet_pton(AF_INET, "127.0.0.1", &server.sin_addr);

	std::cout << "Connection successful" << std::endl;
	//create UDP socket for the GUI
	SOCKET GUISock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);

	//create TCP socket for the server
	SOCKET serverSock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

	//connect to server
	int res = connect(serverSock, (SOCKADDR*)&server, sizeof(server));
	if (res != 0) {
		std::cout << "conncetion failed to server" << std::endl;
		closesocket(serverSock);
		WSACleanup();
		return -1;
	}
	std::cout << "Connection successful" << std::endl;

	int seqNum = 0;
	int playerID = 0;


	char buf[BUFLEN];
	char buff[BUFLEN];

	JoinMsg joinMsg;
	joinMsg.head.id = 0;
	joinMsg.head.length = sizeof(joinMsg);
	joinMsg.head.type = Join;
	memcpy((void*)sendBuf, (void*)&joinMsg, sizeof(joinMsg));
	int BytesSent = send(serverSock, sendBuf, sizeof(sendBuf), 0);
	seqNum = seqNum + 1;
	int rec = recv(serverSock, recvBuf, BUFLEN, 0);

	
	
	

	std::thread sendInfo(sendInfo, serverSock, sendBuf, seqNum, playerID);
	std::thread receiveInfo(receiveInfo, serverSock, recvBuf, seqNum);

	while (true) {

	}


	//command prompt

	//while (true) {

		/*std::cout << "::>";
		std::getline(std::cin, GUImsg);
		if (GUImsg._Equal("exit")) {
			break;
		}
		std::cin >> val;
		/*

		/*switch (val)
		{
		case 1:
			JoinMsg joinMsg;
			joinMsg.head.id = 0;
			joinMsg.head.length = sizeof(joinMsg);
			joinMsg.head.type = Join;
			memcpy((void*)buf, (void*)&joinMsg, sizeof(joinMsg));
			break;

		case 2:
			LeaveMsg leaveMsg;
			leaveMsg.head.id = 0;
			leaveMsg.head.length = sizeof(leaveMsg);
			leaveMsg.head.type = Leave;
			memcpy((void*)buf, (void*)&leaveMsg, sizeof(leaveMsg));
			break;

		case 3:
			MoveEvent moveMsg;
			moveMsg.event.type = Move;
			moveMsg.pos.x = -99;
			moveMsg.pos.y = -100;
			moveMsg.event.head.id = 0;
			moveMsg.event.head.length = sizeof(moveMsg);
			moveMsg.event.head.type = Event;
		}

		*/






		/*int BytesSent = send(serverSock, buf, sizeof(buf), 0);
		std::fill_n(buf, sizeof(buf), 0);
		//recv(serverSock, buf, sizeof(buf), 0);


		int BytesRecv = recv(serverSock, buf, sizeof(buf), 0);
		std::cout << "received: " << BytesRecv;
		/*

		//write out to the socket
		/*int sendOK = sendto(GUISock, buf, sizeof(buf) + 1, 0, (sockaddr*)&GUI, sizeof(GUI));
		if (sendOK == SOCKET_ERROR) {
			std::cout << "ERROR" << WSAGetLastError() << std::endl;
		}

		//receiving acks
		std::fill_n(buf, sizeof(buf), 0);
		recv(GUISock, buf, sizeof(buf), 0);
		std::cout << buf << std::endl;

		*/






		//}


		//close the socket
	closesocket(GUISock);

	//close down winsock
	WSACleanup();


	return 0;



}


