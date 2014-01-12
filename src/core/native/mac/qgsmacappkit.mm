
#include "qgsmacappkit.h"

#include <Cocoa/Cocoa.h>

class QgsNSRunningApplication::Private
{
  public:
//    NSObject *obj;
};

QgsNSRunningApplication::QgsNSRunningApplication()
{
//  d = new Private;
//  d->obj = [NSObject someFunction];
}

QgsNSRunningApplication::~QgsNSRunningApplication()
{
//  [d->obj release];
//  delete d;
//  d = 0;
}

const char* QgsNSRunningApplication::currentAppLocalizedName()
{
  return [[[NSRunningApplication currentApplication] localizedName] UTF8String];
}

void QgsNSRunningApplication::currentAppActivateIgnoringOtherApps()
{
  // valid for Mac OS X >= 10.6
  [[NSRunningApplication currentApplication] activateWithOptions:
    (NSApplicationActivateAllWindows | NSApplicationActivateIgnoringOtherApps)];
}
