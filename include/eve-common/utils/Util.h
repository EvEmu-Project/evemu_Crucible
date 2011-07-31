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

#ifndef _UTIL_H
#define _UTIL_H

#include "common.h"

#ifdef WIN32
#  define KILL_THIS_PROCESS TerminateProcess(GetCurrentProcess(), 0);
#else
#  define KILL_THIS_PROCESS kill(getpid(), -9);
#endif


	std::vector<std::string> StrSplit(const std::string &src, const std::string &sep);

	// This HAS to be called outside the threads __try / __except block!
	void SetThreadName(const char* format, ...);
	time_t convTimePeriod ( uint32 dLength, char dType);
	int32 ascent_roundf(float v);
	int32 ascent_roundd(double v);
	bool CheckIPs(const char* szIPList);
	int get_tokens(const char* szInput, char** pOutput, int iMaxCount, char cSeperator);

#ifdef WIN32
	typedef struct tagTHREADNAME_INFO
	{
		DWORD dwType; // must be 0x1000
		LPCSTR szName; // pointer to name (in user addr space)
		DWORD dwThreadID; // thread ID (-1=caller thread)
		DWORD dwFlags; // reserved for future use, must be zero
	} THREADNAME_INFO;
#endif

	int32 GetTimePeriodFromString(const char * str);
	std::string ConvertTimeStampToString(uint32 timestamp);
	std::string ConvertTimeStampToStringNC(uint32 timestamp);
	std::string ConvertTimeStampToDataTime(uint32 timestamp);

	// returns true if the ip hits the mask, otherwise false
	bool ParseCIDRBan(unsigned int IP, unsigned int Mask, unsigned int MaskBits);
	unsigned int MakeIP(const char * str);

/************************************************************************/
/* Next piece of code borrowed from the OpenFrag project, original      */
/* header below this comment chunk.                                     */
/* @note I also added tweaks of my own, adding wide string support      */
/************************************************************************/

/*
	------------------------------------------------------------------------------------
	LICENSE:
	------------------------------------------------------------------------------------
	This file is part of OpenFrag
	Copyright 2003 - 2008 The OpenFrag Team
	For the latest information visit http://www.openfrag.org 
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
	Usefull utility's for strings.
	Thanks goes also to yake - this class is a little bit bigger.
	------------------------------------------------------------------------------------
	Author:		Lars Wesselius
*/

namespace Utils
{
class Strings
{
public:
	/* trim
	 *
	 * Remove leading and trailing spaces from a string.
	 *
	 * @param str[in] - The string to trim.
	 * @param left[in] - Remove it on the left side.
	 * @param right[in] - Remove it on the right side.
	 * \return void (changed string)
	 */
	static std::string				trim(const std::string& str, bool left, bool right);

	/* trim
	 *
	 * Remove leading and trailing spaces from a string.
	 *
	 * @param str[in][out] - The string to trim.
	 * @param left[in] - Remove it on the left side.
	 * @param right[in] - Remove it on the right side.
	 */
	static void						trim(std::string& str, bool left, bool right);

	/* trim
	 *
	 * Remove leading and trailing spaces from a string.
	 *
	 * @param str[in] - The string to trim.
	 * @param left[in] - Remove it on the left side.
	 * @param right[in] - Remove it on the right side.
	 * \return void (changed string)
	 */
	static std::wstring				trim(const std::wstring& str, bool left, bool right);

	/* trim
	 *
	 * Remove leading and trailing spaces from a string.
	 *
	 * @param str[in][out] - The string to trim.
	 * @param left[in] - Remove it on the left side.
	 * @param right[in] - Remove it on the right side.
	 */
	static void						trim(std::wstring& str, bool left, bool right);

	/* toUpperCase
	 *
	 * Transforms a string into upper case characters.
	 *
	 * @param TString[in] - The string to convert.
	 * \return the string containing upper case characters.
	 */
	static std::string				toUpperCase(const std::string& TString);

	/* toUpperCase
	 *
	 * Transforms a string into upper case characters.
	 *
	 * @param[in][out] TString - The string to convert.
	 */
	static void						toUpperCase(std::string& TString);

	/* toUpperCase
	 *
	 * Transforms a string into upper case characters.
	 *
	 * @param TString[in] - The string to convert.
	 * \return the string containing upper case characters.
	 */
	static std::wstring				toUpperCase(const std::wstring& TString);

	/* toUpperCase
	 *
	 * Transforms a string into upper case characters.
	 *
	 * @param[in][out] TString - The string to convert.
	 */
	static void						toUpperCase(std::wstring& TString);

	/* toLowerCase
	 *
	 * Transforms a string into lower case characters.
	 *
	 * @param[in] TString - The string to convert.
	 * \return the string containing lower case characters.
	 */
	static std::string				toLowerCase(const std::string& TString);

	/* toLowerCase
	 *
	 * Transforms a string into lower case characters.
	 *
	 * @param TString[in][out] - The string to convert.
	 */
	static void						toLowerCase(std::string& TString);

	/* toLowerCase
	 *
	 * Transforms a string into lower case characters.
	 *
	 * @param[in] TString - The string to convert.
	 * \return the string containing lower case characters.
	 */
	static std::wstring				toLowerCase(const std::wstring& TString);

	/* toLowerCase
	 *
	 * Transforms a string into lower case characters.
	 *
	 * @param[in][out] TString - The string to convert.
	 */
	static void						toLowerCase(std::wstring& TString);

