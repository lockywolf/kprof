/***************************************************************************
                          cprofileinfo.cpp  -  description
                             -------------------
    begin                : Wed Jul 17 2002
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
 **************************************************************************/

#include "cprofileinfo.h"
#include <qtextstream.h>
#include <qregexp.h>

CProfileInfo::CProfileInfo ()
		:	previous (NULL),
			cumPercent (0.0),
			cumSeconds (0.0),
			selfSeconds (0.0),
			totalMsPerCall (0.0),
			calls (0),
			ind (0),
			recursive (false),
			multipleSignatures (false),
			deleted (false)
{
	memset (&custom, 0, sizeof (custom));
}

CProfileInfo::~CProfileInfo()
{
}

void CProfileInfo::dumpHtml()
{
	QFile dumpFile;
	QString fileName = "/tmp/" + htmlName + "::" + method + ".html" ;
	dumpFile.setName(fileName);

	if (dumpFile.exists())
	{
		dumpFile.remove();
	}

	dumpFile.open(IO_ReadWrite);

	QByteArray dumpText;
	QTextOStream stream (dumpText);

	stream << "<HTML><BODY>" << endl;
	stream << "<H1>" << htmlName << "::" << method << "</H1>" << endl;
	stream << "<P>" <<endl;
	stream << "Arguments\t" << arguments << endl;
	stream << "</P>" << endl;
	stream << "Recursive\t";
	if (recursive == true)
	{
		stream << "True" << endl;
	}
	else
	{
		stream << "False" << endl;
	}
	stream << "<P>Called\t" << calls << " times.</P>" << endl;
	stream << "<P>Cumulative seconds\t" << cumSeconds <<  "</P>" <<endl;
	stream << "<P>Average time per call (ms)\t" << totalMsPerCall << "</P>" << endl;
	stream << "<P>Cumulative percentage\t" << cumPercent << "</P>" << endl;
	stream << "<H2>Callers</H2>" << endl;
	if (callers.count ())
	{
		for (uint i = 0; i < callers.count (); i++)
		{
			CProfileInfo *p = callers[i];
			stream << "<LI><A HREF=\"/tmp/"<< p->htmlName << "::" << p->method << ".html\"> "
						<< p->htmlName << "::" << p->method <<"</A></LI>" << endl;
  		}
   	}
	stream << "<H2>Called</H2>" << endl;
	if (callers.count ())
	{
		for (uint i = 0; i < called.count (); i++)
		{
			CProfileInfo *p = called[i];
//			QString objectName = p->object;
//			objectName.replace(QRegExp("<"), "[");
//			objectName.replace(QRegExp(">"),"]");
			stream << "<LI><A HREF=\"/tmp/"<< p->htmlName << "::" << p->method << ".html\"> "
						<< p->htmlName << "::" << p->method <<"</A></LI>" << endl;
   		}
   	}
	
	
	stream << "</BODY></HTML >" << endl;

	dumpFile.writeBlock (dumpText);
	dumpFile.close();


}