/***************************************************************************
                          parseprofile.h  -  description
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
#ifndef _PARSEPROFILE_H_
#define _PARSEPROFILE_H_

#include <qstring.h>
#include <qvector.h>

class CProfileInfo;

typedef struct						// structure is used while parsing the call graph for gprof and Function Check
{
	QString	name;
	long	line;
	bool	primary;
	bool	recursive;
} SCallGraphEntry;

class CParseProfile
{
public:
	CParseProfile();
	virtual ~CParseProfile(){};

	virtual bool valid () const =0;

protected:
	void processCallGraphBlock (const QVector<SCallGraphEntry> &data, QVector<CProfileInfo>& profile);
	bool mValid;
	CProfileInfo *locateProfileEntry (const QString& name, QVector<CProfileInfo>& profile);

   	enum								// states while parsing the gprof output
	{
		ANALYZING,
		PROCESS_CYCLES,					// used for Function Check parsing
		SEARCH_FLAT_PROFILE,			// for GPROF (to remove later)
		PROCESS_FLAT_PROFILE,
		PROCESS_MIN_MAX_TIME,			// used for Function Check parsing
		SEARCH_CALL_GRAPH,				// for GPROF (to remove later)
		PROCESS_CALL_GRAPH,
		DISCARD_CALL_GRAPH_ENTRY
	};

};

#endif
