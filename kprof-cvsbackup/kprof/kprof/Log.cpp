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

#include <kdebug.h>
#include <cassert>
#include "Log.h"



//Rückgabe der Versionsnummer des Modules Log.h
const QString Log::getVersion(void){
  return ("$Id$");
};


		/** @short Begin of a function
        @param modul filename (use __file__)
        @param zeile linenumber (use __line__)
        @param fmt  string in kind of printf
        @return gleich 0 anything Ok, -1 Out of Memory, -2 cannot write
  	*/
const int Log::Begin(	const char * const modul,				//Name des Moduls
		      						const int zeile,      					//Zeilennummer
				      				const char * const fmt,		 			//Funktionsname, ggf. mit Params
        						...);







 
#ifdef laslfasdfasfdsfs

#include "signals.h"
#include "global.h"
#include <cstdio>
#include <sys/timeb.h>
#include <fstream>
#include <map>
#include <iostream>
#include <ansidecl.h>
using namespace std;




//Modul-lokale Variablen
static	Log::LogType							LEVEL		=Log::RUN;	//Ausgabelevel (wird in Init gesetzt)
static  char*                     SYSLOG=0;
static  char*                     VKLOG=0;
static  char*                     LOG=0;
static 	map<string, Log::LogType>	MODULLIST;					//DebugLevel für einzelne Module
static  int	                      INDENTLEVEL=0;			//Einrückungsebene
static  bool                      WRITE;              //soll ins Logfile geschrieben werden







