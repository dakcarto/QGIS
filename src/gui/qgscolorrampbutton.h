/***************************************************************************
    qgscolorrampbutton.h - Color ramp button
     --------------------------------------
    Date                 : November 27, 2016
    Copyright            : (C) 2016 by Mathieu Pellerin
    Email                : nirvn dot asia at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#ifndef QGSCOLORRAMPBUTTON_H
#define QGSCOLORRAMPBUTTON_H

#include "qgscolorrampbutton.h"

#include "qgscolorramp.h"
#include "qgsstyle.h"

#include <QToolButton>

class QgsPanelWidget;

/** \ingroup gui
 * \class QgsColorRampButton
 * A cross platform button subclass for selecting color ramps. Will open color ramp dialogs when clicked.
 * Offers live updates to button from color ramp dialog. An attached drop down menu allows for access to
 * saved color ramps, as well as option to invert the current color ramp and create new ramps.
 * \note Added in version 3.0
 */

class GUI_EXPORT QgsColorRampButton : public QToolButton
{
    Q_OBJECT
    Q_PROPERTY( QString colorRampDialogTitle READ colorRampDialogTitle WRITE setColorRampDialogTitle )
    Q_PROPERTY( bool acceptLiveUpdates READ acceptLiveUpdates WRITE setAcceptLiveUpdates )
    Q_PROPERTY( bool showMenu READ showMenu WRITE setShowMenu )
    Q_PROPERTY( QgsColorRamp* defaultColorRamp READ defaultColorRamp WRITE setDefaultColorRamp )
    Q_PROPERTY( QString context READ context WRITE setContext )

  public:

    /** Construct a new color ramp button.
     * @param parent The parent QWidget for the dialog
     * @param dialogTitle The title to show in the color ramp dialog
     */
    QgsColorRampButton( QWidget *parent = nullptr, const QString& dialogTitle = QString() );

    virtual ~QgsColorRampButton();

    virtual QSize sizeHint() const override;

    /** Return the current color ramp.
     * @returns currently selected color
     * @see setColor
     */
    QgsColorRamp* colorRamp() const;

    /** Set the title for the color ramp dialog window.
     * @param title Title for the color ramp dialog
     * @see colorRampDialogTitle
     */
    void setColorRampDialogTitle( const QString& title );

    /** Returns the title for the color ramp dialog window.
     * @returns title for the color ramp dialog
     * @see setColorRampDialogTitle
     */
    QString colorRampDialogTitle() const;

    /** Returns whether the button accepts live updates from QgsColorRampDialog.
     * @returns true if the button will be accepted immediately when the dialog's color ramp changes
     * @see setAcceptLiveUpdates
     */
    bool acceptLiveUpdates() const { return mAcceptLiveUpdates; }

    /** Sets whether the button accepts live updates from QgsColorRampDialog. Live updates may cause changes
     * that are not undoable on QColorRampDialog cancel.
     * @param accept set to true to enable live updates
     * @see acceptLiveUpdates
     */
    void setAcceptLiveUpdates( const bool accept ) { mAcceptLiveUpdates = accept; }

    /** Sets whether the drop down menu should be shown for the button. The default behaviour is to
     * show the menu.
     * @param showMenu set to false to hide the drop down menu
     * @see showMenu
     */
    void setShowMenu( const bool showMenu );

    /** Returns whether the drop down menu is shown for the button.
     * @returns true if drop down menu is shown
     * @see setShowMenu
     */
    bool showMenu() const { return menu() ? true : false; }

    /** Sets the default color ramp for the button, which is shown in the button's drop down menu for the
     * "default color ramp" option.
     * @param colorramp default color ramp for the button. Set to a null pointer to disable the default color
     * ramp option.
     * @see defaultColorRamp
     */
    void setDefaultColorRamp( QgsColorRamp* colorramp );

    /** Returns the default color ramp for the button, which is shown in the button's drop down menu for the
     * "default color ramp" option.
     * @returns default color ramp for the button. Returns a null pointer if the default color ramp
     * option is disabled.
     * @see setDefaultColorRamp
     */
    QgsColorRamp* defaultColorRamp() const { return mDefaultColorRamp; }

    /** Sets whether a random colors option is shown in the button's drop down menu.
     * @param showRandom set to true to show a random colors option
     * @see showRandom()
     */
    void setShowRandomColorRamp( bool showRandom ) { mShowRandomColorRamp = showRandom; }

    /** Returns whether random colors option is shown in the button's drop down menu.
     * @see setShowRandom()
     */
    bool showRandomColorRamp() const { return mShowRandomColorRamp; }

    /** Returns true if the current color is null.
     * @see setShowNull()
     * @see showNull()
     */
    bool isRandomColorRamp() const;

