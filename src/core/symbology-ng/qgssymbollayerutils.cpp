/***************************************************************************
 qgssymbollayerutils.cpp
 ---------------------
 begin                : November 2009
 copyright            : (C) 2009 by Martin Dobias
 email                : wonder dot sk at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgssymbollayerutils.h"

#include "qgssymbollayer.h"
#include "qgssymbollayerregistry.h"
#include "qgssymbol.h"
#include "qgscolorramp.h"
#include "qgsexpression.h"
#include "qgspainteffect.h"
#include "qgspainteffectregistry.h"
#include "qgsapplication.h"
#include "qgsproject.h"
#include "qgsogcutils.h"
#include "qgslogger.h"
#include "qgsrendercontext.h"
#include "qgsunittypes.h"

#include <QColor>
#include <QFont>
#include <QDomDocument>
#include <QDomNode>
#include <QDomElement>
#include <QIcon>
#include <QPainter>
#include <QSettings>
#include <QRegExp>
#include <QPicture>

QString QgsSymbolLayerUtils::encodeColor( const QColor& color )
{
  return QString( "%1,%2,%3,%4" ).arg( color.red() ).arg( color.green() ).arg( color.blue() ).arg( color.alpha() );
}

QColor QgsSymbolLayerUtils::decodeColor( const QString& str )
{
  QStringList lst = str.split( ',' );
  if ( lst.count() < 3 )
  {
    return QColor( str );
  }
  int red, green, blue, alpha;
  red = lst[0].toInt();
  green = lst[1].toInt();
  blue = lst[2].toInt();
  alpha = 255;
  if ( lst.count() > 3 )
  {
    alpha = lst[3].toInt();
  }
  return QColor( red, green, blue, alpha );
}

QString QgsSymbolLayerUtils::encodeSldAlpha( int alpha )
{
  return QString::number( alpha / 255.0, 'f', 2 );
}

int QgsSymbolLayerUtils::decodeSldAlpha( const QString& str )
{
  bool ok;
  double alpha = str.toDouble( &ok );
  if ( !ok || alpha > 1 )
    alpha = 255;
  else if ( alpha < 0 )
    alpha = 0;
  return alpha * 255;
}

QString QgsSymbolLayerUtils::encodeSldFontStyle( QFont::Style style )
{
  switch ( style )
  {
    case QFont::StyleNormal:
      return "normal";
    case QFont::StyleItalic:
      return "italic";
    case QFont::StyleOblique:
      return "oblique";
    default:
      return "";
  }
}

QFont::Style QgsSymbolLayerUtils::decodeSldFontStyle( const QString& str )
{
  if ( str == "normal" ) return QFont::StyleNormal;
  if ( str == "italic" ) return QFont::StyleItalic;
  if ( str == "oblique" ) return QFont::StyleOblique;
  return QFont::StyleNormal;
}

QString QgsSymbolLayerUtils::encodeSldFontWeight( int weight )
{
  if ( weight == 50 ) return "normal";
  if ( weight == 75 ) return "bold";

  // QFont::Weight is between 0 and 99
  // CSS font-weight is between 100 and 900
  if ( weight < 0 ) return "100";
  if ( weight > 99 ) return "900";
  return QString::number( weight * 800 / 99 + 100 );
}

int QgsSymbolLayerUtils::decodeSldFontWeight( const QString& str )
{
  bool ok;
  int weight = str.toInt( &ok );
  if ( !ok )
    return static_cast< int >( QFont::Normal );

  // CSS font-weight is between 100 and 900
  // QFont::Weight is between 0 and 99
  if ( weight > 900 ) return 99;
  if ( weight < 100 ) return 0;
  return ( weight - 100 ) * 99 / 800;
}

QString QgsSymbolLayerUtils::encodePenStyle( Qt::PenStyle style )
{
  switch ( style )
  {
    case Qt::NoPen:
      return "no";
    case Qt::SolidLine:
      return "solid";
    case Qt::DashLine:
      return "dash";
    case Qt::DotLine:
      return "dot";
    case Qt::DashDotLine:
      return "dash dot";
    case Qt::DashDotDotLine:
      return "dash dot dot";
    default:
      return "???";
  }
}

Qt::PenStyle QgsSymbolLayerUtils::decodePenStyle( const QString& str )
{
  if ( str == "no" ) return Qt::NoPen;
  if ( str == "solid" ) return Qt::SolidLine;
  if ( str == "dash" ) return Qt::DashLine;
  if ( str == "dot" ) return Qt::DotLine;
  if ( str == "dash dot" ) return Qt::DashDotLine;
  if ( str == "dash dot dot" ) return Qt::DashDotDotLine;
  return Qt::SolidLine;
}

QString QgsSymbolLayerUtils::encodePenJoinStyle( Qt::PenJoinStyle style )
{
  switch ( style )
  {
    case Qt::BevelJoin:
      return "bevel";
    case Qt::MiterJoin:
      return "miter";
    case Qt::RoundJoin:
      return "round";
    default:
      return "???";
  }
}

Qt::PenJoinStyle QgsSymbolLayerUtils::decodePenJoinStyle( const QString& str )
{
  if ( str == "bevel" ) return Qt::BevelJoin;
  if ( str == "miter" ) return Qt::MiterJoin;
  if ( str == "round" ) return Qt::RoundJoin;
  return Qt::BevelJoin;
}

QString QgsSymbolLayerUtils::encodeSldLineJoinStyle( Qt::PenJoinStyle style )
{
  switch ( style )
  {
    case Qt::BevelJoin:
      return "bevel";
    case Qt::MiterJoin:
      return "mitre";
    case Qt::RoundJoin:
      return "round";
    default:
      return "";
  }
}

Qt::PenJoinStyle QgsSymbolLayerUtils::decodeSldLineJoinStyle( const QString& str )
{
  if ( str == "bevel" ) return Qt::BevelJoin;
  if ( str == "mitre" ) return Qt::MiterJoin;
  if ( str == "round" ) return Qt::RoundJoin;
  return Qt::BevelJoin;
}

QString QgsSymbolLayerUtils::encodePenCapStyle( Qt::PenCapStyle style )
{
  switch ( style )
  {
    case Qt::SquareCap:
      return "square";
    case Qt::FlatCap:
      return "flat";
    case Qt::RoundCap:
      return "round";
    default:
      return "???";
  }
}

Qt::PenCapStyle QgsSymbolLayerUtils::decodePenCapStyle( const QString& str )
{
  if ( str == "square" ) return Qt::SquareCap;
  if ( str == "flat" ) return Qt::FlatCap;
  if ( str == "round" ) return Qt::RoundCap;
  return Qt::SquareCap;
}

QString QgsSymbolLayerUtils::encodeSldLineCapStyle( Qt::PenCapStyle style )
{
  switch ( style )
  {
    case Qt::SquareCap:
      return "square";
    case Qt::FlatCap:
      return "butt";
    case Qt::RoundCap:
      return "round";
    default:
      return "";
  }
}

Qt::PenCapStyle QgsSymbolLayerUtils::decodeSldLineCapStyle( const QString& str )
{
  if ( str == "square" ) return Qt::SquareCap;
  if ( str == "butt" ) return Qt::FlatCap;
  if ( str == "round" ) return Qt::RoundCap;
  return Qt::SquareCap;
}

QString QgsSymbolLayerUtils::encodeBrushStyle( Qt::BrushStyle style )
{
  switch ( style )
  {
    case Qt::SolidPattern :
      return "solid";
    case Qt::HorPattern :
      return "horizontal";
    case Qt::VerPattern :
      return "vertical";
    case Qt::CrossPattern :
      return "cross";
    case Qt::BDiagPattern :
      return "b_diagonal";
    case Qt::FDiagPattern :
      return  "f_diagonal";
    case Qt::DiagCrossPattern :
      return "diagonal_x";
    case Qt::Dense1Pattern  :
      return "dense1";
    case Qt::Dense2Pattern  :
      return "dense2";
    case Qt::Dense3Pattern  :
      return "dense3";
    case Qt::Dense4Pattern  :
      return "dense4";
    case Qt::Dense5Pattern  :
      return "dense5";
    case Qt::Dense6Pattern  :
      return "dense6";
    case Qt::Dense7Pattern  :
      return "dense7";
    case Qt::NoBrush :
      return "no";
    default:
      return "???";
  }
}

Qt::BrushStyle QgsSymbolLayerUtils::decodeBrushStyle( const QString& str )
{
  if ( str == "solid" ) return Qt::SolidPattern;
  if ( str == "horizontal" ) return Qt::HorPattern;
  if ( str == "vertical" ) return Qt::VerPattern;
  if ( str == "cross" ) return Qt::CrossPattern;
  if ( str == "b_diagonal" ) return Qt::BDiagPattern;
  if ( str == "f_diagonal" ) return Qt::FDiagPattern;
  if ( str == "diagonal_x" ) return Qt::DiagCrossPattern;
  if ( str == "dense1" ) return Qt::Dense1Pattern;
  if ( str == "dense2" ) return Qt::Dense2Pattern;
  if ( str == "dense3" ) return Qt::Dense3Pattern;
  if ( str == "dense4" ) return Qt::Dense4Pattern;
  if ( str == "dense5" ) return Qt::Dense5Pattern;
  if ( str == "dense6" ) return Qt::Dense6Pattern;
  if ( str == "dense7" ) return Qt::Dense7Pattern;
  if ( str == "no" ) return Qt::NoBrush;
  return Qt::SolidPattern;
}

QString QgsSymbolLayerUtils::encodeSldBrushStyle( Qt::BrushStyle style )
{
  switch ( style )
  {
    case Qt::CrossPattern:
      return "cross";
    case Qt::DiagCrossPattern:
      return "x";

      /* The following names are taken from the presentation "GeoServer
       * Cartographic Rendering" by Andrea Aime at the FOSS4G 2010.
       * (see http://2010.foss4g.org/presentations/3588.pdf)
       */
    case Qt::HorPattern:
      return "horline";
    case Qt::VerPattern:
      return "line";
    case Qt::BDiagPattern:
      return "slash";
    case Qt::FDiagPattern:
      return "backslash";

      /* define the other names following the same pattern used above */
    case Qt::Dense1Pattern:
    case Qt::Dense2Pattern:
    case Qt::Dense3Pattern:
    case Qt::Dense4Pattern:
    case Qt::Dense5Pattern:
    case Qt::Dense6Pattern:
    case Qt::Dense7Pattern:
      return QString( "brush://%1" ).arg( encodeBrushStyle( style ) );

    default:
      return QString();
  }
}

Qt::BrushStyle QgsSymbolLayerUtils::decodeSldBrushStyle( const QString& str )
{
  if ( str == "horline" ) return Qt::HorPattern;
  if ( str == "line" ) return Qt::VerPattern;
  if ( str == "cross" ) return Qt::CrossPattern;
  if ( str == "slash" ) return Qt::BDiagPattern;
  if ( str == "backshash" ) return Qt::FDiagPattern;
  if ( str == "x" ) return Qt::DiagCrossPattern;

  if ( str.startsWith( "brush://" ) )
    return decodeBrushStyle( str.mid( 8 ) );

  return Qt::NoBrush;
}

QString QgsSymbolLayerUtils::encodePoint( QPointF point )
{
  return QString( "%1,%2" ).arg( qgsDoubleToString( point.x() ), qgsDoubleToString( point.y() ) );
}

QPointF QgsSymbolLayerUtils::decodePoint( const QString& str )
{
  QStringList lst = str.split( ',' );
  if ( lst.count() != 2 )
    return QPointF( 0, 0 );
  return QPointF( lst[0].toDouble(), lst[1].toDouble() );
}

QString QgsSymbolLayerUtils::encodeMapUnitScale( const QgsMapUnitScale& mapUnitScale )
{
  return QString( "%1,%2,%3,%4,%5,%6" ).arg( qgsDoubleToString( mapUnitScale.minScale ),
         qgsDoubleToString( mapUnitScale.maxScale ) )
         .arg( mapUnitScale.minSizeMMEnabled ? 1 : 0 )
         .arg( mapUnitScale.minSizeMM )
         .arg( mapUnitScale.maxSizeMMEnabled ? 1 : 0 )
         .arg( mapUnitScale.maxSizeMM );
}

QgsMapUnitScale QgsSymbolLayerUtils::decodeMapUnitScale( const QString& str )
{
  QStringList lst = str.split( ',' );
  if ( lst.count() < 2 )
    return QgsMapUnitScale();

  if ( lst.count() < 6 )
  {
    // old format
    return QgsMapUnitScale( lst[0].toDouble(), lst[1].toDouble() );
  }

  QgsMapUnitScale s( lst[0].toDouble(), lst[1].toDouble() );
  s.minSizeMMEnabled = lst[2].toInt();
  s.minSizeMM = lst[3].toDouble();
  s.maxSizeMMEnabled = lst[4].toInt();
  s.maxSizeMM = lst[5].toDouble();
  return s;
}

QString QgsSymbolLayerUtils::encodeSldUom( QgsUnitTypes::RenderUnit unit, double *scaleFactor )
{
  switch ( unit )
  {
    case QgsUnitTypes::RenderMapUnits:
      if ( scaleFactor )
        *scaleFactor = 0.001; // from millimeters to meters
      return "http://www.opengeospatial.org/se/units/metre";

    case QgsUnitTypes::RenderMillimeters:
    default:
      // pixel is the SLD default uom. The "standardized rendering pixel
      // size" is defined to be 0.28mm × 0.28mm (millimeters).
      if ( scaleFactor )
        *scaleFactor = 1 / 0.28;  // from millimeters to pixels

      // http://www.opengeospatial.org/sld/units/pixel
      return QString();
  }
}

QgsUnitTypes::RenderUnit QgsSymbolLayerUtils::decodeSldUom( const QString& str, double *scaleFactor )
{
  if ( str == "http://www.opengeospatial.org/se/units/metre" )
  {
    if ( scaleFactor )
      *scaleFactor = 1000.0;  // from meters to millimeters
    return QgsUnitTypes::RenderMapUnits;
  }
  else if ( str == "http://www.opengeospatial.org/se/units/foot" )
  {
    if ( scaleFactor )
      *scaleFactor = 304.8; // from feet to meters
    return QgsUnitTypes::RenderMapUnits;
  }

  // pixel is the SLD default uom. The "standardized rendering pixel
  // size" is defined to be 0.28mm x 0.28mm (millimeters).
  if ( scaleFactor )
    *scaleFactor = 1 / 0.00028; // from pixels to millimeters
  return QgsUnitTypes::RenderMillimeters;
}

QString QgsSymbolLayerUtils::encodeRealVector( const QVector<qreal>& v )
{
  QString vectorString;
  QVector<qreal>::const_iterator it = v.constBegin();
  for ( ; it != v.constEnd(); ++it )
  {
    if ( it != v.constBegin() )
    {
      vectorString.append( ';' );
    }
    vectorString.append( QString::number( *it ) );
  }
  return vectorString;
}

QVector<qreal> QgsSymbolLayerUtils::decodeRealVector( const QString& s )
{
  QVector<qreal> resultVector;

  QStringList realList = s.split( ';' );
  QStringList::const_iterator it = realList.constBegin();
  for ( ; it != realList.constEnd(); ++it )
  {
    resultVector.append( it->toDouble() );
  }

  return resultVector;
}

QString QgsSymbolLayerUtils::encodeSldRealVector( const QVector<qreal>& v )
{
  QString vectorString;
  QVector<qreal>::const_iterator it = v.constBegin();
  for ( ; it != v.constEnd(); ++it )
  {
    if ( it != v.constBegin() )
    {
      vectorString.append( ' ' );
    }
    vectorString.append( QString::number( *it ) );
  }
  return vectorString;
}

QVector<qreal> QgsSymbolLayerUtils::decodeSldRealVector( const QString& s )
{
  QVector<qreal> resultVector;

  QStringList realList = s.split( ' ' );
  QStringList::const_iterator it = realList.constBegin();
  for ( ; it != realList.constEnd(); ++it )
  {
    resultVector.append( it->toDouble() );
  }

  return resultVector;
}

QString QgsSymbolLayerUtils::encodeScaleMethod( QgsSymbol::ScaleMethod scaleMethod )
{
  QString encodedValue;

  switch ( scaleMethod )
  {
    case QgsSymbol::ScaleDiameter:
      encodedValue = "diameter";
      break;
    case QgsSymbol::ScaleArea:
      encodedValue = "area";
      break;
  }
  return encodedValue;
}

QgsSymbol::ScaleMethod QgsSymbolLayerUtils::decodeScaleMethod( const QString& str )
{
  QgsSymbol::ScaleMethod scaleMethod;

  if ( str == "diameter" )
  {
    scaleMethod = QgsSymbol::ScaleDiameter;
  }
  else
  {
    scaleMethod = QgsSymbol::ScaleArea;
  }

  return scaleMethod;
}

