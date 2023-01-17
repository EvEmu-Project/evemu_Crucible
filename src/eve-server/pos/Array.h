
/**
 * @name Array.h
 *   Class for POS Array Modules.
 *
 * @Author:         Allan
 * @date:   28 December 17
 */


#ifndef EVEMU_POS_ARRAY_H_
#define EVEMU_POS_ARRAY_H_


#include "pos/Structure.h"


class ArraySE
: public StructureSE
{
public:
    ArraySE(StructureItemRef structure, EVEServiceManager& services, SystemManager* system, const FactionData& data);
    virtual ~ArraySE()                                  { /* do nothing here */ }

    /* class type pointer querys. */
    virtual ArraySE*            GetArraySE()            { return this; }

    /* class type tests. */
    virtual bool                IsArraySE()             { return true; }

    /* SystemEntity interface */
    virtual void                Process();

    /* virtual functions default to base class and overridden as needed */
    virtual void Init();


};

#endif  // EVEMU_POS_ARRAY_H_
