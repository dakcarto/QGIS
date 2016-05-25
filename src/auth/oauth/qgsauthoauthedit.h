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

#ifndef QGSAUTHOAUTHEDIT_H
#define QGSAUTHOAUTHEDIT_H

#include <QWidget>
#include "qgsauthmethodedit.h"
#include "ui_qgsauthoauthedit.h"

#include "qgsauthconfig.h"


class QgsAuthOauthEdit : public QgsAuthMethodEdit, private Ui::QgsAuthOauthEdit
{
    Q_OBJECT

  public:
    explicit QgsAuthOauthEdit( QWidget *parent = nullptr );
    virtual ~QgsAuthOauthEdit();

    bool validateConfig() override;

    QgsStringMap configMap() const override;

  public slots:
    void loadConfig( const QgsStringMap &configmap ) override;

    void resetConfig() override;

    void clearConfig() override;

  private slots:
    void on_leId_textChanged( const QString& txt );

    void on_chkSecretShow_stateChanged( int state );

  private:
    QgsStringMap mConfigMap;
    bool mValid;
};

#endif // QGSAUTHOAUTHEDIT_H