QPainter::CompositionMode QgsSymbolLayerUtils::decodeBlendMode( const QString &s )
{
  if ( s.compare( "Lighten", Qt::CaseInsensitive ) == 0 ) return QPainter::CompositionMode_Lighten;
  if ( s.compare( "Screen", Qt::CaseInsensitive ) == 0 ) return QPainter::CompositionMode_Screen;
  if ( s.compare( "Dodge", Qt::CaseInsensitive ) == 0 ) return QPainter::CompositionMode_ColorDodge;
  if ( s.compare( "Addition", Qt::CaseInsensitive ) == 0 ) return QPainter::CompositionMode_Plus;
  if ( s.compare( "Darken", Qt::CaseInsensitive ) == 0 ) return QPainter::CompositionMode_Darken;
  if ( s.compare( "Multiply", Qt::CaseInsensitive ) == 0 ) return QPainter::CompositionMode_Multiply;
  if ( s.compare( "Burn", Qt::CaseInsensitive ) == 0 ) return QPainter::CompositionMode_ColorBurn;
  if ( s.compare( "Overlay", Qt::CaseInsensitive ) == 0 ) return QPainter::CompositionMode_Overlay;
  if ( s.compare( "SoftLight", Qt::CaseInsensitive ) == 0 ) return QPainter::CompositionMode_SoftLight;
  if ( s.compare( "HardLight", Qt::CaseInsensitive ) == 0 ) return QPainter::CompositionMode_HardLight;
  if ( s.compare( "Difference", Qt::CaseInsensitive ) == 0 ) return QPainter::CompositionMode_Difference;
  if ( s.compare( "Subtract", Qt::CaseInsensitive ) == 0 ) return QPainter::CompositionMode_Exclusion;
  return QPainter::CompositionMode_SourceOver; // "Normal"
}

QIcon QgsSymbolLayerUtils::symbolPreviewIcon( QgsSymbol* symbol, QSize size )
{
  return QIcon( symbolPreviewPixmap( symbol, size ) );
}

QPixmap QgsSymbolLayerUtils::symbolPreviewPixmap( QgsSymbol* symbol, QSize size, QgsRenderContext* customContext )
{
  Q_ASSERT( symbol );

  QPixmap pixmap( size );
  pixmap.fill( Qt::transparent );
  QPainter painter;
  painter.begin( &pixmap );
  painter.setRenderHint( QPainter::Antialiasing );
  if ( customContext )
    customContext->setPainter( &painter );
  symbol->drawPreviewIcon( &painter, size, customContext );
  painter.end();
  return pixmap;
}

double QgsSymbolLayerUtils::estimateMaxSymbolBleed( QgsSymbol* symbol )
{
  double maxBleed = 0;
  for ( int i = 0; i < symbol->symbolLayerCount(); i++ )
  {
    QgsSymbolLayer* layer = symbol->symbolLayer( i );
    double layerMaxBleed = layer->estimateMaxBleed();
    maxBleed = layerMaxBleed > maxBleed ? layerMaxBleed : maxBleed;
  }

  return maxBleed;
}

QPicture QgsSymbolLayerUtils::symbolLayerPreviewPicture( QgsSymbolLayer* layer, QgsUnitTypes::RenderUnit units, QSize size, const QgsMapUnitScale& scale )
{
  QPicture picture;
  QPainter painter;
  painter.begin( &picture );
  painter.setRenderHint( QPainter::Antialiasing );
  QgsRenderContext renderContext = createRenderContext( &painter );
  renderContext.setForceVectorOutput( true );
  QgsSymbolRenderContext symbolContext( renderContext, units, 1.0, false, 0, nullptr, QgsFields(), scale );
  layer->drawPreviewIcon( symbolContext, size );
  painter.end();
  return picture;
}

QIcon QgsSymbolLayerUtils::symbolLayerPreviewIcon( QgsSymbolLayer* layer, QgsUnitTypes::RenderUnit u, QSize size, const QgsMapUnitScale& scale )
{
  QPixmap pixmap( size );
  pixmap.fill( Qt::transparent );
  QPainter painter;
  painter.begin( &pixmap );
  painter.setRenderHint( QPainter::Antialiasing );
  QgsRenderContext renderContext = createRenderContext( &painter );
  QgsSymbolRenderContext symbolContext( renderContext, u, 1.0, false, 0, nullptr, QgsFields(), scale );
  layer->drawPreviewIcon( symbolContext, size );
  painter.end();
  return QIcon( pixmap );
}

QIcon QgsSymbolLayerUtils::colorRampPreviewIcon( QgsColorRamp* ramp, QSize size )
{
  return QIcon( colorRampPreviewPixmap( ramp, size ) );
}

QPixmap QgsSymbolLayerUtils::colorRampPreviewPixmap( QgsColorRamp* ramp, QSize size )
{
  QPixmap pixmap( size );
  pixmap.fill( Qt::transparent );
  // pixmap.fill( Qt::white ); // this makes the background white instead of transparent
  QPainter painter;
  painter.begin( &pixmap );

  //draw stippled background, for transparent images
  drawStippledBackground( &painter, QRect( 0, 0, size.width(), size.height() ) );

  // antialising makes the colors duller, and no point in antialiasing a color ramp
  // painter.setRenderHint( QPainter::Antialiasing );
  for ( int i = 0; i < size.width(); i++ )
  {
    QPen pen( ramp->color( static_cast< double >( i ) / size.width() ) );
    painter.setPen( pen );
    painter.drawLine( i, 0, i, size.height() - 1 );
  }
  painter.end();
  return pixmap;
}

void QgsSymbolLayerUtils::drawStippledBackground( QPainter* painter, QRect rect )
{
  // create a 2x2 checker-board image
  uchar pixDataRGB[] = { 255, 255, 255, 255,
                         127, 127, 127, 255,
                         127, 127, 127, 255,
                         255, 255, 255, 255
                       };
  QImage img( pixDataRGB, 2, 2, 8, QImage::Format_ARGB32 );
  // scale it to rect so at least 5 patterns are shown
  int width = ( rect.width() < rect.height() ) ?
              rect.width() / 2.5 : rect.height() / 2.5;
  QPixmap pix = QPixmap::fromImage( img.scaled( width, width ) );
  // fill rect with texture
  QBrush brush;
  brush.setTexture( pix );
  painter->fillRect( rect, brush );
}

#include <QPolygonF>

#include <cmath>
#include <cfloat>

static QPolygonF makeOffsetGeometry( const QgsPolyline& polyline )
{
  int i, pointCount = polyline.count();

  QPolygonF resultLine;
  resultLine.resize( pointCount );

  const QgsPoint* tempPtr = polyline.data();

  for ( i = 0; i < pointCount; ++i, tempPtr++ )
    resultLine[i] = QPointF( tempPtr->x(), tempPtr->y() );

  return resultLine;
}
static QList<QPolygonF> makeOffsetGeometry( const QgsPolygon& polygon )
{
  QList<QPolygonF> resultGeom;
  resultGeom.reserve( polygon.size() );
  for ( int ring = 0; ring < polygon.size(); ++ring )
    resultGeom.append( makeOffsetGeometry( polygon[ ring ] ) );
  return resultGeom;
}

QList<QPolygonF> offsetLine( QPolygonF polyline, double dist, QgsWkbTypes::GeometryType geometryType )
{
  QList<QPolygonF> resultLine;

  if ( polyline.count() < 2 )
  {
    resultLine.append( polyline );
    return resultLine;
  }

  unsigned int i, pointCount = polyline.count();

  QgsPolyline tempPolyline( pointCount );
  QPointF* tempPtr = polyline.data();
  for ( i = 0; i < pointCount; ++i, tempPtr++ )
    tempPolyline[i] = QgsPoint( tempPtr->rx(), tempPtr->ry() );

  QgsGeometry tempGeometry = geometryType == QgsWkbTypes::PolygonGeometry ? QgsGeometry::fromPolygon( QgsPolygon() << tempPolyline ) : QgsGeometry::fromPolyline( tempPolyline );
  if ( !tempGeometry.isEmpty() )
  {
    int quadSegments = 0; // we want mitre joins, not round joins
    double mitreLimit = 2.0; // the default value in GEOS (5.0) allows for fairly sharp endings
    QgsGeometry offsetGeom;
    if ( geometryType == QgsWkbTypes::PolygonGeometry )
      offsetGeom = tempGeometry.buffer( -dist, quadSegments, QgsGeometry::CapFlat,
                                        QgsGeometry::JoinStyleMitre, mitreLimit );
    else
      offsetGeom = tempGeometry.offsetCurve( dist, quadSegments, QgsGeometry::JoinStyleMitre, mitreLimit );

    if ( !offsetGeom.isEmpty() )
    {
      tempGeometry = offsetGeom;

      if ( QgsWkbTypes::flatType( tempGeometry.wkbType() ) == QgsWkbTypes::LineString )
      {
        QgsPolyline line = tempGeometry.asPolyline();
        // Reverse the line if offset was negative, see
        // http://hub.qgis.org/issues/13811
        if ( dist < 0 ) std::reverse( line.begin(), line.end() );
        resultLine.append( makeOffsetGeometry( line ) );
        return resultLine;
      }
      else if ( QgsWkbTypes::flatType( tempGeometry.wkbType() ) == QgsWkbTypes::Polygon )
      {
        resultLine.append( makeOffsetGeometry( tempGeometry.asPolygon() ) );
        return resultLine;
      }
      else if ( QgsWkbTypes::flatType( tempGeometry.wkbType() ) == QgsWkbTypes::MultiLineString )
      {
        QgsMultiPolyline tempMPolyline = tempGeometry.asMultiPolyline();
        resultLine.reserve( tempMPolyline.count() );
        for ( int part = 0; part < tempMPolyline.count(); ++part )
        {
          resultLine.append( makeOffsetGeometry( tempMPolyline[ part ] ) );
        }
        return resultLine;
      }
      else if ( QgsWkbTypes::flatType( tempGeometry.wkbType() ) == QgsWkbTypes::MultiPolygon )
      {
        QgsMultiPolygon tempMPolygon = tempGeometry.asMultiPolygon();
        resultLine.reserve( tempMPolygon.count() );
        for ( int part = 0; part < tempMPolygon.count(); ++part )
        {
          resultLine.append( makeOffsetGeometry( tempMPolygon[ part ] ) );
        }
        return resultLine;
      }
    }
  }

  // returns original polyline when 'GEOSOffsetCurve' fails!
  resultLine.append( polyline );
  return resultLine;
}

QList<QPolygonF> offsetLine( const QPolygonF& polyline, double dist )
{
  QgsWkbTypes::GeometryType geometryType = QgsWkbTypes::PointGeometry;
  int pointCount = polyline.count();

  if ( pointCount > 3 && qgsDoubleNear( polyline[ 0 ].x(), polyline[ pointCount - 1 ].x() ) && qgsDoubleNear( polyline[ 0 ].y(), polyline[ pointCount - 1 ].y() ) )
  {
    geometryType = QgsWkbTypes::PolygonGeometry;
  }
  else if ( pointCount > 1 )
  {
    geometryType = QgsWkbTypes::LineGeometry;
  }
  return offsetLine( polyline, dist, geometryType );
}

/////


QgsSymbol* QgsSymbolLayerUtils::loadSymbol( const QDomElement &element )
{
  QgsSymbolLayerList layers;
  QDomNode layerNode = element.firstChild();

  while ( !layerNode.isNull() )
  {
    QDomElement e = layerNode.toElement();
    if ( !e.isNull() )
    {
      if ( e.tagName() != "layer" )
      {
        QgsDebugMsg( "unknown tag " + e.tagName() );
      }
      else
      {
        QgsSymbolLayer* layer = loadSymbolLayer( e );

        if ( layer )
        {
          // Dealing with sub-symbols nested into a layer
          QDomElement s = e.firstChildElement( "symbol" );
          if ( !s.isNull() )
          {
            QgsSymbol* subSymbol = loadSymbol( s );
            bool res = layer->setSubSymbol( subSymbol );
            if ( !res )
            {
              QgsDebugMsg( "symbol layer refused subsymbol: " + s.attribute( "name" ) );
            }
          }
          layers.append( layer );
        }
      }
    }
    layerNode = layerNode.nextSibling();
  }

  if ( layers.isEmpty() )
  {
    QgsDebugMsg( "no layers for symbol" );
    return nullptr;
  }

  QString symbolType = element.attribute( "type" );

  QgsSymbol* symbol = nullptr;
  if ( symbolType == "line" )
    symbol = new QgsLineSymbol( layers );
  else if ( symbolType == "fill" )
    symbol = new QgsFillSymbol( layers );
  else if ( symbolType == "marker" )
    symbol = new QgsMarkerSymbol( layers );
  else
  {
    QgsDebugMsg( "unknown symbol type " + symbolType );
    return nullptr;
  }

  if ( element.hasAttribute( "outputUnit" ) )
  {
    symbol->setOutputUnit( QgsUnitTypes::decodeRenderUnit( element.attribute( "outputUnit" ) ) );
  }
  if ( element.hasAttribute(( "mapUnitScale" ) ) )
  {
    QgsMapUnitScale mapUnitScale;
    mapUnitScale.minScale = element.attribute( "mapUnitMinScale", "0.0" ).toDouble();
    mapUnitScale.maxScale = element.attribute( "mapUnitMaxScale", "0.0" ).toDouble();
    symbol->setMapUnitScale( mapUnitScale );
  }
  symbol->setAlpha( element.attribute( "alpha", "1.0" ).toDouble() );
  symbol->setClipFeaturesToExtent( element.attribute( "clip_to_extent", "1" ).toInt() );

  return symbol;
}

QgsSymbolLayer* QgsSymbolLayerUtils::loadSymbolLayer( QDomElement& element )
{
  QString layerClass = element.attribute( "class" );
  bool locked = element.attribute( "locked" ).toInt();
  int pass = element.attribute( "pass" ).toInt();

  // parse properties
  QgsStringMap props = parseProperties( element );

  QgsSymbolLayer* layer;
  layer = QgsSymbolLayerRegistry::instance()->createSymbolLayer( layerClass, props );
  if ( layer )
  {
    layer->setLocked( locked );
    layer->setRenderingPass( pass );

    //restore layer effect
    QDomElement effectElem = element.firstChildElement( "effect" );
    if ( !effectElem.isNull() )
    {
      layer->setPaintEffect( QgsPaintEffectRegistry::instance()->createEffect( effectElem ) );
    }
    return layer;
  }
  else
  {
    QgsDebugMsg( "unknown class " + layerClass );
    return nullptr;
  }
}

static QString _nameForSymbolType( QgsSymbol::SymbolType type )
{
  switch ( type )
  {
    case QgsSymbol::Line:
      return "line";
    case QgsSymbol::Marker:
      return "marker";
    case QgsSymbol::Fill:
      return "fill";
    default:
      return "";
  }
}

QDomElement QgsSymbolLayerUtils::saveSymbol( const QString& name, QgsSymbol* symbol, QDomDocument& doc )
{
  Q_ASSERT( symbol );
  QDomElement symEl = doc.createElement( "symbol" );
  symEl.setAttribute( "type", _nameForSymbolType( symbol->type() ) );
  symEl.setAttribute( "name", name );
  symEl.setAttribute( "alpha", QString::number( symbol->alpha() ) );
  symEl.setAttribute( "clip_to_extent", symbol->clipFeaturesToExtent() ? "1" : "0" );
  //QgsDebugMsg( "num layers " + QString::number( symbol->symbolLayerCount() ) );

  for ( int i = 0; i < symbol->symbolLayerCount(); i++ )
  {
    QgsSymbolLayer* layer = symbol->symbolLayer( i );

    QDomElement layerEl = doc.createElement( "layer" );
    layerEl.setAttribute( "class", layer->layerType() );
    layerEl.setAttribute( "locked", layer->isLocked() );
    layerEl.setAttribute( "pass", layer->renderingPass() );
    saveProperties( layer->properties(), doc, layerEl );
    if ( !QgsPaintEffectRegistry::isDefaultStack( layer->paintEffect() ) )
      layer->paintEffect()->saveProperties( doc, layerEl );

    if ( layer->subSymbol() )
    {
      QString subname = QString( "@%1@%2" ).arg( name ).arg( i );
      QDomElement subEl = saveSymbol( subname, layer->subSymbol(), doc );
      layerEl.appendChild( subEl );
    }
    symEl.appendChild( layerEl );
  }

  return symEl;
}

QString QgsSymbolLayerUtils::symbolProperties( QgsSymbol* symbol )
{
  QDomDocument doc( "qgis-symbol-definition" );
  QDomElement symbolElem = saveSymbol( "symbol", symbol, doc );
  QString props;
  QTextStream stream( &props );
  symbolElem.save( stream, -1 );
  return props;
}

