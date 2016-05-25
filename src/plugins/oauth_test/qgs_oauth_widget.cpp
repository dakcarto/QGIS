#include "qgs_oauth_widget.h"
#include "ui_qgs_oauth_widget.h"

#include <QDir>
#include "qgsauthmanager.h"
#include "qgsauthconfigselect.h"
#include <QPushButton>


TestWidget::TestWidget( QWidget *parent )
    : QWidget( parent )
    , mAuthSelector(nullptr)
{
  setupUi( this );
  connect( buttonBox, SIGNAL( rejected() ), this, SLOT( close() ) );
  connect( buttonBox, SIGNAL( accepted() ), this, SLOT( onOk() ) );

  mAuthSelector = new QgsAuthConfigSelect(this);
  connect( mAuthSelector, SIGNAL( selectedConfigIdChanged(QString) ),
           this, SLOT(updateGUI()) );

  authSelectLayout->addWidget( mAuthSelector );
  updateGUI();
}

TestWidget::~TestWidget()
{
}

void TestWidget::updateGUI()
{
  buttonBox->button(QDialogButtonBox::Ok)->setEnabled( !mAuthSelector->configId().isEmpty() );
}

void TestWidget::onOk()
{
  QNetworkRequest request;
  request.setAttribute( QNetworkRequest::User, QVariant(teOut->toPlainText()) );
  QgsAuthManager::instance()->updateNetworkRequest(request, mAuthSelector->configId(), "twitter");
  close();
  // deleteLater();
}
