/***************************************************************************
    qgsfloatingwidget.h
    -------------------
    begin                : April 2016
    copyright            : (C) Nyall Dawson
    email                : nyall dot dawson at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#ifndef QGSFLOATINGWIDGET_H
#define QGSFLOATINGWIDGET_H

#include <QWidget>


/** \ingroup gui
 * \class QgsFloatingWidget
 * A QWidget subclass for creating widgets which float outside of the normal Qt layout
 * system. Floating widgets use an "anchor widget" to determine how they are anchored
 * within their parent widget.
 * \note Added in version 2.16
 */

class GUI_EXPORT QgsFloatingWidget: public QWidget
{
    Q_OBJECT

  public:

    //! Reference points for anchoring widget position
    enum AnchorPoint
    {
      TopLeft, //!< Top-left of widget
      TopMiddle, //!< Top center of widget
      TopRight, //!< Top-right of widget
      MiddleLeft, //!< Middle left of widget
      Middle, //!< Middle of widget
      MiddleRight, //!< Middle right of widget
      BottomLeft, //!< Bottom-left of widget
      BottomMiddle, //!< Bottom center of widget
      BottomRight, //!< Bottom-right of widget
    };

    /** Constructor for QgsFloatingWidget.
     * @param parent parent widget
     */
    QgsFloatingWidget( QWidget* parent = nullptr );

    /** Sets the widget to "anchor" the floating widget to. The floating widget will be repositioned whenever the
     * anchor widget moves or is resized so that it maintains the same relative position to the anchor widget.
     * @param widget anchor widget. Both the floating widget and the anchor widget must share some common parent.
     * @see anchorWidget()
     */
    void setAnchorWidget( QWidget* widget );

    /** Returns the widget that the floating widget is "anchored" tto. The floating widget will be repositioned whenever the
     * anchor widget moves or is resized so that it maintains the same relative position to the anchor widget.
     * @see setAnchorWidget()
     */
    QWidget* anchorWidget();

    /** Returns the floating widget's anchor point, which corresponds to the point on the widget which should remain
     * fixed in the same relative position whenever the widget's parent is resized or moved.
     * @see setAnchorPoint()
     */
    AnchorPoint anchorPoint() const { return mFloatAnchorPoint; }

    /** Sets the floating widget's anchor point, which corresponds to the point on the widget which should remain
     * fixed in the same relative position whenever the widget's parent is resized or moved.
     * @param point anchor point
     * @see anchorPoint()
     */
    void setAnchorPoint( AnchorPoint point ) { mFloatAnchorPoint = point; anchorPointChanged(); }

    /** Returns the anchor widget's anchor point, which corresponds to the point on the anchor widget which
     * the floating widget should "attach" to. The floating widget should remain fixed in the same relative position
     * to this anchor widget whenever the widget's parent is resized or moved.
     * @see setAnchorWidgetPoint()
     */
    AnchorPoint anchorWidgetPoint() const { return mAnchorWidgetAnchorPoint; }

    /** Returns the anchor widget's anchor point, which corresponds to the point on the anchor widget which
     * the floating widget should "attach" to. The floating widget should remain fixed in the same relative position
     * to this anchor widget whenever the widget's parent is resized or moved.
     * @see setAnchorWidgetPoint()
     */
    void setAnchorWidgetPoint( AnchorPoint point ) { mAnchorWidgetAnchorPoint = point; anchorPointChanged(); }

  protected:
    void showEvent( QShowEvent* e ) override;
    virtual void paintEvent( QPaintEvent* e ) override;

  private slots:

    //! Repositions the floating widget to a changed anchor point
    void anchorPointChanged();

  private:

    QWidget* mAnchorWidget;
    QObject* mParentEventFilter;
    QObject* mAnchorEventFilter;
    AnchorPoint mFloatAnchorPoint;
    AnchorPoint mAnchorWidgetAnchorPoint;

};

/// @cond PRIVATE

class QgsFloatingWidgetEventFilter: public QObject
{
    Q_OBJECT

  public:

    QgsFloatingWidgetEventFilter( QWidget* parent = nullptr );

    virtual bool eventFilter( QObject* object, QEvent* event ) override;

  signals:

    //! Emitted when the filter's parent is moved or resized
    void anchorPointChanged();

};

/// @endcond

#endif // QGSFLOATINGWIDGET_H