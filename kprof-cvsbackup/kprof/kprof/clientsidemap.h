/***************************************************************************
                          clientsidemap.h  -  description
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
#ifndef _CLIENTSIDEMAP_H_
#define _CLIENTSIDEMAP_H_

class QFile;
class QTextStream;
class QString;

class ClientSideMap
{
public:
	ClientSideMap(QTextStream& serverSideMap, QFile& file);
	~ClientSideMap(){};

private:
	ClientSideMap();
};

#endif
