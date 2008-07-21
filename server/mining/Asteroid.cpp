
#include "Asteroid.h"
#include "../common/DestinyStructs.h"
#include "../common/gpoint.h"
#include "../common/logsys.h"
#include "../inventory/InventoryItem.h"
#include "../system/Damage.h"

using namespace Destiny;

Asteroid::Asteroid(
	SystemManager *system,
	InventoryItem *self
)
: ItemSystemEntity(self),
  m_system(system)
{
}

Asteroid::~Asteroid() {
}
	
void Asteroid::Grow() {
}

void Asteroid::ApplyDamage(Damage &d) {
	//ignore it.
	_log(MINING__DEBUG, "Asteroid %lu Ignoring %d damage from %lu", m_self->itemID(), d.GetTotal(), d.source->GetName());
}

void Asteroid::EncodeDestiny(std::vector<byte> &into) const {
	#pragma pack(1)
	struct AddBall_Asteroid {
		BallHeader head;
		DSTBALL_RIGID_Struct main;
		NameStruct name;
	};
	#pragma pack()
	int start = into.size();
	into.resize(start 
		+ sizeof(AddBall_Asteroid)
		);
	byte *ptr = &into[start];
	AddBall_Asteroid *item = (AddBall_Asteroid *) ptr;
	ptr += sizeof(AddBall_Asteroid);
	
	const GPoint &position = GetPosition();
	
	item->head.entityID = GetID();
	item->head.mode = Destiny::DSTBALL_RIGID;
	item->head.radius = GetRadius();
	item->head.x = position.x;
	item->head.y = position.y;
	item->head.z = position.z;
	item->head.sub_type = AddBallSubType_cargoContainer_asteroid;
	item->main.formationID = 0xFF;
	
	item->name.name_len = 0;
	//strcpy_fake_unicode(item->name.name, data.itemName.c_str());
}

/*PyRepDict *Asteroid::MakeSlimItem() const {
	PyRepDict *slim = new PyRepDict();
	slim->add("typeID", new PyRepInteger(m_typeID));
	slim->add("ownerID", new PyRepInteger(500021));	//no idea.
	slim->add("itemID", new PyRepInteger(m_itemID));
	return(slim);
}*/