bool QgsSymbolLayerUtils::createSymbolLayerListFromSld( QDomElement& element,
    QgsWkbTypes::GeometryType geomType,
    QgsSymbolLayerList &layers )
{
  QgsDebugMsg( "Entered." );

  if ( element.isNull() )
    return false;

  QgsSymbolLayer *l = nullptr;

  QString symbolizerName = element.localName();

  if ( symbolizerName == "PointSymbolizer" )
  {
    // first check for Graphic element, nothing will be rendered if not found
    QDomElement graphicElem = element.firstChildElement( "Graphic" );
    if ( graphicElem.isNull() )
    {
      QgsDebugMsg( "Graphic element not found in PointSymbolizer" );
    }
    else
    {
      switch ( geomType )
      {
        case QgsWkbTypes::PolygonGeometry:
          // polygon layer and point symbolizer: draw poligon centroid
          l = QgsSymbolLayerRegistry::instance()->createSymbolLayerFromSld( "CentroidFill", element );
          if ( l )
            layers.append( l );

          break;

        case QgsWkbTypes::PointGeometry:
          // point layer and point symbolizer: use markers
          l = createMarkerLayerFromSld( element );
          if ( l )
            layers.append( l );

          break;

        case QgsWkbTypes::LineGeometry:
          // line layer and point symbolizer: draw central point
          l = QgsSymbolLayerRegistry::instance()->createSymbolLayerFromSld( "SimpleMarker", element );
          if ( l )
            layers.append( l );

          break;

        default:
          break;
      }
    }
  }

  if ( symbolizerName == "LineSymbolizer" )
  {
    // check for Stroke element, nothing will be rendered if not found
    QDomElement strokeElem = element.firstChildElement( "Stroke" );
    if ( strokeElem.isNull() )
    {
      QgsDebugMsg( "Stroke element not found in LineSymbolizer" );
    }
    else
    {
      switch ( geomType )
      {
        case QgsWkbTypes::PolygonGeometry:
        case QgsWkbTypes::LineGeometry:
          // polygon layer and line symbolizer: draw polygon outline
          // line layer and line symbolizer: draw line
          l = createLineLayerFromSld( element );
          if ( l )
            layers.append( l );

          break;

        case QgsWkbTypes::PointGeometry:
          // point layer and line symbolizer: draw a little line marker
          l = QgsSymbolLayerRegistry::instance()->createSymbolLayerFromSld( "MarkerLine", element );
          if ( l )
            layers.append( l );

          break;

        default:
          break;
      }
    }
  }

  if ( symbolizerName == "PolygonSymbolizer" )
  {
    // get Fill and Stroke elements, nothing will be rendered if both are missing
    QDomElement fillElem = element.firstChildElement( "Fill" );
    QDomElement strokeElem = element.firstChildElement( "Stroke" );
    if ( fillElem.isNull() && strokeElem.isNull() )
    {
      QgsDebugMsg( "neither Fill nor Stroke element not found in PolygonSymbolizer" );
    }
    else
    {
      QgsSymbolLayer *l = nullptr;

      switch ( geomType )
      {
        case QgsWkbTypes::PolygonGeometry:
          // polygon layer and polygon symbolizer: draw fill

          l = createFillLayerFromSld( element );
          if ( l )
          {
            layers.append( l );

            // SVGFill and SimpleFill symbolLayerV2 supports outline internally,
            // so don't go forward to create a different symbolLayerV2 for outline
            if ( l->layerType() == "SimpleFill" || l->layerType() == "SVGFill" )
              break;
          }

          // now create polygon outline
          // polygon layer and polygon symbolizer: draw polygon outline
          l = createLineLayerFromSld( element );
          if ( l )
            layers.append( l );

          break;

        case QgsWkbTypes::LineGeometry:
          // line layer and polygon symbolizer: draw line
          l = createLineLayerFromSld( element );
          if ( l )
            layers.append( l );

          break;

        case QgsWkbTypes::PointGeometry:
          // point layer and polygon symbolizer: draw a square marker
          convertPolygonSymbolizerToPointMarker( element, layers );
          break;

        default:
          break;
      }
    }
  }

  return true;
}

QgsSymbolLayer* QgsSymbolLayerUtils::createFillLayerFromSld( QDomElement &element )
{
  QDomElement fillElem = element.firstChildElement( "Fill" );
  if ( fillElem.isNull() )
  {
    QgsDebugMsg( "Fill element not found" );
    return nullptr;
  }

  QgsSymbolLayer *l = nullptr;

  if ( needLinePatternFill( element ) )
    l = QgsSymbolLayerRegistry::instance()->createSymbolLayerFromSld( "LinePatternFill", element );
  else if ( needPointPatternFill( element ) )
    l = QgsSymbolLayerRegistry::instance()->createSymbolLayerFromSld( "PointPatternFill", element );
  else if ( needSvgFill( element ) )
    l = QgsSymbolLayerRegistry::instance()->createSymbolLayerFromSld( "SVGFill", element );
  else
    l = QgsSymbolLayerRegistry::instance()->createSymbolLayerFromSld( "SimpleFill", element );

  return l;
}

QgsSymbolLayer* QgsSymbolLayerUtils::createLineLayerFromSld( QDomElement &element )
{
  QDomElement strokeElem = element.firstChildElement( "Stroke" );
  if ( strokeElem.isNull() )
  {
    QgsDebugMsg( "Stroke element not found" );
    return nullptr;
  }

  QgsSymbolLayer *l = nullptr;

  if ( needMarkerLine( element ) )
    l = QgsSymbolLayerRegistry::instance()->createSymbolLayerFromSld( "MarkerLine", element );
  else
    l = QgsSymbolLayerRegistry::instance()->createSymbolLayerFromSld( "SimpleLine", element );

  return l;
}

QgsSymbolLayer* QgsSymbolLayerUtils::createMarkerLayerFromSld( QDomElement &element )
{
  QDomElement graphicElem = element.firstChildElement( "Graphic" );
  if ( graphicElem.isNull() )
  {
    QgsDebugMsg( "Graphic element not found" );
    return nullptr;
  }

  QgsSymbolLayer *l = nullptr;

  if ( needFontMarker( element ) )
    l = QgsSymbolLayerRegistry::instance()->createSymbolLayerFromSld( "FontMarker", element );
  else if ( needSvgMarker( element ) )
    l = QgsSymbolLayerRegistry::instance()->createSymbolLayerFromSld( "SvgMarker", element );
  else if ( needEllipseMarker( element ) )
    l = QgsSymbolLayerRegistry::instance()->createSymbolLayerFromSld( "EllipseMarker", element );
  else
    l = QgsSymbolLayerRegistry::instance()->createSymbolLayerFromSld( "SimpleMarker", element );

  return l;
}

bool QgsSymbolLayerUtils::hasExternalGraphic( QDomElement &element )
{
  QDomElement graphicElem = element.firstChildElement( "Graphic" );
  if ( graphicElem.isNull() )
    return false;

  QDomElement externalGraphicElem = graphicElem.firstChildElement( "ExternalGraphic" );
  if ( externalGraphicElem.isNull() )
    return false;

  // check for format
  QDomElement formatElem = externalGraphicElem.firstChildElement( "Format" );
  if ( formatElem.isNull() )
    return false;

  QString format = formatElem.firstChild().nodeValue();
  if ( format != "image/svg+xml" )
  {
    QgsDebugMsg( "unsupported External Graphic format found: " + format );
    return false;
  }

  // check for a valid content
  QDomElement onlineResourceElem = externalGraphicElem.firstChildElement( "OnlineResource" );
  QDomElement inlineContentElem = externalGraphicElem.firstChildElement( "InlineContent" );
  if ( !onlineResourceElem.isNull() )
  {
    return true;
  }
#if 0
  else if ( !inlineContentElem.isNull() )
  {
    return false; // not implemented yet
  }
#endif
  else
  {
    return false;
  }
}

bool QgsSymbolLayerUtils::hasWellKnownMark( QDomElement &element )
{
  QDomElement graphicElem = element.firstChildElement( "Graphic" );
  if ( graphicElem.isNull() )
    return false;

  QDomElement markElem = graphicElem.firstChildElement( "Mark" );
  if ( markElem.isNull() )
    return false;

  QDomElement wellKnownNameElem = markElem.firstChildElement( "WellKnownName" );
  if ( wellKnownNameElem.isNull() )
    return false;

  return true;
}


bool QgsSymbolLayerUtils::needFontMarker( QDomElement &element )
{
  QDomElement graphicElem = element.firstChildElement( "Graphic" );
  if ( graphicElem.isNull() )
    return false;

  QDomElement markElem = graphicElem.firstChildElement( "Mark" );
  if ( markElem.isNull() )
    return false;

  // check for format
  QDomElement formatElem = markElem.firstChildElement( "Format" );
  if ( formatElem.isNull() )
    return false;

  QString format = formatElem.firstChild().nodeValue();
  if ( format != "ttf" )
  {
    QgsDebugMsg( "unsupported Graphic Mark format found: " + format );
    return false;
  }

  // check for a valid content
  QDomElement onlineResourceElem = markElem.firstChildElement( "OnlineResource" );
  QDomElement inlineContentElem = markElem.firstChildElement( "InlineContent" );
  if ( !onlineResourceElem.isNull() )
  {
    // mark with ttf format has a markIndex element
    QDomElement markIndexElem = markElem.firstChildElement( "MarkIndex" );
    if ( !markIndexElem.isNull() )
      return true;
  }
  else if ( !inlineContentElem.isNull() )
  {
    return false; // not implemented yet
  }

  return false;
}

bool QgsSymbolLayerUtils::needSvgMarker( QDomElement &element )
{
  return hasExternalGraphic( element );
}

bool QgsSymbolLayerUtils::needEllipseMarker( QDomElement &element )
{
  QDomElement graphicElem = element.firstChildElement( "Graphic" );
  if ( graphicElem.isNull() )
    return false;

  QgsStringMap vendorOptions = QgsSymbolLayerUtils::getVendorOptionList( graphicElem );
  for ( QgsStringMap::iterator it = vendorOptions.begin(); it != vendorOptions.end(); ++it )
  {
    if ( it.key() == "widthHeightFactor" )
    {
      return true;
    }
  }

  return false;
}

bool QgsSymbolLayerUtils::needMarkerLine( QDomElement &element )
{
  QDomElement strokeElem = element.firstChildElement( "Stroke" );
  if ( strokeElem.isNull() )
    return false;

  QDomElement graphicStrokeElem = strokeElem.firstChildElement( "GraphicStroke" );
  if ( graphicStrokeElem.isNull() )
    return false;

  return hasWellKnownMark( graphicStrokeElem );
}

bool QgsSymbolLayerUtils::needLinePatternFill( QDomElement &element )
{
  QDomElement fillElem = element.firstChildElement( "Fill" );
  if ( fillElem.isNull() )
    return false;

  QDomElement graphicFillElem = fillElem.firstChildElement( "GraphicFill" );
  if ( graphicFillElem.isNull() )
    return false;

  QDomElement graphicElem = graphicFillElem.firstChildElement( "Graphic" );
  if ( graphicElem.isNull() )
    return false;

  // line pattern fill uses horline wellknown marker with an angle

  QString name;
  QColor fillColor, borderColor;
  double size, borderWidth;
  Qt::PenStyle borderStyle;
  if ( !wellKnownMarkerFromSld( graphicElem, name, fillColor, borderColor, borderStyle, borderWidth, size ) )
    return false;

  if ( name != "horline" )
    return false;

  QString angleFunc;
  if ( !rotationFromSldElement( graphicElem, angleFunc ) )
    return false;

  bool ok;
  double angle = angleFunc.toDouble( &ok );
  if ( !ok || qgsDoubleNear( angle, 0.0 ) )
    return false;

  return true;
}

bool QgsSymbolLayerUtils::needPointPatternFill( QDomElement &element )
{
  Q_UNUSED( element );
  return false;
}

bool QgsSymbolLayerUtils::needSvgFill( QDomElement &element )
{
  QDomElement fillElem = element.firstChildElement( "Fill" );
  if ( fillElem.isNull() )
    return false;

  QDomElement graphicFillElem = fillElem.firstChildElement( "GraphicFill" );
  if ( graphicFillElem.isNull() )
    return false;

  return hasExternalGraphic( graphicFillElem );
}


bool QgsSymbolLayerUtils::convertPolygonSymbolizerToPointMarker( QDomElement &element, QgsSymbolLayerList &layerList )
{
  QgsDebugMsg( "Entered." );

  /* SE 1.1 says about PolygonSymbolizer:
  if a point geometry is referenced instead of a polygon,
  then a small, square, ortho-normal polygon should be
  constructed for rendering.
   */

  QgsSymbolLayerList layers;

  // retrieve both Fill and Stroke elements
  QDomElement fillElem = element.firstChildElement( "Fill" );
  QDomElement strokeElem = element.firstChildElement( "Stroke" );

  // first symbol layer
  {
    bool validFill = false, validBorder = false;

    // check for simple fill
    // Fill element can contain some SvgParameter elements
    QColor fillColor;
    Qt::BrushStyle fillStyle;

    if ( fillFromSld( fillElem, fillStyle, fillColor ) )
      validFill = true;

    // check for simple outline
    // Stroke element can contain some SvgParameter elements
    QColor borderColor;
    Qt::PenStyle borderStyle;
    double borderWidth = 1.0, dashOffset = 0.0;
    QVector<qreal> customDashPattern;

    if ( lineFromSld( strokeElem, borderStyle, borderColor, borderWidth,
                      nullptr, nullptr, &customDashPattern, &dashOffset ) )
      validBorder = true;

    if ( validFill || validBorder )
    {
      QgsStringMap map;
      map["name"] = "square";
      map["color"] = encodeColor( validFill ? fillColor : Qt::transparent );
      map["color_border"] = encodeColor( validBorder ? borderColor : Qt::transparent );
      map["size"] = QString::number( 6 );
      map["angle"] = QString::number( 0 );
      map["offset"] = encodePoint( QPointF( 0, 0 ) );
      layers.append( QgsSymbolLayerRegistry::instance()->createSymbolLayer( "SimpleMarker", map ) );
    }
  }

  // second symbol layer
  {
    bool validFill = false, validBorder = false;

    // check for graphic fill
    QString name, format;
    int markIndex = -1;
    QColor fillColor, borderColor;
    double borderWidth = 1.0, size = 0.0, angle = 0.0;
    QPointF anchor, offset;

    // Fill element can contain a GraphicFill element
    QDomElement graphicFillElem = fillElem.firstChildElement( "GraphicFill" );
    if ( !graphicFillElem.isNull() )
    {
      // GraphicFill element must contain a Graphic element
      QDomElement graphicElem = graphicFillElem.firstChildElement( "Graphic" );
      if ( !graphicElem.isNull() )
      {
        // Graphic element can contains some ExternalGraphic and Mark element
        // search for the first supported one and use it
        bool found = false;

        QDomElement graphicChildElem = graphicElem.firstChildElement();
        while ( !graphicChildElem.isNull() )
        {
          if ( graphicChildElem.localName() == "Mark" )
          {
            // check for a well known name
            QDomElement wellKnownNameElem = graphicChildElem.firstChildElement( "WellKnownName" );
            if ( !wellKnownNameElem.isNull() )
            {
              name = wellKnownNameElem.firstChild().nodeValue();
              found = true;
              break;
            }
          }

          if ( graphicChildElem.localName() == "ExternalGraphic" || graphicChildElem.localName() == "Mark" )
          {
            // check for external graphic format
            QDomElement formatElem = graphicChildElem.firstChildElement( "Format" );
            if ( formatElem.isNull() )
              continue;

            format = formatElem.firstChild().nodeValue();

            // TODO: remove this check when more formats will be supported
            // only SVG external graphics are supported in this moment
            if ( graphicChildElem.localName() == "ExternalGraphic" && format != "image/svg+xml" )
              continue;

            // TODO: remove this check when more formats will be supported
            // only ttf marks are supported in this moment
            if ( graphicChildElem.localName() == "Mark" && format != "ttf" )
              continue;

            // check for a valid content
            QDomElement onlineResourceElem = graphicChildElem.firstChildElement( "OnlineResource" );
            QDomElement inlineContentElem = graphicChildElem.firstChildElement( "InlineContent" );

            if ( !onlineResourceElem.isNull() )
            {
              name = onlineResourceElem.attributeNS( "http://www.w3.org/1999/xlink", "href" );

              if ( graphicChildElem.localName() == "Mark" && format == "ttf" )
              {
                // mark with ttf format may have a name like ttf://fontFamily
                if ( name.startsWith( "ttf://" ) )
                  name = name.mid( 6 );

                // mark with ttf format has a markIndex element
                QDomElement markIndexElem = graphicChildElem.firstChildElement( "MarkIndex" );
                if ( markIndexElem.isNull() )
                  continue;

                bool ok;
                int v = markIndexElem.firstChild().nodeValue().toInt( &ok );
                if ( !ok || v < 0 )
                  continue;

                markIndex = v;
              }

              found = true;
              break;
            }
#if 0
            else if ( !inlineContentElem.isNull() )
              continue; // TODO: not implemented yet
#endif
            else
              continue;
          }

          // if Mark element is present but it doesn't contains neither
          // WellKnownName nor OnlineResource nor InlineContent,
          // use the default mark (square)
          if ( graphicChildElem.localName() == "Mark" )
          {
            name = "square";
            found = true;
            break;
          }
        }

        // if found a valid Mark, check for its Fill and Stroke element
        if ( found && graphicChildElem.localName() == "Mark" )
        {
          // XXX: recursive definition!?! couldn't be dangerous???
          // to avoid recursion we handle only simple fill and simple stroke

          // check for simple fill
          // Fill element can contain some SvgParameter elements
          Qt::BrushStyle markFillStyle;

          QDomElement markFillElem = graphicChildElem.firstChildElement( "Fill" );
          if ( fillFromSld( markFillElem, markFillStyle, fillColor ) )
            validFill = true;

          // check for simple outline
          // Stroke element can contain some SvgParameter elements
          Qt::PenStyle borderStyle;
          double borderWidth = 1.0, dashOffset = 0.0;
          QVector<qreal> customDashPattern;

          QDomElement markStrokeElem = graphicChildElem.firstChildElement( "Stroke" );
          if ( lineFromSld( markStrokeElem, borderStyle, borderColor, borderWidth,
                            nullptr, nullptr, &customDashPattern, &dashOffset ) )
            validBorder = true;
        }

        if ( found )
        {
          // check for Opacity, Size, Rotation, AnchorPoint, Displacement
          QDomElement opacityElem = graphicElem.firstChildElement( "Opacity" );
          if ( !opacityElem.isNull() )
            fillColor.setAlpha( decodeSldAlpha( opacityElem.firstChild().nodeValue() ) );

          QDomElement sizeElem = graphicElem.firstChildElement( "Size" );
          if ( !sizeElem.isNull() )
          {
            bool ok;
            double v = sizeElem.firstChild().nodeValue().toDouble( &ok );
            if ( ok && v > 0 )
              size = v;
          }

          QString angleFunc;
          if ( rotationFromSldElement( graphicElem, angleFunc ) && !angleFunc.isEmpty() )
          {
            bool ok;
            double v = angleFunc.toDouble( &ok );
            if ( ok )
              angle = v;
          }

          displacementFromSldElement( graphicElem, offset );
        }
      }
    }

    if ( validFill || validBorder )
    {
      if ( format == "image/svg+xml" )
      {
        QgsStringMap map;
        map["name"] = name;
        map["fill"] = fillColor.name();
        map["outline"] = borderColor.name();
        map["outline-width"] = QString::number( borderWidth );
        if ( !qgsDoubleNear( size, 0.0 ) )
          map["size"] = QString::number( size );
        if ( !qgsDoubleNear( angle, 0.0 ) )
          map["angle"] = QString::number( angle );
        if ( !offset.isNull() )
          map["offset"] = encodePoint( offset );
        layers.append( QgsSymbolLayerRegistry::instance()->createSymbolLayer( "SvgMarker", map ) );
      }
      else if ( format == "ttf" )
      {
        QgsStringMap map;
        map["font"] = name;
        map["chr"] = markIndex;
        map["color"] = encodeColor( validFill ? fillColor : Qt::transparent );
        if ( size > 0 )
          map["size"] = QString::number( size );
        if ( !qgsDoubleNear( angle, 0.0 ) )
          map["angle"] = QString::number( angle );
        if ( !offset.isNull() )
          map["offset"] = encodePoint( offset );
        layers.append( QgsSymbolLayerRegistry::instance()->createSymbolLayer( "FontMarker", map ) );
      }
    }
  }

  if ( layers.isEmpty() )
    return false;

  layerList << layers;
  layers.clear();
  return true;
}

