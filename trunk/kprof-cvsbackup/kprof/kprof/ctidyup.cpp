/***************************************************************************
                          ctidyup.cpp  -  description
                             -------------------
    begin                : Wed Oct 2 2002
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

#include "ctidyup.h"
#include <iostream>
#include <kio/job.h>
#include <kdirlister.h>
#include <qvaluelist.h>

using namespace std;

CTidyUp::CTidyUp(QString& dir)
{
	url = dir;
}

CTidyUp::~CTidyUp()
{
}


//Empty the temp directory of all the junk .html
//files that have been dumped there. There should
//be no sub-directories and this is not check for.
void CTidyUp::removeDir()
{
	listDir = new QDir(url);
	QStringList fileList = listDir->entryList();
	QStringList::iterator iter = fileList.begin();
	while(iter != fileList.end())
	{
		QString fileName = *iter;
		if ((fileName != ".") && (fileName != ".."))
		{
			QFile file(url+"/" + fileName);
			file.remove();
		}
		iter++;
	}
	listDir->rmdir(url);

}
