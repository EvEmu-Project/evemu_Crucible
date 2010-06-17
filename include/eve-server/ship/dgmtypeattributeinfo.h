#ifndef dgmtypeattributeinfo_h__
#define dgmtypeattributeinfo_h__

// this file should have all stuff regarding damage type attribute caching..

enum EVIL_NUMBER_TYPE
{
    evil_number_nan,
    evil_number_int,
    evil_number_float,
};

//@brief this is a class that kinda mimics how python polymorph's numbers.
#pragma pack(push,1)
class EvilNumber
{
public:
    typedef union _GenVal{
        float fVal;
        __int32 iVal;
    } GenVal;

    EvilNumber() : mType(evil_number_nan) {mValue.iVal = 0;}
    EvilNumber(float val) : mType(evil_number_float) {mValue.fVal = val;}
    EvilNumber(int val) : mType(evil_number_int) {mValue.iVal = val;}

    EvilNumber operator*(const EvilNumber& val)
    {
        Multiply((EvilNumber &)val);
        return *this;
    }

    EvilNumber operator/(const EvilNumber& val)
    {
        Divide((EvilNumber &)val);
        return *this;
    }

    std::string to_str()
    {
        char buff[32]; // max uint32 will result in a 10 char string, a float will result in a ? char string.
        if (mType == evil_number_int)
            sprintf(buff, "%d", mValue.iVal);
        else if (mType == evil_number_float)
            sprintf(buff, "%f", mValue.fVal);
        else
            __asm{int 3}; // bleh crash..
        return buff;
    }

private:
    GenVal mValue;
    EVIL_NUMBER_TYPE mType;

protected:
    // check if its possible a integer and do the conversion
    // this is kinda a drain on performance... but at least
    // integer math is faster than floating point math.
    inline void CheckIntegrety()
    {
        // check if we are a integer
        int cmp_val = (int)mValue.fVal;
        if (float(cmp_val) == mValue.fVal) {
            //we are a integer.... /me cheers...
            mValue.iVal = cmp_val;
            mType = evil_number_int;
        }
    }

    // multiply this with val
    inline void Multiply(EvilNumber & val)
    {
        if (val.mType == mType) {
            if (mType == evil_number_float) {
                this->mValue.fVal = this->mValue.fVal * val.mValue.fVal;
            } else if (mType == evil_number_int) {
                this->mValue.iVal = this->mValue.iVal * val.mValue.iVal;
            }
        } else {
            // we assume that the val argument is the opposite of the 'this' type
            if (mType == evil_number_float) {
                this->mValue.fVal = this->mValue.fVal*float(val.mValue.iVal);
            } else if (mType == evil_number_int) {
                float tVal = (float)mValue.iVal; // normal integer number
                this->mValue.fVal = tVal*val.mValue.fVal;
                mType = evil_number_float;
            } else {
                __asm{int 3}; // crash
            }

            // check if we are a integer
            CheckIntegrety();
        }
    }

    // divide a generic number is evil as it should be possible todo int(2) / int(4) = float(0.5).
    inline void Divide(EvilNumber & val)
    {
        if (val.mType == mType) {
            if (mType == evil_number_float) {
                this->mValue.fVal = this->mValue.fVal / val.mValue.fVal;
            } else if (mType == evil_number_int) {
                // make sure we can do things like 2 / 4 = 0.5f
                this->mValue.fVal = float(this->mValue.iVal) / float(val.mValue.iVal);
                mType = evil_number_float;
                // check if its possibly a integer
                CheckIntegrety();
            }
        } else {
            // we assume that the val argument is the opposite of the 'this' type
            if (mType == evil_number_float) {
                this->mValue.fVal = this->mValue.fVal/float(val.mValue.iVal);
            } else if (mType == evil_number_int) {
                float tVal = (float)mValue.iVal; // normal integer number
                this->mValue.fVal = tVal/val.mValue.fVal;
                mType = evil_number_float;
            } else {
                __asm{int 3}; // crash
            }

            // check if we are a integer
            CheckIntegrety();
        }
    }
};

// this represends 1 attribute modifier
class DmgTypeAttribute
{
public:
    uint16 attributeID;
    EvilNumber number;
};
#pragma pack(pop)

// this represends a collection of attribute modifiers for a single typeID
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
