/***************************************************************************
                          Log.cpp  -  description
                             -------------------
    begin                : Mit Jul 17 2002
    copyright            : (C) 2002 by Andrey Behrens
    email                : Andrey.Behrens@gmx.de
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "Log.h"
#include <qmap.h>
#include <kdebug.h>
#include <kapp.h>
#include <kconfig.h>
#include <klocale.h>
#include <cstdarg>
#include <cstdio>
#include <sys/timeb.h>
#include <cassert>



//Modul-lokale Variablen
static	Log::LogType							LEVEL		=Log::RUN;	//Ausgabelevel (wird in Init gesetzt)
static 	QMap<QString, Log::LogType>	MODULLIST;					//DebugLevel für einzelne Module
static  int	                      INDENTLEVEL=0;			//
static	int												NO=80000;						//KDE program number (for kdebug)


#ifdef NDEBUG
//disable asserts and some debug messages
#warning "NDEBUG is defined (so debug + assert are disabled)"
#else
//Enable asserts and some
#warning "NDEBUG is not defined (so debug + assert are enabled)"
#endif


//Rückgabe der Versionsnummer des Modules Log.h
const QString Log::getVersion(void){
  return ("$Id$");
};


const Log::LogType GetModul(const QString& modul){ //Rückgabe DbgLevel für Modul oder Level für alle Module
	assert(modul.length()!=0);
  if(MODULLIST.find(modul)==MODULLIST.end())
  	return LEVEL;
  else
  	return MODULLIST[modul];
}



void Log::ReduceIndentLevel(void)
{
	INDENTLEVEL--;
}




char const * const DateAndTime(void){
	static char a[30];
  timeb	 now;
  ftime(&now);
  tm *	z=localtime(&(now.time));
	sprintf(a,"%2.2d%2.2d %2.2d:%2.2d:%2.2d ",  z->tm_mday, z->tm_mon+1,
  																				  	z->tm_hour, z->tm_min, z->tm_sec);
  return a;
}




 

//Rückgabe des aktuellen Loglevels
const Log::LogType Log::GetCurrentLevel(void)
{
  return LEVEL;
}









void Log::AddModul(const QString& modul, const QString &Level){
	if (Level == "OFF") {
		kdDebug(NO) << "Log::AddModul(" << modul << " = OFF)" << endl;
		MODULLIST[modul]=OFF;
	} else if (Level == "ERR") {
		kdDebug(NO) << "Log::AddModul(" << modul << " = ERR)" << endl;
		MODULLIST[modul]=ERR;
	} else if (Level == "RUN") {
		kdDebug(NO) << "Log::AddModul(" << modul << " = RUN)" << endl;
		MODULLIST[modul]=RUN;
	} else if (Level == "DBG") {
		kdDebug(NO) << "Log::AddModul(" << modul << " = DBG)" << endl;
		MODULLIST[modul]=DBG;
	} else if (Level == "SIG") {
		kdDebug(NO) << "Log::AddModul(" << modul << " = SIG)" << endl;
		MODULLIST[modul]=SIG;
	} else if (Level == "TRC") {
		kdDebug(NO) << "Log::AddModul(" << modul << " = TRC)" << endl;
		MODULLIST[modul]=TRC;
	} else if (Level.length()==0){
		kdDebug(NO) << "Log::AddModul(" << modul << "): Use Default-Level" <<endl;
  } else {
		kdDebug(NO) << "Error in Log::AddModul(" << modul << "): Level was \""<<Level<<"\""<<endl;
  }
}



void Log::RemModul(const QString& modul){
	assert(modul.length()!=0);
  if(MODULLIST.find(modul)!=MODULLIST.end())
  	MODULLIST.erase(modul);
}






//Jetzt kommen die "oeffentlichen" Funktionen
///////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////




///////////////////////////////////////////////////////////////
//Initialisierung des Log-Moduls
const int Log::Init(const QString&	level, const int Number)
{
	if (level == "OFF") {
    return Init(OFF,Number);
	} else if (level == "ERR") {
    return Init(ERR,Number);
	} else if (level == "RUN") {
    return Init(RUN,Number);
	} else if (level == "DBG") {
    return Init(DBG,Number);
	} else if (level == "SIG") {
    return Init(SIG,Number);
	} else if (level == "TRC") {
    return Init(TRC,Number);
	}

	//wrong level string
  kdDebug(NO) << "Unknown LogLevel " << level << "! Use Default Level Log::RUN." <<endl;
  return Init(RUN,Number);
}




const int Log::Init(const LogType	level, const int Number)
{
	assert(Number>0);
	LEVEL = level;
	NO=Number;
  switch(level){
  case OFF: kdDebug(NO) << "Default LogLevel is OFF" <<endl; break;
  case ERR: kdDebug(NO) << "Default LogLevel is ERR" <<endl; break;
  case RUN: kdDebug(NO) << "Default LogLevel is RUN" <<endl; break;
  case SIG: kdDebug(NO) << "Default LogLevel is SIG" <<endl; break;
  case DBG: kdDebug(NO) << "Default LogLevel is DBG" <<endl; break;
  case TRC: kdDebug(NO) << "Default LogLevel is TRC" <<endl; break;
  }
	return 0;
}






const int Log::DbgLog(const char * const file,
										  const int line,      						//Zeilennummer
								      const LogType level,						//Meldungstyp
								      const char * const fmt, 				//Formatstring
        				...
							)
{
  assert(file);
  assert(strlen(file)>0);
  assert(fmt);
  assert(strlen(fmt)>0);
  assert(line > 0);
  //no assert for func, could be 0

 	if(GetModul(file) < level) return 0;
 	if(level == OFF)       			return 0;

  QString result="";

  //Add Time and date
  //if(LEVEL >= DBG){
  //  result+=DateAndTime();
  //}

  //add filename and line number
  char pos[30];
  snprintf(pos, sizeof(pos)-1, "%15s(%4d) ", file, line);
  pos[sizeof(pos)]=0;
  result = result + " " + pos;

  //add kind of message
	switch(level){
		case ERR: result+="ERR "; break;
		case SIG: result+="SIG "; break;
		case RUN: result+="RUN "; break;
		case DBG: result+="DBG "; break;
		case TRC: result+="TRC "; break;
		case OFF: result+="OFF "; break; //wird nicht benutzt, um compiler warnung zu vermeiden
	};

 	//Indent
  if(LEVEL >= OFF){
    for (int i=0; i<INDENTLEVEL;i++){
      result+="    ";
    }
  }

  //add message
	va_list ap;				//Liste der ... Parameter
  int n, size = 100;//Puffergroesse
  char *p=0;
  if  ((p = (char*)malloc (size)) == 0){
    	kdDebug(NO) << i18n("Out of Memory") << endl;
      return -1;
  };

  while (1) { //try to write in buffer
      va_start(ap, fmt);
      n = vsnprintf (p, size, fmt, ap);
      va_end(ap);

      //if ok
      if (n > -1 && n < size){
        result+=p;
        free(p);
        break;
      }

      //else
      size *= 2; //twice the size
      char *pp=p;
      if ((p = (char*)realloc (p, size)) == 0){
        free(pp);
    		kdDebug(NO) << i18n("Out of Memory") << endl;
      	return -1;
      }
  }

  //print the stuff
	kdDebug(NO) << result << endl;
	return 0;
}


const int Log::Begin(	const char * const file,
										  const int line,
											const char * const func
                    )
{
	assert(file);

	QString a="";
	if(func != 0) a = func;
	a+=" {";
	int res=DbgLog(file, line, TRC, a);
  INDENTLEVEL++;
 	return res;
}

