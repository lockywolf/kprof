/***************************************************************************
                          clientsidemap.cpp  -  description
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

#ifndef _CLIENTSIDEMAP_H_
#include "clientsidemap.h"
#endif

#include <qstring.h>
#include <qfile.h>
#include <qtextstream.h>

#include <iostream.h>

//This class maps a Server Side Image Map onto a
//Client Side Image Map
ClientSideMap::ClientSideMap(QTextStream& serverSideMap, QFile& file)
{
	QByteArray map;
	QTextOStream stream (map);

	stream << "<HTML><BODY>" << endl;
	stream << "<IMG SRC=\"" << "/tmp/graphViz.jpg" <<"\" USEMAP=\"#kprof\">" << endl;

	stream << "<MAP NAME=\"kprof\">" << endl;

	QString line;
	serverSideMap.setEncoding(QTextStream::Latin1);

	QString shape;
	QString name;
	QString coordOne;
	QString coordTwo;
	QString N;
	QString E;
	QString S;
	QString W;

	line = serverSideMap.readLine();   //first line is a comment
	line = serverSideMap.readLine();   //second line is redundant
	line = serverSideMap.readLine();   //third line is a comment
	
	while (!serverSideMap.eof())
	{
		line = serverSideMap.readLine();
		if (line.length() && line[0] != 35)
		{
			shape = line.section(' ', 0,0);        //First element is the shape of the object
			name = line. section(' ',1,1);        //Second is the name of the file to be referenced
			coordOne = line.section(' ',2,2);  //Third is the first coordinate
			coordTwo = line.section(' ',3,3); //Fourth is the last coordinate
			N = coordOne.section(',',0,0);
			E = coordOne.section(',',1,1);
			S = coordTwo.section(',',0,0);
			W = coordTwo.section(',',1,1);
			stream << "<AREA SHAPE=\"RECT\" COORDS = \"" << N << "," << W << "," << S << "," << E << "\" HREF = \"" << name << "\" />" << endl;
		}
	}

	stream << "</MAP>" << endl;
	stream << "</BODY></HTML >" << endl;

	file.writeBlock (map);

}


