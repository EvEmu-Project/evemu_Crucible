


class ModifyShipAttributesComponent
{
public:
	ModifyShipAttributesComponent(GenericModule * mod, ShipRef ship)
	{
		m_Mod = mod;
		m_Ship = ship;
	}

	~ModifyShipAttributesComponent()
	{

	}

	void ModifyShipAttribute(uint32 targetAttrID, uint32 sourceAttrID, EVECalculationType type)
	{
		//includes stacking calculation
		m_Ship->SetAttribute(targetAttrID, _calculateNewValue(targetAttrID, sourceAttrID, type, m_Ship->GetStackedItems(m_Mod->typeID(), m_Mod->GetModulePowerLevel())));
	}

private:

	EvilNumber _calculateNewValue(uint32 targetAttrID, uint32 sourceAttrID, EVECalculationType type, GenericModule * mods)
	{
		EvilNumber finalAttr;
		EvilNumber modVal = m_Ship->GetAttribute(targetAttrID);

		for(int i = 0; i < stacked; i++)
			


		EvilNumber finalAttr;

		//consume mods
		delete[] mods;
		mods = NULL;
	}

	GenericModule *m_Mod;
	ShipRef m_Ship;


};