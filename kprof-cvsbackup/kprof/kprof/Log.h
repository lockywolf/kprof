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
using namespace std;



#define 	LOGPOS	              __FILE__, __LINE__

#define   HERE                  Log::DbgLog(LOGPOS, Log::ERR, ">>>Hier<<<");

#define   SNDSIG(sig,tar)       Log::SndSignal(LOGPOS, sig, tar)
#define   GETSIG(sig)           Log::GetSignal(LOGPOS, sig)

#ifndef NDEBUG
	#define   BEGIN(expr)           Log::Begin(LOGPOS, expr)
	#define   END                   Log::End(LOGPOS); return

	#define   TRC(txt)              Log::DbgLog(LOGPOS, Log::TRC, txt)
	#define   TRC1(txt,par1)        Log::DbgLog(LOGPOS, Log::TRC, txt, par1)
	#define   TRC2(txt,par1,par2)   Log::DbgLog(LOGPOS, Log::TRC, txt, par1,par2)

	#define   DBG(txt)              Log::DbgLog(LOGPOS, Log::DBG, txt)
	#define   DBG1(txt,par1)        Log::DbgLog(LOGPOS, Log::DBG, txt, par1)
	#define   DBG2(txt,par1,par2)   Log::DbgLog(LOGPOS, Log::DBG, txt, par1,par2)
#else
	#define   BEGIN(expr)           //Log::Begin(LOGPOS, txt)
	#define   END                   //Log::End(LOGPOS); return

	#define   TRC(txt)              //Log::DbgLog(LOGPOS, Log::TRC, txt)
	#define   TRC1(txt,par1)        //Log::DbgLog(LOGPOS, Log::TRC, txt, par1)
	#define   TRC2(txt,par1,par2)   //Log::DbgLog(LOGPOS, Log::TRC, txt, par1,par2)

	#define   DBG(txt)              //Log::DbgLog(LOGPOS, Log::DBG, txt)
	#define   DBG1(txt,par1)        //Log::DbgLog(LOGPOS, Log::DBG, txt, par1)
	#define   DBG2(txt,par1,par2)   //Log::DbgLog(LOGPOS, Log::DBG, txt, par1,par2)
#endif

#define   RUN(txt)              Log::DbgLog(LOGPOS, Log::RUN, txt)
#define   RUN1(txt,par1)        Log::DbgLog(LOGPOS, Log::RUN, txt, par1)
#define   RUN2(txt,par1,par2)   Log::DbgLog(LOGPOS, Log::RUN, txt, par1,par2)

#define   ERR(txt)              Log::DbgLog(LOGPOS, Log::ERR, txt)
#define   ERR1(txt,par1)        Log::DbgLog(LOGPOS, Log::ERR, txt, par1)
#define   ERR2(txt,par1,par2)   Log::DbgLog(LOGPOS, Log::ERR, txt, par1,par2)




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
  	const int Begin		 	(	const char * const modul,
													const int line,
													const char * const fmt,
													...);


}












