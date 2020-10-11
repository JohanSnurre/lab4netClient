//Author: Johan Larsson, Johlax-8 . 2020-10-11
//This is the client program for the game.
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



#define SERVER "192.168.10.162"	//ip address of tcp server
#define GUISERVER "::1" //GUI adress
#define BUFLEN 512	//Max length of buffer
#define GUIPORT 5000	//The port on which to send to the GUI
#define SERVERPORT 49152 // the port on which to listen for incoming data
#define MAPSIZE 201	//the mapsize
/*std::unordered_map<int, Coordinate> field;
std::unordered_map<int, Coordinate>::iterator iter;*/
int field[MAPSIZE][MAPSIZE];
int thisPlayerID;




//returns the position of the requested player in the form of [x][y]
//if no player is found, the value [500][500] is returned
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

//inserts the player onto the requested position
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

//send a move request to the server. The inputs are the socket which to send over, the direction the player wants to move in(x and y), and the sequence number
void sendMoveReq(SOCKET sock, int x, int y, int seqNum) {

	char sendBuf[BUFLEN];
	int *currPos = getPos(thisPlayerID);
	//std::cout << currPos[0] << " : " << currPos[1] << std::endl;
	
	MoveEvent moveMsg;
	moveMsg.pos.x = x;
	moveMsg.pos.y = y;
	std::cout << "Your were at at: " << currPos[0] - 100 << " : " << currPos[1] - 100 << std::endl;
	moveMsg.event.head.type = Event;
	moveMsg.event.head.id = thisPlayerID;
	moveMsg.event.type = Move;
	moveMsg.event.head.length = sizeof(moveMsg);
	moveMsg.event.head.seq_num = seqNum;
	memcpy((void*)sendBuf, (void*)&moveMsg, sizeof(moveMsg));

	int sent = send(sock, sendBuf, sizeof(sendBuf), 0);
	

}
//sends a leave message to the server, The inputs are the socket which to send over and the sequence number
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

//A function which runs on a seperate thread. Gathers the input and sends a move request based on that input. Sends it through the provided socket and with the provided sequence numeber
void sendInfo(SOCKET sock, int seqNum) {
	
	
	

	char action;
	while (true) {
		action = _getch();

		switch (action) {

		case 'w':
			sendMoveReq(sock, 0, -1, seqNum);
			seqNum++;
			break;

		case 'a':
			sendMoveReq(sock, -1, 0, seqNum);
			seqNum++;
			break;

		case 's':
			sendMoveReq(sock, 0, 1, seqNum);
			seqNum++;
			break;

		case 'd':
			sendMoveReq(sock, 1, 0, seqNum);
			seqNum++;
			break;

		case 'l':
			sendLeave(sock, seqNum);
			seqNum++;
			break;

		default:
			break;

		}



	}
}

//A funciton which updates the GUI. The inputs are the socket to send the information through, the playerID that has moved, the GUI structure and the message to the gui(a string)
void updateGUI(SOCKET sock, int ID, sockaddr_in6 GUI, std::string GUImsg) {
	char buf[BUFLEN];
	std::fill_n(buf, BUFLEN, 0);
	memcpy((void*)buf, (void*)&GUImsg, sizeof(GUImsg));
	int sendOK = sendto(sock, GUImsg.c_str(),GUImsg.size() , 0, (struct sockaddr*)&GUI, sizeof(GUI));
	if (sendOK == SOCKET_ERROR) {
		std::cout << "ERROR " << WSAGetLastError() << std::endl;
	}


}

//A function which runs on a different thread. It received informaion from the server and decides what to do with it. The inputs are the socket to receice information from, the GUI socket to send GUI updates to, the GUI structure and the sequence number
void receiveInfo(SOCKET serverSock, SOCKET GUISock, sockaddr_in6 GUI,  int seqNum) {
	char recvBuf[BUFLEN];
	int receive;
	

	while (true) {

		receive = recv(serverSock, recvBuf, BUFLEN, 0);
		MsgHead* head = (MsgHead*)recvBuf;
		std::cout << "Got packet with playerID: " << head->id << std::endl;
		int ID = head->id;

		if (head->type == Join) {
			
			if (thisPlayerID == 0) {
				std::cout << "Join";
				thisPlayerID = head->id;
			}
			
			
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
			std::cout << "Change" << std::endl;
			ChangeMsg* change = (ChangeMsg*)head;
			NewPlayerPositionMsg* NPPmsg = (NewPlayerPositionMsg*)head;
			if (change->type == NewPlayerPosition) {
				std::cout << "NewPlayerPosition" << std::endl;
 
				
				int x, y;
				
				x = NPPmsg->pos.x;
				y = NPPmsg->pos.y;
				std::cout << "Player " << head->id << " is now at position x=" << x - 100 << " and y=" << y - 100 << std::endl;
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
				int y = *(remove +1);
				std::cout << x << " . " << y << std::endl;
				
				
				std::string GUImsg;
				GUImsg = std::to_string(x) + "," + std::to_string(y) + ",white";
				updateGUI(GUISock, ID, GUI, GUImsg);
				field[x][y] = 0;
			}

		}
	}



}



//the main funciton
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
	GUI.sin6_port = htons(GUIPORT);
	inet_pton(AF_INET6, GUISERVER, &GUI.sin6_addr);

	//create hint structure for the server
	sockaddr_in server;
	server.sin_family = AF_INET;
	server.sin_port = htons(SERVERPORT);
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



	std::thread sendInfo(sendInfo, serverSock, seqNum);
	std::thread receiveInfo(receiveInfo, serverSock, GUISock, GUI, seqNum);

	while (true) {

	}

		//close the socket
	closesocket(GUISock);

	//close down winsock
	WSACleanup();


	return 0;



}


