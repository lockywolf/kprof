/***************************************************************************
                          vcgCallGraph.h  -  description
                             -------------------
    begin                : Mon Jul 8 2002
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
#ifndef _VCGCALLGRAPH_H_
#define  _VCGCALLGRAPH_H_

class QFile;
class CProfileInfo;
#include <qvector.h>

class VCGCallGraph
{
public:
	VCGCallGraph (QFile& file, bool currentSelectionOnly, QVector<CProfileInfo>& profile);
	~VCGCallGraph() {};

private:

	VCGCallGraph();
};

#endif

