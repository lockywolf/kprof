/***************************************************************************
                          Log.h  -  description
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

/**
\file Log.h
\brief Namespace for easy logging
*/


#ifndef		LOG_HEADER_FILE
#define		LOG_HEADER_FILE

#include <qstring.h>
#include <sstream>
using namespace std;


/* Version 2.4 and later of GCC define a magical variable `__PRETTY_FUNCTION__'
   which contains the name of the function currently being defined.
   This is broken in G++ before version 2.6.
   C9x has a similar variable called __func__, but prefer the GCC one since
   it demangles C++ function names.  */
# if defined __cplusplus ? __GNUC_PREREQ (2, 6) : __GNUC_PREREQ (2, 4)
#   define __LOGFUNC__						__PRETTY_FUNCTION__
# else                      	
#  if defined __STDC_VERSION__ && __STDC_VERSION__ >= 199901L
#   define __LOGFUNC__							__func__
#  else
#   define __LOGFUNC__							((__const char *) 0)
#  endif
# endif

#define   HERE                  Log::DbgLog(LOGPOS, Log::ERR, ">>>Here<<<");

#define		LOGPOS								__FILE__, __LINE__

#define   SNDSIG(sig,tar)       Log::SndSignal(LOGPOS, sig, tar)
#define   GETSIG(sig)           Log::GetSignal(LOGPOS, sig)

#ifndef NDEBUG
	//the following debug messages are only in debug mode
	#define   BEGIN           			Log::Begin(LOGPOS, __LOGFUNC__)
	#define   END                   Log::End(LOGPOS)
	#define   ENDRES			         	Log::End(LOGPOS,res); 

	#define   TRC(txt)                Log::DbgLog(LOGPOS, Log::TRC, txt)
	#define   TRC1(txt,par1)          Log::DbgLog(LOGPOS, Log::TRC, txt, par1)
	#define   TRC2(txt,par1,par2)     Log::DbgLog(LOGPOS, Log::TRC, txt, par1,par2)
	#define   TRC3(txt,par1,par2,par3)Log::DbgLog(LOGPOS, Log::TRC, txt, par1,par2,par3)

	#define   DBG(txt)                Log::DbgLog(LOGPOS, Log::DBG, txt)
	#define   DBG1(txt,par1)          Log::DbgLog(LOGPOS, Log::DBG, txt, par1)
	#define   DBG2(txt,par1,par2)     Log::DbgLog(LOGPOS, Log::DBG, txt, par1,par2)
	#define   DBG3(txt,par1,par2,par3)Log::DbgLog(LOGPOS, Log::DBG, txt, par1,par2,par3)
#else
	//disable debug messages with -DNDEBUG
	#define   BEGIN           			//Log::Begin(LOGPOS, __LOGFUNC__)
	#define   END                   //Log::End(LOGPOS)
	#define   ENDRES		          	//Log::End(LOGPOS,res)

	#define   TRC(txt)                //Log::DbgLog(LOGPOS, Log::TRC, txt)
	#define   TRC1(txt,par1)          //Log::DbgLog(LOGPOS, Log::TRC, txt, par1)
	#define   TRC2(txt,par1,par2)     //Log::DbgLog(LOGPOS, Log::TRC, txt, par1,par2)
	#define   TRC3(txt,par1,par2,par3)//Log::DbgLog(LOGPOS, Log::TRC, txt, par1,par2,par3)

	#define   DBG(txt)                //Log::DbgLog(LOGPOS, Log::DBG, txt)
	#define   DBG1(txt,par1)          //Log::DbgLog(LOGPOS, Log::DBG, txt, par1)
	#define   DBG2(txt,par1,par2)     //Log::DbgLog(LOGPOS, Log::DBG, txt, par1,par2)
	#define   DBG3(txt,par1,par2,par3)//Log::DbgLog(LOGPOS, Log::DBG, txt, par1,par2,par3)
#endif



#define   RUN(txt)              Log::DbgLog(LOGPOS, Log::RUN, txt)
#define   RUN1(txt,par1)        Log::DbgLog(LOGPOS, Log::RUN, txt, par1)
#define   RUN2(txt,par1,par2)   Log::DbgLog(LOGPOS, Log::RUN, txt, par1,par2)
#define   RUN3(txt,par1,par2,p3)Log::DbgLog(LOGPOS, Log::RUN, txt, par1,par2,p3)

#define   ERR(txt)              Log::DbgLog(LOGPOS, Log::ERR, txt)
#define   ERR1(txt,par1)        Log::DbgLog(LOGPOS, Log::ERR, txt, par1)
#define   ERR2(txt,par1,par2)   Log::DbgLog(LOGPOS, Log::ERR, txt, par1,par2)
#define   ERR3(txt,p1,p2,p3)    Log::DbgLog(LOGPOS, Log::ERR, txt, p1,p2,p3)



/**
\def HERE
\brief It prints just an "here" text
*/


