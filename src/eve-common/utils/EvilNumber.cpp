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

#include "eve-common.h"

#include "python/PyRep.h"
#include "utils/EvilNumber.h"

const EvilNumber EvilTime_Second = 10000000;
const EvilNumber EvilTime_Minute = Win32Time_Second * 60;
const EvilNumber EvilTime_Hour = Win32Time_Minute * 60;
const EvilNumber EvilTime_Day = Win32Time_Hour * 24;
const EvilNumber EvilTime_Month = Win32Time_Day * 30;
const EvilNumber EvilTime_Year = Win32Time_Month * 12;

const EvilNumber EvilPI = 3.14159265358979323846;
const EvilNumber EvilE = 2.71828182845904523536;


// CONSTRUCTORS:

EvilNumber::EvilNumber() : mType(evil_number_int)
{
    mValue.iVal = 0;
}

EvilNumber::EvilNumber( int8 val ) : mType(evil_number_int)
{
    mValue.iVal = val;
}

EvilNumber::EvilNumber( uint8 val ) : mType(evil_number_int)
{
    mValue.iVal = val;
}

EvilNumber::EvilNumber( int16 val ) : mType(evil_number_int)
{
    mValue.iVal = val;
}

EvilNumber::EvilNumber( uint16 val ) : mType(evil_number_int)
{
    mValue.iVal = val;
}

EvilNumber::EvilNumber( int32 val ) : mType(evil_number_int)
{
    mValue.iVal = val;
}

EvilNumber::EvilNumber( uint32 val ) : mType(evil_number_int)
{
    mValue.iVal = val;
}

EvilNumber::EvilNumber( int64 val ) : mType(evil_number_int)
{
    mValue.iVal = val;
}

/* this is tricky, as we only handle signed calculations.
 * so this has the potentional to go wrong because uint64 can
 * hold a larger positive number than int64 can hold.  int64 can
 * hold only a number as big as half as big as the largest number
 * that uint64 can hold, so we need to be clear what the compiler
 * will do in the cast of a too-large uint64 to an int64
 */
EvilNumber::EvilNumber( uint64 val ) : mType(evil_number_int)
{
    // TODO: put some check in here for greater than maximum positive integer
    if( val > MAX_EVIL_INTEGER )
        mValue.iVal = MAX_EVIL_INTEGER;     // Intentionally saturate the integer value to maximum positive value if incoming uint64 is larger
    else
        mValue.iVal = *((int64*)&val);
}

EvilNumber::EvilNumber( float val ) : mType(evil_number_float)
{
    mValue.fVal = val;
}

EvilNumber::EvilNumber( double val ) : mType(evil_number_float)
{
    mValue.fVal = val;
}


// PUBLIC FUNCTIONS:

EvilNumber EvilNumber::sin( const EvilNumber & val )
{
    EvilNumber result;

    if( val.mType == evil_number_float )
        result.mValue.fVal = std::sin( val.mValue.fVal );
    else {
        result.mValue.fVal = std::sin( (double)(val.mValue.iVal) );
    }
    result.mType = evil_number_float;

    return result;
}

EvilNumber EvilNumber::cos( const EvilNumber & val )
{
    EvilNumber result;

    if( val.mType == evil_number_float )
        result.mValue.fVal = std::cos( val.mValue.fVal );
    else {
        result.mValue.fVal = std::cos( (double)(val.mValue.iVal) );
    }
    result.mType = evil_number_float;

    return result;
}

EvilNumber EvilNumber::tan( const EvilNumber & val )
{
    EvilNumber result;

    if( val.mType == evil_number_float )
        result.mValue.fVal = std::tan( val.mValue.fVal );
    else {
        result.mValue.fVal = std::tan( (double)(val.mValue.iVal) );
    }
    result.mType = evil_number_float;

    return result;
}

EvilNumber EvilNumber::asin( const EvilNumber & val )
{
    EvilNumber result;

    if( val.mType == evil_number_float )
        result.mValue.fVal = std::asin( val.mValue.fVal );
    else {
        result.mValue.fVal = std::asin( (double)(val.mValue.iVal) );
    }
    result.mType = evil_number_float;

    return result;
}

