

class ModifyShipAttributesComponent
{
public:
	ModifyShipAttributesComponent(InventoryItemRef item, ShipRef ship)
	{
		m_Item = item;
		m_Ship = ship;
	}

	void ModifyShipAttribute(uint32 targetAttrID, uint32 sourceAttrID, EVECalculationType type)
	{
		//need to check for stacking
		
	}

private:

	//probably wrap this into a class
	int _numberOfStackedItems()
	{

	}

	InventoryItemRef m_Item;
	ShipRef m_Ship;


};