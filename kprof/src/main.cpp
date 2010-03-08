/*
 * main.cpp
 *
 * $Id: main.cpp,v 1.11 2002/10/29 21:52:14 cdesmond Exp $
 *
 * Copyright (c) 2000 Florent Pillet <florent.pillet@wanadoo.fr>
 *
 * Requires the Qt widget libraries, available at no cost at
 * http://www.trolltech.com/
 *
 * Requires the K Desktop Environment 2.0 (KDE 2.0) libraries, available
 * at no cost at http://www.kde.org/
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

#include <KApplication>
#include <KLocale>
#include <KCmdLineArgs>
#include <KAboutData>

#include "kproftoplevel.h"

#define VERSION "1.9"

int main(int argc, char **argv)
{
  KAboutData aboutData("kprof", 0, ki18n("KProf"), VERSION,
                       ki18n("Gprof frontend for KDE."),
                       KAboutData::License_GPL,
                       ki18n("&copy; 2000-2010, Florent Pillet, Colin Desmond, "
                             "Martin Mor&aacute;&#x10d;ek"),
                       KLocalizedString(),
                       "http://kprof.sourceforge.net/",
                       "mori_bundus@users.sourceforge.net");

  KCmdLineOptions options;
  options.add("f <file>", ki18n("file to open"), "");
  options.add("c <file>", ki18n("file to compare with"), "");

  KCmdLineArgs::init(argc, argv, &aboutData);
  KCmdLineArgs::addCmdLineOptions(options);

  aboutData.addAuthor(ki18n("Florent Pillet"), ki18n(""),
                      "florent.pillet@wanadoo.fr");
  aboutData.addAuthor(ki18n("Colin Desmond"), ki18n(""),
                      "colin.desmond@btopenworld.com");
  aboutData.addAuthor(ki18n("Martin Mor&aacute;&#x10d;ek"), ki18n(""),
                      "mori_bundus@users.sourceforge.net");

  KApplication app;

  // session management
  if (app.isSessionRestored()) {
    kRestoreMainWindows<KProfTopLevel>();
  }
  else {
    KProfTopLevel * ktl = new KProfTopLevel(0);
    Q_CHECK_PTR(ktl);
    ktl->setObjectName("KProf main#");
    ktl->show();
  }

  return  app.exec();
}
