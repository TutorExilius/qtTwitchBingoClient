#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow( QWidget *parent )
: QMainWindow{ parent }
, ui{ new Ui::MainWindow }
, twitchChat{ nullptr }
, meJoinedBingoGame{ false }
{
    this->ui->setupUi( this );

    this->ui->lineEdit_bingoBotName->setText( "TutorExiliusBot" );
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

void MainWindow::on_pushButton_login_clicked()
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

        const QPair<QString,MESSAGE_TYPE> messagePacket = TwitchIrcConnectionData::extractMessage( line, this->twitchChat->getCurrentUsersNick() );

        const QString message = messagePacket.first.toLower();
        const MESSAGE_TYPE type = messagePacket.second;

        qDebug() << userName << ": " << message;
        qDebug() << "BINGO-BOT-NAME: " << this->getBingoBotName();

        const QString bingoBotName{ this->getBingoBotName() };

        if( userName == bingoBotName )
        {
            if( !this->meJoinedBingoGame &&
                type == MESSAGE_TYPE::PRIVMSG &&
                message == "!bingostart" )
            {
                this->twitchChat->write( "!bingoMe" );

                this->meJoinedBingoGame = true;
            }
            else if( this->numbers.empty() &&
                     type == MESSAGE_TYPE::WHISPER )
            {
                this->extractNumbers( message );
            }
        }
    }

    this->updateUi();
}

void MainWindow::updateUi()
{
    // update listWidgets_numbers
    this->ui->listWidget_numbers->clear();

    for( const int &num : this->numbers )
    {
        this->ui->listWidget_numbers->addItem( QString::number( num ) );
    }
}

void MainWindow::extractNumbers( const QString &message )
{
    for( const QString &numStr : message.split( ',' ) )
    {
        bool ok = false;

        const int number = numStr.toInt( &ok );

        if( !ok )
        {
            this->numbers.clear();
            return;
        }

        this->numbers.insert( number );
    }
}

QString MainWindow::getBingoBotName() const
{
    return this->ui->lineEdit_bingoBotName->text().toLower();
}

void MainWindow::on_pushButton_reset_clicked()
{
    this->meJoinedBingoGame = false;
    this->numbers.clear();
    this->updateUi();
}
