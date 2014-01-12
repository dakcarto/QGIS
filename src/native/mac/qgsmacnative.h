#ifndef QGSMAC_H
#define QGSMAC_H

class QgsMacNative
{
  public:
    virtual ~QgsMacNative();

    // QgsNsApplication interface

    virtual void activateIgnoringOtherApps() = 0;
};

#endif // QGSMAC_H
