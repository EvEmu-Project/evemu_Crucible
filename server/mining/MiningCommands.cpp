/*  EVEmu: EVE Online Server Emulator

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; version 2 of the License.
  
  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY except by those people which sell it, which
  are required to give you total support for your newly bought product;
  without even the implied warranty of MERCHANTABILITY or FITNESS FOR
  A PARTICULAR PURPOSE.  See the GNU General Public License for more details.
	
  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/

#include "EvemuPCH.h"

double randf() {
	return rand() / (RAND_MAX + 1.);
}

PyResult Command_roid(Client *who, CommandDB *db, PyServiceMgr *services, const Seperator &args) {
	if(!args.IsNumber(1))
		throw(PyException(MakeCustomError("Argument 1 should be an item type ID")));

	if(!args.IsNumber(2))
		throw(PyException(MakeCustomError("Argument 2 should be a radius")));

	double radius = atof(args.arg[2]);
	if(radius <= 0)
		throw(PyException(MakeCustomError("Invalid radius.")));

	if(!who->IsInSpace()) 
		throw(PyException(MakeCustomError("You must be in space to spawn things.")));

	_log(COMMAND__MESSAGE, "Roid %s of radius %f", args.arg[1], radius);

	GPoint position(who->GetPosition());
	position.x += radius + 1 + who->GetRadius();	//put it raw enough away to not push us around.
	
	GetAsteroid(who, atoi(args.arg[1]), radius, position);

	return(new PyRepString("Spawn successsfull."));
}

PyResult Command_spawnbelt(Client *who, CommandDB *db, PyServiceMgr *services, const Seperator &args) {
	if(!who->IsInSpace()) 
		throw(PyException(MakeCustomError("You must be in space to spawn things.")));

	GPoint position(who->GetPosition());
	
	double beltradius = 100000.0;
	double beltdistance = 25000.0;
	double roidradius = 600.0;
	int pcs = 160 + (randf() - 0.5) * 20;
	double beltangle = M_PI * 2 / 3;
	double R = sqrt(position.x * position.x + position.z * position.z);

	GPoint r = position * (R + beltdistance - beltradius) / R, mposition;

	double alpha;
	double phi = atan(position.x / position.z);
	if (position.z < 0) {
		phi += M_PI;
	}

	SystemManager * sys = who->System();
	std::map<double, uint32> roidDist;
	if(!db->GetRoidDist(sys->GetSystemSecurity(), roidDist)) {
		codelog(SERVICE__ERROR, "Couldn't get roid list for system security %s", sys->GetSystemSecurity());
		throw(PyException(MakeCustomError("Couldn't get roid list for system security %s", sys->GetSystemSecurity())));
	}

	for ( int i=0;i<pcs;i++ ) {
		alpha = (randf() - 0.5) * beltangle;
		mposition.x = beltradius * sin(phi + alpha) + randf() * 15 * roidradius;
		mposition.z = beltradius * cos(phi + alpha) + randf() * 15 * roidradius;
		mposition.y = position.y - r.y + randf() * 15 * roidradius;

		GetAsteroid(who, GetRoidType(randf(), roidDist), (randf() + 0.5) * roidradius, r + mposition);
	}

	return(new PyRepString("Spawn successsfull."));
}

uint32 GetRoidType(double p, const std::map<double, uint32> & roids) {
	std::map<double, uint32>::const_iterator cur, end;
	cur = roids.begin();
	end = roids.end();

	for (;cur!=end;cur++) {
		if ((*cur).first > p) return (*cur).second;
	}

	return 1230; // return Veldspar
}
void GetAsteroid(Client *who, uint32 typeID, double radius, const Ga::GaVec3 & position) {
	InventoryItem *i;
	//TODO: make item in IsUniverseAsteroid() range...
	i = who->GetServices()->item_factory->SpawnSingleton(
		typeID,
		1,	//who->GetCorporationID(),	//owner
		who->GetLocationID(),
		flagAutoFit,
		NULL,	//name
		position
		);
	if(i == NULL)
		throw(PyException(MakeCustomError("Unable to spawn item of type %lu.", typeID)));

	i->Set_radius(radius);
	i->Save();

	SystemManager *sys = who->System();
	Asteroid *new_roid = new Asteroid(sys, i);	//takes a ref.
	//TODO: check for a local asteroid belt object?
	//TODO: actually add this to the asteroid belt too...
	sys->AddEntity(new_roid);
}

PyResult Command_growbelt(Client *who, CommandDB *db, PyServiceMgr *services, const Seperator &args) {
	throw(PyException(MakeCustomError("Not implemented yet.")));
}














