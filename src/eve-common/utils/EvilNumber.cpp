#include "EVECommonPCH.h"
#include "utils/EvilNumber.h"
#include "python/PyRep.h"
#include <limits.h>

const EvilNumber EvilTime_Second = 10000000;
const EvilNumber EvilTime_Minute = Win32Time_Second * 60;
const EvilNumber EvilTime_Hour = Win32Time_Minute * 60;
const EvilNumber EvilTime_Day = Win32Time_Hour * 24;
const EvilNumber EvilTime_Month = Win32Time_Day * 30;
const EvilNumber EvilTime_Year = Win32Time_Month * 12;


// CONSTRUCTORS:

EvilNumber::EvilNumber(char val) : mType(evil_number_int)
{
    mValue.iVal = val;
}

EvilNumber::EvilNumber(unsigned char val) : mType(evil_number_int)
{
    mValue.iVal = val;
}

EvilNumber::EvilNumber() : mType(evil_number_nan)
{
    mValue.iVal = 0;
}

EvilNumber::EvilNumber( int val ) : mType(evil_number_int)
{
    mValue.iVal = val;
}

EvilNumber::EvilNumber(unsigned int val) : mType(evil_number_int)
{
    mValue.iVal = val;
}

EvilNumber::EvilNumber(long val) : mType(evil_number_int)
{
    mValue.iVal = val;
}

EvilNumber::EvilNumber(unsigned long val) : mType(evil_number_int)
{
    mValue.iVal = val;
}

EvilNumber::EvilNumber( int64 val ) : mType(evil_number_int)
{
    mValue.iVal = val;
}

/* this is tricky, as we only handle signed calculations.
 * so this has the potentional to go wrong.
 */
EvilNumber::EvilNumber( uint64 val ) : mType(evil_number_int)
{
	// TODO: put some check in here for greater than maximum positive integer
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

inline void EvilNumber::CheckIntegrety()
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

        /* this checks if the type convention lost stuff behind the comma */
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


void EvilNumber::Multiply( EvilNumber & val )
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

void EvilNumber::Divide( EvilNumber & val )
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

void EvilNumber::Add( EvilNumber & val )
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
            this->mValue.fVal = this->mValue.fVal+float(val.mValue.iVal);
        } else if (mType == evil_number_int) {
            float tVal = (float)mValue.iVal; // normal integer number
            this->mValue.fVal = tVal+val.mValue.fVal;
            mType = evil_number_float;
        } else {
            assert(false); // crash
        }

        // check if we are a integer
        CheckIntegrety();
    }
}

void EvilNumber::Sub( EvilNumber & val )
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
            this->mValue.fVal = this->mValue.fVal-float(val.mValue.iVal);
        } else if (mType == evil_number_int) {
            float tVal = (float)mValue.iVal; // normal integer number
            this->mValue.fVal = tVal-val.mValue.fVal;
            mType = evil_number_float;
        } else {
            assert(false); // crash
        }

        // check if we are a integer
        CheckIntegrety();
    }
}

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
