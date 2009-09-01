/*
	------------------------------------------------------------------------------------
	LICENSE:
	------------------------------------------------------------------------------------
	This file is part of EVEmu: EVE Online Server Emulator
	Copyright 2006 - 2008 The EVEmu Team
	For the latest information visit http://evemu.mmoforge.org
	------------------------------------------------------------------------------------
	This program is free software; you can redistribute it and/or modify it under
	the terms of the GNU Lesser General Public License as published by the Free Software
	Foundation; either version 2 of the License, or (at your option) any later
	version.

	This program is distributed in the hope that it will be useful, but WITHOUT
	ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
	FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License for more details.

	You should have received a copy of the GNU Lesser General Public License along with
	this program; if not, write to the Free Software Foundation, Inc., 59 Temple
	Place - Suite 330, Boston, MA 02111-1307, USA, or go to
	http://www.gnu.org/copyleft/lesser.txt.
	------------------------------------------------------------------------------------
	Author:		Zhur
*/

#include "EvemuPCH.h"

/* Things todo or missing
	[missing]
		* atribute dep's
		* GetDefaultValueOfAttribute
		* SESSIONCHANGEDELAY = (30 * 10000000L)
		* create a SID system (session ID system)
*/

ClientSession::ClientSession() : m_dirty(false)
{
}

void ClientSession::Set_userType(uint32 v)
{
	if(m_current.userType == 0)
		m_current.userType = new uint32;
	*(m_current.userType) = v;
	m_dirty = true;
}

void ClientSession::Clear_userType()
{
	delete m_current.userType;
	m_current.userType = 0;
	m_dirty = true;
}

void ClientSession::Set_userid(uint32 v)
{
	if(m_current.userid == 0)
		m_current.userid = new uint32;
	*(m_current.userid) = v;
	m_dirty = true;
}

void ClientSession::Clear_userid()
{
	delete m_current.userid;
	m_current.userid = 0;
	m_dirty = true;
}

void ClientSession::Set_address(const char *v)
{
	if(m_current.address == 0)
		m_current.address = new std::string;
	*(m_current.address) = v;
	m_dirty = true;
}

void ClientSession::Clear_address()
{
	delete m_current.address;
	m_current.address = 0;
	m_dirty = true;
}

void ClientSession::Set_role(uint32 v)
{
	if(m_current.role == 0)
	m_current.role = new uint32;
	*(m_current.role) = v;
	m_dirty = true;
}

void ClientSession::Clear_role()
{
	delete m_current.role;
	m_current.role = 0;
	m_dirty = true;
}

void ClientSession::Set_languageID(const char *v)
{
	if(m_current.languageID == 0)
	m_current.languageID = new std::string;
	*(m_current.languageID) = v;
	m_dirty = true;
}

void ClientSession::Clear_languageID()
{
	delete m_current.languageID;
	m_current.languageID = 0;
	m_dirty = true;
}

void ClientSession::Set_constellationid(uint32 v)
{
	if(m_current.constellationid == 0)
		m_current.constellationid = new uint32;
	*(m_current.constellationid) = v;
	m_dirty = true;
}

void ClientSession::Clear_constellationid()
{
	delete m_current.constellationid;
	m_current.constellationid = 0;
	m_dirty = true;
}

void ClientSession::Set_corpid(uint32 v)
{
	if(m_current.corpid == 0)
		m_current.corpid = new uint32;
	*(m_current.corpid) = v;
	m_dirty = true;
}

void ClientSession::Clear_corpid()
{
	delete m_current.corpid;
	m_current.corpid = 0;
	m_dirty = true;
}

void ClientSession::Set_regionid(uint32 v)
{
	if(m_current.regionid == 0)
		m_current.regionid = new uint32;
	*(m_current.regionid) = v;
	m_dirty = true;
}

void ClientSession::Clear_regionid()
{
	delete m_current.regionid;
	m_current.regionid = 0;
	m_dirty = true;
}

void ClientSession::Set_stationid(uint32 v)
{
	if(m_current.stationid == 0)
		m_current.stationid = new uint32;
	*(m_current.stationid) = v;
	m_dirty = true;
}

void ClientSession::Clear_stationid()
{
	delete m_current.stationid;
	m_current.stationid = 0;
	m_dirty = true;
}

void ClientSession::Set_solarsystemid(uint32 v)
{
	if(m_current.solarsystemid == 0)
		m_current.solarsystemid = new uint32;
	*(m_current.solarsystemid) = v;
	m_dirty = true;
}

void ClientSession::Clear_solarsystemid()
{
	delete m_current.solarsystemid;
	m_current.solarsystemid = 0;
	m_dirty = true;
}

void ClientSession::Set_locationid(uint32 v)
{
	if(m_current.locationid == 0)
		m_current.locationid = new uint32;
	*(m_current.locationid) = v;
	m_dirty = true;
}

void ClientSession::Clear_locationid()
{
	delete m_current.locationid;
	m_current.locationid = 0;
	m_dirty = true;
}

void ClientSession::Set_gangrole(uint32 v)
{
	if(m_current.gangrole == 0)
		m_current.gangrole = new uint32;
	*(m_current.gangrole) = v;
	m_dirty = true;
}

void ClientSession::Clear_gangrole()
{
	delete m_current.gangrole;
	m_current.gangrole = 0;
	m_dirty = true;
}

void ClientSession::Set_hqID(uint32 v)
{
	if(m_current.hqID == 0)
		m_current.hqID = new uint32;
	*(m_current.hqID) = v;
	m_dirty = true;
}

void ClientSession::Clear_hqID()
{
	delete m_current.hqID;
	m_current.hqID = 0;
	m_dirty = true;
}

void ClientSession::Set_solarsystemid2(uint32 v)
{
	if(m_current.solarsystemid2 == 0)
		m_current.solarsystemid2 = new uint32;
	*(m_current.solarsystemid2) = v;
	m_dirty = true;
}

void ClientSession::Clear_solarsystemid2()
{
	delete m_current.solarsystemid2;
	m_current.solarsystemid2 = 0;
	m_dirty = true;
}