void QgsSymbolLayerUtils::fillToSld( QDomDocument &doc, QDomElement &element, Qt::BrushStyle brushStyle, const QColor& color )
{
  QString patternName;
  switch ( brushStyle )
  {
    case Qt::NoBrush:
      return;

    case Qt::SolidPattern:
      if ( color.isValid() )
      {
        element.appendChild( createSvgParameterElement( doc, "fill", color.name() ) );
        if ( color.alpha() < 255 )
          element.appendChild( createSvgParameterElement( doc, "fill-opacity", encodeSldAlpha( color.alpha() ) ) );
      }
      return;

    case Qt::CrossPattern:
    case Qt::DiagCrossPattern:
    case Qt::HorPattern:
    case Qt::VerPattern:
    case Qt::BDiagPattern:
    case Qt::FDiagPattern:
    case Qt::Dense1Pattern:
    case Qt::Dense2Pattern:
    case Qt::Dense3Pattern:
    case Qt::Dense4Pattern:
    case Qt::Dense5Pattern:
    case Qt::Dense6Pattern:
    case Qt::Dense7Pattern:
      patternName = encodeSldBrushStyle( brushStyle );
      break;

    default:
      element.appendChild( doc.createComment( QString( "Qt::BrushStyle '%1'' not supported yet" ).arg( brushStyle ) ) );
      return;
  }

  QDomElement graphicFillElem = doc.createElement( "se:GraphicFill" );
  element.appendChild( graphicFillElem );

  QDomElement graphicElem = doc.createElement( "se:Graphic" );
  graphicFillElem.appendChild( graphicElem );

  QColor fillColor = patternName.startsWith( "brush://" ) ? color : QColor();
  QColor borderColor = !patternName.startsWith( "brush://" ) ? color : QColor();

  /* Use WellKnownName tag to handle QT brush styles. */
  wellKnownMarkerToSld( doc, graphicElem, patternName, fillColor, borderColor, Qt::SolidLine, -1, -1 );
}

bool QgsSymbolLayerUtils::fillFromSld( QDomElement &element, Qt::BrushStyle &brushStyle, QColor &color )
{
  QgsDebugMsg( "Entered." );

  brushStyle = Qt::SolidPattern;
  color = QColor( "#808080" );

  if ( element.isNull() )
  {
    brushStyle = Qt::NoBrush;
    color = QColor();
    return true;
  }

  QDomElement graphicFillElem = element.firstChildElement( "GraphicFill" );
  // if no GraphicFill element is found, it's a solid fill
  if ( graphicFillElem.isNull() )
  {
    QgsStringMap svgParams = getSvgParameterList( element );
    for ( QgsStringMap::iterator it = svgParams.begin(); it != svgParams.end(); ++it )
    {
      QgsDebugMsg( QString( "found SvgParameter %1: %2" ).arg( it.key(), it.value() ) );

      if ( it.key() == "fill" )
        color = QColor( it.value() );
      else if ( it.key() == "fill-opacity" )
        color.setAlpha( decodeSldAlpha( it.value() ) );
    }
  }
  else  // wellKnown marker
  {
    QDomElement graphicElem = graphicFillElem.firstChildElement( "Graphic" );
    if ( graphicElem.isNull() )
      return false; // Graphic is required within GraphicFill

    QString patternName = "square";
    QColor fillColor, borderColor;
    double borderWidth, size;
    Qt::PenStyle borderStyle;
    if ( !wellKnownMarkerFromSld( graphicElem, patternName, fillColor, borderColor, borderStyle, borderWidth, size ) )
      return false;

    brushStyle = decodeSldBrushStyle( patternName );
    if ( brushStyle == Qt::NoBrush )
      return false; // unable to decode brush style

    QColor c = patternName.startsWith( "brush://" ) ? fillColor : borderColor;
    if ( c.isValid() )
      color = c;
  }

  return true;
}

void QgsSymbolLayerUtils::lineToSld( QDomDocument &doc, QDomElement &element,
                                     Qt::PenStyle penStyle, const QColor& color, double width,
                                     const Qt::PenJoinStyle *penJoinStyle, const Qt::PenCapStyle *penCapStyle,
                                     const QVector<qreal> *customDashPattern, double dashOffset )
{
  QVector<qreal> dashPattern;
  const QVector<qreal> *pattern = &dashPattern;

  if ( penStyle == Qt::CustomDashLine && !customDashPattern )
  {
    element.appendChild( doc.createComment( "WARNING: Custom dash pattern required but not provided. Using default dash pattern." ) );
    penStyle = Qt::DashLine;
  }

  switch ( penStyle )
  {
    case Qt::NoPen:
      return;

    case Qt::SolidLine:
      break;

    case Qt::DashLine:
      dashPattern.push_back( 4.0 );
      dashPattern.push_back( 2.0 );
      break;
    case Qt::DotLine:
      dashPattern.push_back( 1.0 );
      dashPattern.push_back( 2.0 );
      break;
    case Qt::DashDotLine:
      dashPattern.push_back( 4.0 );
      dashPattern.push_back( 2.0 );
      dashPattern.push_back( 1.0 );
      dashPattern.push_back( 2.0 );
      break;
    case Qt::DashDotDotLine:
      dashPattern.push_back( 4.0 );
      dashPattern.push_back( 2.0 );
      dashPattern.push_back( 1.0 );
      dashPattern.push_back( 2.0 );
      dashPattern.push_back( 1.0 );
      dashPattern.push_back( 2.0 );
      break;

    case Qt::CustomDashLine:
      Q_ASSERT( customDashPattern );
      pattern = customDashPattern;
      break;

    default:
      element.appendChild( doc.createComment( QString( "Qt::BrushStyle '%1'' not supported yet" ).arg( penStyle ) ) );
      return;
  }

  if ( color.isValid() )
  {
    element.appendChild( createSvgParameterElement( doc, "stroke", color.name() ) );
    if ( color.alpha() < 255 )
      element.appendChild( createSvgParameterElement( doc, "stroke-opacity", encodeSldAlpha( color.alpha() ) ) );
  }
  if ( width > 0 )
    element.appendChild( createSvgParameterElement( doc, "stroke-width", qgsDoubleToString( width ) ) );
  if ( penJoinStyle )
    element.appendChild( createSvgParameterElement( doc, "stroke-linejoin", encodeSldLineJoinStyle( *penJoinStyle ) ) );
  if ( penCapStyle )
    element.appendChild( createSvgParameterElement( doc, "stroke-linecap", encodeSldLineCapStyle( *penCapStyle ) ) );

  if ( !pattern->isEmpty() )
  {
    element.appendChild( createSvgParameterElement( doc, "stroke-dasharray", encodeSldRealVector( *pattern ) ) );
    if ( !qgsDoubleNear( dashOffset, 0.0 ) )
      element.appendChild( createSvgParameterElement( doc, "stroke-dashoffset", qgsDoubleToString( dashOffset ) ) );
  }
}


bool QgsSymbolLayerUtils::lineFromSld( QDomElement &element,
                                       Qt::PenStyle &penStyle, QColor &color, double &width,
                                       Qt::PenJoinStyle *penJoinStyle, Qt::PenCapStyle *penCapStyle,
                                       QVector<qreal> *customDashPattern, double *dashOffset )
{
  QgsDebugMsg( "Entered." );

  penStyle = Qt::SolidLine;
  color = QColor( "#000000" );
  width = 1;
  if ( penJoinStyle )
    *penJoinStyle = Qt::BevelJoin;
  if ( penCapStyle )
    *penCapStyle = Qt::SquareCap;
  if ( customDashPattern )
    customDashPattern->clear();
  if ( dashOffset )
    *dashOffset = 0;

  if ( element.isNull() )
  {
    penStyle = Qt::NoPen;
    color = QColor();
    return true;
  }

  QgsStringMap svgParams = getSvgParameterList( element );
  for ( QgsStringMap::iterator it = svgParams.begin(); it != svgParams.end(); ++it )
  {
    QgsDebugMsg( QString( "found SvgParameter %1: %2" ).arg( it.key(), it.value() ) );

    if ( it.key() == "stroke" )
    {
      color = QColor( it.value() );
    }
    else if ( it.key() == "stroke-opacity" )
    {
      color.setAlpha( decodeSldAlpha( it.value() ) );
    }
    else if ( it.key() == "stroke-width" )
    {
      bool ok;
      double w = it.value().toDouble( &ok );
      if ( ok )
        width = w;
    }
    else if ( it.key() == "stroke-linejoin" && penJoinStyle )
    {
      *penJoinStyle = decodeSldLineJoinStyle( it.value() );
    }
    else if ( it.key() == "stroke-linecap" && penCapStyle )
    {
      *penCapStyle = decodeSldLineCapStyle( it.value() );
    }
    else if ( it.key() == "stroke-dasharray" )
    {
      QVector<qreal> dashPattern = decodeSldRealVector( it.value() );
      if ( !dashPattern.isEmpty() )
      {
        // convert the dasharray to one of the QT pen style,
        // if no match is found then set pen style to CustomDashLine
        bool dashPatternFound = false;

        if ( dashPattern.count() == 2 )
        {
          if ( dashPattern.at( 0 ) == 4.0 &&
               dashPattern.at( 1 ) == 2.0 )
          {
            penStyle = Qt::DashLine;
            dashPatternFound = true;
          }
          else if ( dashPattern.at( 0 ) == 1.0 &&
                    dashPattern.at( 1 ) == 2.0 )
          {
            penStyle = Qt::DotLine;
            dashPatternFound = true;
          }
        }
        else if ( dashPattern.count() == 4 )
        {
          if ( dashPattern.at( 0 ) == 4.0 &&
               dashPattern.at( 1 ) == 2.0 &&
               dashPattern.at( 2 ) == 1.0 &&
               dashPattern.at( 3 ) == 2.0 )
          {
            penStyle = Qt::DashDotLine;
            dashPatternFound = true;
          }
        }
        else if ( dashPattern.count() == 6 )
        {
          if ( dashPattern.at( 0 ) == 4.0 &&
               dashPattern.at( 1 ) == 2.0 &&
               dashPattern.at( 2 ) == 1.0 &&
               dashPattern.at( 3 ) == 2.0 &&
               dashPattern.at( 4 ) == 1.0 &&
               dashPattern.at( 5 ) == 2.0 )
          {
            penStyle = Qt::DashDotDotLine;
            dashPatternFound = true;
          }
        }

        // default case: set pen style to CustomDashLine
        if ( !dashPatternFound )
        {
          if ( customDashPattern )
          {
            penStyle = Qt::CustomDashLine;
            *customDashPattern = dashPattern;
          }
          else
          {
            QgsDebugMsg( "custom dash pattern required but not provided. Using default dash pattern." );
            penStyle = Qt::DashLine;
          }
        }
      }
    }
    else if ( it.key() == "stroke-dashoffset" && dashOffset )
    {
      bool ok;
      double d = it.value().toDouble( &ok );
      if ( ok )
        *dashOffset = d;
    }
  }

  return true;
}

void QgsSymbolLayerUtils::externalGraphicToSld( QDomDocument &doc, QDomElement &element,
    const QString& path, const QString& mime,
    const QColor& color, double size )
{
  QDomElement externalGraphicElem = doc.createElement( "se:ExternalGraphic" );
  element.appendChild( externalGraphicElem );

  createOnlineResourceElement( doc, externalGraphicElem, path, mime );

  //TODO: missing a way to handle svg color. Should use <se:ColorReplacement>
  Q_UNUSED( color );

  if ( size >= 0 )
  {
    QDomElement sizeElem = doc.createElement( "se:Size" );
    sizeElem.appendChild( doc.createTextNode( qgsDoubleToString( size ) ) );
    element.appendChild( sizeElem );
  }
}

bool QgsSymbolLayerUtils::externalGraphicFromSld( QDomElement &element,
    QString &path, QString &mime,
    QColor &color, double &size )
{
  QgsDebugMsg( "Entered." );
  Q_UNUSED( color );

  QDomElement externalGraphicElem = element.firstChildElement( "ExternalGraphic" );
  if ( externalGraphicElem.isNull() )
    return false;

  onlineResourceFromSldElement( externalGraphicElem, path, mime );

  QDomElement sizeElem = element.firstChildElement( "Size" );
  if ( !sizeElem.isNull() )
  {
    bool ok;
    double s = sizeElem.firstChild().nodeValue().toDouble( &ok );
    if ( ok )
      size = s;
  }

  return true;
}

void QgsSymbolLayerUtils::externalMarkerToSld( QDomDocument &doc, QDomElement &element,
    const QString& path, const QString& format, int *markIndex,
    const QColor& color, double size )
{
  QDomElement markElem = doc.createElement( "se:Mark" );
  element.appendChild( markElem );

  createOnlineResourceElement( doc, markElem, path, format );

  if ( markIndex )
  {
    QDomElement markIndexElem = doc.createElement( "se:MarkIndex" );
    markIndexElem.appendChild( doc.createTextNode( QString::number( *markIndex ) ) );
    markElem.appendChild( markIndexElem );
  }

  // <Fill>
  QDomElement fillElem = doc.createElement( "se:Fill" );
  fillToSld( doc, fillElem, Qt::SolidPattern, color );
  markElem.appendChild( fillElem );

  // <Size>
  if ( !qgsDoubleNear( size, 0.0 ) && size > 0 )
  {
    QDomElement sizeElem = doc.createElement( "se:Size" );
    sizeElem.appendChild( doc.createTextNode( qgsDoubleToString( size ) ) );
    element.appendChild( sizeElem );
  }
}

