/***************************************************************************
                          parseprofile_fnccheck.h  -  description
                             -------------------
    begin                : Tue Jul 9 2002
    copyright            : (C) 2002 by Colin Desmond
    email                : colin.desmond@btopenworld.com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#ifndef _CPARSEPROFILE_FNCCHECK_H_
#define _CPARSEPROFILE_FNCCHECK_H_

#ifndef _PARSEPROFILE_H_
#include "parseprofile.h"
#endif

class CProfileInfo;
class QTextStream;

class CParseProfile_fnccheck : public CParseProfile
{
public:
	CParseProfile_fnccheck(QTextStream& t, QVector<CProfileInfo>& profile);
	~CParseProfile_fnccheck(){};

	bool valid () const;
	
private:
	CParseProfile_fnccheck();

};

#endif
