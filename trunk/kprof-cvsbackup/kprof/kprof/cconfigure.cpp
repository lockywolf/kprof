
#include "cconfigure.h"

#include <qwidget.h>
#include <kcolordialog.h>

CConfigure::CConfigure(QWidget *parent)
{
	parentWindow = parent;
	QString defaultColour("red");
	graphHighColour.setNamedColor(defaultColour);
}


CConfigure::~CConfigure()
{
	
}

void CConfigure::chooseGraphHighColour()
{
	int result = KColorDialog::getColor(graphHighColour, parentWindow);
}

const QColor& CConfigure::highColour() const
{
	return graphHighColour;
}