	/************************************************************************/
	/* end of the OpenFrag project header                                   */
	/************************************************************************/

	/**
	 * CaseFold
	 *
	 * Folds a a strings case.
	 *
	 */
	static std::string CaseFold(std::string & str);

	/**
	 * CaseFold
	 *
	 * Folds a a strings case.
	 *
	 */
	static std::wstring CaseFold(std::wstring & str);
};

/**
 * \class Hash
 *
 * @brief a class to keep track of all the utility hash functions
 *
 * 
 *
 * @author Captnoord.
 * @date January 2009
 */
class Hash
{
public:
	/**
	 * @brief djb2 algorithm taken from http://www.cse.yorku.ca/~oz/hash.html slightly modified
	 *
	 * 
	 *
	 * @param[in] oStr string that needs to be hashed.
	 * @return djb2 hash of the string.
	 */
	static uint32 djb2_hash(const char* str)
	{
		uint32 hash = 5381;
		int c;

		while ((c = *str++))
			hash = ((hash << 5) + hash) + c; /* hash * 33 + c */

		return hash;
	}

	/**
	* @brief djb2 algorithm taken from http://www.cse.yorku.ca/~oz/hash.html slightly modified
	*
	* 
	*
	* @param[in] oStr string that needs to be hashed.
	* @return djb2 hash of the string.
	*/
	static uint32 djb2_hash(const wchar_t *str)
	{
		uint32 hash = 5381;
		int c;

		while ((c = *str++))
			hash = ((hash << 5) + hash) + c; /* hash * 33 + c */

		return hash;
	}

	/**
	 * @brief djb2 algorithm taken from http://www.cse.yorku.ca/~oz/hash.html slightly modified
	 *
	 * 
	 *
	 * @param[in] oStr string that needs to be hashed.
	 * @return djb2 hash of the string.
	 */
	static uint32 djb2_hash(const std::string& oStr)
	{
		const char * str = oStr.c_str();

		uint32 hash = 5381;
		int c;

		while ((c = *str++))
			hash = ((hash << 5) + hash) + c; /* hash * 33 + c */

		return hash;
	}

	/**
	* @brief djb2 algorithm taken from http://www.cse.yorku.ca/~oz/hash.html slightly modified
	*
	* 
	*
	* @param[in] oStr string that needs to be hashed.
	* @return djb2 hash of the string.
	*/
	static uint32 djb2_hash(const std::wstring& oStr)
	{
		const wchar_t * str = oStr.c_str();

		uint32 hash = 5381;
		int c;

		while ((c = *str++))
			hash = ((hash << 5) + hash) + c; /* hash * 33 + c */

		return hash;
	}

	/**
	 * @brief djb2 algorithm taken from http://www.cse.yorku.ca/~oz/hash.html slightly modified
	 *
	 * 
	 *
	 * @param[in] str string that needs to be hashed.
	 * @param[in] len the size of the string.
	 * @return djb2 hash of the string.
	 */
	static uint32 djb2_hash(const char* str, int len)
	{
		uint32 hash = 5381;
		int c, i;

		for (i = 0; i < len; i++)
		{
			c = str[i];
			hash = ((hash << 5) + hash) + c; /* hash * 33 + c */
		}

		return hash;
	}

	/**
	 * @brief sdbm algorithm taken from http://www.cse.yorku.ca/~oz/hash.html slightly modified
	 *
	 * 
	 *
	 * @param[in] oStr string that needs to be hashed.
	 * @return sdbm hash of the string.
	 */
	static uint32 sdbm_hash(std::string &oStr)
	{
		uint8 *str = (uint8*)oStr.c_str();
		uint32 hash = 0;
		int c;

		while ((c = *str++))
			hash = c + (hash << 6) + (hash << 16) - hash;

		return hash;
	}

	/**
	 * @brief sdbm algorithm taken from http://www.cse.yorku.ca/~oz/hash.html slightly modified
	 *
	 * 
	 *
	 * @param[in] oStr string that needs to be hashed.
	 * @return sdbm hash of the string.
	 */
	static uint32 sdbm_hash(const char *str)
	{
		uint32 hash = 0;
		int c;

		while ((c = *str++))
			hash = c + (hash << 6) + (hash << 16) - hash;

		return hash;
	}

	/**
	* @brief sdbm algorithm taken from http://www.cse.yorku.ca/~oz/hash.html slightly modified
	*
	* 
	*
	* @param[in] oStr string that needs to be hashed.
	* @return sdbm hash of the string.
	*/
	static uint32 sdbm_hash(const wchar_t *str)
	{
		uint32 hash = 0;
		int c;

		while ((c = *str++))
			hash = c + (hash << 6) + (hash << 16) - hash;

		return hash;
	}

	/**
	* @brief sdbm algorithm taken from http://www.cse.yorku.ca/~oz/hash.html slightly modified
	*
	* 
	*
	* @param[in] oStr string that needs to be hashed.
	* @param[in] len length of the string that needs to be hashed.
	* @return sdbm hash of the string.
	*/
	static uint32 sdbm_hash(const char *str, int len)
	{
		uint32 hash = 0;
		int c;

		for (int i = 0; i < len; i++)
		{
			c = str[i];
			hash = c + (hash << 6) + (hash << 16) - hash;
		}

		return hash;
	}
};
}// namespace Utils

#endif//_UTIL_H