EvilNumber EvilNumber::acos( const EvilNumber & val )
{
    EvilNumber result;

    if( val.mType == evil_number_float )
        result.mValue.fVal = std::acos( val.mValue.fVal );
    else {
        result.mValue.fVal = std::acos( (double)(val.mValue.iVal) );
    }
    result.mType = evil_number_float;

    return result;
}

EvilNumber EvilNumber::atan( const EvilNumber & val )
{
    EvilNumber result;

    if( val.mType == evil_number_float )
        result.mValue.fVal = std::atan( val.mValue.fVal );
    else {
        result.mValue.fVal = std::atan( (double)(val.mValue.iVal) );
    }
    result.mType = evil_number_float;

    return result;
}

EvilNumber EvilNumber::sqrt( const EvilNumber & val )
{
    EvilNumber result;

    if( val.mType == evil_number_float )
        result.mValue.fVal = std::sqrt( val.mValue.fVal );
    else {
        result.mValue.fVal = std::sqrt( (double)(val.mValue.iVal) );
    }
    result.mType = evil_number_float;

    return result;
}

EvilNumber EvilNumber::pow( const EvilNumber & val1, const EvilNumber & val2 )
{
    EvilNumber exponent;
    EvilNumber result;

    if( val2.mType == evil_number_float )
        exponent.mValue.fVal = val2.mValue.fVal;
    else
        exponent.mValue.fVal = (double)(val2.mValue.iVal);
    exponent.mType = evil_number_float;

    if( val1.mType == evil_number_float )
        result.mValue.fVal = std::pow( val1.mValue.fVal, exponent.mValue.fVal );
    else {
        result.mValue.fVal = std::pow( (double)(val1.mValue.iVal), exponent.mValue.fVal );
    }
    result.mType = evil_number_float;

    return result;
}

EvilNumber EvilNumber::log( const EvilNumber & val )
{
    EvilNumber result;

    if( val.mType == evil_number_float )
        result.mValue.fVal = std::log( val.mValue.fVal );
    else {
        result.mValue.fVal = std::log( (float)(val.mValue.iVal) );
    }
    result.mType = evil_number_float;

    return result;
}

EvilNumber EvilNumber::log10( const EvilNumber & val )
{
    EvilNumber result;

    if( val.mType == evil_number_float )
        result.mValue.fVal = std::log10( val.mValue.fVal );
    else {
        result.mValue.fVal = std::log10( (float)(val.mValue.iVal) );
    }
    result.mType = evil_number_float;

    return result;
}

EvilNumber EvilNumber::exp( const EvilNumber & val )
{
    EvilNumber result;

    if( val.mType == evil_number_float )
        result.mValue.fVal = std::exp( val.mValue.fVal );
    else {
        result.mValue.fVal = std::exp( (float)(val.mValue.iVal) );
    }
    result.mType = evil_number_float;

    return result;
}

PyRep* EvilNumber::GetPyObject()
{
    if (mType == evil_number_int) {
        if ( mValue.iVal > INT_MAX || mValue.iVal < INT_MIN)
            return (PyRep*)new PyLong(mValue.iVal);
        else
            return (PyRep*)new PyInt((int32)(mValue.iVal));
    }
    else if (mType == evil_number_float)
        return (PyRep*)new PyFloat(mValue.fVal);
    else {
        assert(false);
        return false;
    }
}

inline void EvilNumber::CheckIntegrity()
{
    // check if we are a integer
    int cmp_val = (int)mValue.fVal;
    if (float(cmp_val) == mValue.fVal) {
        //we are a integer.... /me cheers...
        mValue.iVal = cmp_val;
        mType = evil_number_int;
    }
}

bool EvilNumber::to_int()
{
    // First check to see if this value is already integer and if so, do nothing:
    if( mType == evil_number_int )
        return true;

    // We're converting from double to int64, so check to see if the int64
    // type can contain the magnitude of the value in the double, and if not,
    // return false:
    double testValue = mValue.fVal;
    if( testValue < 0 )
        testValue *= -1;    // Make sign positive

    if( testValue <= (double)MAX_EVIL_INTEGER )
    {
        mValue.iVal = (int64)(mValue.fVal);
        mType = evil_number_int;
    }
    else
        return false;

    return true;
}

