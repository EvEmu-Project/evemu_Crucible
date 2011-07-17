

#include "EVEServerPCH.h"

//GenericModule class definitions
#pragma region GenericModuleClass
GenericModule::GenericModule(InventoryItemRef item, ShipRef ship)
{
	m_Item = item;
	m_Ship = ship;
}

void GenericModule::Process()
{

}

void GenericModule::Online()
{

}

void GenericModule::Offline()
{

}

void GenericModule::Deactivate()
{

}

void GenericModule::Repair()
{

}

void GenericModule::Unload()
{

}


void GenericModule::Overload()
{

}

void GenericModule::DeOverload()
{

}

void GenericModule::Load()
{

}

void GenericModule::DestroyRig()
{

}

uint32 GenericModule::itemID()
{
	return m_Item->itemID();
}

EVEItemFlags GenericModule::flag()
{
	return m_Ship->flag();
}

EvilNumber GenericModule::CalculateNewAttributeValue(EvilNumber attrVal, EvilNumber attrMod, EVECalculationType type)
{
	switch(type)
	{
	case ADD :					return Add(attrVal, attrMod);  
	case SUBTRACT :				return Subtract(attrVal, attrMod);
	case DIVIDE :				return Divide(attrVal, attrMod);
	case MULTIPLY :				return Multiply(attrVal, attrMod);
	case ADD_PERCENT :			return AddPercent(attrVal, attrMod);
	case ADD_AS_PERCENT :		return AddAsPercent(attrVal, attrMod);
	case SUBTRACT_PERCENT :		return SubtractPercent(attrVal, attrMod);
	case SUBTRACT_AS_PERCENT :  return SubtractAsPercent(attrVal, attrMod);
	}

}

void GenericModule::SetAttribute(uint32 attrID, EvilNumber val)
{
	m_Item->SetAttribute(attrID, val);
}

EvilNumber GenericModule::GetAttribute(uint32 attrID)
{
	return m_Item->GetAttribute(attrID);
}

#pragma endregion