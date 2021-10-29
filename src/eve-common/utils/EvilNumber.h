/*
    ------------------------------------------------------------------------------------
    LICENSE:
    ------------------------------------------------------------------------------------
    This file is part of EVEmu: EVE Online Server Emulator
    Copyright 2006 - 2021 The EVEmu Team
    For the latest information visit https://evemu.dev
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
    Updates:    Allan
*/

#ifndef EvilNumber_h__
#define EvilNumber_h__

#define MAX_EVIL_INTEGER    0x7FFFFFFFFFFFFFFFLL      // 64-bit signed integer

#include "eve-compat.h"


// this file should have all stuff regarding attribute caching..
enum EVIL_NUMBER_TYPE
{
    evil_number_nan = 0,
    evil_number_int,
    evil_number_float,
};


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
 */

class EvilNumber
{
private:
    double fVal;
    int64 iVal;
    EVIL_NUMBER_TYPE mType;

public:
    EvilNumber();
    EvilNumber(int8 val);
    EvilNumber(uint8 val);
    EvilNumber(int16 val);
    EvilNumber(uint16 val);
    EvilNumber(int32 val);
    EvilNumber(uint32 val);
    EvilNumber(int64 val);
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

	EvilNumber operator-()
	{    return _Multiply(EvilNumber(-1.0), *this);   }

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

    /* this causes errors when enabled
     *  there are comparisons using (ENum + (float) > ENum) [mainly in ship cap/hp]
     * this causes a return of float then tries to use '>' for which the inputs are on wrong sides
     * will need offending code rewrote to use this system
     * may not be worth the effort, as things are working as-is
     */
    /**
     * @brief Code generation macro to create basic binary math operator overload using supplied operator and type being compared to EvilNumber
     *
     * @param[in] a
     * @param[in] b
     * @return operator overload function for operator 'a'  to a value 'val' of type 'b'
     */
    /*
    #define MATH_OPERATOR(a, b) \
        b operator a ( b val) \
        { \
            if (this->mType == evil_number_int) \
                return this->iVal a static_cast<int64>(val); \
            else \
                return this->fVal a static_cast<double>(val); \
        }
*/
    /**
     * @brief Code generation macro to create all basic binary math operator overload functions for a specified type
     *
     * @param[in] a
     * @return all basic binary math operator overload functions for the specified type
     */
    /*
    #define MATH_OPERATORS_FOR( type ) \
        MATH_OPERATOR( +, type ) \
        MATH_OPERATOR( -, type ) \
        MATH_OPERATOR( *, type ) \
        MATH_OPERATOR( /, type )
    //    MATH_OPERATOR( %, type )
*/
    /** use a code generating macro to generate basic binary math operator handlers
     */
/*
	MATH_OPERATORS_FOR( const int8)
    MATH_OPERATORS_FOR( const uint8)
    MATH_OPERATORS_FOR( const int16)
    MATH_OPERATORS_FOR( const uint16)
    MATH_OPERATORS_FOR( const int32)
    MATH_OPERATORS_FOR( const uint32)
    MATH_OPERATORS_FOR( const int64)
    MATH_OPERATORS_FOR( const float)
    MATH_OPERATORS_FOR( const double)
*/
    /************************************************************************/
    /* End of EvilNumber manipulation operator handlers                     */
    /************************************************************************/


    /************************************************************************/
    /* EvilNumber logic operator handlers                                   */
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
            if (this->mType == evil_number_int) { \
                return this->iVal a static_cast<int64>(val); \
            } else if (this->mType == evil_number_float) { \
                return this->fVal a static_cast<double>(val); \
            } else { \
                assert(false); \
                return false; \
            } \
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
    LOGIC_OPERATORS_FOR( const float)
    LOGIC_OPERATORS_FOR( const double)


    /**
     * @brief '==' operator overload
     *
     * @param[in] val
     * @return will return true when this and val are equal
     * @note can I create 2 floats that would be equal when doing normal float compare and not equal when doing integer compare.
     */
    bool operator==(const EvilNumber& val)
    {
        if (this->mType == evil_number_int && val.mType == evil_number_int)
            return this->iVal == val.iVal;
        else if (this->mType == evil_number_float && val.mType == evil_number_float)
            return this->fVal == val.fVal;
        else if (this->mType == evil_number_float)
            return this->fVal == double(val.iVal);
        else
            return double(this->iVal) == val.fVal;
    }

