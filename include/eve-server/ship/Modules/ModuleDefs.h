
#ifndef MODULE_DEFS_H
#define MODULE_DEFS_H

//more will go here
//this is to avoid include complications and multiple dependancies etc..
typedef enum ModuleCommand
{
	CMD_ERROR,
	ONLINE,
	OFFLINE,
	ACTIVATE,
	DEACTIVATE,
	OVERLOAD,   //idk if this is used yet - or what it's called :)
	DEOVERLOAD  //idk if this is used
};

#endif