bool QgsSymbolLayerUtils::externalMarkerFromSld( QDomElement &element,
    QString &path, QString &format, int &markIndex,
    QColor &color, double &size )
{
  QgsDebugMsg( "Entered." );

  color = QColor();
  markIndex = -1;
  size = -1;

  QDomElement markElem = element.firstChildElement( "Mark" );
  if ( markElem.isNull() )
    return false;

  onlineResourceFromSldElement( markElem, path, format );

  QDomElement markIndexElem = markElem.firstChildElement( "MarkIndex" );
  if ( !markIndexElem.isNull() )
  {
    bool ok;
    int i = markIndexElem.firstChild().nodeValue().toInt( &ok );
    if ( ok )
      markIndex = i;
  }

  // <Fill>
  QDomElement fillElem = markElem.firstChildElement( "Fill" );
  Qt::BrushStyle b = Qt::SolidPattern;
  fillFromSld( fillElem, b, color );
  // ignore brush style, solid expected

  // <Size>
  QDomElement sizeElem = element.firstChildElement( "Size" );
  if ( !sizeElem.isNull() )
  {
    bool ok;
    double s = sizeElem.firstChild().nodeValue().toDouble( &ok );
    if ( ok )
      size = s;
  }

  return true;
}

void QgsSymbolLayerUtils::wellKnownMarkerToSld( QDomDocument &doc, QDomElement &element,
    const QString& name, const QColor& color, const QColor& borderColor,
    double borderWidth, double size )
{
  wellKnownMarkerToSld( doc, element, name, color, borderColor, Qt::SolidLine, borderWidth, size );
}

void QgsSymbolLayerUtils::wellKnownMarkerToSld( QDomDocument &doc, QDomElement &element,
    const QString& name, const QColor& color, const QColor& borderColor, Qt::PenStyle borderStyle,
    double borderWidth, double size )
{
  QDomElement markElem = doc.createElement( "se:Mark" );
  element.appendChild( markElem );

  QDomElement wellKnownNameElem = doc.createElement( "se:WellKnownName" );
  wellKnownNameElem.appendChild( doc.createTextNode( name ) );
  markElem.appendChild( wellKnownNameElem );

  // <Fill>
  if ( color.isValid() )
  {
    QDomElement fillElem = doc.createElement( "se:Fill" );
    fillToSld( doc, fillElem, Qt::SolidPattern, color );
    markElem.appendChild( fillElem );
  }

  // <Stroke>
  if ( borderColor.isValid() )
  {
    QDomElement strokeElem = doc.createElement( "se:Stroke" );
    lineToSld( doc, strokeElem, borderStyle, borderColor, borderWidth );
    markElem.appendChild( strokeElem );
  }

  // <Size>
  if ( !qgsDoubleNear( size, 0.0 ) && size > 0 )
  {
    QDomElement sizeElem = doc.createElement( "se:Size" );
    sizeElem.appendChild( doc.createTextNode( qgsDoubleToString( size ) ) );
    element.appendChild( sizeElem );
  }
}

bool QgsSymbolLayerUtils::wellKnownMarkerFromSld( QDomElement &element,
    QString &name, QColor &color, QColor &borderColor,
    double &borderWidth, double &size )
{
  Qt::PenStyle borderStyle;
  return wellKnownMarkerFromSld( element, name, color, borderColor, borderStyle, borderWidth, size );
}

bool QgsSymbolLayerUtils::wellKnownMarkerFromSld( QDomElement &element,
    QString &name, QColor &color, QColor &borderColor, Qt::PenStyle &borderStyle,
    double &borderWidth, double &size )
{
  QgsDebugMsg( "Entered." );

  name = "square";
  color = QColor();
  borderColor = QColor( "#000000" );
  borderWidth = 1;
  size = 6;

  QDomElement markElem = element.firstChildElement( "Mark" );
  if ( markElem.isNull() )
    return false;

  QDomElement wellKnownNameElem = markElem.firstChildElement( "WellKnownName" );
  if ( !wellKnownNameElem.isNull() )
  {
    name = wellKnownNameElem.firstChild().nodeValue();
    QgsDebugMsg( "found Mark with well known name: " + name );
  }

  // <Fill>
  QDomElement fillElem = markElem.firstChildElement( "Fill" );
  Qt::BrushStyle b = Qt::SolidPattern;
  fillFromSld( fillElem, b, color );
  // ignore brush style, solid expected

  // <Stroke>
  QDomElement strokeElem = markElem.firstChildElement( "Stroke" );
  lineFromSld( strokeElem, borderStyle, borderColor, borderWidth );
  // ignore border style, solid expected

  // <Size>
  QDomElement sizeElem = element.firstChildElement( "Size" );
  if ( !sizeElem.isNull() )
  {
    bool ok;
    double s = sizeElem.firstChild().nodeValue().toDouble( &ok );
    if ( ok )
      size = s;
  }

  return true;
}

void QgsSymbolLayerUtils::createRotationElement( QDomDocument &doc, QDomElement &element, const QString& rotationFunc )
{
  if ( !rotationFunc.isEmpty() )
  {
    QDomElement rotationElem = doc.createElement( "se:Rotation" );
    createExpressionElement( doc, rotationElem, rotationFunc );
    element.appendChild( rotationElem );
  }
}

bool QgsSymbolLayerUtils::rotationFromSldElement( QDomElement &element, QString &rotationFunc )
{
  QDomElement rotationElem = element.firstChildElement( "Rotation" );
  if ( !rotationElem.isNull() )
  {
    return functionFromSldElement( rotationElem, rotationFunc );
  }
  return true;
}


void QgsSymbolLayerUtils::createOpacityElement( QDomDocument &doc, QDomElement &element, const QString& alphaFunc )
{
  if ( !alphaFunc.isEmpty() )
  {
    QDomElement opacityElem = doc.createElement( "se:Opacity" );
    createExpressionElement( doc, opacityElem, alphaFunc );
    element.appendChild( opacityElem );
  }
}

bool QgsSymbolLayerUtils::opacityFromSldElement( QDomElement &element, QString &alphaFunc )
{
  QDomElement opacityElem = element.firstChildElement( "Opacity" );
  if ( !opacityElem.isNull() )
  {
    return functionFromSldElement( opacityElem, alphaFunc );
  }
  return true;
}

void QgsSymbolLayerUtils::createDisplacementElement( QDomDocument &doc, QDomElement &element, QPointF offset )
{
  if ( offset.isNull() )
    return;

  QDomElement displacementElem = doc.createElement( "se:Displacement" );
  element.appendChild( displacementElem );

  QDomElement dispXElem = doc.createElement( "se:DisplacementX" );
  dispXElem.appendChild( doc.createTextNode( qgsDoubleToString( offset.x() ) ) );

  QDomElement dispYElem = doc.createElement( "se:DisplacementY" );
  dispYElem.appendChild( doc.createTextNode( qgsDoubleToString( offset.y() ) ) );

  displacementElem.appendChild( dispXElem );
  displacementElem.appendChild( dispYElem );
}

bool QgsSymbolLayerUtils::displacementFromSldElement( QDomElement &element, QPointF &offset )
{
  offset = QPointF( 0, 0 );

  QDomElement displacementElem = element.firstChildElement( "Displacement" );
  if ( displacementElem.isNull() )
    return true;

  QDomElement dispXElem = displacementElem.firstChildElement( "DisplacementX" );
  if ( !dispXElem.isNull() )
  {
    bool ok;
    double offsetX = dispXElem.firstChild().nodeValue().toDouble( &ok );
    if ( ok )
      offset.setX( offsetX );
  }

  QDomElement dispYElem = displacementElem.firstChildElement( "DisplacementY" );
  if ( !dispYElem.isNull() )
  {
    bool ok;
    double offsetY = dispYElem.firstChild().nodeValue().toDouble( &ok );
    if ( ok )
      offset.setY( offsetY );
  }

  return true;
}

void QgsSymbolLayerUtils::labelTextToSld( QDomDocument &doc, QDomElement &element,
    const QString& label, const QFont& font,
    const QColor& color, double size )
{
  QDomElement labelElem = doc.createElement( "se:Label" );
  labelElem.appendChild( doc.createTextNode( label ) );
  element.appendChild( labelElem );

  QDomElement fontElem = doc.createElement( "se:Font" );
  element.appendChild( fontElem );

  fontElem.appendChild( createSvgParameterElement( doc, "font-family", font.family() ) );
#if 0
  fontElem.appendChild( createSldParameterElement( doc, "font-style", encodeSldFontStyle( font.style() ) ) );
  fontElem.appendChild( createSldParameterElement( doc, "font-weight", encodeSldFontWeight( font.weight() ) ) );
#endif
  fontElem.appendChild( createSvgParameterElement( doc, "font-size", QString::number( size ) ) );

  // <Fill>
  if ( color.isValid() )
  {
    QDomElement fillElem = doc.createElement( "Fill" );
    fillToSld( doc, fillElem, Qt::SolidPattern, color );
    element.appendChild( fillElem );
  }
}

QString QgsSymbolLayerUtils::ogrFeatureStylePen( double width, double mmScaleFactor, double mapUnitScaleFactor, const QColor& c,
    Qt::PenJoinStyle joinStyle,
    Qt::PenCapStyle capStyle,
    double offset,
    const QVector<qreal>* dashPattern )
{
  QString penStyle;
  penStyle.append( "PEN(" );
  penStyle.append( "c:" );
  penStyle.append( c.name() );
  penStyle.append( ",w:" );
  //dxf driver writes ground units as mm? Should probably be changed in ogr
  penStyle.append( QString::number( width * mmScaleFactor ) );
  penStyle.append( "mm" );

  //dash dot vector
  if ( dashPattern && !dashPattern->isEmpty() )
  {
    penStyle.append( ",p:\"" );
    QVector<qreal>::const_iterator pIt = dashPattern->constBegin();
    for ( ; pIt != dashPattern->constEnd(); ++pIt )
    {
      if ( pIt != dashPattern->constBegin() )
      {
        penStyle.append( ' ' );
      }
      penStyle.append( QString::number( *pIt * mapUnitScaleFactor ) );
      penStyle.append( 'g' );
    }
    penStyle.append( '\"' );
  }

  //cap
  penStyle.append( ",cap:" );
  switch ( capStyle )
  {
    case Qt::SquareCap:
      penStyle.append( 'p' );
      break;
    case Qt::RoundCap:
      penStyle.append( 'r' );
      break;
    case Qt::FlatCap:
    default:
      penStyle.append( 'b' );
  }

  //join
  penStyle.append( ",j:" );
  switch ( joinStyle )
  {
    case Qt::BevelJoin:
      penStyle.append( 'b' );
      break;
    case Qt::RoundJoin:
      penStyle.append( 'r' );
      break;
    case Qt::MiterJoin:
    default:
      penStyle.append( 'm' );
  }

  //offset
  if ( !qgsDoubleNear( offset, 0.0 ) )
  {
    penStyle.append( ",dp:" );
    penStyle.append( QString::number( offset * mapUnitScaleFactor ) );
    penStyle.append( 'g' );
  }

  penStyle.append( ')' );
  return penStyle;
}

QString QgsSymbolLayerUtils::ogrFeatureStyleBrush( const QColor& fillColor )
{
  QString brushStyle;
  brushStyle.append( "BRUSH(" );
  brushStyle.append( "fc:" );
  brushStyle.append( fillColor.name() );
  brushStyle.append( ')' );
  return brushStyle;
}

void QgsSymbolLayerUtils::createGeometryElement( QDomDocument &doc, QDomElement &element, const QString& geomFunc )
{
  if ( geomFunc.isEmpty() )
    return;

  QDomElement geometryElem = doc.createElement( "Geometry" );
  element.appendChild( geometryElem );

  /* About using a function withing the Geometry tag.
   *
   * The SLD specification <= 1.1 is vague:
   * "In principle, a fixed geometry could be defined using GML or
   * operators could be defined for computing the geometry from
   * references or literals. However, using a feature property directly
   * is by far the most commonly useful method."
   *
   * Even if it seems that specs should take care all the possible cases,
   * looking at the XML schema fragment that encodes the Geometry element,
   * it has to be a PropertyName element:
   *   <xsd:element name="Geometry">
   *       <xsd:complexType>
   *           <xsd:sequence>
   *               <xsd:element ref="ogc:PropertyName"/>
   *           </xsd:sequence>
   *       </xsd:complexType>
   *   </xsd:element>
   *
   * Anyway we will use a ogc:Function to handle geometry transformations
   * like offset, centroid, ...
   */

  createExpressionElement( doc, geometryElem, geomFunc );
}

bool QgsSymbolLayerUtils::geometryFromSldElement( QDomElement &element, QString &geomFunc )
{
  QDomElement geometryElem = element.firstChildElement( "Geometry" );
  if ( geometryElem.isNull() )
    return true;

  return functionFromSldElement( geometryElem, geomFunc );
}

bool QgsSymbolLayerUtils::createExpressionElement( QDomDocument &doc, QDomElement &element, const QString& function )
{
  // let's use QgsExpression to generate the SLD for the function
  QgsExpression expr( function );
  if ( expr.hasParserError() )
  {
    element.appendChild( doc.createComment( "Parser Error: " + expr.parserErrorString() + " - Expression was: " + function ) );
    return false;
  }
  QDomElement filterElem = QgsOgcUtils::expressionToOgcExpression( expr, doc );
  if ( !filterElem.isNull() )
    element.appendChild( filterElem );
  return true;
}


bool QgsSymbolLayerUtils::createFunctionElement( QDomDocument &doc, QDomElement &element, const QString& function )
{
  // let's use QgsExpression to generate the SLD for the function
  QgsExpression expr( function );
  if ( expr.hasParserError() )
  {
    element.appendChild( doc.createComment( "Parser Error: " + expr.parserErrorString() + " - Expression was: " + function ) );
    return false;
  }
  QDomElement filterElem = QgsOgcUtils::expressionToOgcFilter( expr, doc );
  if ( !filterElem.isNull() )
    element.appendChild( filterElem );
  return true;
}

bool QgsSymbolLayerUtils::functionFromSldElement( QDomElement &element, QString &function )
{
  QDomElement elem = element;
  if ( element.tagName() != "Filter" )
  {
    QDomNodeList filterNodes = element.elementsByTagName( "Filter" );
    if ( !filterNodes.isEmpty() )
    {
      elem = filterNodes.at( 0 ).toElement();
    }
  }

  if ( elem.isNull() )
  {
    return false;
  }


  QgsExpression *expr = QgsOgcUtils::expressionFromOgcFilter( elem );
  if ( !expr )
    return false;

  bool valid = !expr->hasParserError();
  if ( !valid )
  {
    QgsDebugMsg( "parser error: " + expr->parserErrorString() );
  }
  else
  {
    function = expr->expression();
  }

  delete expr;
  return valid;
}

void QgsSymbolLayerUtils::createOnlineResourceElement( QDomDocument &doc, QDomElement &element,
    const QString& path, const QString& format )
{
  // get resource url or relative path
  QString url = symbolPathToName( path );
  QDomElement onlineResourceElem = doc.createElement( "se:OnlineResource" );
  onlineResourceElem.setAttribute( "xlink:type", "simple" );
  onlineResourceElem.setAttribute( "xlink:href", url );
  element.appendChild( onlineResourceElem );

  QDomElement formatElem = doc.createElement( "se:Format" );
  formatElem.appendChild( doc.createTextNode( format ) );
  element.appendChild( formatElem );
}

bool QgsSymbolLayerUtils::onlineResourceFromSldElement( QDomElement &element, QString &path, QString &format )
{
  QgsDebugMsg( "Entered." );

  QDomElement onlineResourceElem = element.firstChildElement( "OnlineResource" );
  if ( onlineResourceElem.isNull() )
    return false;

  path = onlineResourceElem.attributeNS( "http://www.w3.org/1999/xlink", "href" );

  QDomElement formatElem = element.firstChildElement( "Format" );
  if ( formatElem.isNull() )
    return false; // OnlineResource requires a Format sibling element

  format = formatElem.firstChild().nodeValue();
  return true;
}


QDomElement QgsSymbolLayerUtils::createSvgParameterElement( QDomDocument &doc, const QString& name, const QString& value )
{
  QDomElement nodeElem = doc.createElement( "se:SvgParameter" );
  nodeElem.setAttribute( "name", name );
  nodeElem.appendChild( doc.createTextNode( value ) );
  return nodeElem;
}

QgsStringMap QgsSymbolLayerUtils::getSvgParameterList( QDomElement &element )
{
  QgsStringMap params;

  QDomElement paramElem = element.firstChildElement();
  while ( !paramElem.isNull() )
  {
    if ( paramElem.localName() == "SvgParameter" || paramElem.localName() == "CssParameter" )
    {
      QString name = paramElem.attribute( "name" );
      QString value = paramElem.firstChild().nodeValue();

      if ( !name.isEmpty() && !value.isEmpty() )
        params[ name ] = value;
    }

    paramElem = paramElem.nextSiblingElement();
  }

  return params;
}

