#ifndef EvilNumber_h__
#define EvilNumber_h__

// this file should have all stuff regarding damage type attribute caching..
enum EVIL_NUMBER_TYPE
{
    evil_number_nan,
    evil_number_int,
    evil_number_float,
};

/* doing the pragma pack.. results in less memory but more cpu */
#pragma pack(push,1)
class PyRep;

/**
 * @class EvilNumber
 *
 * @brief this is a class that kinda mimics how python polymorph's numbers.
 *
 * This is a class that kinda mimics how python polymorph's numbers.
 * VARIANT design style number class.
 *
 * @author Captnoord.
 * @date Juni 2010
 * @todo try to create a performance test for this.
 * @todo complete the compare operator overloads.
 */

class EvilNumber
{
public:
    /* generic value union */
    typedef union _GenVal {
        double fVal;
        int64 iVal;
    } GenVal;

    EvilNumber();
    EvilNumber(float val);
    EvilNumber(int val);
    EvilNumber(double val);
    EvilNumber(int64 val);
    EvilNumber(uint64 val);
    
    /************************************************************************/
    /* EvilNumber manipulation operator handlers                            */
    /************************************************************************/
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

    EvilNumber operator+(const EvilNumber& val)
    {
        Add((EvilNumber &)val);
        return *this;
    }

    EvilNumber operator+=(const EvilNumber& val)
    {
        Add((EvilNumber &)val);
        return *this;
    }

    EvilNumber operator-(const EvilNumber& val)
    {
        Sub((EvilNumber &)val);
        return *this;
    }

    /************************************************************************/
    /* End of EvilNumber manipulation operator handlers                     */
    /************************************************************************/

    /************************************************************************/
    /* EvilNumber logic logic operator handlers                             */
    /************************************************************************/

    /**
     * @brief '==' operator overload
     *
     * @param[in] val
     * @return will return true when this and val are equal
     * @note can I create 2 floats that would be equal when doing normal float compare and not equal when doing integer compare.
     */
    bool operator==(const EvilNumber& val)
    {
        if (this->mType == val.mType) {
            return this->mValue.iVal == val.mValue.iVal;
        } else {
            // if parameter 1 is int then parameter 2 is float
            if (this->mType == evil_number_int) {
                return double(this->mValue.iVal) == val.mValue.fVal;
            } else {
                return this->mValue.fVal == double(val.mValue.iVal);
            }
        }
    }

    bool operator!=(const EvilNumber& val)
    {
        // see comments from '==' operator.
        if (this->mType == val.mType) {
            return this->mValue.iVal != val.mValue.iVal;
        } else {
            // if parameter 1 is int then parameter 2 is float
            if (this->mType == evil_number_int) {
                return double(this->mValue.iVal) != val.mValue.fVal;
            } else {
                return this->mValue.fVal != double(val.mValue.iVal);
            }
        }
    }

    bool operator>(const EvilNumber& val)
    {
        if (this->mType == evil_number_int && val.mType == evil_number_int)
            return this->mValue.iVal > val.mValue.iVal;
        else if (this->mType == evil_number_float && val.mType == evil_number_float)
            return this->mValue.fVal > val.mValue.fVal;
        else if (this->mType == evil_number_float)
            return this->mValue.fVal > float(val.mValue.iVal);
        else
            return float(this->mValue.iVal) > val.mValue.fVal;
    }

    bool operator<=(const EvilNumber& val)
    {
        if (this->mType == evil_number_int && val.mType == evil_number_int)
            return this->mValue.iVal <= val.mValue.iVal;
        else if (this->mType == evil_number_float && val.mType == evil_number_float)
            return this->mValue.fVal <= val.mValue.fVal;
        else if (this->mType == evil_number_float)
            return this->mValue.fVal <= float(val.mValue.iVal);
        else
            return float(this->mValue.iVal) <= val.mValue.fVal;
    }

    /************************************************************************/
    /* End of EvilNumber logic logic operator handlers                      */
    /************************************************************************/

/* code generating macro... which I hate, but for this solution its the cleanest way
 * @note: we need to add a file type (lets say .xx) to generate the code
 */
#define LOGIC_OPERATOR(a, b) \
    bool operator a ( b val) \
    { \
        if (this->mType == evil_number_int) \
            return this->mValue.iVal < val; \
        else \
            return this->mValue.fVal < double(val); \
    }


