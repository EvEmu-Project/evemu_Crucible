#ifndef dgmtypeattributeinfo_h__
#define dgmtypeattributeinfo_h__

// this file should have all stuff regarding damage type attribute caching..

enum EVIL_NUMBER_TYPE
{
    evil_number_nan,
    evil_number_int,
    evil_number_float,
};

#pragma pack(push,1)
class PyObject;

/**
 * @brief this is a class that kinda mimics how python polymorph's numbers.
 *
 * @author Captnoord.
 * @date Juni 2010
 */
class EvilNumber
{
public:
    typedef union _GenVal{
        float fVal;
        int32 iVal;
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

    bool operator==(const EvilNumber& val)
    {
        if (this->mType != val.mType)
            return false;

        // important to know is that 'if' they are both floats which would be
        // exactly the same value the raw data would be exactly the same.
        // so this check is valid for both floats and ints.
        //
        // @note I wonder... can I create 2 floats that would be equal when
        // doing normal float compare and not equal when doing integer compare.
        if (this->mValue.iVal == val.mValue.iVal)
            return true;
        else
            return false;
    }

    /**
     * @brief converts the EvilNumber value into a string
     *
     * @return the text representative of the value.
     */
    std::string to_str()
    {
        char buff[32]; // max uint32 will result in a 10 char string, a float will result in a ? char string.
        if (mType == evil_number_int)
            sprintf(buff, "%d", mValue.iVal);
        else if (mType == evil_number_float)
            sprintf(buff, "%f", mValue.fVal);
        else
            assert(false); // bleh crash..
        return buff;
    }

    /**
     * @brief converts the EvilNumber into a python object.
     *
     * @return the python object of the EvilNumber.
     */
    inline PyObject* GetPyObject()
    {
        if (mType == evil_number_int)
            return (PyObject*)new PyInt(mValue.iVal);
        else if (mType == evil_number_float)
            return (PyObject*)new PyFloat(mValue.fVal);
        else {
            assert(false);
            return false;
        }
    }

private:
    GenVal mValue;
    EVIL_NUMBER_TYPE mType;

protected:
    /**
     * @brief check if its possible a integer and do the conversion
     *
     * checking every calculation for float/int conversion can be a drain on
     * performance. But as integer math is fast then floating point math.
     *
     */
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

    /**
     * @brief multiply this with @a
     *
     * Multiply this with @a.
     *
     * @param[in] val the value we are multiplying with.
     */
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
                assert(false); // crash
            }

            // check if we are a integer
            CheckIntegrety();
        }
    }

    /**
     * @brief divide this with @a
     *
     * Divide this with @a.
     *
     * @param[in] val the value we are dividing by.
     * @todo handle "int(2) / int(4) = float(0.5)" in a fast anough way
     */
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
                assert(false); // crash
            }

            // check if we are a integer
            CheckIntegrety();
        }
    }
};

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
