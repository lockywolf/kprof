/***************************************************************************
                          kproffile.h  -  description
                             -------------------
    begin                : Thu Sep 5 2002
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

#ifndef KPROFFILE_H
#define KPROFFILE_H

#include <qfile.h>

/**
  *@author Colin Desmond
  */

class KProfFile : public QFile  {
public: 
	KProfFile();
	~KProfFile();
};

#endif
