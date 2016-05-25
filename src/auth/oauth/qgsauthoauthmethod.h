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


#ifndef QGSAUTHOAUTHMETHOD_H
#define QGSAUTHOAUTHMETHOD_H

#include <QObject>
#include <QDialog>

#include "qgsauthconfig.h"
#include "qgsauthmethod.h"
#include "o0settingsstore.h"

class O1Twitter;
class O1Requestor;

class QgsAuthOauthMethod : public QgsAuthMethod
{
    Q_OBJECT

  public:
    explicit QgsAuthOauthMethod();
    ~QgsAuthOauthMethod();

    // QgsAuthMethod interface
    QString key() const override;

    QString description() const override;

    QString displayDescription() const override;

    bool updateNetworkRequest(QNetworkRequest &request, const QString &authcfg,
                               const QString &dataprovider = QString()) override;

    bool updateNetworkReply( QNetworkReply *reply, const QString &authcfg,
                             const QString &dataprovider ) override;

    bool updateDataSourceUriItems( QStringList &connectionItems, const QString &authcfg,
                                   const QString &dataprovider = QString() ) override;

    void clearCachedConfig( const QString &authcfg ) override;

    void updateMethodConfig( QgsAuthMethodConfig &mconfig ) override;

  public slots:
    void onLinkedChanged();
    void onLinkingFailed();
    void onLinkingSucceeded();

    void onOpenBrowser(const QUrl &url);
    void onCloseBrowser();
    void onReplyFinished();
    void onNetworkError(QNetworkReply::NetworkError error);

  private:
    QgsAuthMethodConfig getMethodConfig( const QString &authcfg, bool fullconfig = true );

    void putMethodConfig( const QString &authcfg, const QgsAuthMethodConfig& mconfig );

    void removeMethodConfig( const QString &authcfg );

    static QMap<QString, QgsAuthMethodConfig> mAuthConfigCache;

    QDialog *mDlg;
    O1Twitter *mO1Twit;
    O1Requestor *mReqstr;
    O0SettingsStore *mSettings;
    QString mTweet;
};

#endif // QGSAUTHOAUTHMETHOD_H
