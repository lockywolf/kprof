/***************************************************************************
                          parseprofile_gprof.h  -  description
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

#ifndef _CPARSEPROFILE_GPROF_H_
#define _CPARSEPROFILE_GPROF_H_

#ifndef _PARSEPROFILE_H_
#include "parseprofile.h"
#endif

class CProfileInfo;

class CParseProfile_gprof : public CParseProfile
{
public:
	CParseProfile_gprof(QTextStream& t, QVector<CProfileInfo>& profile);
	~CParseProfile_gprof(){};

	bool valid () const;

private:
	CParseProfile_gprof();

};

#endif
