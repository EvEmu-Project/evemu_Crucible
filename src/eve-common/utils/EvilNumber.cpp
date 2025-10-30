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

#include "eve-common.h"

#include "python/PyRep.h"
#include "utils/EvilNumber.h"

#ifdef __APPLE__
#include <cstdio>  // 用于std::snprintf
#include <cmath>   // 用于std::isnan, std::isinf等数学函数
#endif

EvilNumber EvilZero = EvilNumber();
EvilNumber EvilZerof = EvilNumber(0.0f);
EvilNumber EvilOne = EvilNumber(1);
EvilNumber EvilNegOne = EvilNumber(-1);
const EvilNumber EvilTime_Second = EvE::Time::Second;
const EvilNumber EvilTime_Minute = EvilTime_Second * 60;
const EvilNumber EvilTime_Hour = EvilTime_Minute * 60;
const EvilNumber EvilTime_Day = EvilTime_Hour * 24;
const EvilNumber EvilTime_Month = EvilTime_Day * 30;
const EvilNumber EvilTime_Year = EvilTime_Month * 12 + 5;


// CONSTRUCTORS:

EvilNumber::EvilNumber() : mType(evil_number_int)
{
    iVal = 0;
    fVal = 0.0;
}

EvilNumber::EvilNumber( int8 val ) : mType(evil_number_int)
{
    iVal = val;
    fVal = 0.0;
}

EvilNumber::EvilNumber( uint8 val ) : mType(evil_number_int)
{
    iVal = val;
    fVal = 0.0;
}

EvilNumber::EvilNumber( int16 val ) : mType(evil_number_int)
{
    iVal = val;
    fVal = 0.0;
}

EvilNumber::EvilNumber( uint16 val ) : mType(evil_number_int)
{
    iVal = val;
    fVal = 0.0;
}

EvilNumber::EvilNumber( int32 val ) : mType(evil_number_int)
{
    iVal = val;
    fVal = 0.0;
}

EvilNumber::EvilNumber( uint32 val ) : mType(evil_number_int)
{
    iVal = val;
    fVal = 0.0;
}

EvilNumber::EvilNumber( int64 val ) : mType(evil_number_int)
{
    iVal = val;
    fVal = 0.0;
}

EvilNumber::EvilNumber( float val ) : mType(evil_number_float)
{
    fVal = val;
    iVal = 0;
}

EvilNumber::EvilNumber( double val ) : mType(evil_number_float)
{
    fVal = val;
    iVal = 0;
}


// PUBLIC FUNCTIONS:

PyRep* EvilNumber::GetPyObject()
{
    if (mType == evil_number_int) {
        if ( iVal > INT_MAX || iVal < INT_MIN)
            return (PyRep*)new PyLong(iVal);
        else
            return (PyRep*)new PyInt((int32)(iVal));
    } else if (mType == evil_number_float) {
        return (PyRep*)new PyFloat(fVal);
    } else {
        sLog.Error("EvilNumber::GetPyObject()", "EvilNumber is neither integer nor float.  Returning None");
        EvE::traceStack();
        assert(false);
        return PyStatic.NewNone();
    }
}

inline void EvilNumber::CheckIntegrity()
{
    // check if we are already an integer
    if (mType == evil_number_int)
        return;
    int64 cmp_val = (int64)fVal;
    if (double(cmp_val) == fVal) {
        iVal = cmp_val;
        fVal = 0;
        mType = evil_number_int;
    }
}

bool EvilNumber::isNaN()
{
    if( mType == evil_number_nan )
        return true;
    if ( mType == evil_number_int )
        return false;
    
#ifdef __APPLE__
    return isnan(fVal);
#else    
    return std::isnan(fVal);
#endif    
}

bool EvilNumber::isInf()
{
    if ( mType == evil_number_nan )
        return true;
    if ( mType == evil_number_int )
        return false;
#ifdef __APPLE__
    return isinf(fVal);
#else    
    return std::isinf(fVal);
#endif
}

bool EvilNumber::get_bool()
{
    if (mType == evil_number_float)
        return (fVal != 0.0);
    if (mType == evil_number_int)
        return (iVal != 0);
    return false;
}

int64 EvilNumber::get_int()
{
    if (mType == evil_number_float)
        return (int64)floor(fVal);
    return iVal;
}

uint32 EvilNumber::get_uint32()
{
    uint32 value(0);
    if (mType == evil_number_float)
        value = (uint32)floor(fVal);
    else
        value = (uint32)iVal;

    return value;
}

float EvilNumber::get_float()
{
    if (mType == evil_number_int)
        return (float)iVal;
    return (float)fVal;
}

double EvilNumber::get_double()
{
    if (mType == evil_number_int)
        return (double)iVal;
    return fVal;
}

// public math functions

