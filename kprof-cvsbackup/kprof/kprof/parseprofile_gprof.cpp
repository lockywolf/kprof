/***************************************************************************
                          parseprofile_gprof.cpp  -  description
                             -------------------
    begin                : Tue Jul 9 2002
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
#include "parseprofile_gprof.h"

#include "cprofileinfo.h"
#include "kprofwidget.h"

#include <qtextstream.h>
#include <qvector.h>
#include <qregexp.h>


CParseProfile_gprof::CParseProfile_gprof (QTextStream& t, QVector<CProfileInfo>& profile)
{
	/*
	 * parse a GNU gprof output generated with -b (brief)
	 *
	 */

	// regular expressions we use while parsing
	QRegExp indexRegExp (" *\\[\\d+\\] *$");
	QRegExp floatRegExp ("^[0-9]*\\.[0-9]+$");
	QRegExp countRegExp ("^[0-9]+[/\\+]?[0-9]*$");
	QRegExp dashesRegExp ("^\\-+");
	QRegExp	recurCountRegExp (" *<cycle \\d+.*> *$");

	// while parsing, we temporarily store all entries of a call graph block
	// in an array
	QVector<SCallGraphEntry> callGraphBlock;
	callGraphBlock.setAutoDelete (true);
	callGraphBlock.resize (32);

	int state = SEARCH_FLAT_PROFILE;
	long line = 0;
	QString s;

	t.setEncoding (QTextStream::Latin1);
	while (!t.eof ())
	{
		line++;
		s = t.readLine ();
		if (s.length() && s[0] == 12)
		{
			if (state == PROCESS_CALL_GRAPH)
			{
				processCallGraphBlock (callGraphBlock, profile);
				break;
			}
			if (state == PROCESS_FLAT_PROFILE)
				state = SEARCH_CALL_GRAPH;
			continue;
   		}
		s = s.simplifyWhiteSpace ();

		// remove <cycle ...> and [n] from the end of the line
		if (state == PROCESS_FLAT_PROFILE || state == PROCESS_CALL_GRAPH)
		{
			int pos = indexRegExp.search (s, 0);
			if (pos != -1)
				s = s.left (pos);
			pos = recurCountRegExp.search (s, 0);
			if (pos != -1)
				s = s.left (pos);
		}

		// split the line in tab-delimited fields
		QStringList fields = QStringList::split (" ", s, false);
		if (fields.isEmpty ())
			continue;

		if (fields.count() > 1 && (state == PROCESS_FLAT_PROFILE || state == PROCESS_CALL_GRAPH))
		{
			// the split did also split the function name & args. restore them so that they
			// are only one field
			uint join;
			for (join = 0; join < fields.count (); join++)
			{
				QChar c = fields[join][0];
				if ((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || c == '_')
					break;
			}
			while (join < (fields.count () - 1))
			{
				fields[join] += " " + fields[join + 1];
				fields.remove (fields.at (join + 1));
			}
   		}

		switch (state)
		{
			/*
			 * look for beginning of flat profile
			 *
			 */
			case SEARCH_FLAT_PROFILE:
				if (fields[0]=="time" && fields[1]=="seconds" && fields[2]=="seconds")
					state = PROCESS_FLAT_PROFILE;
				break;

			/*
			 * analyze flat profile entry
			 *
			 */
     		case PROCESS_FLAT_PROFILE:
			{
				CProfileInfo *p = new CProfileInfo;
				p->ind				= profile.count ();
				p->selfSeconds		= fields[2].toFloat ();
				if (fields[3][0].isDigit ())
				{
					p->calls			= fields[3].toLong ();
					p->custom.gprof.selfMsPerCall	= fields[4].toFloat ();
					p->totalMsPerCall	= fields[5].toFloat ();
					p->name				= fields[6];
     			}
				else
				{
					// if the profile was generated with -z, uncalled functions
					// have empty "calls", "selfTsPerCall" and "totalTsPerCall" fields
					p->calls			= 0;
					p->custom.gprof.selfMsPerCall = 0;
					p->totalMsPerCall	= 0;
					p->name				= fields[3];
     			}
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
			 * look for call graph
			 *
			 */
   			case SEARCH_CALL_GRAPH:
				if (fields[0]=="index" && fields[1]=="%")
					state = PROCESS_CALL_GRAPH;
				break;

    		/*
			 * analyze call graph entry
			 *
			 */
    		case PROCESS_CALL_GRAPH:
			{
				// if we reach a dashes line, we finalize the previous call graph block
				// by analyzing the block and updating callers, called & recursive
				// information
				if (dashesRegExp.search (fields[0], 0) == 0)
				{
					processCallGraphBlock (callGraphBlock, profile);
					callGraphBlock.resize (0);
					break;
				}

				QString count;
				SCallGraphEntry *e = new SCallGraphEntry;
				uint field = 0;

				e->line = line;
				e->primary = false;
				e->recursive = false;

				// detect the primary line in the call graph
				if (indexRegExp.search (fields[0], 0) == 0)
				{
					e->primary = true;
					field++;
     			}

				// gather other values (we have to do some guessing to get them right)
				while (field < fields.count ())
				{
					if (countRegExp.search (fields[field], 0) == 0)
						e->recursive = fields[field].find ('+') != -1;
      				else if (floatRegExp.search (fields[field], 0) == -1)
						e->name = fields[field];
      				field++;
				}

				// if we got a call graph block without a primary function name,
				// drop it completely.
				if (e->name == NULL || e->name.length() == 0)
				{
					delete e;
					break;
				}

				if (e->primary == true && count.find ('+') != -1)
					e->recursive = true;
				
				// if this is a primary entry, get the total time and percentage
				if (e->primary == true)
				{
					CProfileInfo *tPrimary = locateProfileEntry (e->name, profile);
					if (tPrimary != NULL)
					{
						tPrimary->cumPercent		= fields[1].toFloat ();
						tPrimary->cumSeconds		= tPrimary->selfSeconds + fields[3].toFloat ();
					}
				}

				if (callGraphBlock.count () == callGraphBlock.size ())
					callGraphBlock.resize (callGraphBlock.size () + 32);
				callGraphBlock.insert (callGraphBlock.count (), e);
				break;
    		}
   		}
	}
}

bool CParseProfile_gprof::valid() const
{
	return mValid;
}
