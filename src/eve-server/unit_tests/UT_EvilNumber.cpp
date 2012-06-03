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
    Author:     Aknor Jaden
*/

#include "eve-server.h"


void UT_EvilNumber()
{

    // EVILNUMBER MATH TESTING:
    sLog.Log( "EvilNumber Testing", ">>>>> BEGIN" );

    EvilNumber fval1(5.0);
    EvilNumber fval2(-12.123);
    EvilNumber ival1(10);
    EvilNumber ival2(123456);
    EvilNumber result;

    // UNARY:
    sLog.Log( "EvilNumber Test", "before ival1++   ival1 = %s", ival1.to_str().c_str(), result.to_str().c_str() );
    result = ival1++;
    sLog.Log( "EvilNumber Test", "ival1++ = %s", ival1.to_str().c_str(), result.to_str().c_str() );

    sLog.Log( "EvilNumber Test", "before ival2--   ival2 = %s", ival2.to_str().c_str(), result.to_str().c_str() );
    result = ival2--;
    sLog.Log( "EvilNumber Test", "ival2-- = %s-- = %s", ival2.to_str().c_str(), result.to_str().c_str() );

    sLog.Log( "EvilNumber Test", "before fval1++   fval1 = %s", fval1.to_str().c_str(), result.to_str().c_str() );
    result = fval1++;
    sLog.Log( "EvilNumber Test", "fval1++ = %s++ = %s", fval1.to_str().c_str(), result.to_str().c_str() );

    sLog.Log( "EvilNumber Test", "before fval2--   fval2 = %s", fval2.to_str().c_str(), result.to_str().c_str() );
    result = fval2--;
    sLog.Log( "EvilNumber Test", "fval2-- = %s-- = %s", fval2.to_str().c_str(), result.to_str().c_str() );

    // MIXED:
    result = fval1 + ival2;
    sLog.Log( "EvilNumber Test", "fval1 + ival2 = %s + %s = %s", fval1.to_str().c_str(), ival2.to_str().c_str(), result.to_str().c_str() );

    result = fval1 - ival2;
    sLog.Log( "EvilNumber Test", "fval1 - ival2 = %s - %s = %s", fval1.to_str().c_str(), ival2.to_str().c_str(), result.to_str().c_str() );

    result = fval1 * ival2;
    sLog.Log( "EvilNumber Test", "fval1 * ival2 = %s * %s = %s", fval1.to_str().c_str(), ival2.to_str().c_str(), result.to_str().c_str() );

    result = fval1 / ival2;
    sLog.Log( "EvilNumber Test", "fval1 / ival2 = %s / %s = %s", fval1.to_str().c_str(), ival2.to_str().c_str(), result.to_str().c_str() );

    // BOTH INT:
    result = ival1 + ival2;
    sLog.Log( "EvilNumber Test", "ival1 + ival2 = %s + %s = %s", ival1.to_str().c_str(), ival2.to_str().c_str(), result.to_str().c_str() );

    result = ival1 - ival2;
    sLog.Log( "EvilNumber Test", "ival1 - ival2 = %s - %s = %s", ival1.to_str().c_str(), ival2.to_str().c_str(), result.to_str().c_str() );

    result = ival1 * ival2;
    sLog.Log( "EvilNumber Test", "ival1 * ival2 = %s * %s = %s", ival1.to_str().c_str(), ival2.to_str().c_str(), result.to_str().c_str() );

    result = ival1 / ival2;
    sLog.Log( "EvilNumber Test", "ival1 / ival2 = %s / %s = %s", ival1.to_str().c_str(), ival2.to_str().c_str(), result.to_str().c_str() );

    // BOTH FLOAT:
    result = fval1 + fval2;
    sLog.Log( "EvilNumber Test", "fval1 + fval2 = %s + %s = %s", fval1.to_str().c_str(), fval2.to_str().c_str(), result.to_str().c_str() );

    result = fval1 - fval2;
    sLog.Log( "EvilNumber Test", "fval1 - fval2 = %s - %s = %s", fval1.to_str().c_str(), fval2.to_str().c_str(), result.to_str().c_str() );

    fval2 = 2.0;
    result = fval1 * fval2;
    sLog.Log( "EvilNumber Test", "fval1 * fval2 = %s * %s = %s", fval1.to_str().c_str(), fval2.to_str().c_str(), result.to_str().c_str() );

    result = fval1 / fval2;
    sLog.Log( "EvilNumber Test", "fval1 / fval2 = %s / %s = %s", fval1.to_str().c_str(), fval2.to_str().c_str(), result.to_str().c_str() );

    result = EvilNumber(6.0) + EvilNumber(100);
    sLog.Log( "EvilNumber Test", "6.0 + 100= %s", result.to_str().c_str() );

    result = EvilNumber(52) - EvilNumber(26.0);
    sLog.Log( "EvilNumber Test", "52 - 26.0= %s", result.to_str().c_str() );

    result = EvilNumber(22) * EvilNumber(10);
    sLog.Log( "EvilNumber Test", "22 * 10= %s", result.to_str().c_str() );

    result = EvilNumber(220.0) / EvilNumber(11);
    sLog.Log( "EvilNumber Test", "220.0 / 11= %s", result.to_str().c_str() );

    result = EvilNumber::sin(EvilNumber(3.14159*1.5));
    sLog.Log( "EvilNumber Test", "sin(3.14159*1.5)= %f, EvilNumber::sin(3.14159*1.5)= %s", sin(3.14159*1.5), result.to_str().c_str() );

    result = EvilNumber::cos(EvilNumber(3.14159*1.5));
    sLog.Log( "EvilNumber Test", "cos(-3.14159*1.5)= %f, EvilNumber::cos(-3.14159*1.5)= %s", cos(-3.14159*1.5), result.to_str().c_str() );

    result = EvilNumber::tan(EvilNumber(3.14159*3));
    sLog.Log( "EvilNumber Test", "tan(-3.14159*3)= %f, EvilNumber:tan(-3.14159*3)= %s", tan(-3.14159*3), result.to_str().c_str() );

    result = EvilNumber::asin(EvilNumber(0.5));
    sLog.Log( "EvilNumber Test", "asin(0.5)= %f, EvilNumber::asin(0.5)= %s", asin(0.5), result.to_str().c_str() );

    result = EvilNumber::acos(EvilNumber(-0.5));
    sLog.Log( "EvilNumber Test", "acos(-0.5)= %f, EvilNumber::acos(-0.5)= %s", acos(-0.5), result.to_str().c_str() );

    result = EvilNumber::atan(EvilNumber(3.0));
    sLog.Log( "EvilNumber Test", "atan(3.0)= %f, EvilNumber::atan(3.0)= %s", atan(3.0), result.to_str().c_str() );

    result = EvilNumber::sqrt(EvilNumber(400.0));
    sLog.Log( "EvilNumber Test", "sqrt(400.0)= %f, EvilNumber::sqrt(400.0)= %s", sqrt(400.0), result.to_str().c_str() );

    result = EvilNumber::pow(EvilNumber(39.0), EvilNumber(2.0));
    sLog.Log( "EvilNumber Test", "pow(39.0,2.0)= %f, EvilNumber::pow(39.0,2.0)= %s", pow(39.0,2.0), result.to_str().c_str() );

    result = EvilNumber::log(EvilNumber(1.5));
    sLog.Log( "EvilNumber Test", "log(1.5)= %f, EvilNumber::log(1.5)= %s", log(1.5), result.to_str().c_str() );

    result = EvilNumber::log10(EvilNumber(1.5));
    sLog.Log( "EvilNumber Test", "log10(1.5)= %f, EvilNumber::log10(1.5)= %s", log10(1.5), result.to_str().c_str() );

    result = EvilNumber::exp(EvilNumber(1.5));
    sLog.Log( "EvilNumber Test", "exp(1.5)= %f, EvilNumber::exp(1.5)= %s", exp(1.5), result.to_str().c_str() );


    sLog.Log( "EvilNumber Testing", "<<<<< END" );

}
