
#include "EVEServerPCH.h"

using namespace Destiny;

Asteroid::Asteroid(
	SystemManager *system,
	InventoryItemRef self
)
: ItemSystemEntity(self),
  m_system(system)
{
}

Asteroid::~Asteroid() {
}
	
void Asteroid::Grow() {
}

bool Asteroid::ApplyDamage(Damage &d)
{
	//ignore it.
	_log( MINING__DEBUG, "Asteroid %u: Ignoring %f damage from %s", m_self->itemID(), d.GetTotal(), d.source->GetName() );

	return false;
}

void Asteroid::EncodeDestiny( Buffer& into ) const
{
	const GPoint& position = GetPosition();
    const std::string itemName( GetName() );

    BallHeader head;
	head.entityID = GetID();
	head.mode = Destiny::DSTBALL_RIGID;
	head.radius = GetRadius();
	head.x = position.x;
	head.y = position.y;
	head.z = position.z;
	head.sub_type = AddBallSubType_cargoContainer_asteroid;
    into.Append( head );

    DSTBALL_RIGID_Struct main;
	main.formationID = 0xFF;
    into.Append( main );

    const uint8 nameLen = utf8::distance( itemName.begin(), itemName.end() );
    into.Append( nameLen );

    const Buffer::iterator<uint16> name = into.end<uint16>();
    into.ResizeAt( name, nameLen );
    utf8::utf8to16( itemName.begin(), itemName.end(), name );
}

/*PyDict *Asteroid::MakeSlimItem() const {
	PyDict *slim = new PyDict();
	slim->add("typeID", new PyInt(m_typeID));
	slim->add("ownerID", new PyInt(500021));	//no idea.
	slim->add("itemID", new PyInt(m_itemID));
	return(slim);
}*/