#ifdef LALALALA

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



  	/**	@short R�ckgabe des Dateinamens des DbgLog

    Das DbgLog enth�lt Daten die f�r die Fehlersuche relevant sind.
    Nur f�r das Syslog sind die LogLevel relevant.
 	  @return	string mit dem Namen des Logfiles. Ggf leerer String
   	*/
   	char const*const GetDbgLog(void);


    /**@short R�ckgabe des Dateinamens des Logfiles f�r Verk�ufe

    Das VkLog enth�lt Daten �ber verkaufte Seriennummern. Nichts anderes.
    @result Dateinamens des Logfiles f�r Verk�ufe
    */
    char const*const GetVkLog (void);


    /**@short R�ckgabe des Dateinamens des Logfiles f�r Verk�ufe

    Das Log enth�lt Daten �ber kaufm�nnisch relevante Vorg�nge am
    Automaten. Etwa empfangene und verkaufte Seriennummern, Geldbewegungen.
    @result Dateinamens des Logfiles f�r Systemmeldungen
    */
    char const*const GetSysLog (void);


  	/**@short Initialisierung des Log-Moduls

    Initialisierung des Log-Moduls mit Log-Level und	Dateiname der Ausgabe-Datei.   Wird als filename ein Leerstring �bergeben
    (Default-Wert), dann wird lediglich nach stdout (cout) protokolliert.
    @param level	Default-Debuglevel f�r alle Module. Kann mit AddModul f�r
      							spezielle Module �berschrieben werden.
    @param Write false(default) es wird nicht in eine Datei geschrieben, sonst true
    @param DbgLog Dateiname f�r das DbgLog.
    @param VkLog Dateiname f�r das VkLog (verkaufte Seriennummern)
    @param Log Dateiname f�r das Log (Informationen �ber Geld, und Handling von Seriennummern)
    @return Wert ungleich 0 wenn nicht in Datei geschrieben werden konnte
    */
		const int Init(	const Log::LogType	level,
                    const bool Write,
  									const string& DbgLog,
  									const string& VkLog,
  									const string& Log
         					);

    /**@short Initialisierung des Log-Moduls

    @see Init
    Initialisierung des Log-Moduls mit Log-Level und	Dateiname der Ausgabe-Datei.   Wird als filename ein Leerstring �bergeben
    (Default-Wert), dann wird lediglich nach stderr (cout) protokolliert.
    @param level	Default-Debuglevel f�r alle Module. Kann mit AddModul f�r
     							spezielle Module �berschrieben werden. Level dabei in der Form ERR, OFF, RUN, ...
    @param Write false(default) es wird nicht in eine Datei geschrieben, sonst true
    @param DbgLog Dateiname f�r das DbgLog.
    @param VkLog Dateiname f�r das VkLog (verkaufte Seriennummern)
    @param Log Dateiname f�r das Log (Informationen �ber Geld, und Handling von Seriennummern)
    @return Wert ungleich 0 wenn nicht in Datei geschrieben werden konnte
    */
		const int Init(	const string& level,	//Loglevel
                    const bool Write,
  									const string& DbgLog,
  									const string& VkLog,
  									const string& Log
         					);



		/**@short speziellen DebugLevel zuf�gen

    Zuf�gen eines speziellen Debuglevels f�r ein spezielles Modul. Der Level eines Modules kann
    jederzeit mit einem neuen Level �berschrieben werden (einfach diese Funktion erneut aufrufen)
    oder auch mit RemModul() gel�scht werden. Dann gilt f�r das betreffende Modul der bei
    Init �bergebene globale Debuglevel.
  	@param modul Name des Modules in exakt gleicher Schreibweise wie das betreffende
    C / C++ File!
    @param level Ausgabelevel f�r dieses Modul in der Form DBG, OFF, ERR, ...
	  */
  	void AddModul(const string& modul, const string& level);

  	/**@short L�schen eines speziellen Debuglevels f�r ein spezielles Modul.

  	@see AddModul()
	  */
		void RemModul(const string& modul);


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
		const int DbgLog(const char * const modul,				//Name des Moduls
										 const int zeile,      					//Zeilennummer
										 const LogType level,						//Meldungstyp
										 const char * const fmt,		 			//Formatstring
        						 ...);


    /** @short Schreibt eine Meldung ins VkLogFile

        @param fmt Formatstring in printf typischer Art
        @return gleich 0 alles Ok, -1 Out of Memory, -2 kann nicht in Datei schreiben.
    */
    const int VkLog (const char * const fmt, ...);


    /** @short Schreibt eine Meldung ins System-Logfile

        @param fmt Formatstring in printf typischer Art
        @return gleich 0 alles Ok, -1 Out of Memory, -2 kann nicht in Datei schreiben.
    */
    const int SysLog   (const char * const fmt, ...);



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
    inline const int DbgLog (const char * const modul,		//Name des Moduls
													   const int zeile,      		  	//Zeilennummer
													   const LogType level,					//Meldungstyp
													   const T& meldung							//Eigentliche Meldung
          								)
   	{
  			std::ostringstream str;
     		str << meldung;;
       	return DbgLog(modul, zeile, level, str.str().c_str());
    }


    /**@short �bersetzt ein Signal in einen lesbaren String

    Signale aus signal.h bzw. dio.h werden �bergeben und in einen lesbaren
    String �bersetzt.

    @param  sig Signal aus signal.h
    @result String mit dem Namen
    */
    char * const GetSignalString(const int sig);


		/** @short Meldung "Signal gesendet"

        Level ist immer Log::SIG
        Die Parameter modul und zeile sind sinnvollerweise durch das Makro LOGPOS ausgef�llt.
        Das �bergebene Signal wird dabei in einen lesbaren Text und teilweise mit
        Kommentaren erg�nzt.
        @param modul  Name der Sourcendatei
        @param zeile  Zeilennummer der Sourcendatei
        @param signal Signal aus signal.h
        @param Ziel   geplanter Empf�nger des Signals
        @return gleich 0 alles Ok, -1 Out of Memory, -2 kann nicht in Datei schreiben.
        \see SNDSIG(sig,tar)

  	*/
    const int SndSignal	(	const char * const modul,				//Name des Moduls
					      					const int zeile,      					//Zeilennummer
								      		const int signal, 						  //Signaltyp
      										const char * const Ziel					//Empf�nger
          				);

		/** Meldung "Signal erhalten" Level ist immer Log::SIG
  			@see SndSignal
        \see GETSIG(sig,tar)
  	*/
    const int GetSignal	(	const char * const modul,				//Name des Moduls
					      					const int zeile,      					//Zeilennummer
								      		const int signal							//Signaltyp
          				);



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
    inline const int End (const char * const modul,				//Name des Moduls
													const int zeile,      					//Zeilennummer
													const T& result									//Return der Funktion
          								)
   	{
				ReduceIndentLevel();
  			std::ostringstream str;
     		str << "} with res=\"" << result << "\"";
       	return DbgLog(modul, zeile, TRC, str.str().c_str());
    }


		/** @short Meldung zum Ende einer Funktion (Level ist immer DBG) ohne result

  			@see int Begin()
  	*/
    inline const int End (const char * const modul,				//Name des Moduls
													const int zeile      					//Zeilennummer
          								)
   	{
			ReduceIndentLevel();
     	return DbgLog(modul, zeile, TRC, "}");
    }


    inline const int End (const char * const modul,				//Name des Moduls
													const int zeile,      					//Zeilennummer
													const bool result								//Return der Funktion
          								)
   	{
    		return End(modul, zeile, result?"true":"false");
    }


    /**@short R�ckgabe des aktuellen Loglevels

    @result Level*/
    const LogType GetCurrentLevel(void);


};

#endif // LALALALA
#endif		//LOG_HEADER_FILE
