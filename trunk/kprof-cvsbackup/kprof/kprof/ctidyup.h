/***************************************************************************
                          ctidyup.h  -  description
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

#ifndef CTIDYUP_H
#define CTIDYUP_H

#include <qdir.h>

/**
  *@author Colin Desmond
  */

class CTidyUp 
{
public: 
	CTidyUp(QString& dir);
	virtual ~CTidyUp();
	void removeDir();

private:
	QDir* listDir;
	QString url;
};

#endif
