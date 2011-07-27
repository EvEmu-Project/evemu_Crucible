
#ifndef ACTIVE_MODULE_PROCESSING_COMPONENT_H
#define ACTIVE_MODULE_PROCESSING_COMPONENT_H

class ActiveModuleProcessingComponent
{
public:

	ActiveModuleProcessingComponent(GenericModule * mod, ShipRef ship, ModifyShipAttributesComponent * shipAttrMod)
	: m_Stop( false ), m_Mod( mod ), m_Ship( ship ), m_ShipAttrModComp( shipAttrMod )
	{

	}

	~ActiveModuleProcessingComponent()
	{
		//nothing to do yet
	}

	void DeactivateCycle()
	{
		m_Stop = true;
	}

	//timing and verification function
	bool ShouldProcessActiveCycle()
	{
		//first check time for cycle timer

		//next check that we have enough capacitor avaiable

		//finally check if we have been told to deactivate
		
	}

	void ProcessActiveCycle()
	{
		//check for stop signal
		if(m_Stop)
			return;

		//else consume capacitor

		
		//then check if we are targeting another ship or not
	}

private:
	//internal storage and record keeping
	bool m_Stop;


	//internal access to owner
	GenericModule *m_Mod;
	ShipRef m_Ship;
	ModifyShipAttributesComponent * m_ShipAttrModComp;

};

#endif