QDomElement QgsSymbolLayerUtils::createVendorOptionElement( QDomDocument &doc, const QString& name, const QString& value )
{
  QDomElement nodeElem = doc.createElement( "VendorOption" );
  nodeElem.setAttribute( "name", name );
  nodeElem.appendChild( doc.createTextNode( value ) );
  return nodeElem;
}

QgsStringMap QgsSymbolLayerUtils::getVendorOptionList( QDomElement &element )
{
  QgsStringMap params;

  QDomElement paramElem = element.firstChildElement( "VendorOption" );
  while ( !paramElem.isNull() )
  {
    QString name = paramElem.attribute( "name" );
    QString value = paramElem.firstChild().nodeValue();

    if ( !name.isEmpty() && !value.isEmpty() )
      params[ name ] = value;

    paramElem = paramElem.nextSiblingElement( "VendorOption" );
  }

  return params;
}


QgsStringMap QgsSymbolLayerUtils::parseProperties( QDomElement& element )
{
  QgsStringMap props;
  QDomElement e = element.firstChildElement();
  while ( !e.isNull() )
  {
    if ( e.tagName() != "prop" )
    {
      QgsDebugMsg( "unknown tag " + e.tagName() );
    }
    else
    {
      QString propKey = e.attribute( "k" );
      QString propValue = e.attribute( "v" );
      props[propKey] = propValue;
    }
    e = e.nextSiblingElement();
  }
  return props;
}


void QgsSymbolLayerUtils::saveProperties( QgsStringMap props, QDomDocument& doc, QDomElement& element )
{
  for ( QgsStringMap::iterator it = props.begin(); it != props.end(); ++it )
  {
    QDomElement propEl = doc.createElement( "prop" );
    propEl.setAttribute( "k", it.key() );
    propEl.setAttribute( "v", it.value() );
    element.appendChild( propEl );
  }
}

QgsSymbolMap QgsSymbolLayerUtils::loadSymbols( QDomElement& element )
{
  // go through symbols one-by-one and load them

  QgsSymbolMap symbols;
  QDomElement e = element.firstChildElement();

  while ( !e.isNull() )
  {
    if ( e.tagName() == "symbol" )
    {
      QgsSymbol* symbol = QgsSymbolLayerUtils::loadSymbol( e );
      if ( symbol )
        symbols.insert( e.attribute( "name" ), symbol );
    }
    else
    {
      QgsDebugMsg( "unknown tag: " + e.tagName() );
    }
    e = e.nextSiblingElement();
  }


  // now walk through the list of symbols and find those prefixed with @
  // these symbols are sub-symbols of some other symbol layers
  // e.g. symbol named "@foo@1" is sub-symbol of layer 1 in symbol "foo"
  QStringList subsymbols;

  for ( QMap<QString, QgsSymbol*>::iterator it = symbols.begin(); it != symbols.end(); ++it )
  {
    if ( it.key()[0] != '@' )
      continue;

    // add to array (for deletion)
    subsymbols.append( it.key() );

    QStringList parts = it.key().split( '@' );
    if ( parts.count() < 3 )
    {
      QgsDebugMsg( "found subsymbol with invalid name: " + it.key() );
      delete it.value(); // we must delete it
      continue; // some invalid syntax
    }
    QString symname = parts[1];
    int symlayer = parts[2].toInt();

    if ( !symbols.contains( symname ) )
    {
      QgsDebugMsg( "subsymbol references invalid symbol: " + symname );
      delete it.value(); // we must delete it
      continue;
    }

    QgsSymbol* sym = symbols[symname];
    if ( symlayer < 0 || symlayer >= sym->symbolLayerCount() )
    {
      QgsDebugMsg( "subsymbol references invalid symbol layer: " + QString::number( symlayer ) );
      delete it.value(); // we must delete it
      continue;
    }

    // set subsymbol takes ownership
    bool res = sym->symbolLayer( symlayer )->setSubSymbol( it.value() );
    if ( !res )
    {
      QgsDebugMsg( "symbol layer refused subsymbol: " + it.key() );
    }


  }

  // now safely remove sub-symbol entries (they have been already deleted or the ownership was taken away)
  for ( int i = 0; i < subsymbols.count(); i++ )
    symbols.take( subsymbols[i] );

  return symbols;
}

QDomElement QgsSymbolLayerUtils::saveSymbols( QgsSymbolMap& symbols, const QString& tagName, QDomDocument& doc )
{
  QDomElement symbolsElem = doc.createElement( tagName );

  // save symbols
  for ( QMap<QString, QgsSymbol*>::iterator its = symbols.begin(); its != symbols.end(); ++its )
  {
    QDomElement symEl = saveSymbol( its.key(), its.value(), doc );
    symbolsElem.appendChild( symEl );
  }

  return symbolsElem;
}

void QgsSymbolLayerUtils::clearSymbolMap( QgsSymbolMap& symbols )
{
  qDeleteAll( symbols );
  symbols.clear();
}


QgsColorRamp* QgsSymbolLayerUtils::loadColorRamp( QDomElement& element )
{
  QString rampType = element.attribute( "type" );

  // parse properties
  QgsStringMap props = QgsSymbolLayerUtils::parseProperties( element );

  if ( rampType == "gradient" )
    return QgsGradientColorRamp::create( props );
  else if ( rampType == "random" )
    return QgsLimitedRandomColorRamp::create( props );
  else if ( rampType == "colorbrewer" )
    return QgsColorBrewerColorRamp::create( props );
  else if ( rampType == "cpt-city" )
    return QgsCptCityColorRamp::create( props );
  else
  {
    QgsDebugMsg( "unknown colorramp type " + rampType );
    return nullptr;
  }
}


QDomElement QgsSymbolLayerUtils::saveColorRamp( const QString& name, QgsColorRamp* ramp, QDomDocument& doc )
{
  QDomElement rampEl = doc.createElement( "colorramp" );
  rampEl.setAttribute( "type", ramp->type() );
  rampEl.setAttribute( "name", name );

  QgsSymbolLayerUtils::saveProperties( ramp->properties(), doc, rampEl );
  return rampEl;
}

QString QgsSymbolLayerUtils::colorToName( const QColor &color )
{
  if ( !color.isValid() )
  {
    return QString();
  }

  //TODO - utilise a color names database (such as X11) to return nicer names
  //for now, just return hex codes
  return color.name();
}

QList<QColor> QgsSymbolLayerUtils::parseColorList( const QString& colorStr )
{
  QList<QColor> colors;

  //try splitting string at commas, spaces or newlines
  QStringList components = colorStr.simplified().split( QRegExp( "(,|\\s)" ) );
  QStringList::iterator it = components.begin();
  for ( ; it != components.end(); ++it )
  {
    QColor result = parseColor( *it, true );
    if ( result.isValid() )
    {
      colors << result;
    }
  }
  if ( colors.length() > 0 )
  {
    return colors;
  }

  //try splitting string at commas or newlines
  components = colorStr.split( QRegExp( "(,|\n)" ) );
  it = components.begin();
  for ( ; it != components.end(); ++it )
  {
    QColor result = parseColor( *it, true );
    if ( result.isValid() )
    {
      colors << result;
    }
  }
  if ( colors.length() > 0 )
  {
    return colors;
  }

  //try splitting string at whitespace or newlines
  components = colorStr.simplified().split( QString( ' ' ) );
  it = components.begin();
  for ( ; it != components.end(); ++it )
  {
    QColor result = parseColor( *it, true );
    if ( result.isValid() )
    {
      colors << result;
    }
  }
  if ( colors.length() > 0 )
  {
    return colors;
  }

  //try splitting string just at newlines
  components = colorStr.split( '\n' );
  it = components.begin();
  for ( ; it != components.end(); ++it )
  {
    QColor result = parseColor( *it, true );
    if ( result.isValid() )
    {
      colors << result;
    }
  }

  return colors;
}

QMimeData * QgsSymbolLayerUtils::colorToMimeData( const QColor &color )
{
  //set both the mime color data (which includes alpha channel), and the text (which is the color's hex
  //value, and can be used when pasting colors outside of QGIS).
  QMimeData *mimeData = new QMimeData;
  mimeData->setColorData( QVariant( color ) );
  mimeData->setText( color.name() );
  return mimeData;
}

QColor QgsSymbolLayerUtils::colorFromMimeData( const QMimeData * mimeData, bool& hasAlpha )
{
  //attempt to read color data directly from mime
  QColor mimeColor = mimeData->colorData().value<QColor>();
  if ( mimeColor.isValid() )
  {
    hasAlpha = true;
    return mimeColor;
  }

  //attempt to intrepret a color from mime text data
  hasAlpha = false;
  QColor textColor = QgsSymbolLayerUtils::parseColorWithAlpha( mimeData->text(), hasAlpha );
  if ( textColor.isValid() )
  {
    return textColor;
  }

  //could not get color from mime data
  return QColor();
}

QgsNamedColorList QgsSymbolLayerUtils::colorListFromMimeData( const QMimeData *data )
{
  QgsNamedColorList mimeColors;

  //prefer xml format
  if ( data->hasFormat( "text/xml" ) )
  {
    //get XML doc
    QByteArray encodedData = data->data( "text/xml" );
    QDomDocument xmlDoc;
    xmlDoc.setContent( encodedData );

    QDomElement dragDataElem = xmlDoc.documentElement();
    if ( dragDataElem.tagName() == "ColorSchemeModelDragData" )
    {
      QDomNodeList nodeList = dragDataElem.childNodes();
      int nChildNodes = nodeList.size();
      QDomElement currentElem;

      for ( int i = 0; i < nChildNodes; ++i )
      {
        currentElem = nodeList.at( i ).toElement();
        if ( currentElem.isNull() )
        {
          continue;
        }

        QPair< QColor, QString> namedColor;
        namedColor.first =  QgsSymbolLayerUtils::decodeColor( currentElem.attribute( "color", "255,255,255,255" ) );
        namedColor.second = currentElem.attribute( "label", "" );

        mimeColors << namedColor;
      }
    }
  }

  if ( mimeColors.length() == 0 && data->hasFormat( "application/x-colorobject-list" ) )
  {
    //get XML doc
    QByteArray encodedData = data->data( "application/x-colorobject-list" );
    QDomDocument xmlDoc;
    xmlDoc.setContent( encodedData );

    QDomNodeList colorsNodes = xmlDoc.elementsByTagName( QString( "colors" ) );
    if ( colorsNodes.length() > 0 )
    {
      QDomElement colorsElem = colorsNodes.at( 0 ).toElement();
      QDomNodeList colorNodeList = colorsElem.childNodes();
      int nChildNodes = colorNodeList.size();
      QDomElement currentElem;

      for ( int i = 0; i < nChildNodes; ++i )
      {
        //li element
        currentElem = colorNodeList.at( i ).toElement();
        if ( currentElem.isNull() )
        {
          continue;
        }

        QDomNodeList colorNodes = currentElem.elementsByTagName( QString( "color" ) );
        QDomNodeList nameNodes = currentElem.elementsByTagName( QString( "name" ) );

        if ( colorNodes.length() > 0 )
        {
          QDomElement colorElem = colorNodes.at( 0 ).toElement();

          QStringList colorParts = colorElem.text().simplified().split( ' ' );
          if ( colorParts.length() < 3 )
          {
            continue;
          }

          int red = colorParts.at( 0 ).toDouble() * 255;
          int green = colorParts.at( 1 ).toDouble() * 255;
          int blue = colorParts.at( 2 ).toDouble() * 255;
          QPair< QColor, QString> namedColor;
          namedColor.first = QColor( red, green, blue );
          if ( nameNodes.length() > 0 )
          {
            QDomElement nameElem = nameNodes.at( 0 ).toElement();
            namedColor.second = nameElem.text();
          }
          mimeColors << namedColor;
        }
      }
    }
  }

  if ( mimeColors.length() == 0 && data->hasText() )
  {
    //attempt to read color data from mime text
    QList< QColor > parsedColors = QgsSymbolLayerUtils::parseColorList( data->text() );
    QList< QColor >::iterator it = parsedColors.begin();
    for ( ; it != parsedColors.end(); ++it )
    {
      mimeColors << qMakePair( *it, QString() );
    }
  }

  if ( mimeColors.length() == 0 && data->hasColor() )
  {
    //attempt to read color data directly from mime
    QColor mimeColor = data->colorData().value<QColor>();
    if ( mimeColor.isValid() )
    {
      mimeColors << qMakePair( mimeColor, QString() );
    }
  }

  return mimeColors;
}

QMimeData* QgsSymbolLayerUtils::colorListToMimeData( const QgsNamedColorList& colorList, const bool allFormats )
{
  //native format
  QMimeData* mimeData = new QMimeData();
  QDomDocument xmlDoc;
  QDomElement xmlRootElement = xmlDoc.createElement( "ColorSchemeModelDragData" );
  xmlDoc.appendChild( xmlRootElement );

  QgsNamedColorList::const_iterator colorIt = colorList.constBegin();
  for ( ; colorIt != colorList.constEnd(); ++colorIt )
  {
    QDomElement namedColor = xmlDoc.createElement( "NamedColor" );
    namedColor.setAttribute( "color", QgsSymbolLayerUtils::encodeColor(( *colorIt ).first ) );
    namedColor.setAttribute( "label", ( *colorIt ).second );
    xmlRootElement.appendChild( namedColor );
  }
  mimeData->setData( "text/xml", xmlDoc.toByteArray() );

  if ( !allFormats )
  {
    return mimeData;
  }

  //set mime text to list of hex values
  colorIt = colorList.constBegin();
  QStringList colorListString;
  for ( ; colorIt != colorList.constEnd(); ++colorIt )
  {
    colorListString << ( *colorIt ).first.name();
  }
  mimeData->setText( colorListString.join( "\n" ) );

  //set mime color data to first color
  if ( colorList.length() > 0 )
  {
    mimeData->setColorData( QVariant( colorList.at( 0 ).first ) );
  }

  return mimeData;
}

bool QgsSymbolLayerUtils::saveColorsToGpl( QFile &file, const QString& paletteName, const QgsNamedColorList& colors )
{
  if ( !file.open( QIODevice::ReadWrite ) )
  {
    return false;
  }

  QTextStream stream( &file );
  stream << "GIMP Palette" << endl;
  if ( paletteName.isEmpty() )
  {
    stream << "Name: QGIS Palette" << endl;
  }
  else
  {
    stream << "Name: " << paletteName << endl;
  }
  stream << "Columns: 4" << endl;
  stream << '#' << endl;

  for ( QgsNamedColorList::ConstIterator colorIt = colors.constBegin(); colorIt != colors.constEnd(); ++colorIt )
  {
    QColor color = ( *colorIt ).first;
    if ( !color.isValid() )
    {
      continue;
    }
    stream << QString( "%1 %2 %3" ).arg( color.red(), 3 ).arg( color.green(), 3 ).arg( color.blue(), 3 );
    stream << "\t" << (( *colorIt ).second.isEmpty() ? color.name() : ( *colorIt ).second ) << endl;
  }
  file.close();

  return true;
}

QgsNamedColorList QgsSymbolLayerUtils::importColorsFromGpl( QFile &file, bool &ok, QString &name )
{
  QgsNamedColorList importedColors;

  if ( !file.open( QIODevice::ReadOnly ) )
  {
    ok = false;
    return importedColors;
  }

  QTextStream in( &file );

  QString line = in.readLine();
  if ( !line.startsWith( "GIMP Palette" ) )
  {
    ok = false;
    return importedColors;
  }

  //find name line
  while ( !in.atEnd() && !line.startsWith( "Name:" ) && !line.startsWith( '#' ) )
  {
    line = in.readLine();
  }
  if ( line.startsWith( "Name:" ) )
  {
    QRegExp nameRx( "Name:\\s*(\\S.*)$" );
    if ( nameRx.indexIn( line ) != -1 )
    {
      name = nameRx.cap( 1 );
    }
  }

  //ignore lines until after "#"
  while ( !in.atEnd() && !line.startsWith( '#' ) )
  {
    line = in.readLine();
  }
  if ( in.atEnd() )
  {
    ok = false;
    return importedColors;
  }

  //ready to start reading colors
  QRegExp rx( "^\\s*(\\d+)\\s+(\\d+)\\s+(\\d+)(\\s.*)?$" );
  while ( !in.atEnd() )
  {
    line = in.readLine();
    if ( rx.indexIn( line ) == -1 )
    {
      continue;
    }
    int red = rx.cap( 1 ).toInt();
    int green = rx.cap( 2 ).toInt();
    int blue = rx.cap( 3 ).toInt();
    QColor color = QColor( red, green, blue );
    if ( !color.isValid() )
    {
      continue;
    }

    //try to read color name
    QString label;
    if ( rx.captureCount() > 3 )
    {
      label = rx.cap( 4 ).simplified();
    }
    else
    {
      label = colorToName( color );
    }

    importedColors << qMakePair( color, label );
  }

  file.close();
  ok = true;
  return importedColors;
}

