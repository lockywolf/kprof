/***************************************************************************
                          parseprofile_pose.h  -  description
                             -------------------
    begin                : Wed Jul 10 2002
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
#ifndef _CPARSEPROFILE_POSE_H_
#define _CPARSEPROFILE_POSE_H_

#ifndef _PARSEPROFILE_H_
#include "parseprofile.h"
#endif

class CProfileInfo;
class QString;

class CParseProfile_pose : public CParseProfile
{
public:
	CParseProfile_pose(QTextStream& t, QVector<CProfileInfo>& profile);
	~CParseProfile_pose(){};

	bool valid () const;
	
private:
	CParseProfile_pose();

	typedef struct					 	// structure holding call-graph data for PalmOS Emulator results
	{
		int		index;
		int		parent;
	} SPoseCallGraph;

};

#endif
