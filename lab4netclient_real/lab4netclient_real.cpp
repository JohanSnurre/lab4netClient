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
#include <unordered_map>
#include "ClientHeader.h"
#pragma comment(lib, "Ws2_32.lib")



#define SERVER "192.168.10.162"	//ip address of udp server
#define GUISERVER "::1" //GUI adress
#define BUFLEN 512	//Max length of buffer
#define PORT 5000	//The port on which to listen for incoming data
#define MAPSIZE 201
/*std::unordered_map<int, Coordinate> field;
std::unordered_map<int, Coordinate>::iterator iter;*/
int field[MAPSIZE][MAPSIZE];
int thisPlayerID;





int * getPos(int ID) {
	int pos[2];
	for (int x = 0; x < MAPSIZE; x++) {
		for (int y = 0; y < MAPSIZE; y++) {
			if (field[x][y] == ID) {
				pos[0] = x;
				pos[1] = y;
				return pos;
			}


		}


	}

	/*iter = field.find(ID);
	int pos[] = { iter->second.x, iter->second.y };*/
	pos[0] = 500;
	pos[1] = 500;
	return pos;
}

void insertToField(int ID, int pos[]) {
	int x = pos[0];
	int y = pos[1];
	if (x == 500 || y == 500) return;

	field[x][y] = ID;
	/*Coordinate coord;
	coord.x = pos[0];
	coord.y = pos[1];
	field[ID] = coord;*/
	return;
}


void sendMoveReq(SOCKET sock, int x, int y, int seqNum) {

	char sendBuf[BUFLEN];
	int *currPos = getPos(thisPlayerID);
	//std::cout << currPos[0] << " : " << currPos[1] << std::endl;
	
	MoveEvent moveMsg;
	moveMsg.pos.x = x;
	moveMsg.pos.y = y;
	std::cout << "Your were at at: " << currPos[0] << " : " << currPos[1] << std::endl;
	moveMsg.event.head.type = Event;
	moveMsg.event.head.id = thisPlayerID;
	moveMsg.event.type = Move;
	moveMsg.event.head.length = sizeof(moveMsg);
	moveMsg.event.head.seq_num = seqNum;
	memcpy((void*)sendBuf, (void*)&moveMsg, sizeof(moveMsg));

	int sent = send(sock, sendBuf, sizeof(sendBuf), 0);
	

}

void sendLeave(SOCKET sock, int seqNum) {
	char sendBuf[BUFLEN];

	LeaveMsg leave;
	leave.head.id = thisPlayerID;
	leave.head.seq_num = seqNum;
	leave.head.type = Leave;
	leave.head.length = sizeof(leave);
	memcpy((void*)sendBuf, (void*)&leave, sizeof(leave));

	int sent = send(sock, sendBuf, sizeof(sendBuf), 0);
}


void sendInfo(SOCKET sock, int seqNum) {
	
	
	

	char action;
	while (true) {
		//std::cout << "::>" << std::endl;
		action = _getch();

		switch (action) {

		case 'w':
			//std::cout << "w" << std::endl;
			sendMoveReq(sock, 0, -1, seqNum);
			seqNum++;
			break;

		case 'a':
			//std::cout << "a" << std::endl;
			sendMoveReq(sock, -1, 0, seqNum);
			seqNum++;
			break;

		case 's':
			//std::cout << "s" << std::endl;
			sendMoveReq(sock, 0, 1, seqNum);
			seqNum++;
			break;

		case 'd':
			//std::cout << "d" << std::endl;
			sendMoveReq(sock, 1, 0, seqNum);
			seqNum++;
			break;

		case 'l':
			sendLeave(sock, seqNum);
			seqNum++;
			break;
		/*case 'W':    //speedhack
			std::cout << "w" << std::endl;
			sendMoveReq(sock, 0, -3, seqNum);
			seqNum++;
			break;

		case 'A':
			std::cout << "a" << std::endl;
			sendMoveReq(sock, -3, 0, seqNum);
			seqNum++;
			break;

		case 'S':
			std::cout << "s" << std::endl;
			sendMoveReq(sock, 0, 3, seqNum);
			seqNum++;
			break;

		case 'D':
			std::cout << "d" << std::endl;
			sendMoveReq(sock, 3, 0, seqNum);
			seqNum++;
			break;*/

		default:
			break;

		}



	}
}
void updateGUI(SOCKET sock, int ID, sockaddr_in6 GUI, std::string GUImsg) {
	char buf[BUFLEN];
	std::fill_n(buf, BUFLEN, 0);
	memcpy((void*)buf, (void*)&GUImsg, sizeof(GUImsg));
	int sendOK = sendto(sock, GUImsg.c_str(),GUImsg.size() , 0, (struct sockaddr*)&GUI, sizeof(GUI));
	if (sendOK == SOCKET_ERROR) {
		std::cout << "ERROR " << WSAGetLastError() << std::endl;
	}


}

