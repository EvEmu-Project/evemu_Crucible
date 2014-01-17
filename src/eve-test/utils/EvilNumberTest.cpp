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

#include "eve-test.h"

int utils_EvilNumberTest( int argc, char* argv[] )
{
    EvilNumber fval1(5.0);
    EvilNumber fval2(-12.123);
    EvilNumber ival1(10);
    EvilNumber ival2(123456);
    EvilNumber result;

	::printf( "\n\n========================================================================\n" );
	::printf( " Testing Evilnumber unary operators:\n\n" );
    // UNARY:
    ::printf( "before ival1++   ival1 = %s\n", ival1.to_str().c_str(), result.to_str().c_str() );
    result = ival1++;
    ::printf( "ival1++ = %s\n\n", ival1.to_str().c_str(), result.to_str().c_str() );

    ::printf( "before ival2--   ival2 = %s\n", ival2.to_str().c_str(), result.to_str().c_str() );
    result = ival2--;
    ::printf( "ival2-- = %s-- = %s\n\n", ival2.to_str().c_str(), result.to_str().c_str() );

    ::printf( "before fval1++   fval1 = %s\n", fval1.to_str().c_str(), result.to_str().c_str() );
    result = fval1++;
    ::printf( "fval1++ = %s++ = %s\n\n", fval1.to_str().c_str(), result.to_str().c_str() );

    ::printf( "before fval2--   fval2 = %s\n", fval2.to_str().c_str(), result.to_str().c_str() );
    result = fval2--;
    ::printf( "fval2-- = %s-- = %s\n\n", fval2.to_str().c_str(), result.to_str().c_str() );

	::printf( "\n\n========================================================================\n" );
	::printf( " Testing Evilnumber binary operators mixing integer-based with float-based:\n\n" );
    // MIXED:
    result = fval1 + ival2;
    ::printf( "fval1 + ival2 = %s + %s = %s\n", fval1.to_str().c_str(), ival2.to_str().c_str(), result.to_str().c_str() );

    result = fval1 - ival2;
    ::printf( "fval1 - ival2 = %s - %s = %s\n", fval1.to_str().c_str(), ival2.to_str().c_str(), result.to_str().c_str() );

    result = fval1 * ival2;
    ::printf( "fval1 * ival2 = %s * %s = %s\n", fval1.to_str().c_str(), ival2.to_str().c_str(), result.to_str().c_str() );

    result = fval1 / ival2;
    ::printf( "fval1 / ival2 = %s / %s = %s\n", fval1.to_str().c_str(), ival2.to_str().c_str(), result.to_str().c_str() );

	::printf( "\n\n========================================================================\n" );
	::printf( " Testing Evilnumber binary operators, both operands integer-based:\n\n" );
    // BOTH INT:
    result = ival1 + ival2;
    ::printf( "ival1 + ival2 = %s + %s = %s\n", ival1.to_str().c_str(), ival2.to_str().c_str(), result.to_str().c_str() );

    result = ival1 - ival2;
    ::printf( "ival1 - ival2 = %s - %s = %s\n", ival1.to_str().c_str(), ival2.to_str().c_str(), result.to_str().c_str() );

    result = ival1 * ival2;
    ::printf( "ival1 * ival2 = %s * %s = %s\n", ival1.to_str().c_str(), ival2.to_str().c_str(), result.to_str().c_str() );

    result = ival1 / ival2;
    ::printf( "ival1 / ival2 = %s / %s = %s\n", ival1.to_str().c_str(), ival2.to_str().c_str(), result.to_str().c_str() );

	::printf( "\n\n========================================================================\n" );
	::printf( " Testing Evilnumber binary operators, both operands float-based:\n\n" );
    // BOTH FLOAT:
    result = fval1 + fval2;
    ::printf( "fval1 + fval2 = %s + %s = %s\n", fval1.to_str().c_str(), fval2.to_str().c_str(), result.to_str().c_str() );

    result = fval1 - fval2;
    ::printf( "fval1 - fval2 = %s - %s = %s\n", fval1.to_str().c_str(), fval2.to_str().c_str(), result.to_str().c_str() );

    fval2 = 2.0;
    result = fval1 * fval2;
    ::printf( "fval1 * fval2 = %s * %s = %s\n", fval1.to_str().c_str(), fval2.to_str().c_str(), result.to_str().c_str() );

    result = fval1 / fval2;
    ::printf( "fval1 / fval2 = %s / %s = %s\n", fval1.to_str().c_str(), fval2.to_str().c_str(), result.to_str().c_str() );

	::printf( "\n\n========================================================================\n" );
	::printf( " Testing Evilnumber binary operators, both operands created using EvilNumber() inline:\n\n" );
    result = EvilNumber(6.0) + EvilNumber(100);
    ::printf( "EvilNumber(6.0) + EvilNumber(100)= %s\n", result.to_str().c_str() );

    result = EvilNumber(52) - EvilNumber(26.0);
    ::printf( "EvilNumber(52) - EvilNumber(26.0)= %s\n", result.to_str().c_str() );

    result = EvilNumber(22) * EvilNumber(10);
    ::printf( "EvilNumber(22) * EvilNumber(10)= %s\n", result.to_str().c_str() );

    result = EvilNumber(220.0) / EvilNumber(11);
    ::printf( "EvilNumber(220.0) / EvilNumber(11)= %s\n", result.to_str().c_str() );

	::printf( "\n\n========================================================================\n" );
	::printf( " Testing Evilnumber trigonometric functions:\n\n" );
    result = EvilNumber::sin(EvilNumber(3.14159*1.5));
    ::printf( "sin(3.14159*1.5)= %f, EvilNumber::sin(3.14159*1.5)= %s\n", sin(3.14159*1.5), result.to_str().c_str() );

    result = EvilNumber::cos(EvilNumber(3.14159*1.5));
    ::printf( "cos(-3.14159*1.5)= %f, EvilNumber::cos(-3.14159*1.5)= %s\n", cos(-3.14159*1.5), result.to_str().c_str() );

    result = EvilNumber::tan(EvilNumber(3.14159*3));
    ::printf( "tan(-3.14159*3)= %f, EvilNumber:tan(-3.14159*3)= %s\n", tan(-3.14159*3), result.to_str().c_str() );

    result = EvilNumber::asin(EvilNumber(0.5));
    ::printf( "asin(0.5)= %f, EvilNumber::asin(0.5)= %s\n", asin(0.5), result.to_str().c_str() );

    result = EvilNumber::acos(EvilNumber(-0.5));
    ::printf( "acos(-0.5)= %f, EvilNumber::acos(-0.5)= %s\n", acos(-0.5), result.to_str().c_str() );

    result = EvilNumber::atan(EvilNumber(3.0));
    ::printf( "atan(3.0)= %f, EvilNumber::atan(3.0)= %s\n", atan(3.0), result.to_str().c_str() );

	::printf( "\n\n========================================================================\n" );
	::printf( " Testing Evilnumber various math functions:\n\n" );
    result = EvilNumber::sqrt(EvilNumber(400.0));
    ::printf( "sqrt(400.0)= %f, EvilNumber::sqrt(400.0)= %s\n", sqrt(400.0), result.to_str().c_str() );

    result = EvilNumber::pow(EvilNumber(39.0), EvilNumber(2.0));
    ::printf( "pow(39.0,2.0)= %f, EvilNumber::pow(39.0,2.0)= %s\n", pow(39.0,2.0), result.to_str().c_str() );

    result = EvilNumber::log(EvilNumber(1.5));
    ::printf( "log(1.5)= %f, EvilNumber::log(1.5)= %s\n", log(1.5), result.to_str().c_str() );

    result = EvilNumber::log10(EvilNumber(1.5));
    ::printf( "log10(1.5)= %f, EvilNumber::log10(1.5)= %s\n", log10(1.5), result.to_str().c_str() );

    result = EvilNumber::exp(EvilNumber(1.5));
    ::printf( "exp(1.5)= %f, EvilNumber::exp(1.5)= %s\n", exp(1.5), result.to_str().c_str() );

	::printf( "\n\n========================================================================\n" );
	::printf( " Testing Evilnumber binary operators with mixed data types:\n\n" );
	
	EvilNumber three = 3.0;

	result = three + 1;
    ::printf( "EvilNumber(3.0) + 1= %s\n", result.to_str().c_str() );

	result = three - 1;
    ::printf( "EvilNumber(3.0) - 1= %s\n", result.to_str().c_str() );

	result = three * 5;
    ::printf( "EvilNumber(3.0) * 5= %s\n", result.to_str().c_str() );

	result = EvilNumber(33.0) / 3;
    ::printf( "EvilNumber(33.0) / 3= %s\n", result.to_str().c_str() );

	result = EvilNumber(33.0) % 5;
    ::printf( "EvilNumber(33.0) %% 5= %s\n", result.to_str().c_str() );

	result = 4 + EvilNumber(3.0);
    ::printf( "4 + EvilNumber(3.0)= %s\n", result.to_str().c_str() );

	result = 1 - EvilNumber(3.0);
    ::printf( "1 - EvilNumber(3.0)= %s\n", result.to_str().c_str() );

	result = 5 * EvilNumber(3.0);
    ::printf( "5 * EvilNumber(3.0)= %s\n", result.to_str().c_str() );

	result = 33 / EvilNumber(3.0);
    ::printf( "33 / EvilNumber(3.0)= %s\n", result.to_str().c_str() );

	result = 33 % EvilNumber(5.0);
    ::printf( "33 %% EvilNumber(5.0)= %s\n", result.to_str().c_str() );

    return EXIT_SUCCESS;
}
