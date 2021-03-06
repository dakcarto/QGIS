/***************************************************************************
    qgsfeaturerequest.cpp
    ---------------------
    begin                : Mai 2012
    copyright            : (C) 2012 by Martin Dobias
    email                : wonder dot sk at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#include "qgsfeaturerequest.h"

#include "qgsfield.h"
#include "qgsgeometry.h"

#include <QStringList>

//constants
const QString QgsFeatureRequest::AllAttributes = QString( "#!allattributes!#" );

QgsFeatureRequest::QgsFeatureRequest()
    : mFilter( FilterNone )
    , mFilterFid( -1 )
    , mFilterExpression( 0 )
    , mFlags( 0 )
{
}

QgsFeatureRequest::QgsFeatureRequest( QgsFeatureId fid )
    : mFilter( FilterFid )
    , mFilterFid( fid )
    , mFilterExpression( 0 )
    , mFlags( 0 )
{
}

QgsFeatureRequest::QgsFeatureRequest( const QgsRectangle& rect )
    : mFilter( FilterRect )
    , mFilterRect( rect )
    , mFilterFid( -1 )
    , mFilterExpression( 0 )
    , mFlags( 0 )
{
}

QgsFeatureRequest::QgsFeatureRequest( const QgsExpression& expr, const QgsExpressionContext &context )
    : mFilter( FilterExpression )
    , mFilterFid( -1 )
    , mFilterExpression( new QgsExpression( expr.expression() ) )
    , mExpressionContext( context )
    , mFlags( 0 )
{
}

QgsFeatureRequest::QgsFeatureRequest( const QgsFeatureRequest &rh )
{
  operator=( rh );
}

QgsFeatureRequest& QgsFeatureRequest::operator=( const QgsFeatureRequest & rh )
{
  mFlags = rh.mFlags;
  mFilter = rh.mFilter;
  mFilterRect = rh.mFilterRect;
  mFilterFid = rh.mFilterFid;
  mFilterFids = rh.mFilterFids;
  if ( rh.mFilterExpression )
  {
    mFilterExpression = new QgsExpression( rh.mFilterExpression->expression() );
  }
  else
  {
    mFilterExpression = 0;
  }
  mExpressionContext = rh.mExpressionContext;
  mAttrs = rh.mAttrs;
  mSimplifyMethod = rh.mSimplifyMethod;
  return *this;
}

QgsFeatureRequest::~QgsFeatureRequest()
{
  delete mFilterExpression;
}

QgsFeatureRequest& QgsFeatureRequest::setFilterRect( const QgsRectangle& rect )
{
  if ( mFilter == FilterNone )
    mFilter = FilterRect;
  mFilterRect = rect;
  return *this;
}

QgsFeatureRequest& QgsFeatureRequest::setFilterFid( QgsFeatureId fid )
{
  mFilter = FilterFid;
  mFilterFid = fid;
  return *this;
}

QgsFeatureRequest&QgsFeatureRequest::setFilterFids( const QgsFeatureIds& fids )
{
  mFilter = FilterFids;
  mFilterFids = fids;
  return *this;
}

QgsFeatureRequest& QgsFeatureRequest::setFilterExpression( const QString& expression )
{
  mFilter = FilterExpression;
  delete mFilterExpression;
  mFilterExpression = new QgsExpression( expression );
  return *this;
}

QgsFeatureRequest &QgsFeatureRequest::setExpressionContext( const QgsExpressionContext &context )
{
  mExpressionContext = context;
  return *this;
}

QgsFeatureRequest& QgsFeatureRequest::setFlags( const QgsFeatureRequest::Flags& flags )
{
  mFlags = flags;
  return *this;
}

QgsFeatureRequest& QgsFeatureRequest::setSubsetOfAttributes( const QgsAttributeList& attrs )
{
  mFlags |= SubsetOfAttributes;
  mAttrs = attrs;
  return *this;
}

QgsFeatureRequest& QgsFeatureRequest::setSubsetOfAttributes( const QStringList& attrNames, const QgsFields& fields )
{
  if ( attrNames.contains( QgsFeatureRequest::AllAttributes ) )
  {
    //attribute string list contains the all attributes flag, so we must fetch all attributes
    return *this;
  }

  mFlags |= SubsetOfAttributes;
  mAttrs.clear();

  Q_FOREACH ( const QString& attrName, attrNames )
  {
    int attrNum = fields.fieldNameIndex( attrName );
    if ( attrNum != -1 && !mAttrs.contains( attrNum ) )
      mAttrs.append( attrNum );
  }

  return *this;
}

QgsFeatureRequest& QgsFeatureRequest::setSimplifyMethod( const QgsSimplifyMethod& simplifyMethod )
{
  mSimplifyMethod = simplifyMethod;
  return *this;
}

bool QgsFeatureRequest::acceptFeature( const QgsFeature& feature )
{
  switch ( mFilter )
  {
    case QgsFeatureRequest::FilterNone:
      return true;
      break;

    case QgsFeatureRequest::FilterRect:
      if ( feature.constGeometry() && feature.constGeometry()->intersects( mFilterRect ) )
        return true;
      else
        return false;
      break;

    case QgsFeatureRequest::FilterFid:
      if ( feature.id() == mFilterFid )
        return true;
      else
        return false;
      break;

    case QgsFeatureRequest::FilterExpression:
      mExpressionContext.setFeature( feature );
      if ( mFilterExpression->evaluate( &mExpressionContext ).toBool() )
        return true;
      else
        return false;
      break;

    case QgsFeatureRequest::FilterFids:
      if ( mFilterFids.contains( feature.id() ) )
        return true;
      else
        return false;
      break;
  }

  return true;
}

#include "qgsfeatureiterator.h"
#include "qgslogger.h"

QgsAbstractFeatureSource::~QgsAbstractFeatureSource()
{
  while ( !mActiveIterators.empty() )
  {
    QgsAbstractFeatureIterator *it = *mActiveIterators.begin();
    QgsDebugMsg( "closing active iterator" );
    it->close();
  }
}

void QgsAbstractFeatureSource::iteratorOpened( QgsAbstractFeatureIterator* it )
{
  mActiveIterators.insert( it );
}

void QgsAbstractFeatureSource::iteratorClosed( QgsAbstractFeatureIterator* it )
{
  mActiveIterators.remove( it );
}