void ClientSession::Set_shipid(uint32 v)
{
	if(m_current.shipid == 0)
		m_current.shipid = new uint32;
	*(m_current.shipid) = v;
	m_dirty = true;
}

void ClientSession::Clear_shipid()
{
	delete m_current.shipid;
	m_current.shipid = 0;
	m_dirty = true;
}

void ClientSession::Set_charid(uint32 v)
{
	if(m_current.charid == 0)
		m_current.charid = new uint32;
	*(m_current.charid) = v;
	m_dirty = true;
}

void ClientSession::Clear_charid()
{
	delete m_current.charid;
	m_current.charid = 0;
	m_dirty = true;
}

void ClientSession::Set_inDetention(uint32 v) {
	if(m_current.inDetention == 0)
		m_current.inDetention = new uint32;
	*(m_current.inDetention) = v;
	m_dirty = true;
}

void ClientSession::Clear_inDetention() {
	delete m_current.inDetention;
	m_current.inDetention = 0;
	m_dirty = true;
}

void ClientSession::Set_corprole(uint64 v)
{
	if(m_current.corprole == 0)
		m_current.corprole = new uint64;
	*(m_current.corprole) = v;
	m_dirty = true;
}

void ClientSession::Clear_corprole()
{
	delete m_current.corprole;
	m_current.corprole = 0;
	m_dirty = true;
}

void ClientSession::Set_rolesAtAll(uint64 v)
{
	if(m_current.rolesAtAll == 0)
		m_current.rolesAtAll = new uint64;
	*(m_current.rolesAtAll) = v;
	m_dirty = true;
}

void ClientSession::Clear_rolesAtAll()
{
	delete m_current.rolesAtAll;
	m_current.rolesAtAll = 0;
	m_dirty = true;
}

void ClientSession::Set_rolesAtBase(uint64 v)
{
	if(m_current.rolesAtBase == 0)
		m_current.rolesAtBase = new uint64;
	*(m_current.rolesAtBase) = v;
	m_dirty = true;
}

void ClientSession::Clear_rolesAtBase()
{
	delete m_current.rolesAtBase;
	m_current.rolesAtBase = 0;
	m_dirty = true;
}

void ClientSession::Set_rolesAtHQ(uint64 v)
{
	if(m_current.rolesAtHQ == 0)
		m_current.rolesAtHQ = new uint64;
	*(m_current.rolesAtHQ) = v;
	m_dirty = true;
}

void ClientSession::Clear_rolesAtHQ()
{
	delete m_current.rolesAtHQ;
	m_current.rolesAtHQ = 0;
	m_dirty = true;
}

void ClientSession::Set_rolesAtOther(uint64 v)
{
	if(m_current.rolesAtOther == 0)
		m_current.rolesAtOther = new uint64;
	*(m_current.rolesAtOther) = v;
	m_dirty = true;
}

void ClientSession::Clear_rolesAtOther()
{
	delete m_current.rolesAtOther;
	m_current.rolesAtOther = 0;
	m_dirty = true;
}


