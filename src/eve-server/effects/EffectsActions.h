/**
 * @name EffectsActions.h
 *   This file is for decoding and processing the effect's action data
 *   Copyright 2017  EVEmu Team
 *
 * @Author:    Allan
 * @date:      27 March 2017
 *
 */


#ifndef _EVE_FX_ACTION_H__
#define _EVE_FX_ACTION_H__

#include "effects/EffectsDataMgr.h"
#include "inventory/InventoryItem.h"


class SystemEntity;

class FxAction
: public Singleton<FxAction>
{
public:
    FxAction()        { /* do nothing here */ }
    ~FxAction()       { /* do nothing here */ }

    void DoAction(uint8 action, SystemEntity* pSE);

protected:

private:

};

#define sFxAct \
( FxAction::get() )

#endif  // _EVE_FX_ACTION_H__
