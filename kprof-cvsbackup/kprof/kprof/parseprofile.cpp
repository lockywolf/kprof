/***************************************************************************
                          parseprofile.cpp  -  description
                             -------------------
    begin                : Tue Jul 9 2002
    copyright            : (C) 2002 by Colin Desmond
    email                : colin@localhost.localdomain
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "parseprofile.h"

#include "cprofileinfo.h"

#include <qstring.h>
#include <qvector.h>
#include <kdebug.h>

CParseProfile::CParseProfile()
{
	mValid = true;
}

void CParseProfile::processCallGraphBlock (const QVector<SCallGraphEntry> &data,
	QVector<CProfileInfo>& profile)
{
	// process a call graph block. The list of entries is stored in the
	// order they are encountered, which is very important because the
	// order matters (see the gprof manual)

	// first, locate the primary entry
	CProfileInfo *primary = NULL;
	uint i;
	for (i = 0; i < data.count(); i++)
	{
		if (data[i]->primary)
		{
			primary = locateProfileEntry (data[i]->name, profile);
			break;
   		}
   	}

	// make sure primary is not NULL !
	if (primary == NULL)
	{
		// @@@ should perform better error reporting here
		if (i != data.count ())
		  kdDebug(80000)  << "missing flat profile entry for '" <<data[i]->name.latin1()
		  								<< "' (line "<<data[i]->line<< ")"<<endl;
  		return;
	}

	// store callers, called funcs and info about primary entry
	bool beforePrimary = true;
	for (i = 0; i < data.count (); i++)
	{
		CProfileInfo *p = locateProfileEntry (data[i]->name, profile);
	 	if (data[i]->primary)
		{
			if (data[i]->recursive)
				p->recursive = true;
			beforePrimary = false;
			continue;
   		}
		if (p)
		{
			if (beforePrimary)
			{
				if (primary->callers.count()==0 || primary->callers.find(p)==-1)
				{
					primary->callers.resize (primary->callers.count () + 1);
					primary->callers [primary->callers.count () - 1] = p;
				}
    		}
			else
			{
				if (primary->called.count()==0 || primary->called.find(p)==-1)
				{
					primary->called.resize (primary->called.count () + 1);
					primary->called [primary->called.count () - 1] = p;
				}
    		}
      	}
	}
}

CProfileInfo *CParseProfile::locateProfileEntry (const QString& name, QVector<CProfileInfo>& profile)
{
	// find an entry in our profile table based on function name
	for (uint j = 0; j < profile.count (); j++)
	{
		if (profile[j]->name == name)
			return profile[j];
	}

	return NULL;
}