/**
\def  LOGPOS
\brief Wird durch Modulname und Zeilennummer ersetzt

Alle Log Funktionen erwarten als ersten Parameter den aktuellen
Dateinamen und die aktuelle Zeilennummer. Um Arbeit zu sparen
kann das Makro LOGPOS benutzt werden.

Ein Beispiel zur Benutzung:

<pre>
Log::Msg(__FILE__, __LINE__, Log::RUN, "Hallo Welt");
Log::Msg(LOGPOS, Log::RUN, "Hallo Welt");
</pre>
*/
/**\def  SNDSIG(sig,tar)
\brief Logging Signal sig an Target tar

Gibt einen Logging String in der Form
"SIG disp.cpp (308) TOUT (Timeout tick fuer Statemashines)" aus.
Die vom Makro aufgerufene Funktion �bersetzt dabei das Signal in
einen lesbaren Text, teilweise mit Erkl�rung des Signals.
\param sig  Signal aus zum Beispiel dio.h
\param tar  Text, welcher das Signalziel enth�lt etwa "gsm"
\see GETSIG(sig)
\see Log::SndSignal()*/


/**\def  GETSIG(sig)
\brief Loggin empfangenes Signal sig

Gibt einen Logging String in der Form
"SIG disp.cpp (308) TOUT (Timeout tick fuer Statemashines)" aus.
Die vom Makro aufgerufene Funktion �bersetzt dabei das Signal in
einen lesbaren Text, teilweise mit Erkl�rung des Signals.
\param sig  Signal aus zum Beispiel dio.h
\see SNDSIG(sig,tar)
*/






 /**@short Ein namespace zum bequemen Logging

 	Das Modul bietet einige Funktionen zur umfassenden Laufzeitprotokollierung
  von Software-Projekten.  W�hrend der Programmlaufzeit kann jederzeit die Protokoll-Datei
  als auch der Protokollierungslevel gesetzt werden.

  Der bei Init �bergebene Debug-Level ist ein globaler Level f�r alle Funktionen.
  Mit Log::AddModul() bzw. Log::RemModul() kann ein spezieller Level f�r jedes Modul festgelegt
  werden.




<h3>Ein Beispiel zur Benutzung</h3>
<pre>
int main(void){
    Log::Init(Log::SIG, "/tmp/test.txt");
    Log::Begin(LOGPOS, "main()");
    ...
    Log::Msg(LOGPOS, Log::DBG, "Message: %s", "Hallo");
    ...
    Log::SndSignal(LOGPOS, Signal, "Empf�nger");
    ...
    Log::End(LOGPOS, "main()");
    return 0;
}

</pre>

	Mit Log::Init() wird die Protokoll-Ausgabe initialisiert. Wird der Funktions-
  Aufruf vergessen, dann wird lediglich nach cout ausgegeben.

  <h3>Protokoll-Level</h3>
  	<li>OFF = Keine Ausgabe
   	<li>ERR = Fehler w�hrend Programmlauf
    <li>RUN = ERR + Laufzeitinformationen (etwa "SMS an xyz verschickt")
    <li>SIG = ERR + RUN + Signalfl�sse in der Software
    <li>DBG = ERR + RUN + SIG + Plus Debugging-Ausgaben wie Eintritt und Austritt in Funktionen.
    <li>TRC = ERR + RUN + SIG + DBG + periodische Vorgaenge

  <h3></h3>

\todo Debug-Level aus Config-File auslesen
\todo Debug-Level f�r einzelne Module aus Config-File auslesen
\todo Farbausgabe f�r Ausgaben auf der Konsole :)
*/




namespace Log {


    /**@short Return version number

    @result Version as a string*/
    const QString getVersion(void);


    /**@short Logging Level*/
		enum LogType{	  OFF=0, 		/**< Keine Meldung*/
							 			ERR=1,		/**< Fehlermeldung	(etwa "Keine Verbindung zu ...")*/
							 			RUN=2,		/**< Laufzeit-Meldungen (etwa "Nummer 0815 verkauft");*/
           					SIG=3,		/**< Signale*/
							 			DBG=4,		/**< Debug-Meldungen*/
							 			TRC=5			/**< periodische Debug-Meldungen, einschlie�lich Funktionsein- und austritt*/
					};


		/** @short Begin of a function
        @param modul filename (use __file__)
        @param line linenumber (use __line__)
        @param fmt  string in kind of printf
        @return 0 anything Ok, -1 Out of Memory, -2 cannot write
  	*/
  	const int Begin		 	(	const char * const file,
													const int line,
													const char * const func);


  	/**@short Initialisierung des Log-Moduls

    Initialisierung des Log-Moduls mit Log-Level und	Dateiname der Ausgabe-Datei.   Wird als filename ein Leerstring �bergeben
    (Default-Wert), dann wird lediglich nach stdout (cout) protokolliert.
    @param level	Default-Debuglevel f�r alle Module. Kann mit AddModul f�r
      							spezielle Module �berschrieben werden.
    @return Wert ungleich 0 wenn nicht in Datei geschrieben werden konnte
    */
		const int Init(	const Log::LogType	level, const int Number);