bool EvilNumber::to_float()
{
    // First check to see if this value is already float and if so, do nothing:
    if( mType == evil_number_float )
        return true;

    // We're converting from int64 to double, and since we know that the double
    // type can hold a magnitude larger than the int64, this is a straight-forward
    // conversion that does not result in a possible false return:
    mValue.fVal = (double)(mValue.iVal);
    mType = evil_number_float;

    return true;
}

int64 EvilNumber::get_int()
{
    if( mType != evil_number_int ) {
        int64 temp = (int64)mValue.fVal;

        /* this checks if the type convention lost stuff behind the decimal point (comma for you Europeans ;) */
        if (double(temp) != mValue.fVal)
            sLog.Warning("EvilNumber", "Invalid call get_int called on a double");

        return (int64)mValue.fVal;
    }
    return mValue.iVal;
}

double EvilNumber::get_float()
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


// PRIVATE FUNCTIONS:

EvilNumber EvilNumber::_Multiply( const EvilNumber & val1, const EvilNumber & val2 )
{
    EvilNumber result;

    // WARNING!  There should be NO implicit or explicit use of the 'this' pointer here!
    if (val2.mType == val1.mType) {
        if (val1.mType == evil_number_float) {
            result.mValue.fVal = val1.mValue.fVal * val2.mValue.fVal;
            result.mType = evil_number_float;
        } else if (val1.mType == evil_number_int) {
            result.mValue.iVal = val1.mValue.iVal * val2.mValue.iVal;
            result.mType = evil_number_int;
        }
    } else {
        // we assume that the val1 argument type is the opposite of the val2 argument type
        if (val1.mType == evil_number_float) {
            result.mValue.fVal = val1.mValue.fVal * double(val2.mValue.iVal);
            result.mType = evil_number_float;
        } else if (val1.mType == evil_number_int) {
            double tVal = (double)val1.mValue.iVal; // normal integer number
            result.mValue.fVal = tVal * val2.mValue.fVal;
            result.mType = evil_number_float;
        } else {
            assert(false); // crash
        }

        // check if we are an integer
        result.CheckIntegrity();
    }
    return result;
}

EvilNumber EvilNumber::_SelfMultiply( const EvilNumber & val )
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
            this->mValue.fVal = this->mValue.fVal * double(val.mValue.iVal);
        } else if (mType == evil_number_int) {
            double tVal = (double)mValue.iVal; // normal integer number
            this->mValue.fVal = tVal * val.mValue.fVal;
            mType = evil_number_float;
        } else {
            assert(false); // crash
        }

        // check if we are an integer
        CheckIntegrity();
    }
    return *this;
}

EvilNumber EvilNumber::_Divide( const EvilNumber & val1, const EvilNumber & val2 )
{
    EvilNumber result;

    // WARNING!  There should be NO implicit or explicit use of the 'this' pointer here!
    if (val2.mType == val1.mType) {
        if (val1.mType == evil_number_float) {
            result.mValue.fVal = val1.mValue.fVal / val2.mValue.fVal;
            result.mType = evil_number_float;
        } else if (val1.mType == evil_number_int) {
            // make sure we can do things like 2 / 4 = 0.5f
            result.mValue.fVal = double(val1.mValue.iVal) / double(val2.mValue.iVal);
            result.mType = evil_number_float;
            // check if its possibly an integer
            result.CheckIntegrity();
        }
    } else {
        // we assume that the val1 argument type is the opposite of the val2 argument type
        if (val1.mType == evil_number_float) {
            result.mValue.fVal = val1.mValue.fVal / double(val2.mValue.iVal);
            result.mType = evil_number_float;
        } else if (val1.mType == evil_number_int) {
            double tVal = (double)val1.mValue.iVal; // normal integer number
            result.mValue.fVal = tVal / val2.mValue.fVal;
            result.mType = evil_number_float;
        } else {
            assert(false); // crash
        }

        // check if we are an integer
        result.CheckIntegrity();
    }
    return result;
}

