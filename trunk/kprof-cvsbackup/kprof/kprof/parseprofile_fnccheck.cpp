/***************************************************************************
                          parseProfile_fnccheck.cpp  -  description
                             -------------------
    begin                : Tue Jul 9 2002
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

#include "parseprofile_fnccheck.h"

#include "cprofileinfo.h"
#include "kprofwidget.h"

#include <qtextstream.h>
#include <qvector.h>
#include <qregexp.h>
#include <qmessagebox.h>
#include <klocale.h>

CParseProfile_fnccheck::CParseProfile_fnccheck (QTextStream& t, QVector<CProfileInfo>& profile)
{
	/*
	 * parse a profile results generated with FUNCTION CHECK
	 *
	 */

	// while parsing, we temporarily store all entries of a call graph block
	// in an array
	QVector<SCallGraphEntry> callGraphBlock;
	callGraphBlock.setAutoDelete (true);
	callGraphBlock.resize (32);

	int state = ANALYZING;
	long line = 0;
	bool processingCallers=false;	// used during call-graph processing
	int curFunctionIndex = -1;		// current function index while processing call-graph
	QString s;

	t.setEncoding (QTextStream::Latin1);
	while (!t.eof ())
	{
		line++;
		s = t.readLine ();
		s = s.simplifyWhiteSpace ();
		if (s.length() == 0)
			continue;
		if (s[0] == '<')			// marks the end of the current section
		{
			state = ANALYZING;
			continue;
		}

		// check whether the file is valid or not
		if (line==1 && s[0]!='>')
		{
			mValid = false;
		}
		else
		{
			mValid = true;

		// look for markers to change the state
		QStringList fields;
		if (s.startsWith (">cycles"))
			state = PROCESS_CYCLES;
		else if (s.startsWith (">profile"))
			state = PROCESS_FLAT_PROFILE;
		else if (s.startsWith (">minmax"))
			state = PROCESS_MIN_MAX_TIME;
		else if (s.startsWith (">callgraph"))
			state = PROCESS_CALL_GRAPH;
		else if (state != ANALYZING)
		{
			uint pos=0,i;
			for (i=0; i<s.length(); i++)
			{
				if (s[i]==' ')
				{
					if (i==pos)
						fields += "";
					else
						fields += s.mid (pos, i-pos);
					pos = i+1;
				}
				else if (pos==i && s[i]=='"')
				{
					while (s[++i] != '"')
						;
					pos++;
					fields += s.mid (pos, i-pos);
					pos = ++i + 1;
				}
			}
			if (i > pos)
				fields += s.mid (pos,i-pos);
			if (fields.isEmpty ())
				continue;
			for (uint i=0; i < fields.count(); i++)
				fields[i] = fields[i].stripWhiteSpace();
		}

		switch (state)
		{
			/*
			 * analyze cycles detected during execution
			 *
			 */
			case PROCESS_CYCLES:
				// @@@ TODO
				break;

			/*
			 * analyze flat profile entry
			 *
			 */
     		case PROCESS_FLAT_PROFILE:
			{
				if (fields.count() != 7)
					break;
				if (!fields[0].length())
					break;
				if (!fields[0][0].isDigit ())
					break;

				CProfileInfo *p = new CProfileInfo;
				p->ind				= profile.count ();
				p->cumPercent		= fields[3].toFloat ();
				p->cumSeconds		= fields[2].toFloat ();
				p->selfSeconds		= fields[0].toFloat ();
				p->calls			= fields[4].toLong ();
				p->totalMsPerCall	= fields[5].toFloat () * 1000.0;
				p->name				= fields[6];

				// p->simplifiedName will be updated in postProcessProfile()
				p->recursive		= false;
				p->object			= KProfWidget::getClassName (p->name);
				p->multipleSignatures = false;				// will be updated in postProcessProfile()

				int argsoff = p->name.find ('(');
				if (argsoff != -1)
				{
					p->method = p->name.mid (p->object.length(), argsoff - p->object.length());
					p->arguments  = p->name.right (p->name.length() - argsoff);
				}
				else
					p->method = p->name.right (p->name.length() - p->object.length());

				if (p->method.startsWith ("::"))
					p->method.remove (0,2);

				int j = profile.size ();
				profile.resize (j + 1);
				profile.insert (j, p);
				break;
			}

			/*
			 * analyze MIN/MAX time per function
			 *
			 */
			case PROCESS_MIN_MAX_TIME:
			{
				// we assume that the flat profile and the min/max time profile
				// are displayed using the same order
				if (fields.count() != 4)
					break;
				if (!fields[0].length())
					break;
				if (!fields[0][0].isDigit ())
					break;

				curFunctionIndex = fields[0].toInt ();
				if (curFunctionIndex < 0 || (uint)curFunctionIndex >= profile.count())
				{
					fprintf (stderr, "kprof: missing flat profile entry for [%d] (line %ld)\n", curFunctionIndex, line);
					break;
				}

				CProfileInfo *p = profile[curFunctionIndex];
				p->custom.fnccheck.minMsPerCall = fields[1].toFloat() * 1000.0;
				p->custom.fnccheck.maxMsPerCall = fields[2].toFloat() * 1000.0;
				break;
			}

    		/*
			 * analyze call graph entry
			 *
			 */
    		case PROCESS_CALL_GRAPH:
			{
				if (s[0]=='"')
				{
					// found another call-graph entry
					processingCallers = fields.count() == 3;
					curFunctionIndex = fields[1].toInt ();
					break;
				}
				if (curFunctionIndex < 0 || (uint)curFunctionIndex >= profile.count())
					break;

				if (fields[0]=="*")
				{
					// "this function does not call anyone we know"
					curFunctionIndex = -1;
					break;
				}

				// process current call-graph entry
				CProfileInfo *p = profile[curFunctionIndex];
				for (uint i=0; i < fields.count(); i++)
				{
					int referred = fields[i].toInt ();
					if (referred < 0 || (uint)referred >= profile.count())
					{
						fprintf (stderr, "kprof: missing flat profile entry for [%d] (line %ld)\n", referred, line);
						break;
					}

					if (referred == curFunctionIndex)
						p->recursive = true;
					else
					{
						CProfileInfo *pi = profile[referred];
						if (processingCallers)
						{
							// "called by"
							if (p->callers.count()==0 || p->callers.find(pi)==-1)
							{
								p->callers.resize (p->callers.count () + 1);
								p->callers [p->callers.count () - 1] = pi;
							}
						}
						else
						{
							// "calls"
							if (p->called.count()==0 || p->called.find(pi)==-1)
							{
								p->called.resize (p->called.count () + 1);
								p->called [p->called.count () - 1] = pi;
							}
						}
					}
				}
				curFunctionIndex = -1;
    		}
   		}
	}
}
}


bool CParseProfile_fnccheck::valid() const
{
	return mValid;
}
