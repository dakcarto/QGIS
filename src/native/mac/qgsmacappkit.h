
#ifndef QGSMACAPPKIT_H
#define QGSMACAPPKIT_H

#include "qgsmacnative.h"

class QgsMacAppKit : public QgsMacNative
{
	public:
    QgsMacAppKit();
    ~QgsMacAppKit();

    void activateIgnoringOtherApps();
	
	private:
		class Private;
		Private* d;
};

#endif // QGSMACAPPKIT_H
