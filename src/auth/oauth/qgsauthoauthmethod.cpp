/***************************************************************************
    begin                : May 25, 2016
    copyright            : (C) 2016 by Boundless Spatial, Inc. USA
    author               : Larry Shaffer
    email                : lshaffer at boundlessgeo dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsauthoauthmethod.h"
#include "qgsauthoauthedit.h"

#include "qgsauthmanager.h"
#include "qgsnetworkaccessmanager.h"
#include "qgslogger.h"
#include "qgsmessagelog.h"

#include <QDir>
#include <QSettings>
#include <QString>
#include <QWebView>

#include "o0globals.h"
#include "o1twitter.h"
#include "o1requestor.h"
#include "o0requestparameter.h"


static const QString AUTH_METHOD_KEY = "OAuth";
static const QString AUTH_METHOD_DESCRIPTION = "OAuth authentication";

QMap<QString, QgsAuthMethodConfig> QgsAuthOauthMethod::mAuthConfigCache = QMap<QString, QgsAuthMethodConfig>();


QgsAuthOauthMethod::QgsAuthOauthMethod()
    : QgsAuthMethod()
    , mDlg(nullptr)
    , mO1Twit(nullptr)
    , mReqstr(nullptr)
    , mSettings(nullptr)
    , mTweet(QString())
{
  setVersion( 2 );
  setExpansions( QgsAuthMethod::NetworkRequest | QgsAuthMethod::DataSourceURI );
  setDataProviders( QStringList()
                    << "twitter" );
}

QgsAuthOauthMethod::~QgsAuthOauthMethod()
{
}

QString QgsAuthOauthMethod::key() const
{
  return AUTH_METHOD_KEY;
}

QString QgsAuthOauthMethod::description() const
{
  return AUTH_METHOD_DESCRIPTION;
}

QString QgsAuthOauthMethod::displayDescription() const
{
  return tr( "OAuth authentication" );
}

bool QgsAuthOauthMethod::updateNetworkRequest(QNetworkRequest &request, const QString &authcfg,
    const QString &dataprovider )
{
  Q_UNUSED( dataprovider )

  mTweet = request.attribute(QNetworkRequest::User).toString();

  QgsAuthMethodConfig mconfig = getMethodConfig( authcfg );
  if ( !mconfig.isValid() )
  {
    QgsMessageLog::logMessage( QString( "Update request config FAILED for authcfg: %1: config invalid" ).arg( authcfg ), "OAuth" );
    return false;
  }

  QString clientid = mconfig.config( "clientid" );
  QString clientsecret = mconfig.config( "clientsecret" );

  mO1Twit = new O1Twitter(this);
  mO1Twit->unlink();
  mO1Twit->setClientId(clientid);
  mO1Twit->setClientSecret(clientsecret);
  connect(mO1Twit, SIGNAL(linkedChanged()), this, SLOT(onLinkedChanged()));
  connect(mO1Twit, SIGNAL(linkingFailed()), this, SLOT(onLinkingFailed()));
  connect(mO1Twit, SIGNAL(linkingSucceeded()), this, SLOT(onLinkingSucceeded()));
  connect(mO1Twit, SIGNAL(openBrowser(QUrl)), this, SLOT(onOpenBrowser(QUrl)));
  connect(mO1Twit, SIGNAL(closeBrowser()), this, SLOT(onCloseBrowser()));

  QSettings* settings = new QSettings(QDir::homePath() + QDir::separator() + ".qgis2/oauth-cache.ini",
                                      QSettings::IniFormat);
  mSettings = new O0SettingsStore(settings, "myencryptionkey");
  mO1Twit->setStore(mSettings);
  mO1Twit->link();

  return true;
}

bool QgsAuthOauthMethod::updateNetworkReply(QNetworkReply *reply, const QString &authcfg, const QString &dataprovider)
{
  Q_UNUSED( reply )
  Q_UNUSED( authcfg )
  Q_UNUSED( dataprovider )

  return true;
}

void QgsAuthOauthMethod::onLinkedChanged() {
  // Linking (login) state has changed.
  // Use o1->linked() to get the actual state
  QgsDebugMsg("Link state changed");
}

void QgsAuthOauthMethod::onLinkingFailed() {
  // Login has failed
  QgsMessageLog::logMessage( "Login has failed", "OAuth");
}

void QgsAuthOauthMethod::onLinkingSucceeded() {
  // Login has succeeded
  QgsMessageLog::logMessage("Login has succeeded", "OAuth");
  QNetworkAccessManager *manager = new QgsNetworkAccessManager(this);
  if (!mReqstr){
    mReqstr = new O1Requestor(manager, mO1Twit, this);
  }

  QgsMessageLog::logMessage(QString("Token: ") + mO1Twit->token(), "OAuth");
  QgsMessageLog::logMessage(QString("TokenSecret: ") + mO1Twit->tokenSecret(), "OAuth");

  QByteArray paramName("status");
  //QByteArray tweetText("My first tweet!");

  QList<O0RequestParameter> requestParams = QList<O0RequestParameter>();
  requestParams << O0RequestParameter(paramName, mTweet.toLatin1());

  // Using Twitter's REST API ver 1.1
  QUrl url = QUrl("https://api.twitter.com/1.1/statuses/update.json");

  QByteArray postData = mO1Twit->createQueryParameters(requestParams);

  QNetworkRequest request(url);
  request.setHeader(QNetworkRequest::ContentTypeHeader, O2_MIME_TYPE_XFORM);

  QNetworkReply *reply = mReqstr->post(request, requestParams, postData);
  connect(reply, SIGNAL(finished()), this, SLOT(onReplyFinished()));
  connect(reply, SIGNAL(error(QNetworkReply::NetworkError)), this, SLOT(onNetworkError(QNetworkReply::NetworkError)));
}

void QgsAuthOauthMethod::onOpenBrowser(const QUrl &url) {
  // Open a web browser or a web view with the given URL.
  // The user will interact with this browser window to
  // enter login name, password, and authorize your application
  // to access the Twitter account
  QgsMessageLog::logMessage( "Open browser requested", "OAuth");

  mDlg = new QDialog( 0 );
  mDlg->setWindowTitle( QObject::tr( "Authenticate" ) );
  QVBoxLayout *layout = new QVBoxLayout( mDlg );

  QWebView *page = new QWebView(mDlg);
  page->load(url);
  layout->addWidget( page );

  mDlg->setLayout( layout );
  mDlg->show();
  mDlg->raise();
  mDlg->activateWindow();
  mDlg->resize( 800, 512 );
}

void QgsAuthOauthMethod::onCloseBrowser() {
  // Close the browser window opened in openBrowser()
  QgsMessageLog::logMessage("Close browser requested", "OAuth");
  mDlg->close();
  mDlg->deleteLater();
}

void QgsAuthOauthMethod::onReplyFinished()
{
  QgsMessageLog::logMessage("Network reply finished", "OAuth");
  QNetworkReply *reply = qobject_cast<QNetworkReply *>(sender());
  QgsMessageLog::logMessage( QString( "Results: %1").arg(QString(reply->readAll())), "OAuth");
}

void QgsAuthOauthMethod::onNetworkError(QNetworkReply::NetworkError error) {
  Q_UNUSED(error);

  QgsMessageLog::logMessage("Network error occurred", "OAuth");
  QNetworkReply *reply = qobject_cast<QNetworkReply *>(sender());
  QgsMessageLog::logMessage( QString( "Error: %1").arg(reply->errorString()), "OAuth");
}

bool QgsAuthOauthMethod::updateDataSourceUriItems( QStringList &connectionItems, const QString &authcfg,
    const QString &dataprovider )
{
  Q_UNUSED( connectionItems )
  Q_UNUSED( authcfg )
  Q_UNUSED( dataprovider )

  return true;
}

void QgsAuthOauthMethod::updateMethodConfig( QgsAuthMethodConfig &mconfig )
{
  if ( mconfig.hasConfig( "oldconfigstyle" ) )
  {
    QgsDebugMsg( "Updating old style auth method config" );

    QStringList conflist = mconfig.config( "oldconfigstyle" ).split( "|||" );
    mconfig.setConfig( "clientid", conflist.at( 0 ) );
    mconfig.setConfig( "clientsecret", conflist.at( 1 ) );
    mconfig.removeConfig( "oldconfigstyle" );
  }

  // TODO: add updates as method version() increases due to config storage changes
}

void QgsAuthOauthMethod::clearCachedConfig( const QString &authcfg )
{
  removeMethodConfig( authcfg );
}

QgsAuthMethodConfig QgsAuthOauthMethod::getMethodConfig( const QString &authcfg, bool fullconfig )
{
  QgsAuthMethodConfig mconfig;

  // check if it is cached
  if ( mAuthConfigCache.contains( authcfg ) )
  {
    mconfig = mAuthConfigCache.value( authcfg );
    QgsDebugMsg( QString( "Retrieved config for authcfg: %1" ).arg( authcfg ) );
    return mconfig;
  }

  // else build oauth bundle
  if ( !QgsAuthManager::instance()->loadAuthenticationConfig( authcfg, mconfig, fullconfig ) )
  {
    QgsDebugMsg( QString( "Retrieve config FAILED for authcfg: %1" ).arg( authcfg ) );
    return QgsAuthMethodConfig();
  }

  // cache bundle
  putMethodConfig( authcfg, mconfig );

  return mconfig;
}

void QgsAuthOauthMethod::putMethodConfig( const QString &authcfg, const QgsAuthMethodConfig& mconfig )
{
  QgsDebugMsg( QString( "Putting oauth config for authcfg: %1" ).arg( authcfg ) );
  mAuthConfigCache.insert( authcfg, mconfig );
}

void QgsAuthOauthMethod::removeMethodConfig( const QString &authcfg )
{
  if ( mAuthConfigCache.contains( authcfg ) )
  {
    mAuthConfigCache.remove( authcfg );
    QgsDebugMsg( QString( "Removed oauth config for authcfg: %1" ).arg( authcfg ) );
  }
}


//////////////////////////////////////////////
// Plugin externals
//////////////////////////////////////////////

/**
 * Required class factory to return a pointer to a newly created object
 */
QGISEXTERN QgsAuthOauthMethod *classFactory()
{
  return new QgsAuthOauthMethod();
}

/** Required key function (used to map the plugin to a data store type)
 */
QGISEXTERN QString authMethodKey()
{
  return AUTH_METHOD_KEY;
}

/**
 * Required description function
 */
QGISEXTERN QString description()
{
  return AUTH_METHOD_DESCRIPTION;
}

/**
 * Required isAuthMethod function. Used to determine if this shared library
 * is an authentication method plugin
 */
QGISEXTERN bool isAuthMethod()
{
  return true;
}

/**
 * Optional class factory to return a pointer to a newly created edit widget
 */
QGISEXTERN QgsAuthOauthEdit *editWidget( QWidget *parent )
{
  return new QgsAuthOauthEdit( parent );
}

/**
 * Required cleanup function
 */
QGISEXTERN void cleanupAuthMethod() // pass QgsAuthMethod *method, then delete method  ?
{
}