EvilNumber EvilNumber::sin( const EvilNumber & val )
{
    EvilNumber result = EvilNumber();
    if ( val.mType == evil_number_float )
        result.fVal = std::sin( val.fVal );
    else
        result.fVal = std::sin( (double)(val.iVal) );

    result.mType = evil_number_float;
    return result;
}

EvilNumber EvilNumber::cos( const EvilNumber & val )
{
    EvilNumber result = EvilNumber();
    if ( val.mType == evil_number_float )
        result.fVal = std::cos( val.fVal );
    else
        result.fVal = std::cos( (double)(val.iVal) );

    result.mType = evil_number_float;
    return result;
}

EvilNumber EvilNumber::tan( const EvilNumber & val )
{
    EvilNumber result = EvilNumber();
    if ( val.mType == evil_number_float )
        result.fVal = std::tan( val.fVal );
    else
        result.fVal = std::tan( (double)(val.iVal) );

    result.mType = evil_number_float;
    return result;
}

EvilNumber EvilNumber::asin( const EvilNumber & val )
{
    EvilNumber result = EvilNumber();
    if ( val.mType == evil_number_float )
        result.fVal = std::asin( val.fVal );
    else
        result.fVal = std::asin( (double)(val.iVal) );

    result.mType = evil_number_float;
    return result;
}

EvilNumber EvilNumber::acos( const EvilNumber & val )
{
    EvilNumber result = EvilNumber();
    if ( val.mType == evil_number_float )
        result.fVal = std::acos( val.fVal );
    else
        result.fVal = std::acos( (double)(val.iVal) );

    result.mType = evil_number_float;
    return result;
}

EvilNumber EvilNumber::atan( const EvilNumber & val )
{
    EvilNumber result = EvilNumber();
    if ( val.mType == evil_number_float )
        result.fVal = std::atan( val.fVal );
    else
        result.fVal = std::atan( (double)(val.iVal) );

    result.mType = evil_number_float;
    return result;
}

EvilNumber EvilNumber::sqrt( const EvilNumber & val )
{
    EvilNumber result = EvilNumber();
    if ( val.mType == evil_number_float )
        result.fVal = std::sqrt( val.fVal );
    else
        result.fVal = std::sqrt( (double)(val.iVal) );

    result.mType = evil_number_float;
    return result;
}

EvilNumber EvilNumber::pow( const EvilNumber & val1, const EvilNumber & val2 )
{
    EvilNumber exponent = EvilNumber();
    EvilNumber result = EvilNumber();
    if ( val2.mType == evil_number_float )
        exponent.fVal = val2.fVal;
    else
        exponent.fVal = (double)(val2.iVal);
    exponent.mType = evil_number_float;

    if( val1.mType == evil_number_float )
        result.fVal = std::pow( val1.fVal, exponent.fVal );
    else
        result.fVal = std::pow( (double)(val1.iVal), exponent.fVal );

    result.mType = evil_number_float;
    return result;
}

EvilNumber EvilNumber::log( const EvilNumber & val )
{
    EvilNumber result = EvilNumber();
    if ( val.mType == evil_number_float )
        result.fVal = std::log( val.fVal );
    else
        result.fVal = std::log( (double)(val.iVal) );

    result.mType = evil_number_float;
    return result;
}

EvilNumber EvilNumber::log10( const EvilNumber & val )
{
    EvilNumber result = EvilNumber();
    if ( val.mType == evil_number_float )
        result.fVal = std::log10( val.fVal );
    else
        result.fVal = std::log10( (double)(val.iVal) );

    result.mType = evil_number_float;
    return result;
}

EvilNumber EvilNumber::exp( const EvilNumber & val )
{
    EvilNumber result = EvilNumber();
    if ( val.mType == evil_number_float )
        result.fVal = std::exp( val.fVal );
    else
        result.fVal = std::exp( (double)(val.iVal) );

    result.mType = evil_number_float;
    return result;
}


// PRIVATE FUNCTIONS:

EvilNumber EvilNumber::_Multiply( const EvilNumber & val1, const EvilNumber & val2 )
{
    EvilNumber result = EvilNumber();
    // WARNING!  There should be NO implicit or explicit use of the 'this' pointer here!
    if (val2.mType == val1.mType) {
        if (val1.mType == evil_number_float) {
            result.fVal = val1.fVal * val2.fVal;
            result.mType = evil_number_float;
        } else if (val1.mType == evil_number_int) {
            result.iVal = val1.iVal * val2.iVal;
            result.mType = evil_number_int;
        } else {
            assert(false); // crash
        }
    } else {
        // we assume that the val1 argument type is the opposite of the val2 argument type
        if (val1.mType == evil_number_float) {
            result.fVal = val1.fVal * val2.iVal;
        } else if (val1.mType == evil_number_int) {
            result.fVal = val2.fVal * val1.iVal;
        } else {
            assert(false); // crash
        }
        result.mType = evil_number_float;
    }
    //result.CheckIntegrity();

    return result;
}