    // doesnt work when compared to 0 (num != 0)
    bool operator!=(const EvilNumber& val)
    {
        if (this->mType == evil_number_int && val.mType == evil_number_int)
            return this->iVal != val.iVal;
        else if (this->mType == evil_number_float && val.mType == evil_number_float)
            return this->fVal != val.fVal;
        else if (this->mType == evil_number_float)
            return this->fVal != double(val.iVal);
        else
            return double(this->iVal) != val.fVal;
    }

    bool operator<(const EvilNumber& val)
    {
        if (this->mType == evil_number_int && val.mType == evil_number_int)
            return this->iVal < val.iVal;
        else if (this->mType == evil_number_float && val.mType == evil_number_float)
            return this->fVal < val.fVal;
        else if (this->mType == evil_number_float)
            return this->fVal < double(val.iVal);
        else
            return double(this->iVal) < val.fVal;
    }

    bool operator>(const EvilNumber& val)
    {
        if (this->mType == evil_number_int && val.mType == evil_number_int)
            return this->iVal > val.iVal;
        else if (this->mType == evil_number_float && val.mType == evil_number_float)
            return this->fVal > val.fVal;
        else if (this->mType == evil_number_float)
            return this->fVal > double(val.iVal);
        else
            return double(this->iVal) > val.fVal;
    }

    bool operator<=(const EvilNumber& val)
    {
        if (this->mType == evil_number_int && val.mType == evil_number_int)
            return this->iVal <= val.iVal;
        else if (this->mType == evil_number_float && val.mType == evil_number_float)
            return this->fVal <= val.fVal;
        else if (this->mType == evil_number_float)
            return this->fVal <= double(val.iVal);
        else
            return double(this->iVal) <= val.fVal;
    }

    bool operator>=(const EvilNumber& val)
    {
        if (this->mType == evil_number_int && val.mType == evil_number_int)
            return this->iVal >= val.iVal;
        else if (this->mType == evil_number_float && val.mType == evil_number_float)
            return this->fVal >= val.fVal;
        else if (this->mType == evil_number_float)
            return this->fVal >= double(val.iVal);
        else
            return double(this->iVal) >= val.fVal;
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
     * @brief converts the EvilNumber value into a string
     *
     * @return the text representative of the value.
     */
    std::string to_str()
    {
        char buff[32]; // max uint32 will result in a 10 char string, a float will result in a ? char string.
        if (mType == evil_number_int)
            snprintf(buff, 32, "%li", iVal);
        else if (mType == evil_number_float)
            snprintf(buff, 32, "%f", fVal);
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

    EVIL_NUMBER_TYPE get_type()                         { return mType; }

    bool isInt()                                        { return ( mType == evil_number_int ); }
    bool isFloat()                                      { return ( mType == evil_number_float ); }
    bool isNaN();
    bool isInf();

    bool get_bool();
    int64 get_int();
    uint32 get_uint32();    // be careful with using this one...no overflow checks
    float get_float();
    double get_double();

protected:
    /**
     * @brief check if its possible a integer and do the conversion
     *
     * checking every calculation for float/int conversion can be a drain on
     * performance. But integer math is faster than floating point math.
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

//////////////////////////////////////////////////////////////////////////
// global operators
EvilNumber operator+(const EvilNumber& val, const EvilNumber& val2);
EvilNumber operator-(const EvilNumber& val, const EvilNumber& val2);
EvilNumber operator*(const EvilNumber& val, const EvilNumber& val2);
EvilNumber operator/(const EvilNumber& val, const EvilNumber& val2);
EvilNumber operator%(const EvilNumber& val, const EvilNumber& val2);

extern EvilNumber EvilZero;
extern EvilNumber EvilZerof;
extern EvilNumber EvilOne;
extern EvilNumber EvilNegOne;
extern const EvilNumber EvilTime_Second;
extern const EvilNumber EvilTime_Minute;
extern const EvilNumber EvilTime_Hour;
extern const EvilNumber EvilTime_Day;
extern const EvilNumber EvilTime_Month;
extern const EvilNumber EvilTime_Year;

#endif // EvilNumber_h__
