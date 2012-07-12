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
    Author:        Zhur
*/

#include "eve-core.h"

#include "utils/DirWalker.h"

DirWalker::DirWalker()
#ifdef HAVE_WINDOWS_H
: mFind( INVALID_HANDLE_VALUE ),
  mFindData(),
  mValid( false ),
  mFirst( false )
#else /* !HAVE_WINDOWS_H */
: mDir( NULL ),
  mFile( NULL ),
  mSuffix()
#endif /* !HAVE_WINDOWS_H */
{
}

DirWalker::~DirWalker()
{
    CloseDir();
}

const char* DirWalker::currentFileName()
{
#ifdef HAVE_WINDOWS_H
    return mValid ? mFindData.cFileName : NULL;
#else /* !HAVE_WINDOWS_H */
    return NULL != mFile ? mFile->d_name : NULL;
#endif /* !HAVE_WINDOWS_H */
}

bool DirWalker::OpenDir( const char* dir, const char* suffix )
{
    CloseDir();

#ifdef HAVE_WINDOWS_H
    // Let Windows do the suffix matching
    std::string d( dir );
    d += "/*";
    d += suffix;

    mFind = ::FindFirstFile( d.c_str(), &mFindData );
    mValid = ( INVALID_HANDLE_VALUE != mFind );
    mFirst = true;

    // If no file was found, see if at least
    // the directory was found ...
    return ( mValid ? true : ( ERROR_PATH_NOT_FOUND != ::GetLastError() ) );
#else /* !HAVE_WINDOWS_H */
    mDir = ::opendir( dir );
    mFile = NULL;
    mSuffix = suffix;

    return ( NULL != mDir );
#endif /* !HAVE_WINDOWS_H */
}

void DirWalker::CloseDir()
{
#ifdef HAVE_WINDOWS_H
    if( INVALID_HANDLE_VALUE != mFind )
        ::FindClose( mFind );

    mValid = false;
    mFirst = false;
#else /* !HAVE_WINDOWS_H */
    if( NULL != mDir )
        ::closedir( mDir );

    mFile = NULL;
    mSuffix.clear();
#endif /* !HAVE_WINDOWS_H */
}

bool DirWalker::NextFile()
{
#ifdef HAVE_WINDOWS_H
    if( INVALID_HANDLE_VALUE == mFind )
        return false;

    if( mFirst )
        mFirst = false;
    else
        mValid = ( TRUE == ::FindNextFile( mFind, &mFindData ) );

    return mValid;
#else /* !HAVE_WINDOWS_H */
    if( NULL == mDir )
        return false;

    // We need to do the suffix matching here
    while( NULL != ( mFile = ::readdir( mDir ) ) )
    {
        const std::string filename( currentFileName() );
        const size_t pos = filename.rfind( mSuffix );

        if( std::string::npos == pos )
            continue;
        else if( filename.length() != ( pos + mSuffix.length() ) )
            continue;
        else
            return true;
    }

    return false;
#endif /* !HAVE_WINDOWS_H */
}