void receiveInfo(SOCKET serverSock, SOCKET GUISock, sockaddr_in6 GUI,  int seqNum) {
	char recvBuf[BUFLEN];
	int receive;
	//std::fill_n(recvBuf, sizeof(recvBuf), 0);


	
	
	

	while (true) {

		receive = recv(serverSock, recvBuf, BUFLEN, 0);
		MsgHead* head = (MsgHead*)recvBuf;
		//std::cout << "Got packet with sequence number: " << head->seq_num << std::endl;
		//std::cout << head->type << " : " << head->id << " : " << head->length << " : " << head->seq_num << std::endl;
		int ID = head->id;

		if (head->type == Join) {
			std::cout << "Join";
			thisPlayerID = head->id;
			
		}

		if (head->type == Leave) {
			std::cout << "Leave";
			




		}

		if (head->type == Event) {
			std::cout << "Event";

			if (head->type == Move) {
				std::cout << "Move";

			}


		}

		

		if (head->type == Change) {
			//std::cout << "Change" << std::endl;
			ChangeMsg* change = (ChangeMsg*)head;
			NewPlayerPositionMsg* NPPmsg = (NewPlayerPositionMsg*)head;
			if (change->type == NewPlayerPosition) {
				//std::cout << "NewPlayerPosition" << std::endl;
				//std::cout << head->id << " : " << head->type << " : " << change->type << " : " << NPPmsg->pos.x << " : " << NPPmsg->pos.y << std::endl;
				
				

				int x, y;
				
				x = NPPmsg->pos.x;
				y = NPPmsg->pos.y;
				//std::cout << NPPmsg->pos.x << " : " << NPPmsg->pos.y << std::endl;
				field[x][y] = NPPmsg->msg.head.id;
				std::string color;
				switch (NPPmsg->msg.head.id) {

				case 0:
					color = "white";
					break;

				case 1:
					color = "blue";
					break;

				case 2:
					color = "green";
					break;

				case 3:
					color = "red";
					break;
				case 4:
					color = "yellow";
					break;
				case 5:
					color = "magenta";
					break;

				case 6:
					color = "orange";
					break;
				case 7:
					color = "pink";
					break;
				case 8:
					color = "cyan";
					break;

				}
				std::string GUImsg;
				GUImsg = std::to_string(x) + "," + std::to_string(y) + "," + color;
				updateGUI(GUISock, ID, GUI, GUImsg);
				
				
			}
			if (change->type == NewPlayer) {
				std::cout << "NewPlayer" << std::endl;
				
				//NewPlayerMsg* NPmsg = (NewPlayerMsg*)recvBuf;
				
			}

			if (change->type == PlayerLeave) {
				std::cout << "PlayerLeave" << std::endl;
				if (head->id == thisPlayerID) {
					std::cout << "Disconnected from server" << std::endl;
					return;
				}
				std::cout << "Player " << head->id << " left. Removing from board" << std::endl;
				int *remove = getPos(head->id);
				int x = *remove;
				int y = *remove + 1;
				std::cout << x << " . " << y << std::endl;
				field[x][y] = 0;
			}

		}
		//std::fill_n(recvBuf, sizeof(recvBuf), 0);
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
	sockaddr_in6 GUI;
	memset(&GUI, 0, sizeof(GUI));
	GUI.sin6_family = AF_INET6;
	GUI.sin6_port = htons(5000);
	inet_pton(AF_INET6, GUISERVER, &GUI.sin6_addr);

	//create hint structure for the server
	sockaddr_in server;
	server.sin_family = AF_INET;
	server.sin_port = htons(49152);
	inet_pton(AF_INET, SERVER, &server.sin_addr);

	//create UDP socket for the GUI
	SOCKET GUISock = socket(PF_INET6, SOCK_DGRAM, 0);
	
	//create TCP socket for the server
	SOCKET serverSock = socket(AF_INET, SOCK_STREAM, 0);

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
	


	JoinMsg joinMsg;
	joinMsg.head.id = 0;
	joinMsg.head.seq_num = seqNum;
	joinMsg.head.length = sizeof(joinMsg);
	joinMsg.head.type = Join;
	memcpy((void*)sendBuf, (void*)&joinMsg, sizeof(joinMsg));


	int BytesSent = send(serverSock, sendBuf, BUFLEN, 0);


	seqNum = seqNum + 1;
	int receive = recv(serverSock, recvBuf, BUFLEN, 0);
	MsgHead* head = (MsgHead*)recvBuf;
	thisPlayerID = head->id;
	std::cout << head->type << " : " << head->id << " : " << head->seq_num <<  std::endl;

	

	std::thread sendInfo(sendInfo, serverSock, seqNum);
	std::thread receiveInfo(receiveInfo, serverSock, GUISock, GUI, seqNum);

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


