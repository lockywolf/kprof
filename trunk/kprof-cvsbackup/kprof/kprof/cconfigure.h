
#ifndef _CCONFIGURE_H_
#define _CCONFIGURE_H_

#include <qcolor.h>
#include <kcolordialog.h>
#include <qwidget.h>


/**
 * This class holds all the configuration data for KProf
 * Colin Desmond
 **/
class CConfigure
{

public:
	CConfigure(QWidget *parent = 0L);
	~CConfigure();
	
	const QColor& highColour() const;
	
	void chooseGraphHighColour();
	
private:
	QWidget* parentWindow;
	
	QColor graphHighColour;
};

#endif