char * const Log::GetSignalString(const int sig)
{
  Signal SignalSig=(Signal)sig;
  switch(SignalSig){
//signale aus signal.h
      case C_05: 			  return "--> C_05"; break;
      case D_NOT C_05: 	return "<-- C_05"; break;
      case C_1: 			  return "--> C_1"; break;
      case D_NOT C_1: 	return "<-- C_1"; break;
      case C_2: 			  return "--> C_2"; break;
      case D_NOT C_2: 	return "<-- C_2"; break;
      case C_5:				  return "--> C_5       (Bill 0 =   500)"; break;
      case D_NOT C_5:		return "<-- C_5       (Bill 0 =   500)"; break;
      case C_10:			  return "--> C_10      (Bill 1 =  1000)"; break;
      case D_NOT C_10:	return "<-- C_10      (Bill 1 =  1000)"; break;
      case C_20:			return "--> C_20      (Bill 2 =  2000)"; break;
      case D_NOT C_20:			return "<-- C_20      (Bill 2 =  2000)"; break;
      case C_50:			return "--> C_50      (Bill 3 =  5000)"; break;
      case D_NOT C_50:			return "<-- C_50      (Bill 3 =  5000)"; break;
      case C_100:			return "--> C_100     (Bill 4 = 10000)"; break;
      case D_NOT C_100:			return "<-- C_100     (Bill 4 = 10000)"; break;
      case alles: 		return "--> alles"; break;
      case D_NOT alles: 		return "<-- alles"; break;
      case nichts: 		return "--> nichts"; break;
      case D_NOT nichts: 		return "<-- nichts"; break;
      case C_4: 			return "--> C_4"; break;
      case D_NOT C_4: 			return "<-- C_4"; break;
      case C_25: 			return "--> C_25"; break;
      case D_NOT C_25: 			return "<-- C_25"; break;
      case M_0: 			return "--> M_0"; break;
      case D_NOT M_0: 			return "<-- M_0"; break;
      case M_1: 			return "--> M_1"; break;
      case D_NOT M_1: 			return "<-- M_1"; break;
      case M_2: 			return "--> M_2"; break;
      case D_NOT M_2: 			return "<-- M_2"; break;
      case B_0: 			return "--> B_0"; break;
      case D_NOT B_0: 			return "<-- B_0"; break;
      case B_1: 			return "--> B_1"; break;
      case D_NOT B_1: 			return "<-- B_1"; break;
      case B_2: 			return "--> B_2"; break;
      case D_NOT B_2: 			return "<-- B_2"; break;
      case B_3: 			return "--> B_3"; break;
      case D_NOT B_3: 			return "<-- B_3"; break;
      case B_4: 			return "--> B_4"; break;
      case D_NOT B_4: 			return "<-- B_4"; break;
      case K_1: 			return "--> K_1 (Taste 1)"; break;
      case D_NOT K_1: 			return "<-- K_1 (Taste 1)"; break;
      case K_2: 			return "--> K_2 (Taste 2)"; break;
      case D_NOT K_2: 			return "<-- K_2 (Taste 2)"; break;
      case K_3: 			return "--> K_3 (Taste 3)"; break;
      case D_NOT K_3: 			return "<-- K_3 (Taste 3)"; break;
      case K_4: 			return "--> K_4 (Taste 4)"; break;
      case D_NOT K_4: 			return "<-- K_4 (Taste 4)"; break;
      case K_5: 			return "--> K_5 (Taste 5)"; break;
      case D_NOT K_5: 			return "<-- K_5 (Taste 5)"; break;
      case K_6: 			return "--> K_6 (Taste 6)"; break;
      case D_NOT K_6: 			return "<-- K_6 (Taste 6)"; break;
      case K_7: 			return "--> K_7 (Taste 7 (z.B. Bon drucken))"; break;
      case D_NOT K_7: 			return "<-- K_7 (Taste 7 (z.B. Bon drucken))"; break;
      case K_8: 			return "--> K_8 (Taste 8 (z.B. Abbruch))"; break;
      case D_NOT K_8: 			return "<-- K_8 (Taste 8 (z.B. Abbruch))"; break;
      case K_9: 			return "--> K_9 (Taste 9)"; break;
      case D_NOT K_9: 			return "<-- K_9 (Taste 9)"; break;
      case K_0: 			return "--> K_0 (Taste 0 (z.B. logout))"; break;
      case D_NOT K_0: 			return "<-- K_0 (Taste 0 (z.B. logout))"; break;
      case K_E: 			return "--> K_E (Taste Enter)"; break;
      case D_NOT K_E: 			return "<-- K_E (Taste Enter)"; break;
      case K_B: 			return "--> K_B (Taste B)"; break; 	// Tasten
      case D_NOT K_B: 			return "<-- K_B (Taste B)"; break; 	// Tasten
      case K_C: 			return "--> K_C (Taste C)"; break; 	// Tasten
      case D_NOT K_C: 			return "<-- K_C (Taste C)"; break; 	// Tasten
      case TOUT: 			return "--> TOUT (Timeout tick fuer Statemashines)"; break;
      case D_NOT TOUT: 			return "<-- TOUT (Timeout tick fuer Statemashines)"; break;
      case HOPP_DONE: return "--> HOPP_DONE (Geldaushabe beendet)"; break;
      case D_NOT HOPP_DONE: return "<-- HOPP_DONE (Geldaushabe beendet)"; break;
      case HOPP_FAIL: return "--> HOPP_FAIL (Geldausgabe fehlgeschlagen)"; break;
      case D_NOT HOPP_FAIL: return "<-- HOPP_FAIL (Geldausgabe fehlgeschlagen)"; break;
      case TEST_SELL: return "--> TEST_SELL (Sell one of this products (TEST only))"; break;
      case D_NOT TEST_SELL: return "<-- TEST_SELL (Sell one of this products (TEST only))"; break;
      case WARTUNG: 	return "--> WARTUNG (Wartungsmenue aufrufen)"; break;
      case D_NOT WARTUNG: 	return "<-- WARTUNG (Wartungsmenue aufrufen)"; break;
      case FAIL: 			return "--> FAIL (Fehler)"; break;
      case D_NOT FAIL: 			return "<-- FAIL (Fehler)"; break;
      case LOCK: 			return "--> LOCK (Fehler)"; break;
      case D_NOT LOCK: 			return "<-- LOCK (Fehler)"; break;
      case UNLOCK: 		return "--> UNLOCK (Sperren/freigeben)"; break;
      case D_NOT UNLOCK: 		return "<-- UNLOCK (Sperren/freigeben)"; break;
      case BLOCK: 		return "--> BLOCK (Menu blockieren)"; break;
      case D_NOT BLOCK: 		return "<-- BLOCK (Menu blockieren)"; break;
      case START_SELL:return "--> START_SELL (Beginn einer Transaktion)"; break;
      case D_NOT START_SELL:return "<-- START_SELL (Beginn einer Transaktion)"; break;
      case ABORT: 		return "--> ABORT (Transaktion fehlgeschlagen)"; break;
      case D_NOT ABORT: 		return "<-- ABORT (Transaktion fehlgeschlagen)"; break;
      case TEST_OK: 	return "--> TEST_OK		(Selbsttest erfolgreich)"; break;//
      case D_NOT TEST_OK: 	return "<-- TEST_OK		(Selbsttest erfolgreich)"; break;//
      case M_REFRESH: return "--> M_REFRESH (Menueinhalt neu ausgeben)"; break;
      case D_NOT M_REFRESH: return "<-- M_REFRESH (Menueinhalt neu ausgeben)"; break;
      case LAST_SIGNAL:return "--> LAST_SIGNAL"; break;
      case D_NOT LAST_SIGNAL:return "<-- LAST_SIGNAL"; break;
      case NONE:      return "--> Bewirkt nichts"; break;
      case D_NOT NONE:      return "<-- Bewirkt nichts"; break;
      default: break;
  }

//DIN Signale aus dio.h

  DIN DINsig = (DIN) sig;
  switch(DINsig){
      case BTN0: 			return "--> BTN0"; break;
      case D_NOT BTN0: 			return "<-- BTN0"; break;
      case BTN1: 			return "--> BTN1"; break;
      case D_NOT BTN1: 			return "<-- BTN1"; break;
      case BTN2: 			return "--> BTN2"; break;
      case D_NOT BTN2: 			return "<-- BTN2"; break;
      case BTN3: 			return "--> BTN3"; break;
      case D_NOT BTN3: 			return "<-- BTN3"; break;
      case BTN4: 			return "--> BTN4"; break;
      case D_NOT BTN4: 			return "<-- BTN4"; break;
      case BTN5: 			return "--> BTN5"; break;
      case D_NOT BTN5: 			return "<-- BTN5"; break;
      case BTN6: 			return "--> BTN6"; break;
      case D_NOT BTN6: 			return "<-- BTN6"; break;
      case BTN7: 			return "--> BTN7"; break;
      case D_NOT BTN7: 			return "<-- BTN7"; break;
      case BTN8: 			return "--> BTN8"; break;
      case D_NOT BTN8: 			return "<-- BTN8"; break;
      case B_STKF: 		return "--> B_STKF"; break;
      case D_NOT B_STKF: 		return "<-- B_STKF"; break;
      case B_ABN: 		return "--> B_ABN"; break;
      case D_NOT B_ABN: 		return "<-- B_ABN"; break;
      case B_BUSY: 		return "--> B_BUSY"; break;
      case D_NOT B_BUSY: 		return "<-- B_BUSY"; break;
      case B_VE3: 		return "--> B_VE3"; break;
      case D_NOT B_VE3: 		return "<-- B_VE3"; break;
      case B_VE2: 		return "--> B_VE2"; break;
      case D_NOT B_VE2: 		return "<-- B_VE2"; break;
      case B_VE1: 		return "--> B_VE1"; break;
      case D_NOT B_VE1: 		return "<-- B_VE1"; break;
      case B_VALID: 	return "--> B_VALID"; break;
      case D_NOT B_VALID: 	return "<-- B_VALID"; break;
      case H_FULL: 		return "--> H_FULL"; break;
      case D_NOT H_FULL: 		return "<-- H_FULL"; break;
      case H_LOW: 		return "--> H_LOW"; break;
      case D_NOT H_LOW: 		return "<-- H_LOW"; break;
      case H_SEC: 		return "--> H_SEC"; break;
      case D_NOT H_SEC: 		return "<-- H_SEC"; break;
      case H_SENS: 		return "--> H_SENS"; break;
      case D_NOT H_SENS: 		return "<-- H_SENS"; break;
      case H_FAIL: 		return "--> H_FAIL"; break;
      case D_NOT H_FAIL: 		return "<-- H_FAIL"; break;
      case H_DONE: 		return "--> H_DONE"; break;
      case D_NOT H_DONE: 		return "<-- H_DONE"; break;
      case M_M0: 			return "--> M_M0"; break;
      case D_NOT M_M0: 			return "<-- M_M0"; break;
      case M_M1: 			return "--> M_M1"; break;
      case D_NOT M_M1: 			return "<-- M_M1"; break;
      case M_M2: 			return "--> M_M2"; break;
      case D_NOT M_M2: 			return "<-- M_M2"; break;
      case M_M3: 			return "--> M_M3"; break;
      case D_NOT M_M3: 			return "<-- M_M3"; break;
      case M_M4: 			return "--> M_M4"; break;
      case D_NOT M_M4: 			return "<-- M_M4"; break;
      case M_M5: 			return "--> M_M5"; break;
      case D_NOT M_M5: 			return "<-- M_M5"; break;
      case M_RUECK: 	return "--> M_RUECK"; break;
      case D_NOT M_RUECK: 	return "<-- M_RUECK"; break;
      case M_RETURN:	return "--> M_RETURN"; break;
      case D_NOT M_RETURN:	return "<-- M_RETURN"; break;
      case E_RET: 		return "--> E_RET"; break;
      case D_NOT E_RET: 		return "<-- E_RET"; break;
      case E_CASH: 		return "--> E_CASH"; break;
      case D_NOT E_CASH: 		return "<-- E_CASH"; break;
      case E_FAIL: 		return "--> E_FAIL"; break;
      case D_NOT E_FAIL: 		return "<-- E_FAIL"; break;
      case E_R_DONE:	return "--> E_R_DONE"; break;
      case D_NOT E_R_DONE:	return "<-- E_R_DONE"; break;
      case E_C_DONE:	return "--> E_C_DONE"; break;
      case D_NOT E_C_DONE:	return "<-- E_C_DONE"; break;
      case P_SHUT: 		return "--> P_SHUT"; break;
      case D_NOT P_SHUT: 		return "<-- P_SHUT"; break;
      case P_FAIL: 		return "--> P_FAIL"; break;
      case D_NOT P_FAIL: 		return "<-- P_FAIL"; break;
      case P_BATLOW:	return "--> P_BATLOW"; break;
      case D_NOT P_BATLOW:	return "<-- P_BATLOW"; break;
      case P_24V: 		return "--> P_24V"; break;
      case D_NOT P_24V: 		return "<-- P_24V"; break;
      case D_1: 			return "--> D_1"; break;
      case D_NOT D_1: 			return "<-- D_1"; break;
      case D_2: 			return "--> D_2"; break;
      case D_NOT D_2: 			return "<-- D_2"; break;
      case D_3: 			return "--> D_3"; break;
      case D_NOT D_3: 			return "<-- D_3"; break;
      case D_4: 			return "--> D_4"; break;
      case D_NOT D_4: 			return "<-- D_4"; break;
      case D_5: 			return "--> D_5"; break;
      case D_NOT D_5: 			return "<-- D_5"; break;
      case D_6: 			return "--> D_6"; break;
      case D_NOT D_6: 			return "<-- D_6"; break;
      case D_7: 			return "--> D_7"; break;
      case D_NOT D_7: 			return "<-- D_7"; break;
      case D_8: 			return "--> D_8"; break;
      case D_NOT D_8: 			return "<-- D_8"; break;
      case VERSION_0:	return "--> VERSION_0"; break;
      case D_NOT VERSION_0:	return "<-- VERSION_0"; break;
      case VERSION_1:	return "--> VERSION_1"; break;
      case D_NOT VERSION_1:	return "<-- VERSION_1"; break;
      case D_OPEN: 		return "--> D_OPEN"; break;
      case D_NOT D_OPEN: 		return "<-- D_OPEN"; break;
      case D_LOCK: 		return "--> D_LOCK"; break;
      case D_NOT D_LOCK: 		return "<-- D_LOCK"; break;
      case L_PAPER: 	return "--> L_PAPER"; break;
      case D_NOT L_PAPER: 	return "<-- L_PAPER"; break;
      case L_HOPPER:	return "--> L_HOPPER"; break;
      case D_NOT L_HOPPER:	return "<-- L_HOPPER"; break;
      case SENSE_2: 	return "--> SENSE_2"; break;
      case D_NOT SENSE_2: 	return "<-- SENSE_2"; break;
      case SENSE_3: 	return "--> SENSE_3"; break;
      case D_NOT SENSE_3: 	return "<-- SENSE_3"; break;
      case H_BLOCK: 	return "--> H_BLOCK"; break;
      case D_NOT H_BLOCK: 	return "<-- H_BLOCK"; break;
      case B_OK: 			return "--> B_OK"; break;
      case D_NOT B_OK: 			return "<-- B_OK"; break;
      case KEY_SWITCH: 			return "--> KEY_SWITCH"; break;
      case D_NOT KEY_SWITCH: 			return "<-- KEY_SWITCH"; break;
      case BTN9: 			return "--> BTN9"; break;
      case D_NOT BTN9: 			return "<-- BTN9"; break;
      case BTN10: 			return "--> BTN10"; break;
      case D_NOT BTN10: 			return "<-- BTN10"; break;
      case BTN11: 			return "--> BTN11"; break;
      case D_NOT BTN11: 			return "<-- BTN11"; break;
      case BTN12: 			return "--> BTN12"; break;
      case D_NOT BTN12: 			return "<-- BTN12"; break;
      case LAST_DIN:	return "--> LAST_DIN"; break;
      case D_NOT LAST_DIN:	return "<-- LAST_DIN"; break;
      default: break;
  }


  //Handling von DOUT Signalen
  DOUT DOUTsig = (DOUT)sig;
  switch(DOUTsig){
      case P_PERMSHUT:return "--> P_PERMSHUT"; break;
      case D_NOT P_PERMSHUT:return "<-- P_PERMSHUT"; break;
      case P_FULL: 		return "--> P_FULL"; break;
      case D_NOT P_FULL: 		return "<-- P_FULL"; break;
      case P_12V_ON:	return "--> P_12V_ON"; break;
      case D_NOT P_12V_ON:	return "<-- P_12V_ON"; break;
      case P_24V_ON:	return "--> P_24V_ON"; break;
      case D_NOT P_24V_ON:	return "<-- P_24V_ON"; break;
      case B_REJ: 		return "--> B_REJ"; break;
      case D_NOT B_REJ: 		return "<-- B_REJ"; break;
      case B_ACK: 		return "--> B_ACK"; break;
      case D_NOT B_ACK: 		return "<-- B_ACK"; break;
      case B_INH: 		return "--> B_INH"; break;
      case D_NOT B_INH: 		return "<-- B_INH"; break;
      case B_POWER: 	return "--> B_POWER"; break;
      case D_NOT B_POWER: 	return "<-- B_POWER"; break;
      case H_SPEND: 	return "--> H_SPEND"; break;
      case D_NOT H_SPEND: 	return "<-- H_SPEND"; break;
      case H_POWER: 	return "--> H_POWER"; break;
      case D_NOT H_POWER: 	return "<-- H_POWER"; break;
      case H_START: 	return "--> H_START"; break;
      case D_NOT H_START: 	return "<-- H_START"; break;
      case E_O_RET: 	return "--> E_O_RET"; break;
      case D_NOT E_O_RET: 	return "<-- E_O_RET"; break;
      case E_O_CASH:	return "--> E_O_CASH"; break;
      case D_NOT E_O_CASH:	return "<-- E_O_CASH"; break;
      case M_SP0: 		return "--> M_SP0"; break;
      case D_NOT M_SP0: 		return "<-- M_SP0"; break;
      case M_SP1: 		return "--> M_SP1"; break;
      case D_NOT M_SP1: 		return "<-- M_SP1"; break;
      case M_SP2: 		return "--> M_SP2"; break;
      case D_NOT M_SP2: 		return "<-- M_SP2"; break;
      case M_SP3: 		return "--> M_SP3"; break;
      case D_NOT M_SP3: 		return "<-- M_SP3"; break;
      case M_SP4: 		return "--> M_SP4"; break;
      case D_NOT M_SP4: 		return "<-- M_SP4"; break;
      case M_SP5: 		return "--> M_SP5"; break;
      case D_NOT M_SP5: 		return "<-- M_SP5"; break;
      case M_SPALL: 	return "--> M_SPALL"; break;
      case D_NOT M_SPALL: 	return "<-- M_SPALL"; break;
      case M_RMOT: 		return "--> M_RMOT"; break;
      case D_NOT M_RMOT: 		return "<-- M_RMOT"; break;
      case T_SP0: 		return "--> T_SP0"; break;
      case D_NOT T_SP0: 		return "<-- T_SP0"; break;
      case T_SP1: 		return "--> T_SP1"; break;
      case D_NOT T_SP1: 		return "<-- T_SP1"; break;
      case T_SP2: 		return "--> T_SP2"; break;
      case D_NOT T_SP2: 		return "<-- T_SP2"; break;
      case T_SP3: 		return "--> T_SP3"; break;
      case D_NOT T_SP3: 		return "<-- T_SP3"; break;
      case T_SP4: 		return "--> T_SP4"; break;
      case D_NOT T_SP4: 		return "<-- T_SP4"; break;
      case T_SP5: 		return "--> T_SP5"; break;
      case D_NOT T_SP5: 		return "<-- T_SP5"; break;
      case T_SPALL:		return "--> T_SPALL"; break;
      case D_NOT T_SPALL:		return "<-- T_SPALL"; break;
      case P_LCD_ON:	return "--> P_LCD_ON"; break;
      case D_NOT P_LCD_ON:	return "<-- P_LCD_ON"; break;
      case LCD_1: 		return "--> LCD_1"; break;
      case D_NOT LCD_1: 		return "<-- LCD_1"; break;
      case LCD_2: 		return "--> LCD_2"; break;
      case D_NOT LCD_2: 		return "<-- LCD_2"; break;
      case LCD_3: 		return "--> LCD_3"; break;
      case D_NOT LCD_3: 		return "<-- LCD_3"; break;
      case LCD_FULL:	return "--> LCD_FULL"; break;
      case D_NOT LCD_FULL:	return "<-- LCD_FULL"; break;
      case LAST_DOUT:	return "--> LAST_DOUT"; break;
      case D_NOT LAST_DOUT:	return "<-- LAST_DOUT"; break;
      default: break;
  }

  //Handling von COUNT Signalen aus dio.h
  COUNT COUNTsig = (COUNT)sig;
  switch(COUNTsig){
      case H_SPEND_TOUT: return "--> H_SPEND_TOUT"; break;
      case D_NOT H_SPEND_TOUT: return "<-- H_SPEND_TOUT"; break;
      case ESCROW: 		return "--> ESCROW"; break;
      case D_NOT ESCROW: 		return "<-- ESCROW"; break;
      case E_C_RET: 	return "--> E_C_RET"; break;
      case D_NOT E_C_RET: 	return "<-- E_C_RET"; break;
      case E_C_CASH:	return "--> E_C_CASH"; break;
      case D_NOT E_C_CASH:	return "<-- E_C_CASH"; break;
      case M_RET: 		      return "--> M_RET"; break;
      case D_NOT M_RET: 		return "<-- M_RET"; break;
      case B_TIMER: 	return "--> B_TIMER"; break;
      case D_NOT B_TIMER: 	return "<-- B_TIMER"; break;
      case B_STATE: 	return "--> B_STATE"; break;
      case D_NOT B_STATE: 	return "<-- B_STATE"; break;
      case P_COUNT: 	return "--> P_COUNT"; break;
      case D_NOT P_COUNT: 	return "<-- P_COUNT"; break;
      case LAST_COUNT:return "--> LAST_COUNT"; break;
      case D_NOT LAST_COUNT:return "<-- LAST_COUNT"; break;
      default: break;
  }
  {
    static char s[50];
    if(sig >= 0)
      snprintf(s,49,"--> unbekanntes Signal %d", sig);
    else
      snprintf(s,49,"<-- unbekanntes Signal %d", sig);
    return s;
  }

}