void ClientSession::EncodeChange(SessionChangeNotification &into) {

	PyTuple *t;

	t = 0;
	if(m_current.userType == 0)
	{
		if(m_last.userType == 0) { } else { do { if(log_type_info[ CLIENT__SESSION ].enabled) { log_message(CLIENT__SESSION, "Session change: " "userType" " is now NULL" ); } } while(false); t = new PyTuple(2); t->items[0] = new PyInt(*(m_last.userType)); t->items[1] = new PyNone(); delete m_last.userType; m_last.userType = 0; } } else { if(m_last.userType == 0) { do { if(log_type_info[ CLIENT__SESSION ].enabled) { log_message(CLIENT__SESSION, "Session change: " "userType" " is no longer NULL." ); } } while(false); t = new PyTuple(2); t->items[0] = new PyNone(); t->items[1] = new PyInt(*(m_current.userType)); m_last.userType = new uint32; } else if(*(m_last.userType) !=
	*(m_current.userType)) { do { if(log_type_info[ CLIENT__SESSION ].enabled) { log_message(CLIENT__SESSION, "Session change: " "userType" " changed in value." ); } } while(false); t = new PyTuple(2); t->items[0] = new PyInt(*(m_last.userType)); t->items[1] = new PyInt(*(m_current.userType)); } *(m_last.userType) =
	*(m_current.userType); }
    if(t != 0)
        into.changes.setStr("userType", t);
	t = 0;
	if(m_current.userid == 0)
	{
		if(m_last.userid == 0) { } else { do { if(log_type_info[ CLIENT__SESSION ].enabled) { log_message(CLIENT__SESSION, "Session change: " "userid" " is now NULL" ); } } while(false); t = new PyTuple(2); t->items[0] = new PyInt(*(m_last.userid)); t->items[1] = new PyNone(); delete m_last.userid; m_last.userid = 0; } } else { if(m_last.userid == 0) { do { if(log_type_info[ CLIENT__SESSION ].enabled) { log_message(CLIENT__SESSION, "Session change: " "userid" " is no longer NULL." ); } } while(false); t = new PyTuple(2); t->items[0] = new PyNone(); t->items[1] = new PyInt(*(m_current.userid)); m_last.userid = new uint32; } else if(*(m_last.userid) !=
	*(m_current.userid)) { do { if(log_type_info[ CLIENT__SESSION ].enabled) { log_message(CLIENT__SESSION, "Session change: " "userid" " changed in value." ); } } while(false); t = new PyTuple(2); t->items[0] = new PyInt(*(m_last.userid)); t->items[1] = new PyInt(*(m_current.userid)); } *(m_last.userid) =
	*(m_current.userid); }
    if(t != 0)
        into.changes.setStr("userid", t);
	t = 0;
	if(m_current.address == 0)
	{
		if(m_last.address == 0) { } else { do { if(log_type_info[ CLIENT__SESSION ].enabled) { log_message(CLIENT__SESSION, "Session change: " "address" " is now NULL" ); } } while(false); t = new PyTuple(2); t->items[0] = new PyString(*(m_last.address)); t->items[1] = new PyNone(); delete m_last.address; m_last.address = 0; } } else { if(m_last.address == 0) { do { if(log_type_info[ CLIENT__SESSION ].enabled) { log_message(CLIENT__SESSION, "Session change: " "address" " is no longer NULL." ); } } while(false); t = new PyTuple(2); t->items[0] = new PyNone(); t->items[1] = new PyString(*(m_current.address)); m_last.address = new std::string; } else if(*(m_last.address) !=
	*(m_current.address)) { do { if(log_type_info[ CLIENT__SESSION ].enabled) { log_message(CLIENT__SESSION, "Session change: " "address" " changed in value." ); } } while(false); t = new PyTuple(2); t->items[0] = new PyString(*(m_last.address)); t->items[1] = new PyString(*(m_current.address)); } *(m_last.address) =
	*(m_current.address); }
    if(t != 0)
        into.changes.setStr("address", t);
	t = 0;
	if(m_current.role == 0)
	{
		if(m_last.role == 0) { } else { do { if(log_type_info[ CLIENT__SESSION ].enabled) { log_message(CLIENT__SESSION, "Session change: " "role" " is now NULL" ); } } while(false); t = new PyTuple(2); t->items[0] = new PyInt(*(m_last.role)); t->items[1] = new PyNone(); delete m_last.role; m_last.role = 0; } } else { if(m_last.role == 0) { do { if(log_type_info[ CLIENT__SESSION ].enabled) { log_message(CLIENT__SESSION, "Session change: " "role" " is no longer NULL." ); } } while(false); t = new PyTuple(2); t->items[0] = new PyNone(); t->items[1] = new PyInt(*(m_current.role)); m_last.role = new uint32; } else if(*(m_last.role) !=
	*(m_current.role)) { do { if(log_type_info[ CLIENT__SESSION ].enabled) { log_message(CLIENT__SESSION, "Session change: " "role" " changed in value." ); } } while(false); t = new PyTuple(2); t->items[0] = new PyInt(*(m_last.role)); t->items[1] = new PyInt(*(m_current.role)); } *(m_last.role) =
	*(m_current.role); }
    if(t != 0)
        into.changes.setStr("role", t);
	t = 0;
	if(m_current.languageID == 0)
	{
		if(m_last.languageID == 0) { } else { do { if(log_type_info[ CLIENT__SESSION ].enabled) { log_message(CLIENT__SESSION, "Session change: " "languageID" " is now NULL" ); } } while(false); t = new PyTuple(2); t->items[0] = new PyString(*(m_last.languageID)); t->items[1] = new PyNone(); delete m_last.languageID; m_last.languageID = 0; } } else { if(m_last.languageID == 0) { do { if(log_type_info[ CLIENT__SESSION ].enabled) { log_message(CLIENT__SESSION, "Session change: " "languageID" " is no longer NULL." ); } } while(false); t = new PyTuple(2); t->items[0] = new PyNone(); t->items[1] = new PyString(*(m_current.languageID)); m_last.languageID = new std::string; } else if(*(m_last.languageID) !=
	*(m_current.languageID)) { do { if(log_type_info[ CLIENT__SESSION ].enabled) { log_message(CLIENT__SESSION, "Session change: " "languageID" " changed in value." ); } } while(false); t = new PyTuple(2); t->items[0] = new PyString(*(m_last.languageID)); t->items[1] = new PyString(*(m_current.languageID)); } *(m_last.languageID) =
	*(m_current.languageID); }
    if(t != 0)
        into.changes.setStr("languageID", t);
	t = 0;
	if(m_current.constellationid == 0)
	{
		if(m_last.constellationid == 0) { } else { do { if(log_type_info[ CLIENT__SESSION ].enabled) { log_message(CLIENT__SESSION, "Session change: " "constellationid" " is now NULL" ); } } while(false); t = new PyTuple(2); t->items[0] = new PyInt(*(m_last.constellationid)); t->items[1] = new PyNone(); delete m_last.constellationid; m_last.constellationid = 0; } } else { if(m_last.constellationid == 0) { do { if(log_type_info[ CLIENT__SESSION ].enabled) { log_message(CLIENT__SESSION, "Session change: " "constellationid" " is no longer NULL." ); } } while(false); t = new PyTuple(2); t->items[0] = new PyNone(); t->items[1] = new PyInt(*(m_current.constellationid)); m_last.constellationid = new uint32; } else if(*(m_last.constellationid) !=
	*(m_current.constellationid)) { do { if(log_type_info[ CLIENT__SESSION ].enabled) { log_message(CLIENT__SESSION, "Session change: " "constellationid" " changed in value." ); } } while(false); t = new PyTuple(2); t->items[0] = new PyInt(*(m_last.constellationid)); t->items[1] = new PyInt(*(m_current.constellationid)); } *(m_last.constellationid) =
	*(m_current.constellationid); }
    if(t != 0)
        into.changes.setStr("constellationid", t);
	t = 0;
	if(m_current.corpid == 0)
	{
		if(m_last.corpid == 0) { } else { do { if(log_type_info[ CLIENT__SESSION ].enabled) { log_message(CLIENT__SESSION, "Session change: " "corpid" " is now NULL" ); } } while(false); t = new PyTuple(2); t->items[0] = new PyInt(*(m_last.corpid)); t->items[1] = new PyNone(); delete m_last.corpid; m_last.corpid = 0; } } else { if(m_last.corpid == 0) { do { if(log_type_info[ CLIENT__SESSION ].enabled) { log_message(CLIENT__SESSION, "Session change: " "corpid" " is no longer NULL." ); } } while(false); t = new PyTuple(2); t->items[0] = new PyNone(); t->items[1] = new PyInt(*(m_current.corpid)); m_last.corpid = new uint32; } else if(*(m_last.corpid) !=
	*(m_current.corpid)) { do { if(log_type_info[ CLIENT__SESSION ].enabled) { log_message(CLIENT__SESSION, "Session change: " "corpid" " changed in value." ); } } while(false); t = new PyTuple(2); t->items[0] = new PyInt(*(m_last.corpid)); t->items[1] = new PyInt(*(m_current.corpid)); } *(m_last.corpid) =
	*(m_current.corpid); }
	if(t != 0)
		into.changes.setStr("corpid", t);
	t = 0;
	if(m_current.regionid == 0)
	{
		if(m_last.regionid == 0) { } else { do { if(log_type_info[ CLIENT__SESSION ].enabled) { log_message(CLIENT__SESSION, "Session change: " "regionid" " is now NULL" ); } } while(false); t = new PyTuple(2); t->items[0] = new PyInt(*(m_last.regionid)); t->items[1] = new PyNone(); delete m_last.regionid; m_last.regionid = 0; } } else { if(m_last.regionid == 0) { do { if(log_type_info[ CLIENT__SESSION ].enabled) { log_message(CLIENT__SESSION, "Session change: " "regionid" " is no longer NULL." ); } } while(false); t = new PyTuple(2); t->items[0] = new PyNone(); t->items[1] = new PyInt(*(m_current.regionid)); m_last.regionid = new uint32; } else if(*(m_last.regionid) !=
	*(m_current.regionid)) { do { if(log_type_info[ CLIENT__SESSION ].enabled) { log_message(CLIENT__SESSION, "Session change: " "regionid" " changed in value." ); } } while(false); t = new PyTuple(2); t->items[0] = new PyInt(*(m_last.regionid)); t->items[1] = new PyInt(*(m_current.regionid)); } *(m_last.regionid) =
	*(m_current.regionid); }
	if(t != 0)
		into.changes.setStr("regionid", t);
	t = 0;
	if(m_current.stationid == 0)
	{
		if(m_last.stationid == 0) { } else { do { if(log_type_info[ CLIENT__SESSION ].enabled) { log_message(CLIENT__SESSION, "Session change: " "stationid" " is now NULL" ); } } while(false); t = new PyTuple(2); t->items[0] = new PyInt(*(m_last.stationid)); t->items[1] = new PyNone(); delete m_last.stationid; m_last.stationid = 0; } } else { if(m_last.stationid == 0) { do { if(log_type_info[ CLIENT__SESSION ].enabled) { log_message(CLIENT__SESSION, "Session change: " "stationid" " is no longer NULL." ); } } while(false); t = new PyTuple(2); t->items[0] = new PyNone(); t->items[1] = new PyInt(*(m_current.stationid)); m_last.stationid = new uint32; } else if(*(m_last.stationid) !=
	*(m_current.stationid)) { do { if(log_type_info[ CLIENT__SESSION ].enabled) { log_message(CLIENT__SESSION, "Session change: " "stationid" " changed in value." ); } } while(false); t = new PyTuple(2); t->items[0] = new PyInt(*(m_last.stationid)); t->items[1] = new PyInt(*(m_current.stationid)); } *(m_last.stationid) =
	*(m_current.stationid); }
	if(t != 0)
		into.changes.setStr("stationid", t);
	t = 0;
	if(m_current.solarsystemid == 0)
	{
		if(m_last.solarsystemid == 0) { } else { do { if(log_type_info[ CLIENT__SESSION ].enabled) { log_message(CLIENT__SESSION, "Session change: " "solarsystemid" " is now NULL" ); } } while(false); t = new PyTuple(2); t->items[0] = new PyInt(*(m_last.solarsystemid)); t->items[1] = new PyNone(); delete m_last.solarsystemid; m_last.solarsystemid = 0; } } else { if(m_last.solarsystemid == 0) { do { if(log_type_info[ CLIENT__SESSION ].enabled) { log_message(CLIENT__SESSION, "Session change: " "solarsystemid" " is no longer NULL." ); } } while(false); t = new PyTuple(2); t->items[0] = new PyNone(); t->items[1] = new PyInt(*(m_current.solarsystemid)); m_last.solarsystemid = new uint32; } else if(*(m_last.solarsystemid) !=
	*(m_current.solarsystemid)) { do { if(log_type_info[ CLIENT__SESSION ].enabled) { log_message(CLIENT__SESSION, "Session change: " "solarsystemid" " changed in value." ); } } while(false); t = new PyTuple(2); t->items[0] = new PyInt(*(m_last.solarsystemid)); t->items[1] = new PyInt(*(m_current.solarsystemid)); } *(m_last.solarsystemid) =
	*(m_current.solarsystemid); }
	if(t != 0)
		into.changes.setStr("solarsystemid", t);
	t = 0;
	if(m_current.locationid == 0)
	{
		if(m_last.locationid == 0) { } else { do { if(log_type_info[ CLIENT__SESSION ].enabled) { log_message(CLIENT__SESSION, "Session change: " "locationid" " is now NULL" ); } } while(false); t = new PyTuple(2); t->items[0] = new PyInt(*(m_last.locationid)); t->items[1] = new PyNone(); delete m_last.locationid; m_last.locationid = 0; } } else { if(m_last.locationid == 0) { do { if(log_type_info[ CLIENT__SESSION ].enabled) { log_message(CLIENT__SESSION, "Session change: " "locationid" " is no longer NULL." ); } } while(false); t = new PyTuple(2); t->items[0] = new PyNone(); t->items[1] = new PyInt(*(m_current.locationid)); m_last.locationid = new uint32; } else if(*(m_last.locationid) !=
	*(m_current.locationid)) { do { if(log_type_info[ CLIENT__SESSION ].enabled) { log_message(CLIENT__SESSION, "Session change: " "locationid" " changed in value." ); } } while(false); t = new PyTuple(2); t->items[0] = new PyInt(*(m_last.locationid)); t->items[1] = new PyInt(*(m_current.locationid)); } *(m_last.locationid) =
	*(m_current.locationid); }
	if(t != 0)
		into.changes.setStr("locationid", t);
	t = 0;
	if(m_current.gangrole == 0)
	{
		if(m_last.gangrole == 0) { } else { do { if(log_type_info[ CLIENT__SESSION ].enabled) { log_message(CLIENT__SESSION, "Session change: " "gangrole" " is now NULL" ); } } while(false); t = new PyTuple(2); t->items[0] = new PyInt(*(m_last.gangrole)); t->items[1] = new PyNone(); delete m_last.gangrole; m_last.gangrole = 0; } } else { if(m_last.gangrole == 0) { do { if(log_type_info[ CLIENT__SESSION ].enabled) { log_message(CLIENT__SESSION, "Session change: " "gangrole" " is no longer NULL." ); } } while(false); t = new PyTuple(2); t->items[0] = new PyNone(); t->items[1] = new PyInt(*(m_current.gangrole)); m_last.gangrole = new uint32; } else if(*(m_last.gangrole) !=
	*(m_current.gangrole)) { do { if(log_type_info[ CLIENT__SESSION ].enabled) { log_message(CLIENT__SESSION, "Session change: " "gangrole" " changed in value." ); } } while(false); t = new PyTuple(2); t->items[0] = new PyInt(*(m_last.gangrole)); t->items[1] = new PyInt(*(m_current.gangrole)); } *(m_last.gangrole) =
	*(m_current.gangrole); }
	if(t != 0)
		into.changes.setStr("gangrole", t);
	t = 0;
	if(m_current.hqID == 0)
	{
		if(m_last.hqID == 0) { } else { do { if(log_type_info[ CLIENT__SESSION ].enabled) { log_message(CLIENT__SESSION, "Session change: " "hqID" " is now NULL" ); } } while(false); t = new PyTuple(2); t->items[0] = new PyInt(*(m_last.hqID)); t->items[1] = new PyNone(); delete m_last.hqID; m_last.hqID = 0; } } else { if(m_last.hqID == 0) { do { if(log_type_info[ CLIENT__SESSION ].enabled) { log_message(CLIENT__SESSION, "Session change: " "hqID" " is no longer NULL." ); } } while(false); t = new PyTuple(2); t->items[0] = new PyNone(); t->items[1] = new PyInt(*(m_current.hqID)); m_last.hqID = new uint32; } else if(*(m_last.hqID) !=
	*(m_current.hqID)) { do { if(log_type_info[ CLIENT__SESSION ].enabled) { log_message(CLIENT__SESSION, "Session change: " "hqID" " changed in value." ); } } while(false); t = new PyTuple(2); t->items[0] = new PyInt(*(m_last.hqID)); t->items[1] = new PyInt(*(m_current.hqID)); } *(m_last.hqID) =
	*(m_current.hqID); }
	if(t != 0)
		into.changes.setStr("hqID", t);
	t = 0;
	if(m_current.solarsystemid2 == 0)
	{
		if(m_last.solarsystemid2 == 0) { } else { do { if(log_type_info[ CLIENT__SESSION ].enabled) { log_message(CLIENT__SESSION, "Session change: " "solarsystemid2" " is now NULL" ); } } while(false); t = new PyTuple(2); t->items[0] = new PyInt(*(m_last.solarsystemid2)); t->items[1] = new PyNone(); delete m_last.solarsystemid2; m_last.solarsystemid2 = 0; } } else { if(m_last.solarsystemid2 == 0) { do { if(log_type_info[ CLIENT__SESSION ].enabled) { log_message(CLIENT__SESSION, "Session change: " "solarsystemid2" " is no longer NULL." ); } } while(false); t = new PyTuple(2); t->items[0] = new PyNone(); t->items[1] = new PyInt(*(m_current.solarsystemid2)); m_last.solarsystemid2 = new uint32; } else if(*(m_last.solarsystemid2) !=
	*(m_current.solarsystemid2)) { do { if(log_type_info[ CLIENT__SESSION ].enabled) { log_message(CLIENT__SESSION, "Session change: " "solarsystemid2" " changed in value." ); } } while(false); t = new PyTuple(2); t->items[0] = new PyInt(*(m_last.solarsystemid2)); t->items[1] = new PyInt(*(m_current.solarsystemid2)); } *(m_last.solarsystemid2) =
	*(m_current.solarsystemid2); }
	if(t != 0)
		into.changes.setStr("solarsystemid2", t);
	t = 0;
	if(m_current.shipid == 0)
	{
		if(m_last.shipid == 0) { } else { do { if(log_type_info[ CLIENT__SESSION ].enabled) { log_message(CLIENT__SESSION, "Session change: " "shipid" " is now NULL" ); } } while(false); t = new PyTuple(2); t->items[0] = new PyInt(*(m_last.shipid)); t->items[1] = new PyNone(); delete m_last.shipid; m_last.shipid = 0; } } else { if(m_last.shipid == 0) { do { if(log_type_info[ CLIENT__SESSION ].enabled) { log_message(CLIENT__SESSION, "Session change: " "shipid" " is no longer NULL." ); } } while(false); t = new PyTuple(2); t->items[0] = new PyNone(); t->items[1] = new PyInt(*(m_current.shipid)); m_last.shipid = new uint32; } else if(*(m_last.shipid) !=
	*(m_current.shipid)) { do { if(log_type_info[ CLIENT__SESSION ].enabled) { log_message(CLIENT__SESSION, "Session change: " "shipid" " changed in value." ); } } while(false); t = new PyTuple(2); t->items[0] = new PyInt(*(m_last.shipid)); t->items[1] = new PyInt(*(m_current.shipid)); } *(m_last.shipid) =
	*(m_current.shipid); }
	if(t != 0)
		into.changes.setStr("shipid", t);
	t = 0;
	if(m_current.charid == 0)
	{
		if(m_last.charid == 0) { } else { do { if(log_type_info[ CLIENT__SESSION ].enabled) { log_message(CLIENT__SESSION, "Session change: " "charid" " is now NULL" ); } } while(false); t = new PyTuple(2); t->items[0] = new PyInt(*(m_last.charid)); t->items[1] = new PyNone(); delete m_last.charid; m_last.charid = 0; } } else { if(m_last.charid == 0) { do { if(log_type_info[ CLIENT__SESSION ].enabled) { log_message(CLIENT__SESSION, "Session change: " "charid" " is no longer NULL." ); } } while(false); t = new PyTuple(2); t->items[0] = new PyNone(); t->items[1] = new PyInt(*(m_current.charid)); m_last.charid = new uint32; } else if(*(m_last.charid) !=
	*(m_current.charid)) { do { if(log_type_info[ CLIENT__SESSION ].enabled) { log_message(CLIENT__SESSION, "Session change: " "charid" " changed in value." ); } } while(false); t = new PyTuple(2); t->items[0] = new PyInt(*(m_last.charid)); t->items[1] = new PyInt(*(m_current.charid)); } *(m_last.charid) =
	*(m_current.charid); }
	if(t != 0)
		into.changes.setStr("charid", t);
	t = 0;
	if(m_current.inDetention == 0)
	{
		if(m_last.inDetention == 0) { } else { do { if(log_type_info[ CLIENT__SESSION ].enabled) { log_message(CLIENT__SESSION, "Session change: " "inDetention" " is now NULL" ); } } while(false); t = new PyTuple(2); t->items[0] = new PyInt(*(m_last.inDetention)); t->items[1] = new PyNone(); delete m_last.inDetention; m_last.inDetention = 0; } } else { if(m_last.inDetention == 0) { do { if(log_type_info[ CLIENT__SESSION ].enabled) { log_message(CLIENT__SESSION, "Session change: " "inDetention" " is no longer NULL." ); } } while(false); t = new PyTuple(2); t->items[0] = new PyNone(); t->items[1] = new PyInt(*(m_current.inDetention)); m_last.inDetention = new uint32; } else if(*(m_last.inDetention) !=
	*(m_current.inDetention)) { do { if(log_type_info[ CLIENT__SESSION ].enabled) { log_message(CLIENT__SESSION, "Session change: " "inDetention" " changed in value." ); } } while(false); t = new PyTuple(2); t->items[0] = new PyInt(*(m_last.inDetention)); t->items[1] = new PyInt(*(m_current.inDetention)); } *(m_last.inDetention) =
	*(m_current.inDetention); }
	if(t != 0)
		into.changes.setStr("inDetention", t);
	t = 0;
	if(m_current.corprole == 0)
	{
		if(m_last.corprole == 0) { } else { do { if(log_type_info[ CLIENT__SESSION ].enabled) { log_message(CLIENT__SESSION, "Session change: " "corprole" " is now NULL" ); } } while(false); t = new PyTuple(2); t->items[0] = new PyInt(*(m_last.corprole)); t->items[1] = new PyNone(); delete m_last.corprole; m_last.corprole = 0; } } else { if(m_last.corprole == 0) { do { if(log_type_info[ CLIENT__SESSION ].enabled) { log_message(CLIENT__SESSION, "Session change: " "corprole" " is no longer NULL." ); } } while(false); t = new PyTuple(2); t->items[0] = new PyNone(); t->items[1] = new PyInt(*(m_current.corprole)); m_last.corprole = new uint64; } else if(*(m_last.corprole) !=
	*(m_current.corprole)) { do { if(log_type_info[ CLIENT__SESSION ].enabled) { log_message(CLIENT__SESSION, "Session change: " "corprole" " changed in value." ); } } while(false); t = new PyTuple(2); t->items[0] = new PyInt(*(m_last.corprole)); t->items[1] = new PyInt(*(m_current.corprole)); } *(m_last.corprole) =
	*(m_current.corprole); }
	if(t != 0)
		into.changes.setStr("corprole", t);
	t = 0;
	if(m_current.rolesAtAll == 0)
	{
		if(m_last.rolesAtAll == 0) { } else { do { if(log_type_info[ CLIENT__SESSION ].enabled) { log_message(CLIENT__SESSION, "Session change: " "rolesAtAll" " is now NULL" ); } } while(false); t = new PyTuple(2); t->items[0] = new PyInt(*(m_last.rolesAtAll)); t->items[1] = new PyNone(); delete m_last.rolesAtAll; m_last.rolesAtAll = 0; } } else { if(m_last.rolesAtAll == 0) { do { if(log_type_info[ CLIENT__SESSION ].enabled) { log_message(CLIENT__SESSION, "Session change: " "rolesAtAll" " is no longer NULL." ); } } while(false); t = new PyTuple(2); t->items[0] = new PyNone(); t->items[1] = new PyInt(*(m_current.rolesAtAll)); m_last.rolesAtAll = new uint64; } else if(*(m_last.rolesAtAll) !=
	*(m_current.rolesAtAll)) { do { if(log_type_info[ CLIENT__SESSION ].enabled) { log_message(CLIENT__SESSION, "Session change: " "rolesAtAll" " changed in value." ); } } while(false); t = new PyTuple(2); t->items[0] = new PyInt(*(m_last.rolesAtAll)); t->items[1] = new PyInt(*(m_current.rolesAtAll)); } *(m_last.rolesAtAll) =
	*(m_current.rolesAtAll); }
	if(t != 0)
		into.changes.setStr("rolesAtAll", t);
	t = 0;
	if(m_current.rolesAtBase == 0)
	{
		if(m_last.rolesAtBase == 0) { } else { do { if(log_type_info[ CLIENT__SESSION ].enabled) { log_message(CLIENT__SESSION, "Session change: " "rolesAtBase" " is now NULL" ); } } while(false); t = new PyTuple(2); t->items[0] = new PyInt(*(m_last.rolesAtBase)); t->items[1] = new PyNone(); delete m_last.rolesAtBase; m_last.rolesAtBase = 0; } } else { if(m_last.rolesAtBase == 0) { do { if(log_type_info[ CLIENT__SESSION ].enabled) { log_message(CLIENT__SESSION, "Session change: " "rolesAtBase" " is no longer NULL." ); } } while(false); t = new PyTuple(2); t->items[0] = new PyNone(); t->items[1] = new PyInt(*(m_current.rolesAtBase)); m_last.rolesAtBase = new uint64; } else if(*(m_last.rolesAtBase) !=
	*(m_current.rolesAtBase)) { do { if(log_type_info[ CLIENT__SESSION ].enabled) { log_message(CLIENT__SESSION, "Session change: " "rolesAtBase" " changed in value." ); } } while(false); t = new PyTuple(2); t->items[0] = new PyInt(*(m_last.rolesAtBase)); t->items[1] = new PyInt(*(m_current.rolesAtBase)); } *(m_last.rolesAtBase) =
	*(m_current.rolesAtBase); }
	if(t != 0)
		into.changes.setStr("rolesAtBase", t);
	t = 0;
	if(m_current.rolesAtHQ == 0)
	{
		if(m_last.rolesAtHQ == 0) { } else { do { if(log_type_info[ CLIENT__SESSION ].enabled) { log_message(CLIENT__SESSION, "Session change: " "rolesAtHQ" " is now NULL" ); } } while(false); t = new PyTuple(2); t->items[0] = new PyInt(*(m_last.rolesAtHQ)); t->items[1] = new PyNone(); delete m_last.rolesAtHQ; m_last.rolesAtHQ = 0; } } else { if(m_last.rolesAtHQ == 0) { do { if(log_type_info[ CLIENT__SESSION ].enabled) { log_message(CLIENT__SESSION, "Session change: " "rolesAtHQ" " is no longer NULL." ); } } while(false); t = new PyTuple(2); t->items[0] = new PyNone(); t->items[1] = new PyInt(*(m_current.rolesAtHQ)); m_last.rolesAtHQ = new uint64; } else if(*(m_last.rolesAtHQ) !=
	*(m_current.rolesAtHQ)) { do { if(log_type_info[ CLIENT__SESSION ].enabled) { log_message(CLIENT__SESSION, "Session change: " "rolesAtHQ" " changed in value." ); } } while(false); t = new PyTuple(2); t->items[0] = new PyInt(*(m_last.rolesAtHQ)); t->items[1] = new PyInt(*(m_current.rolesAtHQ)); } *(m_last.rolesAtHQ) =
	*(m_current.rolesAtHQ); }
	if(t != 0)
		into.changes.setStr("rolesAtHQ", t);
	t = 0;
	if(m_current.rolesAtOther == 0)
	{
		if(m_last.rolesAtOther == 0)
		{
		}
		else
		{
		    do
		    {
		        if(log_type_info[ CLIENT__SESSION ].enabled)
                {
                    log_message(CLIENT__SESSION, "Session change: " "rolesAtOther" " is now NULL" );
                }
            } while(false);
            t = new PyTuple(2);
            t->items[0] = new PyInt(*(m_last.rolesAtOther));
            t->items[1] = new PyNone();
            delete m_last.rolesAtOther;
            m_last.rolesAtOther = 0;
        }
    }
    else
    {
        if(m_last.rolesAtOther == 0)
        {
            do
            {
                if(log_type_info[ CLIENT__SESSION ].enabled)
                {
                    log_message(CLIENT__SESSION, "Session change: " "rolesAtOther" " is no longer NULL." );
                }
            } while(false);
            t = new PyTuple(2);
            t->items[0] = new PyNone();
            t->items[1] = new PyInt(*(m_current.rolesAtOther));
            m_last.rolesAtOther = new uint64;
        }
        else if(*(m_last.rolesAtOther) != *(m_current.rolesAtOther))
        {
            do
            {
                if(log_type_info[ CLIENT__SESSION ].enabled)
                {
                    log_message(CLIENT__SESSION, "Session change: " "rolesAtOther" " changed in value." );
                }
            } while(false);
            t = new PyTuple(2);
            t->items[0] = new PyInt(*(m_last.rolesAtOther));
            t->items[1] = new PyInt(*(m_current.rolesAtOther));
        }
        *(m_last.rolesAtOther) = *(m_current.rolesAtOther);
    }
	if(t != 0)
		into.changes.setStr("rolesAtOther", t);

	m_dirty = false;
}

