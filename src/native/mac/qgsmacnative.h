#ifndef QGSMACNATIVE_H
#define QGSMACNATIVE_H

class QgsMacAppKit
{
  public:
    virtual ~QgsMacAppKit();

    // NSRunningApplication interface
    virtual const char* currentAppLocalizedName() = 0;
    virtual void currentAppActivateIgnoringOtherApps() = 0;
};

#endif // QGSMACNATIVE_H
