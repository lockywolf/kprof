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

//Dump information relevant to the class into a HTML file
//for viewing in one of the tabbed panes.
void CProfileInfo::dumpHtml()
{
	//Open the temporary file used to store the HTML.
	QFile dumpFile;
	QString fileName = "/tmp/" + htmlName + "::" + method + ".html" ;
	dumpFile.setName(fileName);

	//If there is already a file of the same name, delete it.
	if (dumpFile.exists())
	{
		dumpFile.remove();
	}

	dumpFile.open(IO_ReadWrite);

	QByteArray dumpText;
	QTextOStream stream (dumpText);
	
    //Start the HTML code
	stream << "<HTML><BODY>" << endl;
	
	//Put a heading on the page, check for C style functions with no method.
	if (method != "")
	{
		stream << "<H1>" << htmlName << "::" << method << "</H1>" << endl;
	}
	else
	{
		stream << "<H1>" << htmlName << endl;
	}
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

	if(callers.count())
	{
		stream << "<table>" << endl;
		stream << "<TD>Name</TD>";
		stream << "<TD>Cumulative Percent</TD>";
		stream << "<TD>Cumulative Seconds</TD>";
		stream << "<TD>Self Seconds</TD>";
		stream << endl;
	}

	//Iterate through all the callers of the method
	if (callers.count ())
	{
		for (uint i = 0; i < callers.count (); i++)
		{
			stream << "<TR>";
			CProfileInfo *p = callers[i];
			if(p->method == "")
			{
				stream << "<TD><A HREF=\"/tmp/"<< p->htmlName << "::" << p->method << ".html\"> "
							<< p->htmlName <<"</A></TD>" << endl;
			}
			else
			{
				stream << "<TD><A HREF=\"/tmp/"<< p->htmlName << "::" << p->method << ".html\"> "
							<< p->htmlName << "::" << p->method <<"</A></TD>" << endl;
			}
			stream << "<TD>" << p->cumPercent << "</TD>" ;
			stream << "<TD>" << p->cumSeconds << "</TD>";
			stream << "<TD>" << p->selfSeconds;
			stream << "</TR>" << endl;
  		}
   	}
	stream << "</table>" << endl;

	stream << "<table>" << endl;
	
	if (called.count())
	{
		stream << "<TD>Name</TD>";
		stream << "<TD>Cumulative Percent</TD>";
		stream << "<TD>Cumulative Seconds</TD>";
		stream << "<TD>Self Seconds</TD>";
		stream << endl;
	}

	//Iterator through all  the method called by this one.
	stream << "<H2>Called</H2>" << endl;
	if (called.count ())
	{
		for (uint i = 0; i < called.count (); i++)
		{
			stream << "<TR>";
			CProfileInfo *p = called[i];
			if (p->method == "")
			{
				stream << "<TD><A HREF=\"/tmp/"<< p->htmlName << "::" << p->method << ".html\"> "
							<< p->htmlName << "</A></TD>" << endl;
			}
			else
			{
				stream << "<TD><A HREF=\"/tmp/"<< p->htmlName << "::" << p->method << ".html\"> "
							<< p->htmlName << "::" << p->method <<"</A></TD>" << endl;
			}
			stream << "<TD>" << p->cumPercent << "</TD>" ;
			stream << "<TD>" << p->cumSeconds << "</TD>";
			stream << "<TD>" << p->selfSeconds;
			stream << "</TR>" << endl;
		}
	}
	stream << "</table>" << endl;
	
	stream << "</BODY></HTML >" << endl;

	dumpFile.writeBlock (dumpText);
	dumpFile.close();
} //void CProfileInfo::dumpHtml()
