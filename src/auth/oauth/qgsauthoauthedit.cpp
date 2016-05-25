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

#include "qgsauthoauthedit.h"
#include "ui_qgsauthoauthedit.h"


QgsAuthOauthEdit::QgsAuthOauthEdit( QWidget *parent )
    : QgsAuthMethodEdit( parent )
    , mValid( 0 )
{
  setupUi( this );
}

QgsAuthOauthEdit::~QgsAuthOauthEdit()
{
}

bool QgsAuthOauthEdit::validateConfig()
{
  bool curvalid = !leId->text().isEmpty() && !leSecret->text().isEmpty();
  if ( mValid != curvalid )
  {
    mValid = curvalid;
    emit validityChanged( curvalid );
  }
  return curvalid;
}

QgsStringMap QgsAuthOauthEdit::configMap() const
{
  QgsStringMap config;
  config.insert( "clientid", leId->text() );
  config.insert( "clientsecret", leSecret->text() );

  return config;
}

void QgsAuthOauthEdit::loadConfig( const QgsStringMap &configmap )
{
  clearConfig();

  mConfigMap = configmap;
  leId->setText( configmap.value( "clientid" ) );
  leSecret->setText( configmap.value( "clientsecret" ) );

  validateConfig();
}

void QgsAuthOauthEdit::resetConfig()
{
  loadConfig( mConfigMap );
}

void QgsAuthOauthEdit::clearConfig()
{
  leId->clear();
  leSecret->clear();
  chkSecretShow->setChecked( false );
}

void QgsAuthOauthEdit::on_leId_textChanged( const QString &txt )
{
  Q_UNUSED( txt );
  validateConfig();
}

void QgsAuthOauthEdit::on_chkSecretShow_stateChanged( int state )
{
  leSecret->setEchoMode(( state > 0 ) ? QLineEdit::Normal : QLineEdit::Password );
}