    /**@short Initialisierung des Log-Moduls

    @see Init
    Initialisierung des Log-Moduls mit Log-Level und	Dateiname der Ausgabe-Datei.   Wird als filename ein Leerstring �bergeben
    (Default-Wert), dann wird lediglich nach stderr (cout) protokolliert.
    @param level	Default-Debuglevel f�r alle Module. Kann mit AddModul f�r
     							spezielle Module �berschrieben werden. Level dabei in der Form ERR, OFF, RUN, ...
    @return Wert alltimes 0
    */
		const int Init(	const QString& level, const int Number);



		/**@short speziellen DebugLevel zuf�gen

    Zuf�gen eines speziellen Debuglevels f�r ein spezielles Modul. Der Level eines Modules kann
    jederzeit mit einem neuen Level �berschrieben werden (einfach diese Funktion erneut aufrufen)
    oder auch mit RemModul() gel�scht werden. Dann gilt f�r das betreffende Modul der bei
    Init �bergebene globale Debuglevel.
  	@param modul Name des Modules in exakt gleicher Schreibweise wie das betreffende
    C / C++ File!
    @param level Ausgabelevel f�r dieses Modul in der Form DBG, OFF, ERR, ...
	  */
  	void AddModul(const QString& modul, const QString& level);

  	/**@short L�schen eines speziellen Debuglevels f�r ein spezielles Modul.

  	@see AddModul()
	  */
		void RemModul(const QString& modul);


		/** @short Ausgabe einer Meldung in klassischer printf() Art,

        Ausgabe einer Meldung in klassischer printf() Art,
        also mit Formatstring und beliebig vielen weiteren Parametern.
        Die Parameter modul und zeile sind sinnvollerweise durch das Makro LOGPOS ausgef�llt.
        @param modul Name der Sourcendatei
        @param zeile Zeilennummer der Sourcendatei
        @param level Log-Level _dieser_ Message
        @param fmt  String in �blicher printf()-Art mit beliebig vielen weiteren Parametern.
        @return gleich 0 alles Ok, -1 Out of Memory, -2 kann nicht in Datei schreiben.
    */
		const int DbgLog(const char * const file,				//Name des Moduls
										 const int line,      					//Zeilennummer
										 const LogType level,						//Meldungstyp
										 const char * const fmt,		 			//Formatstring
        						 ...);


		/** @short Das gleiche f�r Objekte die nicht �ber die printf()-Funktionalit�t
  			erreicht werden k�nnen.


     		Bedingung: Das Objekt muss in der Lage sein etwas wie cout << objekt << endl;
       	machen zu k�nnen.
        Die Funktion kann keine Formate (wie printf()) auswerten.
        @param modul Name der Sourcendatei
        @param zeile Zeilennummer der Sourcendatei
        @param level Log-Level _dieser_ Message
        @param meldung  auszugebendes Objekte (etwa string).
        @return gleich 0 alles Ok, -1 Out of Memory, -2 kann nicht in Datei schreiben.
     */
   	template<class T>
    inline const int DbgLog (const char * const file,		//Name des Moduls
													   const int line,      		  	//Zeilennummer
													   const LogType level,					//Meldungstyp
													   const T& meldung							//Eigentliche Meldung
          								)
   	{
  			std::ostringstream str;
     		str << meldung;;
       	return DbgLog(file, line, level, str.str().c_str());
    }


  	/**@short Reduzierung der Einr�ckungsfunktion

    Funktion ReduceIndentLevel(void) nur zur internen Benutzung, muss aber
    f�r die folgenden Template-Funktionen hier definiert werden
    */
    void ReduceIndentLevel(void);

		/** Meldung zum Ende einer Funktion (Level ist immer DBG)
        @param modul Name der Sourcendatei
        @param zeile Zeilennummer der Sourcendatei
  			@param result ist ein Objekt als R�ckgabewert (etwa int, string, bool)
        @return gleich 0 alles Ok, -1 Out of Memory, -2 kann nicht in Datei schreiben.
  	*/
   	template<class T>
    inline const int End (const char * const file,				//Name des Moduls
													const int line,      					//Zeilennummer
													const T& result									//Return der Funktion
          								)
   	{
				ReduceIndentLevel();
  			std::ostringstream str;
     		str << "} with res=\"" << result << "\"";
       	return DbgLog(file, line, TRC, str.str().c_str());
    }


		/** @short Meldung zum Ende einer Funktion (Level ist immer DBG) ohne result

  			@see int Begin()
  	*/
    inline const int End (const char * const file,	//filename
													const int line      			//linenumber
          								)
   	{
			ReduceIndentLevel();
     	return DbgLog(file, line, TRC, "}");
    }


    inline const int End (const char * const file,				//Name des Moduls
													const int line,      					//Zeilennummer
													const bool result								//Return der Funktion
          								)
   	{
			ReduceIndentLevel();
     	return DbgLog(file, line, TRC, result?"} with res=\"true\"":"} with res=\"false\"");
    }


    /**@short Return the current Log Level

    @result Level*/
    const LogType GetCurrentLevel(void);


};

#endif		//LOG_HEADER_FILE