EvilNumber EvilNumber::_SelfMultiply( const EvilNumber & val )
{
    if (val.mType == mType) {
        if (mType == evil_number_float) {
            this->fVal = this->fVal * val.fVal;
        } else if (mType == evil_number_int) {
            this->iVal = this->iVal * val.iVal;
        } else {
            assert(false); // crash
        }
    } else {
        // we assume that the val argument is the opposite of the 'this' type
        if (mType == evil_number_float) {
            this->fVal = this->fVal * val.iVal;
        } else if (mType == evil_number_int) {
            this->fVal = val.fVal * iVal;
            mType = evil_number_float;
        } else {
            assert(false); // crash
        }
    }
   // CheckIntegrity();

    return *this;
}

EvilNumber EvilNumber::_Divide( const EvilNumber & val1, const EvilNumber & val2 )
{
    EvilNumber result = EvilNumber();

    // WARNING!  There should be NO implicit or explicit use of the 'this' pointer here!
    if (val2.mType == val1.mType) {
        if (val1.mType == evil_number_float) {
            result.fVal = val1.fVal / val2.fVal;
        } else if (val1.mType == evil_number_int) {
            // make sure we can do things like 2 / 4 = 0.5f
            result.fVal = double(val1.iVal) / double(val2.iVal);
        } else {
            assert(false); // crash
        }
    } else {
        // we assume that the val1 argument type is the opposite of the val2 argument type
        if (val1.mType == evil_number_float) {
            result.fVal = val1.fVal / val2.iVal;
        } else if (val1.mType == evil_number_int) {
            result.fVal = val1.iVal / val2.fVal;
        } else {
            assert(false); // crash
        }
    }
    result.mType = evil_number_float;
   // result.CheckIntegrity();

    return result;
}

EvilNumber EvilNumber::_SelfDivide( const EvilNumber & val )
{
    if (val.mType == mType) {
        if (mType == evil_number_float) {
            this->fVal = this->fVal / val.fVal;
        } else if (mType == evil_number_int) {
            // make sure we can do things like 2 / 4 = 0.5f
            this->fVal = double(this->iVal) / double(val.iVal);
            mType = evil_number_float;
        } else {
            assert(false); // crash
        }
    } else {
        // we assume that the val argument is the opposite of the 'this' type
        if (mType == evil_number_float) {
            this->fVal = this->fVal / val.iVal;
        } else if (mType == evil_number_int) {
            this->fVal = iVal / val.fVal;
            mType = evil_number_float;
        } else {
            assert(false); // crash
        }
    }
    //CheckIntegrity();

    return *this;
}

EvilNumber EvilNumber::_Add( const EvilNumber & val1, const EvilNumber & val2 )
{
    EvilNumber result = EvilNumber();

    // WARNING!  There should be NO implicit or explicit use of the 'this' pointer here!
    if (val2.mType == val1.mType) {
        if (val1.mType == evil_number_float) {
            result.fVal = val1.fVal + val2.fVal;
            result.mType = evil_number_float;
        } else if (val1.mType == evil_number_int) {
            result.iVal = val1.iVal + val2.iVal;
            result.mType = evil_number_int;
        } else {
            assert(false); // crash
        }
    } else {
        // we assume that the val argument is the opposite of the 'this' type
        if (val1.mType == evil_number_float) {
            result.fVal = val1.fVal + val2.iVal;
        } else if (val1.mType == evil_number_int) {
            result.fVal = val1.iVal + val2.fVal;
        } else {
            assert(false); // crash
        }
        result.mType = evil_number_float;
    }
    //result.CheckIntegrity();

    return result;
}

EvilNumber EvilNumber::_SelfAdd( const EvilNumber & val )
{
    if (val.mType == mType) {
        if (mType == evil_number_float) {
            this->fVal = this->fVal + val.fVal;
        } else if (mType == evil_number_int) {
            this->iVal = this->iVal + val.iVal;
        } else {
            assert(false); // crash
        }
    } else {
        // we assume that the val argument is the opposite of the 'this' type
        if (mType == evil_number_float) {
            this->fVal = this->fVal + double(val.iVal);
        } else if (mType == evil_number_int) {
            double tVal = (double)iVal; // normal integer number
            this->fVal = tVal + val.fVal;
            mType = evil_number_float;
        } else {
            assert(false); // crash
        }
    }
    //CheckIntegrity();

    return *this;
}

