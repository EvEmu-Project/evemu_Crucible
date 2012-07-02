/*
    ------------------------------------------------------------------------------------
    LICENSE:
    ------------------------------------------------------------------------------------
    This file is part of EVEmu: EVE Online Server Emulator
    Copyright 2006 - 2011 The EVEmu Team
    For the latest information visit http://evemu.org
    ------------------------------------------------------------------------------------
    This program is free software; you can redistribute it and/or modify it under
    the terms of the GNU Lesser General Public License as published by the Free Software
    Foundation; either version 2 of the License, or (at your option) any later
    version.

    This program is distributed in the hope that it will be useful, but WITHOUT
    ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
    FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public License along with
    this program; if not, write to the Free Software Foundation, Inc., 59 Temple
    Place - Suite 330, Boston, MA 02111-1307, USA, or go to
    http://www.gnu.org/copyleft/lesser.txt.
    ------------------------------------------------------------------------------------
    Author:        Captnoord, Aknor Jaden
*/

#ifndef EvilNumber_h__
#define EvilNumber_h__

#define MAX_EVIL_INTEGER    0x7FFFFFFFFFFFFFFFLL      // 64-bit signed integer


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
    EvilNumber(int8 val);
    EvilNumber(uint8 val);
    EvilNumber(int16 val);
    EvilNumber(uint16 val);
    EvilNumber(int32 val);
    EvilNumber(uint32 val);
    EvilNumber(int64 val);
    EvilNumber(uint64 val);     // WARNING:  Possible data corruption here since internal integer type is 'int64'
    EvilNumber(float val);
    EvilNumber(double val);


    /************************************************************************/
    /* EvilNumber manipulation operator handlers                            */
    /************************************************************************/
    EvilNumber operator*(const EvilNumber& val)
    {    return _Multiply(*this, val);    }

    EvilNumber operator*=(const EvilNumber& val)
    {    return _SelfMultiply(val);      }

    EvilNumber operator/(const EvilNumber& val)
    {    return _Divide(*this, val);     }

    EvilNumber operator/=(const EvilNumber& val)
    {    return _SelfDivide(val);        }

    EvilNumber operator+(const EvilNumber& val)
    {    return _Add(*this, val);        }

    EvilNumber operator+=(const EvilNumber& val)
    {    return _SelfAdd(val);          }

    EvilNumber operator-(const EvilNumber& val)
    {    return _Subtract(*this, val);   }

    EvilNumber operator-=(const EvilNumber& val)
    {    return _SelfSubtract(val);     }

    EvilNumber operator%(const EvilNumber& val)
    {    return _Modulus(*this, val);    }

    EvilNumber operator%=(const EvilNumber& val)
    {    return _SelfModulus(val);      }

    EvilNumber operator++()
    {    return _SelfIncrement();       }

    EvilNumber operator++(int)
    {
        EvilNumber temp = *this;
        _SelfIncrement();
        return temp;
    }

    EvilNumber operator--()
    {    return _SelfDecrement();       }

    EvilNumber operator--(int)
    {
        EvilNumber temp = *this;
        _SelfDecrement();
        return temp;
    }

    /************************************************************************/
    /* End of EvilNumber manipulation operator handlers                     */
    /************************************************************************/


    /************************************************************************/
    /* EvilNumber logic operator handlers                                   */
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

    bool operator<(const EvilNumber& val)
    {
        if (this->mType == evil_number_int && val.mType == evil_number_int)
            return this->mValue.iVal < val.mValue.iVal;
        else if (this->mType == evil_number_float && val.mType == evil_number_float)
            return this->mValue.fVal < val.mValue.fVal;
        else if (this->mType == evil_number_float)
            return this->mValue.fVal < float(val.mValue.iVal);
        else
            return float(this->mValue.iVal) < val.mValue.fVal;
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

    bool operator>=(const EvilNumber& val)
    {
        if (this->mType == evil_number_int && val.mType == evil_number_int)
            return this->mValue.iVal >= val.mValue.iVal;
        else if (this->mType == evil_number_float && val.mType == evil_number_float)
            return this->mValue.fVal >= val.mValue.fVal;
        else if (this->mType == evil_number_float)
            return this->mValue.fVal >= float(val.mValue.iVal);
        else
            return float(this->mValue.iVal) >= val.mValue.fVal;
    }

    /************************************************************************/
    /* End of EvilNumber logic operator handlers                            */
    /************************************************************************/



    /************************************************************************/
    /* EvilNumber Mathematical functions                                    */
    /************************************************************************/
    /**
     * @brief ??
     *
     * @param[in] val
     * @return
     */
    static EvilNumber sin( const EvilNumber & val );
    static EvilNumber cos( const EvilNumber & val );
    static EvilNumber tan( const EvilNumber & val );
    static EvilNumber asin( const EvilNumber & val );
    static EvilNumber acos( const EvilNumber & val );
    static EvilNumber atan( const EvilNumber & val );
    static EvilNumber sqrt( const EvilNumber & val );
    static EvilNumber pow( const EvilNumber & val1, const EvilNumber & val2 );
    static EvilNumber log( const EvilNumber & val );
    static EvilNumber log10( const EvilNumber & val );
    static EvilNumber exp( const EvilNumber & val );

    /************************************************************************/
    /* End of EvilNumber Mathematical functions                             */
    /************************************************************************/


    /**
     * @brief Code generation macro to create comparison operator overload using supplied operator and type being compared to EvilNumber
     *
     * @param[in] a
     * @param[in] b
     * @return operator overload function for operator 'a' comparing to a value 'val' of type 'b'
     */
    #define LOGIC_OPERATOR(a, b) \
        bool operator a ( b val) \
        { \
            if (this->mType == evil_number_int) \
                return this->mValue.iVal a static_cast<int64>(val); \
            else \
                return this->mValue.fVal a static_cast<double>(val); \
        }

    /**
     * @brief Code generation macro to create all comparison operator overload functions for a specified type
     *
     * @param[in] a
     * @return all comparison operator overload functions for the specified type
     */
    #define LOGIC_OPERATORS_FOR( type ) \
        LOGIC_OPERATOR( <, type ) \
        LOGIC_OPERATOR( <=, type ) \
        LOGIC_OPERATOR( >, type ) \
        LOGIC_OPERATOR( >=, type ) \
        LOGIC_OPERATOR( ==, type ) \
        LOGIC_OPERATOR( !=, type )

    /* using a code generating macro to generate logic operator handlers
     * @note expand these if needed.
     */
    LOGIC_OPERATORS_FOR( const int8)
    LOGIC_OPERATORS_FOR( const uint8)
    LOGIC_OPERATORS_FOR( const int16)
    LOGIC_OPERATORS_FOR( const uint16)
    LOGIC_OPERATORS_FOR( const int32)
    LOGIC_OPERATORS_FOR( const uint32)
    LOGIC_OPERATORS_FOR( const int64)
    LOGIC_OPERATORS_FOR( const uint64)
    LOGIC_OPERATORS_FOR( const float)
    LOGIC_OPERATORS_FOR( const double)


    /**
     * @brief converts the EvilNumber value into a string
     *
     * @return the text representative of the value.
     */
    std::string to_str()
    {
        char buff[32]; // max uint32 will result in a 10 char string, a float will result in a ? char string.
        if (mType == evil_number_int)
            snprintf(buff, 32, "%" PRId64, mValue.iVal);
        else if (mType == evil_number_float)
            snprintf(buff, 32, "%f", mValue.fVal);
        else
            assert(false); // bleh crash..
        return std::string(buff);
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

    bool to_int();
    bool to_float();

    int64 get_int();
    double get_float();
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
    void CheckIntegrity();

    /**
     * @brief multiply this with @a
     *
     * Multiply this with @a.
     *
     * @param[in] val the value we are multiplying with.
     */
    EvilNumber _Multiply(const EvilNumber & val1, const EvilNumber & val2);
    EvilNumber _SelfMultiply(const EvilNumber & val);

    /**
     * @brief divide this with @a
     *
     * Divide this with @a.
     *
     * @param[in] val the value we are dividing by.
     * @todo handle "int(2) / int(4) = float(0.5)" in a fast enough way
     */
    EvilNumber _Divide(const EvilNumber & val1, const EvilNumber & val2);
    EvilNumber _SelfDivide(const EvilNumber & val);

    /**
     * @brief add this with @a
     *
     * Add this with @a.
     *
     * @param[in] val the value we are adding by.
     */
    EvilNumber _Add(const EvilNumber & val1, const EvilNumber & val2);
    EvilNumber _SelfAdd(const EvilNumber & val);

    /**
     * @brief subtract this with @a
     *
     * Subtract this with @a.
     *
     * @param[in] val the value we are subtracting by.
     */
    EvilNumber _Subtract(const EvilNumber & val1, const EvilNumber & val2);
    EvilNumber _SelfSubtract(const EvilNumber & val);

    /**
     * @brief modulus this with @a
     *
     * this modulus @a.
     *
     * @param[in] val the value we are using as the rvalue in the modulus.
     */
    EvilNumber _Modulus(const EvilNumber & val1, const EvilNumber & val2);
    EvilNumber _SelfModulus(const EvilNumber & val);

    /**
     * @brief increment by 1
     *
     * increment by 1
     */
    EvilNumber _SelfIncrement();

    /**
     * @brief decrement by 1
     *
     * decrement by 1
     */
    EvilNumber _SelfDecrement();
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

extern const EvilNumber EvilPI;
extern const EvilNumber EvilE;

extern EvilNumber EvilTimeNow();

#endif // EvilNumber_h__
