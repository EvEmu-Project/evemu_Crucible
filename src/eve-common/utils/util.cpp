/*
 * Ascent MMORPG Server
 * Copyright (C) 2005-2008 Ascent Team <http://www.ascentemu.com/>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Affero General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Affero General Public License for more details.
 *
 * You should have received a copy of the GNU Affero General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include "EVECommonPCH.h" // I don't want to include this here..
#include "common.h"
//#include "Threading/Threading.h"

using namespace std;

int32 ascent_roundf(float v)
{
	float int_part = floorf(v);
	float float_part = v - int_part;
	if( float_part > 0.5f )
		return float2int32(int_part + 1.0f);
	else
		return float2int32(int_part);
}

int32 ascent_roundd(double v)
{
	double int_part = floor(v);
	double float_part = v - int_part;
	if( float_part > 0.5f )
		return int32(int_part + 1.0f);
	else
		return int32(int_part);
}

vector<string> StrSplit(const string &src, const string &sep)
{
	vector<string> r;
	string s;
	for (string::const_iterator i = src.begin(); i != src.end(); i++) {
		if (sep.find(*i) != string::npos) {
			if (s.length()) r.push_back(s);
			s = "";
		} else {
			s += *i;
		}
	}
	if (s.length()) r.push_back(s);
	return r;
}

// for debug purposes... we can set the name of the thread.
#if 0
void SetThreadName(const char* format, ...)
{
	// This isn't supported on nix?
	va_list ap;
	va_start(ap, format);

#ifdef WIN32

	char thread_name[200];
	vsnprintf(thread_name, 200, format, ap);

	THREADNAME_INFO info;
	info.dwType = 0x1000;
	info.dwThreadID = GetCurrentThreadId();
	info.dwFlags = 0;
	info.szName = thread_name;

	__try
	{
#ifdef _WIN64
		RaiseException(0x406D1388, 0, sizeof(info)/sizeof(DWORD), (ULONG_PTR*)&info);
#else
		RaiseException(0x406D1388, 0, sizeof(info)/sizeof(DWORD), (DWORD*)&info);
#endif
	}
	__except(EXCEPTION_CONTINUE_EXECUTION)
	{

	}

#endif

	va_end(ap);
}
#endif//

time_t convTimePeriod ( uint32 dLength, char dType )
{
	time_t rawtime = 0;
	if (dLength == 0)
		return rawtime;
	struct tm * ti = localtime( &rawtime );
	switch(dType)
	{
		case 'h':		// hours
			ti->tm_hour += dLength;
			break;
		case 'd':		// days
			ti->tm_mday += dLength;
			break;
		case 'w':		// weeks
			ti->tm_mday += 7 * dLength;
			break;
		case 'm':		// months
			ti->tm_mon += dLength;
			break;
		case 'y':		// years
			// are leap years considered ? do we care ?
			ti->tm_year += dLength;
			break;
		default:		// minutes
			ti->tm_min += dLength;
			break;
	}
	return mktime(ti);
}
#if 0
int32 GetTimePeriodFromString(const char * str)
{
	uint32 time_to_ban = 0;
	char * p = (char*)str;
	uint32 multiplier;
	string number_temp;
	uint32 multipliee;
	number_temp.reserve(10);

	while(*p != 0)
	{
		// always starts with a number.
		if(!isdigit(*p))
			break;

		number_temp.clear();
		while(isdigit(*p) && *p != 0)
		{
			number_temp += *p;
			++p;
		}

		// try and find a letter
		if(*p == 0)
			break;

		// check the type
		switch(tolower(*p))
		{
		case 'y':
			multiplier = TIME_YEAR;		// eek!
			break;

		case 'm':
			multiplier = TIME_MONTH;
			break;

		case 'd':
			multiplier = TIME_DAY;
			break;

		case 'h':
			multiplier = TIME_HOUR;
			break;

		default:
			return -1;
			break;
		}

		++p;
		multipliee = atoi(number_temp.c_str());
		time_to_ban += (multiplier * multipliee);
	}

	return time_to_ban;
}
#endif

const char * szDayNames[7] = {
	"Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday"
};

const char * szMonthNames[12] = {
	"January", "February", "March", "April", "May", "June", "July", "August", "September", "October", "November", "December"
};

void MakeIntString(char * buf, int num)
{
	if(num<10)
	{
		buf[0] = '0';
		//itoa(num, &buf[1], 10);
		sprintf(&buf[1], "%u", num);
	}
	else
	{
		//itoa(num,buf,10);
		sprintf(buf,"%u",num);
	}
}

void MakeIntStringNoZero(char * buf, int num)
{
	//itoa(num,buf,10);
	sprintf(buf,"%u",num);
}

string ConvertTimeStampToString(uint32 timestamp)
{
	int seconds = (int)timestamp;
	int mins=0;
	int hours=0;
	int days=0;
	int months=0;
	int years=0;
	if(seconds >= 60)
	{
		mins = seconds / 60;
		if(mins)
		{
			seconds -= mins*60;
			if(mins >= 60)
			{
				hours = mins / 60;
				if(hours)
				{
					mins -= hours*60;
					if(hours >= 24)
					{
						days = hours/24;
						if(days)
						{
							hours -= days*24;
							if(days >= 30)
							{
								months = days / 30;
								if(months)
								{
									days -= months*30;
									if(months >= 12)
									{
										years = months / 12;
										if(years)
										{
											months -= years*12;
										}
									}
								}
							}
						}
					}
				}
			}
		}
	}

	char szTempBuf[100];
	string szResult;

	if(years) {
		MakeIntStringNoZero(szTempBuf, years);
		szResult += szTempBuf;
		szResult += " years, ";
	}

	if(months) {
		MakeIntStringNoZero(szTempBuf, months);
		szResult += szTempBuf;
		szResult += " months, ";
	}

	if(days) {
		MakeIntStringNoZero(szTempBuf, days);
		szResult += szTempBuf;
		szResult += " days, ";
	}

	if(hours) {
		MakeIntStringNoZero(szTempBuf, hours);
		szResult += szTempBuf;
		szResult += " hours, ";
	}

	if(mins) {
		MakeIntStringNoZero(szTempBuf, mins);
		szResult += szTempBuf;
		szResult += " minutes, ";
	}

	if(seconds) {
		MakeIntStringNoZero(szTempBuf, seconds);
		szResult += szTempBuf;
		szResult += " seconds";
	}

	return szResult;
}

string ConvertTimeStampToStringNC(uint32 timestamp)
{
	int seconds = (int)timestamp;
	int mins=0;
	int hours=0;
	int days=0;
	int months=0;
	int years=0;
	if(seconds >= 60)
	{
		mins = seconds / 60;
		if(mins)
		{
			seconds -= mins*60;
			if(mins >= 60)
			{
				hours = mins / 60;
				if(hours)
				{
					mins -= hours*60;
					if(hours >= 24)
					{
						days = hours/24;
						if(days)
						{
							hours -= days*24;
							if(days >= 30)
							{
								months = days / 30;
								if(months)
								{
									days -= months*30;
									if(months >= 12)
									{
										years = months / 12;
										if(years)
										{
											months -= years*12;
										}
									}
								}
							}
						}
					}
				}
			}
		}
	}

	char szTempBuf[100];
	string szResult;

	if(years) {
		MakeIntStringNoZero(szTempBuf, years);
		szResult += szTempBuf;
		szResult += " yrs ";
	}

	if(months) {
		MakeIntStringNoZero(szTempBuf, months);
		szResult += szTempBuf;
		szResult += " months ";
	}

	if(days) {
		MakeIntStringNoZero(szTempBuf, days);
		szResult += szTempBuf;
		szResult += " days ";
	}

	if(hours) {
		MakeIntStringNoZero(szTempBuf, hours);
		szResult += szTempBuf;
		szResult += " hrs ";
	}

	if(mins) {
		MakeIntStringNoZero(szTempBuf, mins);
		szResult += szTempBuf;
		szResult += " mins ";
	}

	if(seconds) {
		MakeIntStringNoZero(szTempBuf, seconds);
		szResult += szTempBuf;
		szResult += " secs";
	}

	return szResult;
}

string ConvertTimeStampToDataTime(uint32 timestamp)
{
	char szTempBuf[100];
	time_t t = (time_t)timestamp;
	tm * pTM = localtime(&t);

	string szResult;
	szResult += szDayNames[pTM->tm_wday];
	szResult += ", ";

	MakeIntString(szTempBuf, pTM->tm_mday);
	szResult += szTempBuf;
	szResult += " ";

	szResult += szMonthNames[pTM->tm_mon];
	szResult += " ";

	MakeIntString(szTempBuf, pTM->tm_year+1900);
	szResult += szTempBuf;
	szResult += ", ";
	MakeIntString(szTempBuf, pTM->tm_hour);
	szResult += szTempBuf;
	szResult += ":";
	MakeIntString(szTempBuf, pTM->tm_min);
	szResult += szTempBuf;
	szResult += ":";
	MakeIntString(szTempBuf, pTM->tm_sec);
	szResult += szTempBuf;

	return szResult;
}

// returns true if the ip hits the mask, otherwise false
bool ParseCIDRBan(unsigned int IP, unsigned int Mask, unsigned int MaskBits)
{
	// CIDR bans are a compacted form of IP / Submask
	// So 192.168.1.0/255.255.255.0 would be 192.168.1.0/24
	// IP's in the 192.168l.1.x range would be hit, others not.
	unsigned char * source_ip = (unsigned char*)&IP;
	unsigned char * mask = (unsigned char*)&Mask;
	int full_bytes = MaskBits / 8;
	int leftover_bits = MaskBits % 8;
	//int byte;

	// sanity checks for the data first
	if( MaskBits > 32 )
	{
		return false;
	}

	// this is the table for comparing leftover bits
	static const unsigned char leftover_bits_compare[9] = {
		0x00,			// 00000000
		0x80,			// 10000000
		0xC0,			// 11000000
		0xE0,			// 11100000
		0xF0,			// 11110000
		0xF8,			// 11111000
		0xFC,			// 11111100
		0xFE,			// 11111110
		0xFF,			// 11111111 - This one isn't used
	};

	// if we have any full bytes, compare them with memcpy
	if( full_bytes > 0 )
	{
		if( memcmp( source_ip, mask, full_bytes ) != 0 )
		{
			return false;
		}
	}

	// compare the left over bits
	if( leftover_bits > 0 )
	{
		if( ( source_ip[full_bytes] & leftover_bits_compare[leftover_bits] ) !=
			( mask[full_bytes] & leftover_bits_compare[leftover_bits] ) )
		{
			// one of the bits does not match
			return false;
		}
	}

	// all of the bits match that were testable
	return true;
}

unsigned int MakeIP(const char * str)
{
	unsigned int bytes[4];
	unsigned int res;
	if( sscanf(str, "%u.%u.%u.%u", &bytes[0], &bytes[1], &bytes[2], &bytes[3]) != 4 )
	{
		return 0;
	}

	res = bytes[0] | (bytes[1] << 8) | (bytes[2] << 16) | (bytes[3] << 24);
	return res;
}

int get_tokens(const char* szInput, char** pOutput, int iMaxCount, char cSeperator)
{
	char* p;
	char* q;
	int count = 0;

	p = (char*)szInput;
	q = p;

	for(;;)
	{
		// get some tokenz!
		while(*p != '\0' && *p != cSeperator)
		{
			++p;
		}

		// add token
		if( *p == 0 )
		{
			// end of string.
			// do we have data?
			if( p != q )
				pOutput[count++] = q;

			// bai.
			break;
		}

		*p = 0;
		++p;

		// set output pointer
		pOutput[count++] = q;

		// skip while bytes are the same as the seperator
		while(*p == cSeperator)
			++p;

		// reset q pointer
		q = p;

		// make sure we're not at EOL
		if( *p == '\0' )
			break;

		// make sure we don't overflow
		if( count == iMaxCount )
			break;
	}

	return count;
}

bool CheckIPs(const char* szIPList)
{
#ifndef WIN32
	return true;
#endif

	char* tokens[30];
	int masks[30];
	unsigned int ips[30];
	hostent* ent;
	char computer_name[100] = {0};
	char* ip_string = strdup(szIPList);
	int count = get_tokens(ip_string, (char**)tokens, 30, ':');
	int i, j;
	
	char* t;
	bool pass = false;
#ifdef WIN32
	DWORD computer_name_len = 100;
	GetComputerNameA(computer_name, &computer_name_len);
#endif
	for(i = 0; i < count; ++i)
	{
		t = strchr(tokens[i], '/');
		if( t != NULL )
		{
			*t = 0;
			masks[i] = atoi(t+1);
		}
		else
			masks[i] = 32;

		ips[i] = inet_addr(tokens[i]);
	}

	ent = gethostbyname(computer_name);
	if( ent == NULL )
	{
		free(ip_string);
		return false;
	}

	for( i = 0; ent->h_addr_list[i] != NULL; ++i )
	{
		pass = false;
		for( j = 0; j < count; ++j )
		{
			if( ParseCIDRBan(*(unsigned int*)ent->h_addr_list[i], ips[j], masks[j]) )
			{
				pass = true;
				break;
			}
		}

		if( !pass )
		{
			free(ip_string);
			return false;
		}
	}

	free(ip_string);
	return true;
}

/************************************************************************/
/* Start of OpenFrag borrowed code                                      */
/************************************************************************/
namespace Utils
{
std::string Strings::trim(const std::string& str, bool left, bool right)
{
	size_t lspaces, rspaces, len = str.length(), i;

	lspaces = rspaces = 0;

	if(left == true)
	{
		for(i = 0;
			i < len && (str[i] == ' ' || str[i] == '\t' || str[i] == '\r');
			++lspaces, ++i);
	}

	if(right == true && lspaces < len)
	{
		for(i = len - 1;
			i >= 0 && (str[i] == ' ' || str[i] == '\t' || str[i] == '\r');
			rspaces++, i--);
	}

	return str.substr(lspaces, len-lspaces-rspaces);
}

void Strings::trim(std::string& str, bool left, bool right)
{
	size_t lspaces, rspaces, len = str.length(), i;

	lspaces = rspaces = 0;

	if(left == true)
	{
		for(i = 0;
			i < len && (str[i] == ' ' || str[i] == '\t' || str[i] == '\r');
			++lspaces, ++i);
	}

	if(right == true && lspaces < len)
	{
		for(i = len - 1;
			i >= 0 && (str[i] == ' ' || str[i] == '\t' || str[i] == '\r');
			rspaces++, i--);
	}

	// TODO fit for improvement.
	str = str.substr(lspaces, len-lspaces-rspaces);
}

std::wstring Strings::trim(const std::wstring& str, bool left, bool right)
{
	size_t lspaces, rspaces, len = str.length(), i;

	lspaces = rspaces = 0;

	if(left == true)
	{
		for(i = 0;
			i < len && (str[i] == ' ' || str[i] == '\t' || str[i] == '\r');
			++lspaces, ++i);
	}

	if(right == true && lspaces < len)
	{
		for(i = len - 1;
			i >= 0 && (str[i] == ' ' || str[i] == '\t' || str[i] == '\r');
			rspaces++, i--);
	}

	return str.substr(lspaces, len-lspaces-rspaces);
}

void Strings::trim(std::wstring& str, bool left, bool right)
{
	size_t lspaces, rspaces, len = str.length(), i;

	lspaces = rspaces = 0;

	if(left == true)
	{
		for(i = 0;
			i < len && (str[i] == ' ' || str[i] == '\t' || str[i] == '\r');
			++lspaces, ++i);
	}

	if(right == true && lspaces < len)
	{
		for(i = len - 1;
			i >= 0 && (str[i] == ' ' || str[i] == '\t' || str[i] == '\r');
			rspaces++, i--);
	}

	// TODO fit for improvement.
	str = str.substr(lspaces, len-lspaces-rspaces);
}

std::string Strings::toUpperCase(const std::string& TString)
{
	std::string retval(TString);
	std::transform(retval.begin(), retval.end(), retval.begin(), static_cast<int(*)(int)>(::toupper));
	return retval;
}

void Strings::toUpperCase(std::string& TString)
{
	std::transform(TString.begin(), TString.end(), TString.begin(), static_cast<int(*)(int)>(::toupper));
}

std::wstring Strings::toUpperCase(const std::wstring& TString) 
{
	std::wstring retval(TString);
	std::transform(retval.begin(), retval.end(), retval.begin(), static_cast<int(*)(int)>(::toupper));
	return retval;
}

void Strings::toUpperCase(std::wstring& TString)
{
	std::transform(TString.begin(), TString.end(), TString.begin(), static_cast<int(*)(int)>(::toupper));
}

std::string Strings::toLowerCase(const std::string& TString) 
{
	std::string retval(TString);
	std::transform(retval.begin(), retval.end(), retval.begin(), static_cast<int(*)(int)>(::tolower));
	return retval;
}

void Strings::toLowerCase(std::string& TString)
{
	std::transform(TString.begin(), TString.end(), TString.begin(), static_cast<int(*)(int)>(::tolower));
}

std::wstring Strings::toLowerCase(const std::wstring& TString) 
{
	std::wstring retval(TString);
	std::transform(retval.begin(), retval.end(), retval.begin(), static_cast<int(*)(int)>(::tolower));
	return retval;
}

void Strings::toLowerCase(std::wstring& TString)
{
	std::transform(TString.begin(), TString.end(), TString.begin(), static_cast<int(*)(int)>(::tolower));
}

#ifndef WIN32
/**

 * C++ version char* style "itoa":

 */

char* itoa( int value, char* result, int base ) {

	// check that the base if valid

	if (base < 2 || base > 16) { *result = 0; return result; }

	char* out = result;

	int quotient = value;

	do {

		*out = "0123456789abcdef"[ std::abs( quotient % base ) ];

		++out;

		quotient /= base;

	} while ( quotient );

	// Only apply negative sign for base 10

	if ( value < 0 && base == 10) *out++ = '-';

	std::reverse( result, out );

	*out = 0;

	return result;
}
#endif

/************************************************************************/
/* End of OpenFrag borrowed code                                        */
/************************************************************************/
std::string Strings::CaseFold( std::string & str )
{
	std::string s2 = str;
	Utils::Strings::toUpperCase(s2);
	Utils::Strings::toLowerCase(s2);
	if (s2 != str)
		return CaseFold(s2);
	return s2;
}

std::wstring Strings::CaseFold( std::wstring & str )
{
	std::wstring s2 = str;
	Utils::Strings::toUpperCase(s2);
	Utils::Strings::toLowerCase(s2);
	if (s2 != str)
		return CaseFold(s2);
	return s2;
}

std::wstring Strings::StringToWString( std::string & str )
{
    /* convert from multi byte strings to wide character strings */
    std::wstring wstr;
    wstr.resize( str.size() );
    int ret_len = mbstowcs( &wstr[0], str.c_str(), str.size() );

    if (ret_len != str.size()) {

        sLog.Error("Utils::Strings::StringToWString", "unable to convert std::string to std::wstring, wide character string");
    }
    return wstr;
}

}//namespace Utils