EvilNumber EvilNumber::_Subtract( const EvilNumber & val1, const EvilNumber & val2 )
{
    EvilNumber result = EvilNumber();

    // WARNING!  There should be NO implicit or explicit use of the 'this' pointer here!
    if (val2.mType == val1.mType) {
        if (val1.mType == evil_number_float) {
            result.fVal = val1.fVal - val2.fVal;
            result.mType = evil_number_float;
        } else if (val1.mType == evil_number_int) {
            result.iVal = val1.iVal - val2.iVal;
            result.mType = evil_number_int;
        } else {
            assert(false); // crash
        }
    } else {
        // we assume that the val argument is the opposite of the 'this' type
        if (val1.mType == evil_number_float) {
            result.fVal = val1.fVal - double(val2.iVal);
            result.mType = evil_number_float;
        } else if (val1.mType == evil_number_int) {
            double tVal = (double)val1.iVal; // normal integer number
            result.fVal = tVal - val2.fVal;
            result.mType = evil_number_float;
        } else {
            assert(false); // crash
        }
    }
    //result.CheckIntegrity();

    return result;
}

EvilNumber EvilNumber::_SelfSubtract( const EvilNumber & val )
{
    if (val.mType == mType) {
        if (mType == evil_number_float) {
            this->fVal = this->fVal - val.fVal;
        } else if (mType == evil_number_int) {
            this->iVal = this->iVal - val.iVal;
        } else {
            assert(false); // crash
        }
    } else {
        // we assume that the val argument is the opposite of the 'this' type
        if (mType == evil_number_float) {
            this->fVal = this->fVal - double(val.iVal);
        } else if (mType == evil_number_int) {
            double tVal = (double)iVal; // normal integer number
            this->fVal = tVal - val.fVal;
            mType = evil_number_float;
        } else {
            assert(false); // crash
        }
    }
    //CheckIntegrity();

    return *this;
}

EvilNumber EvilNumber::_Modulus( const EvilNumber & val1, const EvilNumber & val2 )
{
    EvilNumber result = EvilNumber();

    // WARNING!  There should be NO implicit or explicit use of the 'this' pointer here!
    if (val2.mType == val1.mType) {
        if (val1.mType == evil_number_float) {
            result.iVal = (int64)(val1.fVal) % (int64)(val2.fVal);
            result.mType = evil_number_int;
        } else if (val1.mType == evil_number_int) {
            result.iVal = val1.iVal % val2.iVal;
            result.mType = evil_number_int;
        } else {
            assert(false); // crash
        }
    } else {
        // we assume that the val1 argument type is the opposite of the val2 argument type
        if (val1.mType == evil_number_float) {
            result.iVal = (int64)(val1.fVal) % val2.iVal;
            result.mType = evil_number_int;
        } else if (val1.mType == evil_number_int) {
            result.iVal = val1.iVal % (int64)(val2.fVal);
            result.mType = evil_number_int;
        } else {
            assert(false); // crash
        }
    }
   // result.CheckIntegrity();

    return result;
}

EvilNumber EvilNumber::_SelfModulus( const EvilNumber & val )
{
    if (val.mType == mType) {
        if (mType == evil_number_float) {
            this->iVal = (int64)(this->fVal) % (int64)(val.fVal);
            mType = evil_number_int;
        } else if (mType == evil_number_int) {
            this->iVal = this->iVal % val.iVal;
            mType = evil_number_int;
        } else {
            assert(false); // crash
        }

    } else {
        // we assume that the val argument is the opposite of the 'this' type
        if (mType == evil_number_float) {
            this->iVal = (int64)(this->fVal) % val.iVal;
            mType = evil_number_int;
        } else if (mType == evil_number_int) {
            this->iVal = this->iVal % (int64)(val.fVal);
            mType = evil_number_int;
        } else {
            assert(false); // crash
        }
    }
   // CheckIntegrity();

    return *this;
}

EvilNumber EvilNumber::_SelfIncrement()
{
    return _SelfAdd(EvilOne);
}

EvilNumber EvilNumber::_SelfDecrement()
{
    return _SelfSubtract(EvilOne);
}


// GLOBAL FUNCTIONS:

EvilNumber operator+(const EvilNumber& val, const EvilNumber& val2)
{
    EvilNumber result(val);
    result += val2;
    return result;
}

EvilNumber operator-(const EvilNumber& val, const EvilNumber& val2)
{
    EvilNumber result(val);
    result -= val2;
    return result;
}

EvilNumber operator*(const EvilNumber& val, const EvilNumber& val2)
{
    EvilNumber result(val);
    result *= val2;
    return result;
}

EvilNumber operator/(const EvilNumber& val, const EvilNumber& val2)
{
    EvilNumber result(val);
    result /= val2;
    return result;
}

EvilNumber operator%(const EvilNumber& val, const EvilNumber& val2)
{
    EvilNumber result(val);
    result %= val2;
    return result;
}
