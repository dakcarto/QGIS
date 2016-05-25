#ifndef TESTWIDGET_H
#define TESTWIDGET_H

#include "ui_qgs_oauth_widget.h"

#include <QNetworkReply>

class QgsAuthConfigSelect;

class TestWidget : public QWidget, private Ui::TestWidget
{
    Q_OBJECT

  public:
    explicit TestWidget( QWidget *parent = 0 );
    ~TestWidget();

  public slots:

    void onOk();

  private slots:
    void updateGUI();

  private:
     QgsAuthConfigSelect *mAuthSelector;
};

#endif // TESTWIDGET_H
