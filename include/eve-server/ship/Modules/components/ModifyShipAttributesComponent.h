

#include <vector>
#include <cmath>


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
		//nothing to do yet
	}

	// implements a rudimentary but working stacking penalty.  Currently only penalizes for stacking same item, 
	// but should penalize for modifying the same attribute, with some exceptions.  These exceptions are why
	// it has not been implemented fully, as more data is needed and this is just a proof of concept.
	// No module code will have to be changed to implement the fully functional stacking penalty
	void ModifyShipAttribute(uint32 targetAttrID, uint32 sourceAttrID, EVECalculationType type)
	{
		//first we must reset the attribute in order to properly recalculate the attribute
		m_Ship->ResetAttribute(targetAttrID, false);

		//recalculate the attribute for the ship with the new modifier
		m_Ship->SetAttribute(targetAttrID, _calculateNewValue(targetAttrID, sourceAttrID, type, m_Ship->GetStackedItems(m_Mod->typeID(), m_Mod->GetModulePowerLevel())));
	}

private:

	EvilNumber _calculateNewValue(uint32 targetAttrID, uint32 sourceAttrID, EVECalculationType type, std::vector<GenericModule *> mods)
	{
		//based on http://wiki.eve-id.net/Stacking
		//EVEDev had a mistake in their formula, however I have corrected it and verified my results in excel

		//note - we must have already been put online to be on the list.  This is just a check for people
		//who may have implemented their Online function incorrectly
		if( !(m_Mod->isOnline()) )
			mods.push_back(m_Mod);

		std::vector<GenericModule *> sortedMods = _sortModules(targetAttrID, mods);

		EvilNumber finalVal;
		EvilNumber startVal = m_Ship->GetAttribute(targetAttrID);  //start value

		//iterate through all the modules, largest first
		for(int i = 0; i < mods.size(); i++)
		{
			finalVal = _calculateNewAttributeValue(mods[i]->GetAttribute(sourceAttrID), startVal, type, i );
			startVal = finalVal; //set the starting value as the calculated value
		}

			
		return finalVal;
	}

	//calculate the new value including the stacking penalty
	EvilNumber _calculateNewAttributeValue( EvilNumber sourceAttr, EvilNumber targetAttr, EVECalculationType type, int stackNumber )
	{
		EvilNumber effectiveness = exp(-pow((double)(stackNumber - 1),2)/7.1289);  //should be correct, but should be checked
		return CalculateNewAttributeValue(targetAttr, sourceAttr * effectiveness, type);
	}

	//sorts a vector of modules in descending order by arbitrary attribute.  That is array[0] > array[1]
	std::vector<GenericModule *> _sortModules(uint32 sortAttrID, std::vector<GenericModule *> mods)
	{

		//begin basic bubble sort - this needs to be checked thoroughly for bugs
		bool done = false;

		while(!done)  //check if sorted
		{
			done = true;  //assume sorted

			for( int i = 0; i != mods.size() + 1; i++)  //iterate though list
			{
				if( mods[i]->GetAttribute(sortAttrID) > mods[i+1]->GetAttribute(sortAttrID) )  //check if each pair is sorted
				{
					//it's not, so flip the values
					GenericModule * tmp = mods[i];  
					mods[i] = mods[i+1];
					mods[i] = tmp;

					done = false;  //we weren't sorted, so now go back and check if we are
				}
			}
		}

		return mods;  //return sorted list
	}


	//internal access to owner
	GenericModule *m_Mod;
	ShipRef m_Ship;

};