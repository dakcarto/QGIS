
#include "qgsmacappkit.h"

#include <Cocoa/Cocoa.h>

class QgsMacAppKit::Private
{
  public:
//    NSObject *obj;
};

QgsMacAppKit::QgsMacAppKit()
{
//  d = new Private;
//  d->obj = [NSObject someFunction];
}

QgsMacAppKit::~QgsMacAppKit()
{
//  [d->obj release];
//  delete d;
}

void QgsMacAppKit::activateIgnoringOtherApps()
{
  // NSLog(@"%@", [[NSRunningApplication currentApplication] localizedName]);
  // valid for Mac OS X >= 10.6
  [[NSRunningApplication currentApplication] activateWithOptions:
    (NSApplicationActivateAllWindows | NSApplicationActivateIgnoringOtherApps)];
}
