#ifndef dgmtypeattributeinfo_h__
#define dgmtypeattributeinfo_h__

// this file should have all stuff regarding damage type attribute caching..

#pragma pack(push,1)

// this represents 1 attribute modifier
class DmgTypeAttribute
{
public:
    uint16 attributeID;
    EvilNumber number;
};
#pragma pack(pop)

// this represents a collection of attribute modifiers for a single typeID
struct DgmTypeAttributeSet
{
    std::list<DmgTypeAttribute *> attributeset;
};

typedef std::map<uint32, DgmTypeAttributeSet*>  DgmTypeAttributeMap;
typedef DgmTypeAttributeMap::iterator           DgmTypeAttributeMapItr;

// class that does all the magic of caching the info
class dgmtypeattributemgr
{
public:
    dgmtypeattributemgr(); // also do init stuff, db loading

    DgmTypeAttributeSet* GetDmgTypeAttributeSet(uint32 typeID);
private:
    DgmTypeAttributeMap mDgmTypeAttrInfo;
};

extern dgmtypeattributemgr * _sDgmTypeAttrMgr;
#define sDgmTypeAttrMgr (*_sDgmTypeAttrMgr)

#endif // dgmtypeattributeinfo_h__
