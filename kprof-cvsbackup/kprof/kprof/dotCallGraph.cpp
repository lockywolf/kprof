/***************************************************************************
                          dotCallGraph.cpp  -  description
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
#include  "dotCallGraph.h"
#endif

#include <qfile.h>
#include <qtextstream.h>
#include <qvector.h>
#include <qcolor.h>
#include <iostream>
using namespace std;
#ifndef __CPROFILEINFO_H__
#include "cprofileinfo.h"
#endif

DotCallGraph::DotCallGraph (QFile& file, bool currentSelectionOnly,
			bool imageMap, QVector<CProfileInfo>& mProfile,
			const QString& tempDir, const QColor& fillColour)
{
	// generate a call-graph to a .dot file in a format compatible with
	// GraphViz, a free graph generator from ATT (http://www.research.att.com/sw/tools/graphviz/)
	QByteArray graph;
	QTextOStream stream (graph);

	stream << "Digraph \"kprof-call-graph\" {\nratio=fill\n";
	stream << "node [style=filled];" << endl;

	CProfileInfo* record = 0;
	
	// first create all the nodes
	for (uint i = 0; i < mProfile.count (); i++)
	{ 
		
		//if (currentSelectionOnly && mProfile[i]->output==false)
		//	continue;

		record = mProfile[i];
		
		QString className = record->object;
		if (className.length () && (!imageMap))
			className += "\\n";

		if(record->method != "")
		{
			stream << i << " [label=\"" << className << "::" <<  record->method;
		}
		else
		{
        	stream << i << "[label=\"" << className;
		}
		
		if (record->multipleSignatures)
			stream << "\\n" << record->arguments;
		stream << "\"]";
		
		if (record->cumPercent != 0)
		{	
			int h, s, v;                                 
			fillColour.hsv(&h,&s,&v);
			float hh = 100.0 - (((record->cumPercent/100.0))*100.0);
			
			stream 	<< "[color=\"" << hh/255.0  <<" " << s/255.0  <<" "
				<< v << "\"]";
		}
		else
		{
			stream << "[color=\"" << 100.0/255.0 << " 1 1\"]";
		}
		
		if (record->callers.count()==0 || record->called.count()==0)
			stream << "[shape=box]";
		
		if (imageMap)
		{
			stream << "[URL=\"" << tempDir << record->htmlName << "::" << record->method << ".html\"]";
		}
		stream << ";" << endl;
	
	}

	// then output the nodes relationships
	for (uint i = 0; i < mProfile.count (); i++)
	{
		//if (currentSelectionOnly && mProfile[i]->output==false)
		//	continue;

		record = mProfile[i];
		
		if (record->recursive)
			stream << i << " -> " << i << " [style=dotted];\n";

		if (record->called.count ())
		{
			stream << i << " -> {";
			for (uint j = 0; j < record->called.count (); j++)
			{
				if (j)
					stream << "; ";
				stream << record->called[j]->ind;
			}
			stream << "};\n";
		}
	}
	stream << "}\n";

	file.writeBlock (graph);
}
