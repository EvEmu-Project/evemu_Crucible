#include "EVECommonPCH.h"
#include "utils/EvilNumber.h"
#include "python/PyRep.h"

const EvilNumber EvilTime_Second(10000000L);
const EvilNumber EvilTime_Minute = Win32Time_Second * 60;
const EvilNumber EvilTime_Hour = Win32Time_Minute * 60;
const EvilNumber EvilTime_Day = Win32Time_Hour * 24;
const EvilNumber EvilTime_Month = Win32Time_Day * 30;
const EvilNumber EvilTime_Year = Win32Time_Month * 12;

PyRep* EvilNumber::GetPyObject()
{
    if (mType == evil_number_int)
        return (PyRep*)new PyLong(mValue.iVal);
    else if (mType == evil_number_float)
        return (PyRep*)new PyFloat(mValue.fVal);
    else {
        assert(false);
        return false;
    }
}

void EvilNumber::CheckIntegrety()
{
    // check if we are a integer
    int cmp_val = (int)mValue.fVal;
    if (float(cmp_val) == mValue.fVal) {
        //we are a integer.... /me cheers...
        mValue.iVal = cmp_val;
        mType = evil_number_int;
    }
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

EvilNumber::EvilNumber() : mType(evil_number_nan)
{
    mValue.iVal = 0;
}

EvilNumber::EvilNumber( float val ) : mType(evil_number_float)
{
    mValue.fVal = val;
}

EvilNumber::EvilNumber( int val ) : mType(evil_number_int)
{
    mValue.iVal = val;
}

EvilNumber::EvilNumber( double val ) : mType(evil_number_float)
{
    mValue.fVal = val;
}

EvilNumber::EvilNumber( int64 val ) : mType(evil_number_int)
{
    mValue.iVal = val;
}

EvilNumber::EvilNumber( uint64 val ) : mType(evil_number_int)
{
    mValue.iVal = *((int64*)&val);
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