    /** Sets whether a set to null (clear) option is shown in the button's drop down menu.
     * @param showNull set to true to show a null option
     * @see showNull()
     * @see isNull()
     */
    void setShowNull( bool showNull );

    /** Returns whether the set to null (clear) option is shown in the button's drop down menu.
     * @see setShowNull()
     * @see isNull()
     */
    bool showNull() const;

    /** Returns true if the current color is null.
     * @see setShowNull()
     * @see showNull()
     */
    bool isNull() const;

    /** Sets the context string for the color ramp button. The context string is passed to all color ramp
     * preview icons shown in the button's drop down menu, to (eventually) allow them to customise their display colors
     * based on the context.
     * @param context context string for the color dialog button's color ramp preview icons
     * @see context
     */
    void setContext( const QString& context ) { mContext = context; }

    /** Returns the context string for the color ramp button. The context string is passed to all color ramp
     * preview icons shown in the button's drop down menu, to (eventually) allow them to customise their display colors
     * based on the context.
     * @returns context context string for the color dialog button's color ramp preview icons
     * @see setContext
     */
    QString context() const { return mContext; }

    /** Sets whether the color ramp button only shows gradient type ramps
     * @param gradientonly set to true to show only gradient type ramps
     * @see showGradientOnly
     */
    void setShowGradientOnly( bool gradientonly ) { mShowGradientOnly = gradientonly; }

    /** Returns true if the color ramp button only shows gradient type ramps
     * @see setShowGradientOnly
     */
    bool showGradientOnly() const { return mShowGradientOnly; }

  public slots:

    /** Sets the current color ramp for the button. Will emit a colorRampChanged() signal if the color ramp is different
     * to the previous color ramp.
     * @param colorramp New color ramp for the button
     * @see setRandomColorRamp, setColorRampFromName, colorRamp
     */
    void setColorRamp( QgsColorRamp* colorramp );

    /** Sets the current color ramp for the button to random colors. Will emit a colorRampChanged() signal
     * if the color ramp is different to the previous color ramp.
     * @see setColorRamp, setColorRampFromName, colorRamp
     */
    void setRandomColorRamp();

    /** Sets the current color ramp for the button using a saved color ramp name. Will emit a colorRampChanged() signal
     * if the color ramp is different to the previous color ramp.
     * @param name Name of saved color ramp
     * @see setColorRamp, setRandomColorRamp, colorRamp
     */
    void setColorRampFromName( QString name = QString() );

    /** Sets the background pixmap for the button based upon current color ramp.
     * @param colorramp Color ramp for button background. If no color ramp is specified, the button's current
     * color ramp will be used
     */
    void setButtonBackground( QgsColorRamp* colorramp = nullptr );

    /** Sets color ramp to the button's default color ramp, if set.
     * @see setDefaultColorRamp
     * @see defaultColorRamp
     * @see setToNull()
     */
    void setToDefaultColorRamp();

    /** Sets color ramp to null.
     * @see setToDefaultColorRamp()
     */
    void setToNull();

  private slots:

    void rampWidgetUpdated();

  signals:

    /** Emitted whenever a new color ramp is set for the button. The color ramp is always valid.
     * In case the new color ramp is the same, no signal is emitted to avoid infinite loops.
     */
    void colorRampChanged();

  protected:

    bool event( QEvent *e ) override;
    void changeEvent( QEvent* e ) override;
    void showEvent( QShowEvent* e ) override;
    void resizeEvent( QResizeEvent *event ) override;

    /**
     * Reimplemented to detect right mouse button clicks on the color ramp button
     */
    void mousePressEvent( QMouseEvent* e ) override;

  private:

    QString mColorRampDialogTitle;
    bool mShowGradientOnly;
    QgsColorRamp* mColorRamp;
    QgsStyle* mStyle;

    QgsColorRamp* mDefaultColorRamp;
    QString mContext;
    bool mAcceptLiveUpdates;
    bool mColorRampSet;
    bool mShowRandomColorRamp;
    bool mShowNull;

    QMenu* mMenu;
    QMenu *mAllRampsMenu;

    QSize mIconSize;

    /** Create a color ramp icon for display in the drop down menu
     * @param colorramp Color ramp to create an icon from
     */
    QPixmap createMenuIcon( QgsColorRamp* colorramp );

  private slots:

    void buttonClicked();

    /** Show a color ramp dialog based on the ramp type
     */
    void showColorRampDialog();

    /** Creates a new color ramp
     */
    void createColorRamp();

    /** Inverts the current color ramp
     */
    void invertColorRamp();

    /** Load a color ramp from a menu entry
     */
    void loadColorRamp();

    /** Creates the drop down menu entries
     */
    void prepareMenu();
};

#endif