EvilNumber EvilNumber::_SelfDivide( const EvilNumber & val )
{
    if (val.mType == mType) {
        if (mType == evil_number_float) {
            this->mValue.fVal = this->mValue.fVal / val.mValue.fVal;
        } else if (mType == evil_number_int) {
            // make sure we can do things like 2 / 4 = 0.5f
            this->mValue.fVal = double(this->mValue.iVal) / double(val.mValue.iVal);
            mType = evil_number_float;
            // check if its possibly a integer
            CheckIntegrity();
        }
    } else {
        // we assume that the val argument is the opposite of the 'this' type
        if (mType == evil_number_float) {
            this->mValue.fVal = this->mValue.fVal / double(val.mValue.iVal);
        } else if (mType == evil_number_int) {
            double tVal = (double)mValue.iVal; // normal integer number
            this->mValue.fVal = tVal / val.mValue.fVal;
            mType = evil_number_float;
        } else {
            assert(false); // crash
        }

        // check if we are a integer
        CheckIntegrity();
    }
    return *this;
}

EvilNumber EvilNumber::_Add( const EvilNumber & val1, const EvilNumber & val2 )
{
    EvilNumber result;

    // WARNING!  There should be NO implicit or explicit use of the 'this' pointer here!
    if (val2.mType == val1.mType) {
        if (val1.mType == evil_number_float) {
            result.mValue.fVal = val1.mValue.fVal + val2.mValue.fVal;
            result.mType = evil_number_float;
        } else if (val1.mType == evil_number_int) {
            result.mValue.iVal = val1.mValue.iVal + val2.mValue.iVal;
            result.mType = evil_number_int;
        }
    } else {
        // we assume that the val argument is the opposite of the 'this' type
        if (val1.mType == evil_number_float) {
            result.mValue.fVal = val1.mValue.fVal + double(val2.mValue.iVal);
            result.mType = evil_number_float;
        } else if (val1.mType == evil_number_int) {
            double tVal = (double)val1.mValue.iVal; // normal integer number
            result.mValue.fVal = tVal + val2.mValue.fVal;
            result.mType = evil_number_float;
        } else {
            assert(false); // crash
        }

        // check if we are a integer
        result.CheckIntegrity();
    }
    return result;
}

EvilNumber EvilNumber::_SelfAdd( const EvilNumber & val )
{
    if (val.mType == mType) {
        if (mType == evil_number_float) {
            this->mValue.fVal = this->mValue.fVal + val.mValue.fVal;
        } else if (mType == evil_number_int) {
            this->mValue.iVal = this->mValue.iVal + val.mValue.iVal;
        }
    } else {
        // we assume that the val argument is the opposite of the 'this' type
        if (mType == evil_number_float) {
            this->mValue.fVal = this->mValue.fVal + double(val.mValue.iVal);
        } else if (mType == evil_number_int) {
            double tVal = (double)mValue.iVal; // normal integer number
            this->mValue.fVal = tVal + val.mValue.fVal;
            mType = evil_number_float;
        } else {
            assert(false); // crash
        }

        // check if we are a integer
        CheckIntegrity();
    }
    return *this;
}

EvilNumber EvilNumber::_Subtract( const EvilNumber & val1, const EvilNumber & val2 )
{
    EvilNumber result;

    // WARNING!  There should be NO implicit or explicit use of the 'this' pointer here!
    if (val2.mType == val1.mType) {
        if (val1.mType == evil_number_float) {
            result.mValue.fVal = val1.mValue.fVal - val2.mValue.fVal;
            result.mType = evil_number_float;
        } else if (val1.mType == evil_number_int) {
            result.mValue.iVal = val1.mValue.iVal - val2.mValue.iVal;
            result.mType = evil_number_int;
        }
    } else {
        // we assume that the val argument is the opposite of the 'this' type
        if (val1.mType == evil_number_float) {
            result.mValue.fVal = val1.mValue.fVal - double(val2.mValue.iVal);
            result.mType = evil_number_float;
        } else if (val1.mType == evil_number_int) {
            double tVal = (double)val1.mValue.iVal; // normal integer number
            result.mValue.fVal = tVal - val2.mValue.fVal;
            result.mType = evil_number_float;
        } else {
            assert(false); // crash
        }

        // check if we are a integer
        result.CheckIntegrity();
    }
    return result;
}