QColor QgsSymbolLayerUtils::parseColor( const QString& colorStr, bool strictEval )
{
  bool hasAlpha;
  return parseColorWithAlpha( colorStr, hasAlpha, strictEval );
}

QColor QgsSymbolLayerUtils::parseColorWithAlpha( const QString& colorStr, bool &containsAlpha, bool strictEval )
{
  QColor parsedColor;

  QRegExp hexColorAlphaRx( "^\\s*#?([0-9a-fA-F]{6})([0-9a-fA-F]{2})\\s*$" );
  int hexColorIndex = hexColorAlphaRx.indexIn( colorStr );

  //color in hex format "#aabbcc", but not #aabbccdd
  if ( hexColorIndex == -1 && QColor::isValidColor( colorStr ) )
  {
    //string is a valid hex color string
    parsedColor.setNamedColor( colorStr );
    if ( parsedColor.isValid() )
    {
      containsAlpha = false;
      return parsedColor;
    }
  }

  //color in hex format, with alpha
  if ( hexColorIndex > -1 )
  {
    QString hexColor = hexColorAlphaRx.cap( 1 );
    parsedColor.setNamedColor( QString( "#" ) + hexColor );
    bool alphaOk;
    int alphaHex = hexColorAlphaRx.cap( 2 ).toInt( &alphaOk, 16 );

    if ( parsedColor.isValid() && alphaOk )
    {
      parsedColor.setAlpha( alphaHex );
      containsAlpha = true;
      return parsedColor;
    }
  }

  if ( !strictEval )
  {
    //color in hex format, without #
    QRegExp hexColorRx2( "^\\s*(?:[0-9a-fA-F]{3}){1,2}\\s*$" );
    if ( hexColorRx2.indexIn( colorStr ) != -1 )
    {
      //add "#" and parse
      parsedColor.setNamedColor( QString( "#" ) + colorStr );
      if ( parsedColor.isValid() )
      {
        containsAlpha = false;
        return parsedColor;
      }
    }
  }

  //color in (rrr,ggg,bbb) format, brackets and rgb prefix optional
  QRegExp rgbFormatRx( "^\\s*(?:rgb)?\\(?\\s*([01]?[0-9]?[0-9]|2[0-4][0-9]|25[0-5])\\s*,\\s*([01]?[0-9]?[0-9]|2[0-4][0-9]|25[0-5])\\s*,\\s*([01]?[0-9]?[0-9]|2[0-4][0-9]|25[0-5])\\s*\\)?\\s*;?\\s*$" );
  if ( rgbFormatRx.indexIn( colorStr ) != -1 )
  {
    int r = rgbFormatRx.cap( 1 ).toInt();
    int g = rgbFormatRx.cap( 2 ).toInt();
    int b = rgbFormatRx.cap( 3 ).toInt();
    parsedColor.setRgb( r, g, b );
    if ( parsedColor.isValid() )
    {
      containsAlpha = false;
      return parsedColor;
    }
  }

  //color in (r%,g%,b%) format, brackets and rgb prefix optional
  QRegExp rgbPercentFormatRx( "^\\s*(?:rgb)?\\(?\\s*(100|0*\\d{1,2})\\s*%\\s*,\\s*(100|0*\\d{1,2})\\s*%\\s*,\\s*(100|0*\\d{1,2})\\s*%\\s*\\)?\\s*;?\\s*$" );
  if ( rgbPercentFormatRx.indexIn( colorStr ) != -1 )
  {
    int r = qRound( rgbPercentFormatRx.cap( 1 ).toDouble() * 2.55 );
    int g = qRound( rgbPercentFormatRx.cap( 2 ).toDouble() * 2.55 );
    int b = qRound( rgbPercentFormatRx.cap( 3 ).toDouble() * 2.55 );
    parsedColor.setRgb( r, g, b );
    if ( parsedColor.isValid() )
    {
      containsAlpha = false;
      return parsedColor;
    }
  }

  //color in (r,g,b,a) format, brackets and rgba prefix optional
  QRegExp rgbaFormatRx( "^\\s*(?:rgba)?\\(?\\s*([01]?[0-9]?[0-9]|2[0-4][0-9]|25[0-5])\\s*,\\s*([01]?[0-9]?[0-9]|2[0-4][0-9]|25[0-5])\\s*,\\s*([01]?[0-9]?[0-9]|2[0-4][0-9]|25[0-5])\\s*,\\s*(0|0?\\.\\d*|1(?:\\.0*)?)\\s*\\)?\\s*;?\\s*$" );
  if ( rgbaFormatRx.indexIn( colorStr ) != -1 )
  {
    int r = rgbaFormatRx.cap( 1 ).toInt();
    int g = rgbaFormatRx.cap( 2 ).toInt();
    int b = rgbaFormatRx.cap( 3 ).toInt();
    int a = qRound( rgbaFormatRx.cap( 4 ).toDouble() * 255.0 );
    parsedColor.setRgb( r, g, b, a );
    if ( parsedColor.isValid() )
    {
      containsAlpha = true;
      return parsedColor;
    }
  }

  //color in (r%,g%,b%,a) format, brackets and rgba prefix optional
  QRegExp rgbaPercentFormatRx( "^\\s*(?:rgba)?\\(?\\s*(100|0*\\d{1,2})\\s*%\\s*,\\s*(100|0*\\d{1,2})\\s*%\\s*,\\s*(100|0*\\d{1,2})\\s*%\\s*,\\s*(0|0?\\.\\d*|1(?:\\.0*)?)\\s*\\)?\\s*;?\\s*$" );
  if ( rgbaPercentFormatRx.indexIn( colorStr ) != -1 )
  {
    int r = qRound( rgbaPercentFormatRx.cap( 1 ).toDouble() * 2.55 );
    int g = qRound( rgbaPercentFormatRx.cap( 2 ).toDouble() * 2.55 );
    int b = qRound( rgbaPercentFormatRx.cap( 3 ).toDouble() * 2.55 );
    int a = qRound( rgbaPercentFormatRx.cap( 4 ).toDouble() * 255.0 );
    parsedColor.setRgb( r, g, b, a );
    if ( parsedColor.isValid() )
    {
      containsAlpha = true;
      return parsedColor;
    }
  }

  //couldn't parse string as color
  return QColor();
}

double QgsSymbolLayerUtils::lineWidthScaleFactor( const QgsRenderContext& c, QgsUnitTypes::RenderUnit u, const QgsMapUnitScale& scale )
{
  switch ( u )
  {
    case QgsUnitTypes::RenderMillimeters:
      return c.scaleFactor();
    case QgsUnitTypes::RenderMapUnits:
    {
      double mup = scale.computeMapUnitsPerPixel( c );
      if ( mup > 0 )
      {
        return 1.0 / mup;
      }
      else
      {
        return 1.0;
      }
    }
    case QgsUnitTypes::RenderPixels:
      return 1.0 / c.rasterScaleFactor();
    case QgsUnitTypes::RenderUnknownUnit:
    case QgsUnitTypes::RenderPercentage:
      //no sensible value
      return 1.0;
  }
  return 1.0;
}

double QgsSymbolLayerUtils::convertToPainterUnits( const QgsRenderContext &c, double size, QgsUnitTypes::RenderUnit unit, const QgsMapUnitScale &scale )
{
  double conversionFactor = lineWidthScaleFactor( c, unit, scale );
  double convertedSize = size * conversionFactor;

  if ( unit == QgsUnitTypes::RenderMapUnits )
  {
    //check max/min size
    if ( scale.minSizeMMEnabled )
      convertedSize = qMax( convertedSize, scale.minSizeMM * c.scaleFactor() );
    if ( scale.maxSizeMMEnabled )
      convertedSize = qMin( convertedSize, scale.maxSizeMM * c.scaleFactor() );
  }

  return convertedSize;
}

double QgsSymbolLayerUtils::convertToMapUnits( const QgsRenderContext &c, double size, QgsUnitTypes::RenderUnit unit, const QgsMapUnitScale &scale )
{
  double mup = c.mapToPixel().mapUnitsPerPixel();

  switch ( unit )
  {
    case QgsUnitTypes::RenderMapUnits:
    {
      // check scale
      double minSizeMU = -DBL_MAX;
      if ( scale.minSizeMMEnabled )
      {
        minSizeMU = scale.minSizeMM * c.scaleFactor() * c.rasterScaleFactor() * mup;
      }
      if ( !qgsDoubleNear( scale.minScale, 0.0 ) )
      {
        minSizeMU = qMax( minSizeMU, size * ( scale.minScale * c.rendererScale() ) );
      }
      size = qMax( size, minSizeMU );

      double maxSizeMU = DBL_MAX;
      if ( scale.maxSizeMMEnabled )
      {
        maxSizeMU = scale.maxSizeMM * c.scaleFactor() * c.rasterScaleFactor() * mup;
      }
      if ( !qgsDoubleNear( scale.maxScale, 0.0 ) )
      {
        maxSizeMU = qMin( maxSizeMU, size * ( scale.maxScale * c.rendererScale() ) );
      }
      size = qMin( size, maxSizeMU );

      return size;
    }
    case QgsUnitTypes::RenderMillimeters:
    {
      return size * c.scaleFactor() * c.rasterScaleFactor() * mup;
    }
    case QgsUnitTypes::RenderPixels:
    {
      return size * mup;
    }

    case QgsUnitTypes::RenderUnknownUnit:
    case QgsUnitTypes::RenderPercentage:
      //no sensible value
      return 0.0;
  }
  return 0.0;
}

double QgsSymbolLayerUtils::pixelSizeScaleFactor( const QgsRenderContext& c, QgsUnitTypes::RenderUnit u, const QgsMapUnitScale& scale )
{
  switch ( u )
  {
    case QgsUnitTypes::RenderMillimeters:
      return ( c.scaleFactor() * c.rasterScaleFactor() );
    case QgsUnitTypes::RenderMapUnits:
    {
      double mup = scale.computeMapUnitsPerPixel( c );
      if ( mup > 0 )
      {
        return c.rasterScaleFactor() / mup;
      }
      else
      {
        return 1.0;
      }
    }
    case QgsUnitTypes::RenderPixels:
      return 1.0;
    case QgsUnitTypes::RenderUnknownUnit:
    case QgsUnitTypes::RenderPercentage:
      //no sensible value
      return 1.0;
  }
  return 1.0;
}

double QgsSymbolLayerUtils::mapUnitScaleFactor( const QgsRenderContext &c, QgsUnitTypes::RenderUnit u, const QgsMapUnitScale &scale )
{
  switch ( u )
  {
    case QgsUnitTypes::RenderMillimeters:
      return scale.computeMapUnitsPerPixel( c ) * c.scaleFactor() * c.rasterScaleFactor();
    case QgsUnitTypes::RenderMapUnits:
    {
      return 1.0;
    }
    case QgsUnitTypes::RenderPixels:
      return scale.computeMapUnitsPerPixel( c );
    case QgsUnitTypes::RenderUnknownUnit:
    case QgsUnitTypes::RenderPercentage:
      //no sensible value
      return 1.0;
  }
  return 1.0;
}

QgsRenderContext QgsSymbolLayerUtils::createRenderContext( QPainter* p )
{
  QgsRenderContext context;
  context.setPainter( p );
  context.setRasterScaleFactor( 1.0 );
  if ( p && p->device() )
  {
    context.setScaleFactor( p->device()->logicalDpiX() / 25.4 );
  }
  else
  {
    context.setScaleFactor( 3.465 ); //assume 88 dpi as standard value
  }
  return context;
}

void QgsSymbolLayerUtils::multiplyImageOpacity( QImage* image, qreal alpha )
{
  if ( !image )
  {
    return;
  }

  QRgb myRgb;
  QImage::Format format = image->format();
  if ( format != QImage::Format_ARGB32_Premultiplied && format != QImage::Format_ARGB32 )
  {
    QgsDebugMsg( "no alpha channel." );
    return;
  }

  //change the alpha component of every pixel
  for ( int heightIndex = 0; heightIndex < image->height(); ++heightIndex )
  {
    QRgb* scanLine = reinterpret_cast< QRgb* >( image->scanLine( heightIndex ) );
    for ( int widthIndex = 0; widthIndex < image->width(); ++widthIndex )
    {
      myRgb = scanLine[widthIndex];
      if ( format == QImage::Format_ARGB32_Premultiplied )
        scanLine[widthIndex] = qRgba( alpha * qRed( myRgb ), alpha * qGreen( myRgb ), alpha * qBlue( myRgb ), alpha * qAlpha( myRgb ) );
      else
        scanLine[widthIndex] = qRgba( qRed( myRgb ), qGreen( myRgb ), qBlue( myRgb ), alpha * qAlpha( myRgb ) );
    }
  }
}

void QgsSymbolLayerUtils::blurImageInPlace( QImage& image, QRect rect, int radius, bool alphaOnly )
{
  // culled from Qt's qpixmapfilter.cpp, see: http://www.qtcentre.org/archive/index.php/t-26534.html
  int tab[] = { 14, 10, 8, 6, 5, 5, 4, 3, 3, 3, 3, 2, 2, 2, 2, 2, 2 };
  int alpha = ( radius < 1 )  ? 16 : ( radius > 17 ) ? 1 : tab[radius-1];

  if ( image.format() != QImage::Format_ARGB32_Premultiplied
       && image.format() != QImage::Format_RGB32 )
  {
    image = image.convertToFormat( QImage::Format_ARGB32_Premultiplied );
  }

  int r1 = rect.top();
  int r2 = rect.bottom();
  int c1 = rect.left();
  int c2 = rect.right();

  int bpl = image.bytesPerLine();
  int rgba[4];
  unsigned char* p;

  int i1 = 0;
  int i2 = 3;

  if ( alphaOnly ) // this seems to only work right for a black color
    i1 = i2 = ( QSysInfo::ByteOrder == QSysInfo::BigEndian ? 0 : 3 );

  for ( int col = c1; col <= c2; col++ )
  {
    p = image.scanLine( r1 ) + col * 4;
    for ( int i = i1; i <= i2; i++ )
      rgba[i] = p[i] << 4;

    p += bpl;
    for ( int j = r1; j < r2; j++, p += bpl )
      for ( int i = i1; i <= i2; i++ )
        p[i] = ( rgba[i] += (( p[i] << 4 ) - rgba[i] ) * alpha / 16 ) >> 4;
  }

  for ( int row = r1; row <= r2; row++ )
  {
    p = image.scanLine( row ) + c1 * 4;
    for ( int i = i1; i <= i2; i++ )
      rgba[i] = p[i] << 4;

    p += 4;
    for ( int j = c1; j < c2; j++, p += 4 )
      for ( int i = i1; i <= i2; i++ )
        p[i] = ( rgba[i] += (( p[i] << 4 ) - rgba[i] ) * alpha / 16 ) >> 4;
  }

  for ( int col = c1; col <= c2; col++ )
  {
    p = image.scanLine( r2 ) + col * 4;
    for ( int i = i1; i <= i2; i++ )
      rgba[i] = p[i] << 4;

    p -= bpl;
    for ( int j = r1; j < r2; j++, p -= bpl )
      for ( int i = i1; i <= i2; i++ )
        p[i] = ( rgba[i] += (( p[i] << 4 ) - rgba[i] ) * alpha / 16 ) >> 4;
  }

  for ( int row = r1; row <= r2; row++ )
  {
    p = image.scanLine( row ) + c2 * 4;
    for ( int i = i1; i <= i2; i++ )
      rgba[i] = p[i] << 4;

    p -= 4;
    for ( int j = c1; j < c2; j++, p -= 4 )
      for ( int i = i1; i <= i2; i++ )
        p[i] = ( rgba[i] += (( p[i] << 4 ) - rgba[i] ) * alpha / 16 ) >> 4;
  }
}

void QgsSymbolLayerUtils::premultiplyColor( QColor &rgb, int alpha )
{
  if ( alpha != 255 && alpha > 0 )
  {
    // Semi-transparent pixel. We need to adjust the colors for ARGB32_Premultiplied images
    // where color values have to be premultiplied by alpha
    double alphaFactor = alpha / 255.;
    int r = 0, g = 0, b = 0;
    rgb.getRgb( &r, &g, &b );

    r *= alphaFactor;
    g *= alphaFactor;
    b *= alphaFactor;
    rgb.setRgb( r, g, b, alpha );
  }
  else if ( alpha == 0 )
  {
    rgb.setRgb( 0, 0, 0, 0 );
  }
}

void QgsSymbolLayerUtils::sortVariantList( QList<QVariant>& list, Qt::SortOrder order )
{
  if ( order == Qt::AscendingOrder )
  {
    //qSort( list.begin(), list.end(), _QVariantLessThan );
    qSort( list.begin(), list.end(), qgsVariantLessThan );
  }
  else // Qt::DescendingOrder
  {
    //qSort( list.begin(), list.end(), _QVariantGreaterThan );
    qSort( list.begin(), list.end(), qgsVariantGreaterThan );
  }
}

