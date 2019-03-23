#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow( QWidget *parent )
: QMainWindow{ parent }
, ui{ new Ui::MainWindow }
, twitchChat{ nullptr }
{
    this->ui->setupUi( this );
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::onTwitchIrcConnection()
{
    QObject::disconnect( this->twitchChat, &TwitchIrcChat::twitchChannelConnection,
                      this, &MainWindow::onTwitchIrcConnection );

    this->twitchChat->subscribeChat( this );
}

void MainWindow::on_pushButton_clicked()
{
    static bool alreadyConnected = false;

    this->twitchChat = new TwitchIrcChat{ this };

    TwitchIrcConnectionData connectionData{
        TwitchIrcConnectionData::getConnectionData( QApplication::applicationDirPath() + "/config.txt" ) };

    if( !alreadyConnected &&
         this->twitchChat->connectToChannel( "tutorexilius", connectionData ) )
    {
            alreadyConnected = true;

            QObject::connect( this->twitchChat, &TwitchIrcChat::twitchChannelConnection,
                              this, &MainWindow::onTwitchIrcConnection,
                              Qt::UniqueConnection );
    }
}

void MainWindow::update()
{
    this->readIrcChatData();
}

void MainWindow::readIrcChatData()
{
    QVector<QString> readData = this->twitchChat->getDataLines();

    for( const auto &line : readData )
    {
        const QString userName = TwitchIrcConnectionData::extractName( line ).toLower();
        const QString message = TwitchIrcConnectionData::extractMessage( line ).toLower();

        qDebug() << userName << ": " << message;

    }
}
