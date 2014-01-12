
#ifndef QGSMACAPPKIT_H
#define QGSMACAPPKIT_H

#include "qgsmacnative.h"

class QgsNSRunningApplication : public QgsMacAppKit
{
	public:
    QgsNSRunningApplication();
    ~QgsNSRunningApplication();

    const char* currentAppLocalizedName();
    void currentAppActivateIgnoringOtherApps();
	
	private:
		class Private;
		Private* d;
};

#endif // QGSMACAPPKIT_H
