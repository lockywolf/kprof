/***************************************************************************
                          vcgCallGraph.cpp  -  description
                             -------------------
    begin                : Mon Jul 8 2002
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

#ifndef _VCGCALLGRAPH_H_
#include "vcgCallGraph.h"
#endif

#include <qfile.h>
#include <qtextstream.h>

#ifndef __CPROFILEINFO_H__
#include "cprofileinfo.h"
#endif


VCGCallGraph ::VCGCallGraph (QFile& file, bool currentSelectionOnly, QVector<CProfileInfo>& profile)
{
	// generate a call-graph to a .vcg file in a format compatible with
	// VCG, a free graph generator available from http://www.cs.uni-sb.de/RW/users/sander/html/gsvcg1.html
	QByteArray graph;
	QTextOStream stream (graph);

	stream << "graph: {\n";
	stream << "spreadlevel: 1\n";
	stream << "treefactor: 0.5\n";
	stream << "splines: yes\n";
	stream << "node_alignment: bottom\n";
	stream << "orientation: left_to_right\n";

	// first create all the nodes
	for (uint i = 0; i < profile.count (); i++)
	{
		if (currentSelectionOnly && profile[i]->output==false)
			continue;

		QString className = profile[i]->object;
		if (className.length ())
			className += "\\n";

		stream << "node: {title:\"" << i << "\" label:\"" << className << profile[i]->method;

		if (profile[i]->multipleSignatures)
			stream << "\\n" << profile[i]->arguments;
		stream << "\"";

		if (!(profile[i]->callers.count()==0 || profile[i]->called.count()==0))
			stream << " shape: ellipse";

		stream << "}\n";
	}

	// then output the nodes relationships (edges)
	for (uint i = 0; i < profile.count (); i++)
	{
		if (currentSelectionOnly && profile[i]->output==false)
			continue;

		if (profile[i]->recursive)
			stream << "edge:{sourcename:\"" << i << "\" targetname:\"" << i << "\" thickness:3}\n";

		if (profile[i]->called.count ())
		{
			for (uint j = 0; j < profile[i]->called.count (); j++)
				stream << "edge:{sourcename:\"" << i << "\" targetname:\"" << profile[i]->called[j]->ind << "\"}\n";
		}
	}
	stream << "}\n";

	file.writeBlock (graph);
}