//Hilfsfunktionen für dieses Modul

const int WriteLogEntry(const char * const fmt, char const * const file, const va_list ap)
{
  assert(fmt);

  //Meldung anhängen
//  if(VKLOG == 0) return -2;

  int n, size = 100;//Puffergroesse
  char *p=0;
  if  ((p = (char*)malloc (size)) == 0){
    	cout << "Out of Memory" << endl;
      return -1;
  };

  while (1) { //Versuch in Buffer zu schreiben;
      n = vsnprintf (p, size, fmt, ap);
      va_end(ap);

      //wenn ok
      if (n > -1 && n < size)
        break;

      //ansonsten
      size *= 2; //Platz verdoppeln
      char *pp=p;
      if ((p = (char*)realloc (p, size)) == 0){
        free(pp);
    		cout << "Out of Memory" << endl;
      	return -1;
      }
  }

  ofstream out;
  out.open(file, ios::app);
  if(out){
    out << p << endl << flush;
   	out.close();
  }else{
    if(p!=0) free(p);
    return -2;
  }
  if(p!=0) free(p);
	return 0;
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


void Log::AddModul(const string& modul, const string &Level){
	if (Level == "OFF") {
		cout << "Log::AddModul(" << modul << " = OFF)" << endl;
		MODULLIST[modul]=OFF;
	} else if (Level == "ERR") {
		cout << "Log::AddModul(" << modul << " = ERR)" << endl;
		MODULLIST[modul]=ERR;
	} else if (Level == "RUN") {
		cout << "Log::AddModul(" << modul << " = RUN)" << endl;
		MODULLIST[modul]=RUN;
	} else if (Level == "DBG") {
		cout << "Log::AddModul(" << modul << " = DBG)" << endl;
		MODULLIST[modul]=DBG;
	} else if (Level == "SIG") {
		cout << "Log::AddModul(" << modul << " = SIG)" << endl;
		MODULLIST[modul]=SIG;
	} else if (Level == "TRC") {
		cout << "Log::AddModul(" << modul << " = TRC)" << endl;
		MODULLIST[modul]=TRC;
	} else if (Level.length()==0){
		cout << "Log::AddModul(" << modul << "): Benutze Default-Level" <<endl;
  } else {
		cout << "Fehler bei Log::AddModul(" << modul << "): Level war \""<<Level<<"\""<<endl;
  }
}



void Log::RemModul(const string& modul){
	assert(modul.length()!=0);
  if(MODULLIST.find(modul)!=MODULLIST.end())
  	MODULLIST.erase(modul);
}



const Log::LogType GetModul(const string& modul){ //Rückgabe DbgLevel für Modul oder Level für alle Module
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




//Jetzt kommen die "oeffentlichen" Funktionen
///////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////




///////////////////////////////////////////////////////////////
//Initialisierung des Log-Moduls
const int Log::Init(const string&	level,				//Loglevel
                    const bool Write,
  									const string& DbgLog,
  									const string& VkLog,
  									const string& Log
         					)

{
	if (level == "OFF") {
    return Init(OFF,Write,DbgLog,VkLog,Log);
	} else if (level == "ERR") {
    return Init(ERR,Write,DbgLog,VkLog,Log);
	} else if (level == "RUN") {
    return Init(RUN,Write,DbgLog,VkLog,Log);
	} else if (level == "DBG") {
    return Init(DBG,Write,DbgLog,VkLog,Log);
	} else if (level == "SIG") {
    return Init(SIG,Write,DbgLog,VkLog,Log);
	} else if (level == "TRC") {
    return Init(TRC,Write,DbgLog,VkLog,Log);
	}

  //Wenn falscher Level
  cout << "Unbekannter LogLevel " << level << "! Benutze Default Level Log::RUN." <<endl;
  return Init(RUN,Write,DbgLog,VkLog,Log);
}




const int Log::Init(const LogType	level,				//Loglevel
                    const bool Write,
  									const string& DbgLog,
  									const string& VkLog,
  									const string& Log
         					)

{
	LEVEL = level;
 	SYSLOG= strdup(DbgLog.c_str());
 	VKLOG = strdup(VkLog.c_str());
 	LOG   = strdup(Log.c_str());


  WRITE=Write;
  if(!WRITE){
    cout << "Schreiben des DbgLog disabled" << endl;
  }

  //Testen der Logdateien
  ofstream out;
  if(WRITE && SYSLOG != 0 && *SYSLOG != 0) {
		out.open(SYSLOG,ios::app);
    if(out.is_open()) {
      cout << "Benutze DbgLog: "  << SYSLOG <<endl;
      out.close();
    } else {
      cout << "Kann DbgLog nicht nach \"" << SYSLOG << "\" schreiben!"<<endl;
      SYSLOG=0;
    }
  }
  if(VKLOG != 0 && *VKLOG != 0) {
		out.open(VKLOG,ios::app);
  	if(!out.is_open()) {
    	cout << "Kann VkLog nicht nach \"" << VKLOG << "\" schreiben!"<<endl;
    	VKLOG=0;
  	} else {
	    cout << "Benutze VkLog: "   << VKLOG <<endl;
  	  out.close();
  	}
	}
  if(LOG != 0 && *LOG != 0) {
		out.open(LOG,ios::app);
  	if(!out.is_open()) {
    	cout << "Kann Log nicht nach \"" << LOG << "\" schreiben!"<<endl;
    	LOG=0;
  	} else {
	    cout << "Benutze Log: "     << LOG <<endl;
  	  out.close();
  	}
	}

  switch(level){
  case OFF: cout << "Default LogLevel ist OFF" <<endl; break;
  case ERR: cout << "Default LogLevel ist ERR" <<endl; break;
  case RUN: cout << "Default LogLevel ist RUN" <<endl; break;
  case SIG: cout << "Default LogLevel ist SIG" <<endl; break;
  case DBG: cout << "Default LogLevel ist DBG" <<endl; break;
  case TRC: cout << "Default LogLevel ist TRC" <<endl; break;
  }

	return 0;
}






const int Log::DbgLog(const char * const modul,	//Name des Moduls
								      const int line,      			//Zeilennummer
								      const LogType level,			//Meldungstyp
								      const char * const fmt, 	//Formatstring
        				...
							)
{
  assert(modul);
  assert(strlen(modul)>0);
  assert(fmt);
  assert(strlen(fmt)>0);
  assert(line > 0);

	//Test ob überhaupt Meldung erzeugt werden muss
 	if(GetModul(modul) < level) return 0;
 	if(level == OFF)       return 0;

  std::string result="";
  if(LEVEL >= DBG){
    result+=DateAndTime();
  }
  //Modul und Zeile anhängen
  char pos[30];
  snprintf(pos, 29, "%14s(%4d) ", modul, line);
  result = result + " " + pos;

  //Meldungstyp speichern
	switch(level){
		case ERR: result+="ERR "; break;
		case SIG: result+="SIG "; break;
		case RUN: result+="RUN "; break;
		case DBG: result+="DBG "; break;
		case TRC: result+="TRC "; break;
		case OFF: result+="OFF "; break; //wird nicht benutzt, um compiler warnung zu vermeiden
	};

 	//Einrückung
  if(LEVEL >= OFF){
    for (int i=0; i<INDENTLEVEL;i++){
      result+="    ";
    }
  }

  //Meldung anhängen
	va_list ap;				//Liste der ... Parameter
  int n, size = 100;//Puffergroesse
  char *p=0;
  if  ((p = (char*)malloc (size)) == 0){
    	cout << "Out of Memory" << endl;
      return -1;
  };

  while (1) { //Versuch in Buffer zu schreiben;
      va_start(ap, fmt);
      n = vsnprintf (p, size, fmt, ap);
      va_end(ap);

      //wenn ok
      if (n > -1 && n < size){
        result+=p;
        free(p);
        break;
      }

      //ansonsten
      size *= 2; //Platz verdoppeln
      char *pp=p;
      if ((p = (char*)realloc (p, size)) == 0){
        free(pp);
    		cout << "Out of Memory" << endl;
      	return -1;
      }
  }

	cout << result << std::endl << std::flush;
  if(SYSLOG == 0) return -2;
  if(WRITE){
    ofstream out;
    out.open(SYSLOG, ios::app);
    if(out){
   		out << result << std::endl << std::flush;
      out.close();
    }else{
      return -2;
    }
  }
	return 0;
}







const int Log::Begin( const char * const modul,				//Name des Moduls
										  const int zeile,      					//Zeilennummer
										  const char * const fmt,		 			//Funktionsname, ggf. mit Params
        						  ...
                    )
{
  assert(modul);
  assert(fmt);
  assert(zeile > 0);

	int res=0;

  //Meldung anhängen
	va_list ap;				//Liste der ... Parameter
  int n, size = 100;//Puffergroesse
  char *p=0;
  if  ((p = (char*)malloc (size)) == 0){
    	cout << "Out of Memory" << endl;
      return -1;
  };

  while (1) { //Versuch in Buffer zu schreiben;
      va_start(ap, fmt);
      n = vsnprintf (p, size-1, fmt, ap);
      va_end(ap);

      //wenn ok
      if (n > -1 && n < size){
        strcat(p, "{");
				res=DbgLog(modul, zeile, TRC, p);
        free(p);
        break;
      }

      //ansonsten
      size *= 2; //Platz verdoppeln
      char *pp=p;
      if ((p = (char*)realloc (p, size)) == 0){
        free(pp);
    		cout << "Out of Memory" << endl;
      	return -1;
      }
  }
  INDENTLEVEL++;
 	return res;
}



//Rückgabe des aktuellen Loglevels
const Log::LogType Log::GetCurrentLevel(void)
{
  return LEVEL;
}




////////////////////////////////////////////////
//short Schreibt eine Meldung ins VkLogFile
const int Log::VkLog(const char * const fmt, ...)
{
  assert(fmt);
  int i;
  VA_OPEN(ap,fmt);
  VA_FIXEDARG(ap, const char * ,fmt);
  if (VKLOG)
		i  = WriteLogEntry(fmt,VKLOG, ap);
  VA_CLOSE(ap);
  return i;
}



////////////////////////////////////////////////
//short Schreibt eine Meldung ins Log
const int Log::SysLog(const char * const fmt, ...)
{
  assert(fmt);
  int i;
  VA_OPEN(ap,fmt);
  VA_FIXEDARG(ap, const char * ,fmt);
  if (LOG)
		i  = WriteLogEntry(fmt,LOG, ap);
  VA_CLOSE(ap);
  return i;
}



const int Log::GetSignal	(	const char * const modul,				//Name des Moduls
										const int zeile,      					//Zeilennummer
										const int signal							//Signaltyp
          				)
{
  assert(modul);
  assert(zeile > 0);
  if(GetModul(modul) != TRC)
    return DbgLog(modul, zeile, SIG, "Got %s", GetSignalString(signal));
  else
    return 0;
}


const int Log::SndSignal	(	const char * const modul,				//Name des Moduls
										const int zeile,      					//Zeilennummer
										const int signal, 						  //Signaltyp
										const char * const Ziel					//Empfänger
          				)
{
  assert(modul);
  assert(Ziel);
  assert(zeile > 0);

  if(GetModul(modul) != TRC)
    return DbgLog(modul, zeile, SIG, "send %s an %s", GetSignalString(signal), Ziel);
  else
    return 0;
}

//Rückgabe des Dateinamens des DbgLog
char const*const Log::GetSysLog(void)
{
	if (SYSLOG==0)	return "";
	else  		return SYSLOG;
}


//Rückgabe des Dateinamens des Logfiles für Verkäufe
char const*const Log::GetVkLog (void)
{
	if (VKLOG==0)	return "";
	else  		return VKLOG;
}

//Rückgabe des Dateinamens des Logfiles für Verkäufe
char const*const Log::GetDbgLog (void)
{
	if (LOG==0)	return "";
	else  		return LOG;
}

#endif //unsinn
