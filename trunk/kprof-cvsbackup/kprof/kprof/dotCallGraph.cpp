/***************************************************************************
                          dotCallGraph.cpp  -  description
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
#ifndef _DOTCALLGRAPH_H_
#include  "dotCallGraph.h"
#endif

#include <qfile.h>
#include <qtextstream.h>
#include <qvector.h>

#ifndef __CPROFILEINFO_H__
#include "cprofileinfo.h"
#endif

DotCallGraph::DotCallGraph (QFile& file, bool currentSelectionOnly,
						bool imageMap, QVector<CProfileInfo>& mProfile)
{
	// generate a call-graph to a .dot file in a format compatible with
	// GraphViz, a free graph generator from ATT (http://www.research.att.com/sw/tools/graphviz/)
	QByteArray graph;
	QTextOStream stream (graph);

	stream << "Digraph \"kprof-call-graph\" {\nratio=fill\n";

	// first create all the nodes
	for (uint i = 0; i < mProfile.count (); i++)
	{
		if (currentSelectionOnly && mProfile[i]->output==false)
			continue;

		QString className = mProfile[i]->object;
		if (className.length () && (!imageMap))
			className += "\\n";

		stream << i << " [label=\"" << className << "::" <<  mProfile[i]->method;
		if (mProfile[i]->multipleSignatures)
			stream << "\\n" << mProfile[i]->arguments;
		stream << "\"";
		if (mProfile[i]->callers.count()==0 || mProfile[i]->called.count()==0)
			stream << ", shape=box";
		stream << "]";
		if (imageMap)
		{
			stream << "[URL=\"/tmp/" << className << "::" << mProfile[i]->method << ".html\"]";
		}
		stream << ";" << endl;
	
	}

	// then output the nodes relationships
	for (uint i = 0; i < mProfile.count (); i++)
	{
		if (currentSelectionOnly && mProfile[i]->output==false)
			continue;

		if (mProfile[i]->recursive)
			stream << i << " -> " << i << " [style=dotted];\n";

		if (mProfile[i]->called.count ())
		{
			stream << i << " -> {";
			for (uint j = 0; j < mProfile[i]->called.count (); j++)
			{
				if (j)
					stream << "; ";
				stream << mProfile[i]->called[j]->ind;
			}
			stream << "};\n";
		}
	}
	stream << "}\n";

	file.writeBlock (graph);
}