    /* using a code generating macro to generate logic operator handlers
     * @note expand these if needed.
     */
    
    /* smaller */
    LOGIC_OPERATOR( < , const uint8)
    LOGIC_OPERATOR( < , const uint16)
    LOGIC_OPERATOR( < , const uint32)
    LOGIC_OPERATOR( < , const double)

    /* bigger */
    LOGIC_OPERATOR( > , const int)
    LOGIC_OPERATOR( > , const uint8)
    LOGIC_OPERATOR( > , const uint16)
    LOGIC_OPERATOR( > , const uint32)

    LOGIC_OPERATOR( > , const double)

    /* not equal */
    LOGIC_OPERATOR( != , const uint32)

    

    /**
     * @brief converts the EvilNumber value into a string
     *
     * @return the text representative of the value.
     */
    std::string to_str()
    {
        char buff[32]; // max uint32 will result in a 10 char string, a float will result in a ? char string.
        if (mType == evil_number_int)
            snprintf(buff, 32, "%d", mValue.iVal);
        else if (mType == evil_number_float)
            snprintf(buff, 32, "%f", mValue.fVal);
        else
            assert(false); // bleh crash..
        return buff;
    }

    /**
     * @brief converts the EvilNumber into a python object.
     *
     * @return the python object of the EvilNumber.
     */
    PyRep* GetPyObject();

    /************************************************************************/
    /* old system support                                                   */
    /* @note we call this old system support because even that it would be  */
    /* faster doing the math with ints and floats we would like to do all   */
    /* the math within the variant system to make sure we don't introduce   */
    /* extra errors into the math.( see warnings on get_int and get_float ) */
    /************************************************************************/
    EVIL_NUMBER_TYPE get_type()
    {
        return mType;
    }

    int64 get_int()
    {
        if( mType != evil_number_int ) {
            int64 temp = (int64)mValue.fVal;

            /* this checks if the type convention lost stuff behind the comma */
            if (double(temp) != mValue.fVal)
                sLog.Warning("EvilNumber", "Invalid call get_int called on a double");

            return (int64)mValue.fVal;
        }
        return mValue.iVal;
    }

    double get_float()
    {
        if( mType != evil_number_float ) {
            double temp = (double)mValue.iVal;
            
            /* this checks if the type convention ended up on a double overflow */
            if (int64(temp) != mValue.iVal)
                sLog.Warning("EvilNumber", "Invalid call get_float called on a int");

            return (double)mValue.iVal;
        }
        return mValue.fVal;
    }
    /************************************************************************/
    /* end of old system support                                            */
    /************************************************************************/

private:
    GenVal mValue;
    EVIL_NUMBER_TYPE mType;

protected:
    /**
     * @brief check if its possible a integer and do the conversion
     *
     * checking every calculation for float/int conversion can be a drain on
     * performance. But as integer math is fast then floating point math.
     */
    void CheckIntegrety();

    /**
     * @brief multiply this with @a
     *
     * Multiply this with @a.
     *
     * @param[in] val the value we are multiplying with.
     */
    void Multiply(EvilNumber & val);

    /**
     * @brief divide this with @a
     *
     * Divide this with @a.
     *
     * @param[in] val the value we are dividing by.
     * @todo handle "int(2) / int(4) = float(0.5)" in a fast enough way
     */
    void Divide(EvilNumber & val);

    // todo double check this..
    void Add(EvilNumber & val);

    // todo double check this..
    void Sub(EvilNumber & val);
};

#pragma pack(pop)

//////////////////////////////////////////////////////////////////////////
// global operators
EvilNumber operator*(const EvilNumber& val, const EvilNumber& val2);

extern const EvilNumber EvilTime_Second;
extern const EvilNumber EvilTime_Minute;
extern const EvilNumber EvilTime_Hour;
extern const EvilNumber EvilTime_Day;
extern const EvilNumber EvilTime_Month;
extern const EvilNumber EvilTime_Year;


#endif // EvilNumber_h__
