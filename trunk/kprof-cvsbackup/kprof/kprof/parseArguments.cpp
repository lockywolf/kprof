/***************************************************************************
                          parseArguments.cpp  -  description
                             -------------------
    begin                : Mon Sep 9 2002
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
#include <kcmdlineargs.h>
#include <qstring.h>

#include "kprofwidget.h"

//Having set up the command line options above, parse them and
//use the data
bool parseArguments(KCmdLineArgs* args, QString& fileName, KProfWidget::ProfilerEnumeration& prof)
{
	bool success = false;

	if (args->isSet("f"))
	{
		fileName = args->getOption("f");
		success = true;
	}
	if(args->isSet("p"))
	{
		QString profiler = args->getOption("p");

		if (profiler == "gprof")
		{
			prof = KProfWidget::FORMAT_GPROF;
			success = true;
		}
		else if (profiler == "fnccheck")
		{
			prof = KProfWidget::FORMAT_FNCCHECK;
			success = true;
		}
		else if (profiler == "pose")
		{
			prof = KProfWidget::FORMAT_POSE;
			success = true;
		}
		else
		{
			KCmdLineArgs::usage();
			success = false;
		}
	}
		return success;
}
