/***************************************************************************
                          dotCallGraph.h  -  description
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
#ifndef _DOTCALLGRAPH_H_
#define _DOTCALLGRAPH_H_

class QFile;
class CProfileInfo;
#include <qvector.h>
#include <qstring.h>

class DotCallGraph
{
public:
	DotCallGraph (QFile& file, bool currentSelectionOnly,
			bool imageMap, QVector<CProfileInfo>& mProfile,
			const QString& tempDir);
	~DotCallGraph() {};

private:
	DotCallGraph();
};

#endif