QPointF QgsSymbolLayerUtils::pointOnLineWithDistance( QPointF startPoint, QPointF directionPoint, double distance )
{
  double dx = directionPoint.x() - startPoint.x();
  double dy = directionPoint.y() - startPoint.y();
  double length = sqrt( dx * dx + dy * dy );
  double scaleFactor = distance / length;
  return QPointF( startPoint.x() + dx * scaleFactor, startPoint.y() + dy * scaleFactor );
}


QStringList QgsSymbolLayerUtils::listSvgFiles()
{
  // copied from QgsMarkerCatalogue - TODO: unify
  QStringList list;
  QStringList svgPaths = QgsApplication::svgPaths();

  for ( int i = 0; i < svgPaths.size(); i++ )
  {
    QDir dir( svgPaths[i] );
    Q_FOREACH ( const QString& item, dir.entryList( QDir::Dirs | QDir::NoDotAndDotDot ) )
    {
      svgPaths.insert( i + 1, dir.path() + '/' + item );
    }

    Q_FOREACH ( const QString& item, dir.entryList( QStringList( "*.svg" ), QDir::Files ) )
    {
      // TODO test if it is correct SVG
      list.append( dir.path() + '/' + item );
    }
  }
  return list;
}

// Stripped down version of listSvgFiles() for specified directory
QStringList QgsSymbolLayerUtils::listSvgFilesAt( const QString& directory )
{
  // TODO anything that applies for the listSvgFiles() applies this also

  QStringList list;
  QStringList svgPaths;
  svgPaths.append( directory );

  for ( int i = 0; i < svgPaths.size(); i++ )
  {
    QDir dir( svgPaths[i] );
    Q_FOREACH ( const QString& item, dir.entryList( QDir::Dirs | QDir::NoDotAndDotDot ) )
    {
      svgPaths.insert( i + 1, dir.path() + '/' + item );
    }

    Q_FOREACH ( const QString& item, dir.entryList( QStringList( "*.svg" ), QDir::Files ) )
    {
      list.append( dir.path() + '/' + item );
    }
  }
  return list;

}

QString QgsSymbolLayerUtils::symbolNameToPath( QString name )
{
  // copied from QgsSymbol::setNamedPointSymbol - TODO: unify

  // we might have a full path...
  if ( QFile( name ).exists() )
    return QFileInfo( name ).canonicalFilePath();

  // or it might be an url...
  if ( name.contains( "://" ) )
  {
    QUrl url( name );
    if ( url.isValid() && !url.scheme().isEmpty() )
    {
      if ( url.scheme().compare( "file", Qt::CaseInsensitive ) == 0 )
      {
        // it's a url to a local file
        name = url.toLocalFile();
        if ( QFile( name ).exists() )
        {
          return QFileInfo( name ).canonicalFilePath();
        }
      }
      else
      {
        // it's a url pointing to a online resource
        return name;
      }
    }
  }

  // SVG symbol not found - probably a relative path was used

  QStringList svgPaths = QgsApplication::svgPaths();
  for ( int i = 0; i < svgPaths.size(); i++ )
  {
    QString svgPath = svgPaths[i];
    if ( svgPath.endsWith( QChar( '/' ) ) )
    {
      svgPath.chop( 1 );
    }

    QgsDebugMsg( "SvgPath: " + svgPath );
    // Not sure why to lowest dir was used instead of full relative path, it was causing #8664
    //QFileInfo myInfo( name );
    //QString myFileName = myInfo.fileName(); // foo.svg
    //QString myLowestDir = myInfo.dir().dirName();
    //QString myLocalPath = svgPath + QString( myLowestDir.isEmpty() ? "" : '/' + myLowestDir ) + '/' + myFileName;
    QString myLocalPath = svgPath + QDir::separator() + name;

    QgsDebugMsg( "Alternative svg path: " + myLocalPath );
    if ( QFile( myLocalPath ).exists() )
    {
      QgsDebugMsg( "Svg found in alternative path" );
      return QFileInfo( myLocalPath ).canonicalFilePath();
    }
  }

  QFileInfo pfi( QgsProject::instance()->fileName() );
  QString alternatePath = pfi.canonicalPath() + QDir::separator() + name;
  if ( pfi.exists() && QFile( alternatePath ).exists() )
  {
    QgsDebugMsg( "Svg found in alternative path" );
    return QFileInfo( alternatePath ).canonicalFilePath();
  }
  else
  {
    QgsDebugMsg( "Svg not found in project path" );
  }
  //couldnt find the file, no happy ending :-(
  QgsDebugMsg( "Computed alternate path but no svg there either" );

  return QString();
}

QString QgsSymbolLayerUtils::symbolPathToName( QString path )
{
  // copied from QgsSymbol::writeXml

  QFileInfo fi( path );
  if ( !fi.exists() )
    return path;

  path = fi.canonicalFilePath();

  QStringList svgPaths = QgsApplication::svgPaths();

  bool isInSvgPathes = false;
  for ( int i = 0; i < svgPaths.size(); i++ )
  {
    QString dir = QFileInfo( svgPaths[i] ).canonicalFilePath();

    if ( !dir.isEmpty() && path.startsWith( dir ) )
    {
      path = path.mid( dir.size() + 1 );
      isInSvgPathes = true;
      break;
    }
  }

  if ( isInSvgPathes )
    return path;

  return QgsProject::instance()->writePath( path );
}

QPointF QgsSymbolLayerUtils::polygonCentroid( const QPolygonF& points )
{
  //Calculate the centroid of points
  double cx = 0, cy = 0;
  double area, sum = 0;
  for ( int i = points.count() - 1, j = 0; j < points.count(); i = j++ )
  {
    const QPointF& p1 = points[i];
    const QPointF& p2 = points[j];
    area = p1.x() * p2.y() - p1.y() * p2.x();
    sum += area;
    cx += ( p1.x() + p2.x() ) * area;
    cy += ( p1.y() + p2.y() ) * area;
  }
  sum *= 3.0;
  if ( qgsDoubleNear( sum, 0.0 ) )
  {
    // the linear ring is invalid -  let's fall back to a solution that will still
    // allow us render at least something (instead of just returning point nan,nan)
    if ( points.count() >= 2 )
      return QPointF(( points[0].x() + points[1].x() ) / 2, ( points[0].y() + points[1].y() ) / 2 );
    else if ( points.count() == 1 )
      return points[0];
    else
      return QPointF(); // hopefully we shouldn't ever get here
  }
  cx /= sum;
  cy /= sum;

  return QPointF( cx, cy );
}

QPointF QgsSymbolLayerUtils::polygonPointOnSurface( const QPolygonF& points )
{
  QPointF centroid = QgsSymbolLayerUtils::polygonCentroid( points );

  // check if centroid inside in polygon
  if ( !QgsSymbolLayerUtils::pointInPolygon( points, centroid ) )
  {
    unsigned int i, pointCount = points.count();

    QgsPolyline polyline( pointCount );
    for ( i = 0; i < pointCount; ++i ) polyline[i] = QgsPoint( points[i].x(), points[i].y() );

    QgsGeometry geom = QgsGeometry::fromPolygon( QgsPolygon() << polyline );
    if ( !geom.isEmpty() )
    {
      QgsGeometry pointOnSurfaceGeom = geom.pointOnSurface();

      if ( !pointOnSurfaceGeom.isEmpty() )
      {
        QgsPoint point = pointOnSurfaceGeom.asPoint();

        return QPointF( point.x(), point.y() );
      }
    }
  }
  return centroid;
}

bool QgsSymbolLayerUtils::pointInPolygon( const QPolygonF &points, QPointF point )
{
  bool inside = false;

  double x = point.x();
  double y = point.y();

  for ( int i = 0, j = points.count() - 1; i < points.count(); i++ )
  {
    const QPointF& p1 = points[i];
    const QPointF& p2 = points[j];

    if ( qgsDoubleNear( p1.x(), x ) && qgsDoubleNear( p1.y(), y ) )
      return true;

    if (( p1.y() < y && p2.y() >= y ) || ( p2.y() < y && p1.y() >= y ) )
    {
      if ( p1.x() + ( y - p1.y() ) / ( p2.y() - p1.y() )*( p2.x() - p1.x() ) <= x )
        inside = !inside;
    }

    j = i;
  }
  return inside;
}

QgsExpression* QgsSymbolLayerUtils::fieldOrExpressionToExpression( const QString& fieldOrExpression )
{
  if ( fieldOrExpression.isEmpty() )
    return nullptr;

  QgsExpression* expr = new QgsExpression( fieldOrExpression );
  if ( !expr->hasParserError() )
    return expr;

  // now try with quoted field name
  delete expr;
  QgsExpression* expr2 = new QgsExpression( QgsExpression::quotedColumnRef( fieldOrExpression ) );
  Q_ASSERT( !expr2->hasParserError() );
  return expr2;
}

QString QgsSymbolLayerUtils::fieldOrExpressionFromExpression( QgsExpression* expression )
{
  const QgsExpression::Node* n = expression->rootNode();

  if ( n && n->nodeType() == QgsExpression::ntColumnRef )
    return static_cast<const QgsExpression::NodeColumnRef*>( n )->name();

  return expression->expression();
}

QList<double> QgsSymbolLayerUtils::prettyBreaks( double minimum, double maximum, int classes )
{
  // C++ implementation of R's pretty algorithm
  // Based on code for determining optimal tick placement for statistical graphics
  // from the R statistical programming language.
  // Code ported from R implementation from 'labeling' R package
  //
  // Computes a sequence of about 'classes' equally spaced round values
  // which cover the range of values from 'minimum' to 'maximum'.
  // The values are chosen so that they are 1, 2 or 5 times a power of 10.

  QList<double> breaks;
  if ( classes < 1 )
  {
    breaks.append( maximum );
    return breaks;
  }

  int minimumCount = static_cast< int >( classes ) / 3;
  double shrink = 0.75;
  double highBias = 1.5;
  double adjustBias = 0.5 + 1.5 * highBias;
  int divisions = classes;
  double h = highBias;
  double cell;
  int U;
  bool small = false;
  double dx = maximum - minimum;

  if ( qgsDoubleNear( dx, 0.0 ) && qgsDoubleNear( maximum, 0.0 ) )
  {
    cell = 1.0;
    small = true;
    U = 1;
  }
  else
  {
    cell = qMax( qAbs( minimum ), qAbs( maximum ) );
    if ( adjustBias >= 1.5 * h + 0.5 )
    {
      U = 1 + ( 1.0 / ( 1 + h ) );
    }
    else
    {
      U = 1 + ( 1.5 / ( 1 + adjustBias ) );
    }
    small = dx < ( cell * U * qMax( 1, divisions ) * 1e-07 * 3.0 );
  }

  if ( small )
  {
    if ( cell > 10 )
    {
      cell = 9 + cell / 10;
      cell = cell * shrink;
    }
    if ( minimumCount > 1 )
    {
      cell = cell / minimumCount;
    }
  }
  else
  {
    cell = dx;
    if ( divisions > 1 )
    {
      cell = cell / divisions;
    }
  }
  if ( cell < 20 * 1e-07 )
  {
    cell = 20 * 1e-07;
  }

  double base = pow( 10.0, floor( log10( cell ) ) );
  double unit = base;
  if (( 2 * base ) - cell < h *( cell - unit ) )
  {
    unit = 2.0 * base;
    if (( 5 * base ) - cell < adjustBias *( cell - unit ) )
    {
      unit = 5.0 * base;
      if (( 10.0 * base ) - cell < h *( cell - unit ) )
      {
        unit = 10.0 * base;
      }
    }
  }
  // Maybe used to correct for the epsilon here??
  int start = floor( minimum / unit + 1e-07 );
  int end = ceil( maximum / unit - 1e-07 );

  // Extend the range out beyond the data. Does this ever happen??
  while ( start * unit > minimum + ( 1e-07 * unit ) )
  {
    start = start - 1;
  }
  while ( end * unit < maximum - ( 1e-07 * unit ) )
  {
    end = end + 1;
  }
  QgsDebugMsg( QString( "pretty classes: %1" ).arg( end ) );

  // If we don't have quite enough labels, extend the range out
  // to make more (these labels are beyond the data :( )
  int k = floor( 0.5 + end - start );
  if ( k < minimumCount )
  {
    k = minimumCount - k;
    if ( start >= 0 )
    {
      end = end + k / 2;
      start = start - k / 2 + k % 2;
    }
    else
    {
      start = start - k / 2;
      end = end + k / 2 + k % 2;
    }
  }
  double minimumBreak = start * unit;
  //double maximumBreak = end * unit;
  int count = end - start;

  breaks.reserve( count );
  for ( int i = 1; i < count + 1; i++ )
  {
    breaks.append( minimumBreak + i * unit );
  }

  if ( breaks.isEmpty() )
    return breaks;

  if ( breaks.first() < minimum )
  {
    breaks[0] = minimum;
  }
  if ( breaks.last() > maximum )
  {
    breaks[breaks.count()-1] = maximum;
  }

  return breaks;
}

double QgsSymbolLayerUtils::rescaleUom( double size, QgsUnitTypes::RenderUnit unit, const QgsStringMap& props )
{
  double scale = 1;
  bool roundToUnit = false;
  if ( unit == QgsUnitTypes::RenderUnknownUnit )
  {
    if ( props.contains( "uomScale" ) )
    {
      bool ok;
      scale = props.value( "uomScale" ).toDouble( &ok );
      if ( !ok )
      {
        return size;
      }
    }
  }
  else
  {
    if ( props.value( "uom" ) == "http://www.opengeospatial.org/se/units/metre" )
    {
      switch ( unit )
      {
        case QgsUnitTypes::RenderMillimeters:
          scale = 0.001;
          break;
        case QgsUnitTypes::RenderPixels:
          scale = 0.00028;
          roundToUnit = true;
          break;
        default:
          scale = 1;
      }
    }
    else
    {
      // target is pixels
      switch ( unit )
      {
        case QgsUnitTypes::RenderMillimeters:
          scale = 1 / 0.28;
          roundToUnit = true;
          break;
          // we don't have a good case for map units, as pixel values won't change based on zoom
        default:
          scale = 1;
      }
    }

  }
  double rescaled = size * scale;
  // round to unit if the result is pixels to avoid a weird looking SLD (people often think
  // of pixels as integers, even if SLD allows for float values in there
  if ( roundToUnit )
  {
    rescaled = qRound( rescaled );
  }
  return rescaled;
}

QPointF QgsSymbolLayerUtils::rescaleUom( const QPointF& point, QgsUnitTypes::RenderUnit unit, const QgsStringMap& props )
{
  double x = rescaleUom( point.x(), unit, props );
  double y = rescaleUom( point.y(), unit, props );
  return QPointF( x, y );
}

QVector<qreal> QgsSymbolLayerUtils::rescaleUom( const QVector<qreal>& array, QgsUnitTypes::RenderUnit unit, const QgsStringMap& props )
{
  QVector<qreal> result;
  QVector<qreal>::const_iterator it = array.constBegin();
  for ( ; it != array.constEnd(); ++it )
  {
    result.append( rescaleUom( *it, unit, props ) );
  }
  return result;
}

void QgsSymbolLayerUtils::applyScaleDependency( QDomDocument& doc, QDomElement& ruleElem, QgsStringMap& props )
{
  if ( !props.value( "scaleMinDenom", "" ).isEmpty() )
  {
    QDomElement scaleMinDenomElem = doc.createElement( "se:MinScaleDenominator" );
    scaleMinDenomElem.appendChild( doc.createTextNode( props.value( "scaleMinDenom", "" ) ) );
    ruleElem.appendChild( scaleMinDenomElem );
  }

  if ( !props.value( "scaleMaxDenom", "" ).isEmpty() )
  {
    QDomElement scaleMaxDenomElem = doc.createElement( "se:MaxScaleDenominator" );
    scaleMaxDenomElem.appendChild( doc.createTextNode( props.value( "scaleMaxDenom", "" ) ) );
    ruleElem.appendChild( scaleMaxDenomElem );
  }
}

void QgsSymbolLayerUtils::mergeScaleDependencies( int mScaleMinDenom, int mScaleMaxDenom, QgsStringMap& props )
{
  if ( mScaleMinDenom != 0 )
  {
    bool ok;
    int parentScaleMinDenom = props.value( "scaleMinDenom", "0" ).toInt( &ok );
    if ( !ok || parentScaleMinDenom <= 0 )
      props[ "scaleMinDenom" ] = QString::number( mScaleMinDenom );
    else
      props[ "scaleMinDenom" ] = QString::number( qMax( parentScaleMinDenom, mScaleMinDenom ) );
  }

  if ( mScaleMaxDenom != 0 )
  {
    bool ok;
    int parentScaleMaxDenom = props.value( "scaleMaxDenom", "0" ).toInt( &ok );
    if ( !ok || parentScaleMaxDenom <= 0 )
      props[ "scaleMaxDenom" ] = QString::number( mScaleMaxDenom );
    else
      props[ "scaleMaxDenom" ] = QString::number( qMin( parentScaleMaxDenom, mScaleMaxDenom ) );
  }
}