
class InventoryItem;
class SystemEntity;
class Client;


//Module inheritance definition
#pragma region Modules
//generic module base class - possibly should inherit from RefPtr...
class GenericModule
{
public:
	GenericModule(InventoryItemRef item, ShipRef ship);
	~GenericModule();

	virtual void Process();
	virtual void Offline();
	virtual void Online();
	virtual void Deactivate();
	virtual void Load();
	virtual void Unload();
	virtual void Repair();
	virtual void Overload();
	virtual void DeOverload();
	virtual void DestroyRig();

	virtual void SetAttribute(uint32 attrID, EvilNumber val);
	virtual EvilNumber GetAttribute(uint32 attrID);

	EvilNumber CalculateNewAttributeValue(EvilNumber attrVal, EvilNumber attrMod, EVECalculationType type);

	//access functions
	uint32 itemID();
	EVEItemFlags flag();


private:
	InventoryItemRef m_Item;
	ShipRef m_Ship;

};

class PassiveModule : public GenericModule
{

};

class RigModule : public PassiveModule
{

};

class SubSystemModule : public PassiveModule
{

};

#pragma endregion