void ClientSession::Dump(LogType type) const {
	if(!log_type_info[ type ].enabled)
		return;	

	do { if(log_type_info[ type ].enabled) { log_message(type, "Session Dump:" ); } } while(false);

	if(m_current.userType == 0) { do { if(log_type_info[ type ].enabled) { log_message(type, "    " "userType" ": NULL" ); } } while(false); } else { do { if(log_type_info[ type ].enabled) { log_message(type, "    " "userType" ": %u",*(m_current.userType)); } } while(false); }
	if(m_current.userid == 0) { do { if(log_type_info[ type ].enabled) { log_message(type, "    " "userid" ": NULL" ); } } while(false); } else { do { if(log_type_info[ type ].enabled) { log_message(type, "    " "userid" ": %u",*(m_current.userid)); } } while(false); }
	if(m_current.address == 0) { do { if(log_type_info[ type ].enabled) { log_message(type, "    " "address" ": NULL" ); } } while(false); } else { do { if(log_type_info[ type ].enabled) { log_message(type, "    " "address" ": '%s'",m_current.address->c_str()); } } while(false); }
	if(m_current.role == 0) { do { if(log_type_info[ type ].enabled) { log_message(type, "    " "role" ": NULL" ); } } while(false); } else { do { if(log_type_info[ type ].enabled) { log_message(type, "    " "role" ": %u",*(m_current.role)); } } while(false); }
	if(m_current.languageID == 0) { do { if(log_type_info[ type ].enabled) { log_message(type, "    " "languageID" ": NULL" ); } } while(false); } else { do { if(log_type_info[ type ].enabled) { log_message(type, "    " "languageID" ": '%s'",m_current.languageID->c_str()); } } while(false); }
	if(m_current.constellationid == 0) { do { if(log_type_info[ type ].enabled) { log_message(type, "    " "constellationid" ": NULL" ); } } while(false); } else { do { if(log_type_info[ type ].enabled) { log_message(type, "    " "constellationid" ": %u",*(m_current.constellationid)); } } while(false); }
	if(m_current.corpid == 0) { do { if(log_type_info[ type ].enabled) { log_message(type, "    " "corpid" ": NULL" ); } } while(false); } else { do { if(log_type_info[ type ].enabled) { log_message(type, "    " "corpid" ": %u",*(m_current.corpid)); } } while(false); }
	if(m_current.regionid == 0) { do { if(log_type_info[ type ].enabled) { log_message(type, "    " "regionid" ": NULL" ); } } while(false); } else { do { if(log_type_info[ type ].enabled) { log_message(type, "    " "regionid" ": %u",*(m_current.regionid)); } } while(false); }
	if(m_current.stationid == 0) { do { if(log_type_info[ type ].enabled) { log_message(type, "    " "stationid" ": NULL" ); } } while(false); } else { do { if(log_type_info[ type ].enabled) { log_message(type, "    " "stationid" ": %u",*(m_current.stationid)); } } while(false); }
	if(m_current.solarsystemid == 0) { do { if(log_type_info[ type ].enabled) { log_message(type, "    " "solarsystemid" ": NULL" ); } } while(false); } else { do { if(log_type_info[ type ].enabled) { log_message(type, "    " "solarsystemid" ": %u",*(m_current.solarsystemid)); } } while(false); }
	if(m_current.locationid == 0) { do { if(log_type_info[ type ].enabled) { log_message(type, "    " "locationid" ": NULL" ); } } while(false); } else { do { if(log_type_info[ type ].enabled) { log_message(type, "    " "locationid" ": %u",*(m_current.locationid)); } } while(false); }
	if(m_current.gangrole == 0) { do { if(log_type_info[ type ].enabled) { log_message(type, "    " "gangrole" ": NULL" ); } } while(false); } else { do { if(log_type_info[ type ].enabled) { log_message(type, "    " "gangrole" ": %u",*(m_current.gangrole)); } } while(false); }
	if(m_current.hqID == 0) { do { if(log_type_info[ type ].enabled) { log_message(type, "    " "hqID" ": NULL" ); } } while(false); } else { do { if(log_type_info[ type ].enabled) { log_message(type, "    " "hqID" ": %u",*(m_current.hqID)); } } while(false); }
	if(m_current.solarsystemid2 == 0) { do { if(log_type_info[ type ].enabled) { log_message(type, "    " "solarsystemid2" ": NULL" ); } } while(false); } else { do { if(log_type_info[ type ].enabled) { log_message(type, "    " "solarsystemid2" ": %u",*(m_current.solarsystemid2)); } } while(false); }
	if(m_current.shipid == 0) { do { if(log_type_info[ type ].enabled) { log_message(type, "    " "shipid" ": NULL" ); } } while(false); } else { do { if(log_type_info[ type ].enabled) { log_message(type, "    " "shipid" ": %u",*(m_current.shipid)); } } while(false); }
	if(m_current.charid == 0) { do { if(log_type_info[ type ].enabled) { log_message(type, "    " "charid" ": NULL" ); } } while(false); } else { do { if(log_type_info[ type ].enabled) { log_message(type, "    " "charid" ": %u",*(m_current.charid)); } } while(false); }
	if(m_current.inDetention == 0) { do { if(log_type_info[ type ].enabled) { log_message(type, "    " "inDetention" ": NULL" ); } } while(false); } else { do { if(log_type_info[ type ].enabled) { log_message(type, "    " "inDetention" ": %u",*(m_current.inDetention)); } } while(false); }
	if(m_current.corprole == 0) { do { if(log_type_info[ type ].enabled) { log_message(type, "    " "corprole" ": NULL" ); } } while(false); } else { do { if(log_type_info[ type ].enabled) { log_message(type, "    " "corprole" ": %I64u",*(m_current.corprole)); } } while(false); }
	if(m_current.rolesAtAll == 0) { do { if(log_type_info[ type ].enabled) { log_message(type, "    " "rolesAtAll" ": NULL" ); } } while(false); } else { do { if(log_type_info[ type ].enabled) { log_message(type, "    " "rolesAtAll" ": %I64u",*(m_current.rolesAtAll)); } } while(false); }
	if(m_current.rolesAtBase == 0) { do { if(log_type_info[ type ].enabled) { log_message(type, "    " "rolesAtBase" ": NULL" ); } } while(false); } else { do { if(log_type_info[ type ].enabled) { log_message(type, "    " "rolesAtBase" ": %I64u",*(m_current.rolesAtBase)); } } while(false); }
	if(m_current.rolesAtHQ == 0) { do { if(log_type_info[ type ].enabled) { log_message(type, "    " "rolesAtHQ" ": NULL" ); } } while(false); } else { do { if(log_type_info[ type ].enabled) { log_message(type, "    " "rolesAtHQ" ": %I64u",*(m_current.rolesAtHQ)); } } while(false); }
	if(m_current.rolesAtOther == 0) { do { if(log_type_info[ type ].enabled) { log_message(type, "    " "rolesAtOther" ": NULL" ); } } while(false); } else { do { if(log_type_info[ type ].enabled) { log_message(type, "    " "rolesAtOther" ": %I64u",*(m_current.rolesAtOther)); } } while(false); }
}

ClientSession::SessionData::SessionData() {

	userType = NULL;
	userid = NULL;
	address = NULL;
	role = NULL;
	languageID = NULL;
	constellationid = NULL;
	corpid = NULL;
	regionid = NULL;
	stationid = NULL;
	solarsystemid = NULL;
	locationid = NULL;
	gangrole = NULL;
	hqID = NULL;
	solarsystemid2 = NULL;
	shipid = NULL;
	charid = NULL;
	corprole = NULL;
	rolesAtAll = NULL;
	rolesAtBase = NULL;
	rolesAtHQ = NULL;
	rolesAtOther = NULL;
	inDetention = NULL;
}

ClientSession::SessionData::~SessionData()
{
	delete userType;
	delete userid;
	delete address;
	delete role;
	delete languageID;
	delete constellationid;
	delete corpid;
	delete regionid;
	delete stationid;
	delete solarsystemid;
	delete locationid;
	delete gangrole;
	delete hqID;
	delete solarsystemid2;
	delete shipid;
	delete charid;
	delete corprole;
	delete rolesAtAll;
	delete rolesAtBase;
	delete rolesAtHQ;
	delete rolesAtOther;
	delete inDetention;
}
