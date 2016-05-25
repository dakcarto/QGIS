#ifndef OAuthTestPlugin_H
#define OAuthTestPlugin_H

// Qt Includes
#include <QObject>
#include <QAction>
#include <QToolBar>

// QGIS Includes
#include <qgisplugin.h>
#include <qgisinterface.h>
#include <qgisgui.h>

class QgsOAuthTestPlugin : public QObject, public QgisPlugin
{
    Q_OBJECT

  public:

    /**
     * Constructor for a plugin. The QgisInterface pointer is passed by
     * QGIS when it attempts to instantiate the plugin.
     * @param theInterface Pointer to the QgisInterface object.
     */
    explicit QgsOAuthTestPlugin( QgisInterface * theInterface );
    //! Destructor
    virtual ~QgsOAuthTestPlugin();

  public slots:

    //! init the gui
    virtual void initGui() override;
    //! Show the dialog box
    void run();
    //! unload the plugin
    void unload() override;
    //! show the help document
    void help();

  private:

    //! Pointer to the QGIS interface object
    QgisInterface *mQGisIface;
    //!pointer to the qaction for this plugin
    QAction * mQActionPointer;

};

#endif //OAuthTestPlugin_H
