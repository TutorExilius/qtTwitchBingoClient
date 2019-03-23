#include "twitchircchat.h"

#include <QDateTime>
#include <QDebug>
#include <QTimer>
#include <QMessageBox>

#include <future>
#include <thread>

TwitchIrcChat::TwitchIrcChat( QObject *parent )
: QObject{ parent }
, socket{ nullptr }
, currentChannel{ "TutorExilius" }
, connectedHost{ "" }
, connectedPort{ 0 }
, timer{ this }
, chatSubsriber{ nullptr }
{
}

TwitchIrcChat::~TwitchIrcChat()
{
    this->disconnectFromServer();
}

bool TwitchIrcChat::connectToChannel( const QString &channel, const TwitchIrcConnectionData &connectionData )
{
    this->currentChannel = channel;
    this->connectedHost = connectionData.host;
    this->connectedPort = connectionData.port;

    const QByteArray lowerNick = connectionData.nick.toUtf8().toLower();
    const QByteArray lowerChannel = channel.toUtf8().toLower();

    // Your nickname (NICK) must be your Twitch user name in lowercase.
    // see: https://dev.twitch.tv/docs/irc (Connecting to Twitch IRC)   [03.03.2018]

    // connect to server
    this->socket = new QTcpSocket{ this };
    this->socket->connectToHost( connectionData.host, connectionData.port );
    qDebug() << "Connect to: " << connectionData.host << ":" << connectionData.port;

    // ckeck socket connection
    if( !this->socket->isOpen() )
    {
        // "socket"-connection failed
        return false;
    }

    // authenticate with twicht server ---

    // Streamers OAUTH Key
    this->send( "PASS " + connectionData.pass.toUtf8() + "\r\n" );
    // Streamers Nick
    this->send( "NICK " + lowerNick + "\r\n" );

    // ---

    // check first RPL_WELCOME (Numeric 001)
    bool RPL_WELCOME_success = false;

    if( this->socket->waitForReadyRead() )
    {
        const QVector<QString> dataLines { this->getDataLines() };
        for( const QString &dataLine : dataLines )
        {
            if( dataLine.startsWith( ":tmi.twitch.tv 001 " ) )
            {
                RPL_WELCOME_success = true;
                break;
            }
        }
    }

    // join after RPL_WELCOME (Numeric 001)
    if( RPL_WELCOME_success )
    {
        // Anweisung an den IRC Server, badges mitzusenden (twitch format)
        this->send( "CAP REQ :twitch.tv/membership\r\nCAP REQ :twitch.tv/tags\r\nCAP REQ :twitch.tv/commands\r\n");

        // Join Channel
        this->send( "JOIN #" + lowerChannel + "\r\n" );


        QObject::connect( this->socket, &QTcpSocket::readyRead,
                          this, &TwitchIrcChat::timerStart,
                          Qt::UniqueConnection );

        return true;
    }
    else
    {
        // handshake with server failed
        return false;
    }
}

void TwitchIrcChat::disconnectFromChannel( const QString &channel )
{
    const QByteArray lowerChannel = channel.toUtf8().toLower();

    // Leave Channel
    this->send( "PART #" + lowerChannel + "\r\n" );

    qDebug() << "Disconnected " << this->connectedHost << ":" << this->connectedPort;
}

const QString& TwitchIrcChat::getCurrentChannel() const
{
    return this->currentChannel;
}

void TwitchIrcChat::write( const QString &text )
{
    QString message = "PRIVMSG #" + this->getCurrentChannel().toLower() + " :" + text + "\r\n";
    this->send( message.toUtf8() );
}

void TwitchIrcChat::send( const QByteArray &message ) const
{
    qDebug() << "SEND to server: " << ((message.startsWith("PASS")) ? "********" : message);
    qDebug() << "\t" << "Succesfully send " << this->socket->write( message ) << " Bytes";
    this->socket->flush();
}

QVector<QString> TwitchIrcChat::getDataLines() const
{
     QVector<QString> chatLines;

     do
     {
        QString readLine = this->socket->readLine();

        // Answer to PING
        if( readLine == "PING :tmi.twitch.tv\r\n" )
        {
            this->send( "PONG :tmi.twitch.tv\r\n" );
        }

        chatLines.push_back( readLine );

     } while( socket->canReadLine() );

     return chatLines;
}

QTcpSocket *TwitchIrcChat::getSocket() const
{
    return this->socket;
}

void TwitchIrcChat::subscribeChat( TwitchChatSubscriber *subscriber )
{
    if( this->chatSubsriber == nullptr )
    {
        this->chatSubsriber = subscriber;
    }
    else
    {
        qDebug() << "ChatSubsriber is already set!";
    }
}

void TwitchIrcChat::timerStart()
{
    qDebug() << "Timer started";

    QObject::disconnect( this->socket, &QTcpSocket::readyRead,
                         this, &TwitchIrcChat::timerStart );

    QObject::connect( &this->timer, &QTimer::timeout,
                     this, &TwitchIrcChat::handleChannelJoin,
                     Qt::UniqueConnection );

    this->timer.start( 3000 );
}

void TwitchIrcChat::handleChannelJoin()
{
    qDebug() << "Timer stopped (timeout)";
    this->timer.stop();

    if( this->successfullyJoined() )
    {
        this->joinedCurrentChannel = true;

        QObject::connect( this->socket, &QTcpSocket::readyRead,
                          this, &TwitchIrcChat::onReadyReadSocket,
                          Qt::UniqueConnection );
    }
    else
    {
        this->joinedCurrentChannel = false;

        QMessageBox msg;
        msg.setText( "Connection to Twitch-Chat failed!");
        msg.setWindowTitle( "Connetion failed" );
        msg.exec();
    }

    emit twitchChannelConnection( this->joinedCurrentChannel );
}

bool TwitchIrcChat::successfullyJoined()
{
    while( socket->canReadLine() )
    {
       QString readLine = this->socket->readLine();
       qDebug() << readLine;

       if( readLine.contains(":End of /NAMES list\r\n") )
       {
           return true;
       }
    }

    return false;
}

bool TwitchIrcChat::isJoinedCurrentChannel() const
{
    return this->joinedCurrentChannel;
}

void TwitchIrcChat::onReadyReadSocket()
{
    // inform subscriber about current chat data
    if( this->chatSubsriber != nullptr )
    {
        this->chatSubsriber->update();
    }
    else // if no subscriber registered, "clear" socket for new chat data
    {
        this->socket->readAll();
    }
}

void TwitchIrcChat::disconnectFromServer()
{
    if( this->socket->isOpen() )
    {
        if( this->joinedCurrentChannel )
        {
            this->disconnectFromChannel( this->currentChannel );
        }

        this->socket->disconnectFromHost();
        this->socket->disconnect();
        this->socket->close();
    }
}