EvilNumber EvilNumber::_SelfSubtract( const EvilNumber & val )
{
    if (val.mType == mType) {
        if (mType == evil_number_float) {
            this->mValue.fVal = this->mValue.fVal - val.mValue.fVal;
        } else if (mType == evil_number_int) {
            this->mValue.iVal = this->mValue.iVal - val.mValue.iVal;
        }
    } else {
        // we assume that the val argument is the opposite of the 'this' type
        if (mType == evil_number_float) {
            this->mValue.fVal = this->mValue.fVal - double(val.mValue.iVal);
        } else if (mType == evil_number_int) {
            double tVal = (double)mValue.iVal; // normal integer number
            this->mValue.fVal = tVal - val.mValue.fVal;
            mType = evil_number_float;
        } else {
            assert(false); // crash
        }

        // check if we are a integer
        CheckIntegrity();
    }
    return *this;
}

EvilNumber EvilNumber::_Modulus( const EvilNumber & val1, const EvilNumber & val2 )
{
    EvilNumber result;

    // WARNING!  There should be NO implicit or explicit use of the 'this' pointer here!
    if (val2.mType == val1.mType) {
        if (val1.mType == evil_number_float) {
            result.mValue.iVal = (int64)(val1.mValue.fVal) % (int64)(val2.mValue.fVal);
            result.mType = evil_number_int;
        } else if (val1.mType == evil_number_int) {
            result.mValue.iVal = val1.mValue.iVal % val2.mValue.iVal;
            result.mType = evil_number_int;
        }
    } else {
        // we assume that the val1 argument type is the opposite of the val2 argument type
        if (val1.mType == evil_number_float) {
            result.mValue.iVal = (int64)(val1.mValue.fVal) % val2.mValue.iVal;
            result.mType = evil_number_int;
        } else if (val1.mType == evil_number_int) {
            result.mValue.iVal = val1.mValue.iVal % (int64)(val2.mValue.fVal);
            result.mType = evil_number_int;
        } else {
            assert(false); // crash
        }

        // check if we are a integer
        result.CheckIntegrity();
    }
    return result;
}

EvilNumber EvilNumber::_SelfModulus( const EvilNumber & val )
{
    if (val.mType == mType) {
        if (mType == evil_number_float) {
            this->mValue.iVal = (int64)(this->mValue.fVal) % (int64)(val.mValue.fVal);
            mType = evil_number_int;
        } else if (mType == evil_number_int) {
            this->mValue.iVal = this->mValue.iVal % val.mValue.iVal;
            mType = evil_number_int;
        }
    } else {
        // we assume that the val argument is the opposite of the 'this' type
        if (mType == evil_number_float) {
            this->mValue.iVal = (int64)(this->mValue.fVal) % val.mValue.iVal;
            mType = evil_number_int;
        } else if (mType == evil_number_int) {
            this->mValue.iVal = this->mValue.iVal % (int64)(val.mValue.fVal);
            mType = evil_number_int;
        } else {
            assert(false); // crash
        }

        // check if we are a integer
        CheckIntegrity();
    }
    return *this;
}

EvilNumber EvilNumber::_SelfIncrement()
{
    return _SelfAdd(EvilNumber(1));
}

EvilNumber EvilNumber::_SelfDecrement()
{
    return _SelfSubtract(EvilNumber(1));
}


// GLOBAL FUNCTIONS:

EvilNumber operator*(const EvilNumber& val, const EvilNumber& val2)
{
    EvilNumber result = val;
    result = result * val2;
    return result;

    //val.Multiply((EvilNumber &)val2);
    //return *this;
    //return EvilNumber(0);
}

EvilNumber EvilTimeNow(){
    return EvilNumber(Win32TimeNow());
}
