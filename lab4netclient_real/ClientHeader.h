#pragma once
#define MAXNAMELEN 32

enum ObjectDesc
{
	Human,
	NonHuman,
	Vehicle,
	StaticObject
};

enum ObjectForm
{
	Cube,
	Sphere,
	Pyramid,
	Cone
};

struct Coordinate
{
	int x;
	int y;
};



// message head
enum MsgType
{
	Join,
	Leave,
	Change,
	Event,
	TextMessage


};

struct MsgHead
{
	unsigned int length;
	unsigned int seq_num;
	unsigned int id;
	MsgType type;
};


//message type Join
struct JoinMsg
{
	MsgHead head;
	ObjectDesc desc;
	ObjectForm form;
	char name[MAXNAMELEN];


};



//message type leave
struct LeaveMsg
{
	MsgHead head;

};



//message type change
enum ChangeType
{
	NewPlayer,
	PlayerLeave,
	NewPlayerPosition
};

struct ChangeMsg
{
	MsgHead head;
	ChangeType type;
};



//variations of ChangeMsg
struct NewPlayerMsg
{
	ChangeMsg msg;
	ObjectDesc desc;
	ObjectForm form;
	char name[MAXNAMELEN];

};

struct PlayerLeaveMsg
{
	ChangeMsg msg;
};

struct NewPlayerPositionMsg
{
	ChangeMsg msg;
	Coordinate pos;
	Coordinate dir;

};


//Message of type event
enum EventType
{
	Move

};

struct EventMsg
{
	MsgHead head;
	EventType type;


};


//variations of EventMsg

struct MoveEvent
{
	EventMsg event;
	Coordinate pos;
	Coordinate dir;

};



//messages of type TextMessage
struct TextMessageMsg
{
	MsgHead head;
	char text[1];


};