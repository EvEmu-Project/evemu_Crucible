#ifndef dgmtypeattributeinfo_h__
#define dgmtypeattributeinfo_h__

#include "utils/EvilNumber.h"



/**
 * DgmTypeAttributeInfo cache
 * The main idea is that we need to cache most of the important db tables and DgmTypeAttributeInfo is one of them.
 * This file contains all the required parts to make this happen for this table. Its not perfect but its good enough
 * for now.
 * The dgmtypeattributemgr loads the data from the db on startup and puts them into DgmTypeAttributeSets. Those
 * sets are comparable to db query results, you iterate trough the result using begin() and end() iterators like
 * we would use normal std systems.
 */

// this represents 1 attribute modifier
#pragma pack(push,1)
class DmgTypeAttribute
{
public:
    uint16 attributeID;
    EvilNumber number;
};
#pragma pack(pop)


// this represents a collection of attribute modifiers for a single typeID
class DgmTypeAttributeSet
{
public:
    ~DgmTypeAttributeSet();

    // a set is a collection of pointers...
    typedef std::list<DmgTypeAttribute *> AttrSet;
    typedef AttrSet::iterator AttrSetItr;

    // wrap these functions so its cleaner..
    AttrSetItr begin() {return attributeset.begin();}
    AttrSetItr end() {return attributeset.end();}

    AttrSet attributeset;
};

typedef std::map<uint32, DgmTypeAttributeSet*>  DgmTypeAttributeMap;
typedef DgmTypeAttributeMap::iterator           DgmTypeAttributeMapItr;

// class that does all the magic of caching the info
/**
 * @class dgmtypeattributemgr
 *
 * @brief database cache system for dgmtypeattribute table.
 *
 * @author Captnoord
 * @date Juni 2010
 */
class dgmtypeattributemgr
{
public:
    dgmtypeattributemgr(); // also do init stuff, db loading
    ~dgmtypeattributemgr();

    DgmTypeAttributeSet* GetDmgTypeAttributeSet(uint32 typeID);
private:
    DgmTypeAttributeMap mDgmTypeAttrInfo;
};

extern dgmtypeattributemgr * _sDgmTypeAttrMgr;
#define sDgmTypeAttrMgr (*_sDgmTypeAttrMgr)

static EvilNumber e_sqrt(EvilNumber num)
{
    if (num.get_type() == evil_number_float)
        return EvilNumber(sqrt(num.get_float()));
    else
        return EvilNumber(sqrt(double(num.get_int())));
}

static EvilNumber e_log(EvilNumber num)
{
    if (num.get_type() == evil_number_float)
        return EvilNumber(log(num.get_float()));
    else
        return EvilNumber(log(double(num.get_int())));
}

static EvilNumber e_pow(EvilNumber num, int power_of)
{
    if (num.get_type() == evil_number_float)
        return EvilNumber(pow(num.get_float(), power_of));
    else
        return EvilNumber(pow(double(num.get_int()), power_of));

}

static EvilNumber e_pow(int num, EvilNumber power_of)
{
    if (power_of.get_type() == evil_number_float)
        return EvilNumber(pow(num, power_of.get_float()));
    else
        return EvilNumber(pow(num, double(power_of.get_int())));

}

#endif // dgmtypeattributeinfo_h__
