/*
 * cprofileinfo.h
 *
 * $Id$
 *
 * Copyright (c) 2000-2001 Florent Pillet <fpillet@users.sourceforge.net>
 *
 * Requires the Qt widget libraries, available at no cost at
 * http://www.trolltech.com/
 *
 * Requires the K Desktop Environment 2.0 (KDE 2.0) libraries or later,
 * available at no cost at http://www.kde.org/
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

#ifndef __CPROFILEINFO_H__
#define __CPROFILEINFO_H__

#include <qvector.h>


/*
 * DESCRIPTION:
 * This class contains all profiling information about a function. We store
 * a master list of CProfileInfo and use pointers to them in the various
 * list items.
 *
 */

class CProfileInfo
{
public:
	QString		name;					// full function/method prototype
	QString		simplifiedName;			// full function/method prototype with templates removed
	QString		object;					// name of object if this is an object method
	QString		method;					// method name without class name nor arguments
	QString		arguments;				// function/method arguments
	QArray<CProfileInfo *> called;		// list of functions called by this one
	QArray<CProfileInfo *> callers;		// list of functions that this one calls

	float		cumPercent;
	float		cumSeconds;
	float		selfSeconds;
	float		selfTsPerCall;
	float		totalTsPerCall;
	long		calls;					// number of times this one was called
	long		selfCycles;				// number of cycles for this entry (when appropriate)
	long		cumCycles;				// cumulative cycles (self + kids)
	uint		ind;					// index of this entry in the flat profile table (used to generate call graphs)
	bool		recursive;
	bool		multipleSignatures;		// if true, this method name has multiple signatures
	bool		output;					// temporary boolean used to output a partial call-graph

public:
	CProfileInfo ()
		:	cumPercent (0.0),
			cumSeconds (0.0),
			selfSeconds (0.0),
			selfTsPerCall (0.0),
			totalTsPerCall (0.0),
			calls (0),
			selfCycles (0),
			cumCycles (0),
			ind (0),
			recursive (false),
			multipleSignatures (false)
		{
		}

	~CProfileInfo()
		{
		}
};

#endif