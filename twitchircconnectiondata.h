#ifndef CONNECTIONDATA_H
#define CONNECTIONDATA_H

#include <QByteArray>
#include <QString>
#include <QFile>
#include <QRegularExpression>
#include <QDebug>
#include <QPair>

enum class MESSAGE_TYPE
{
    PRIVMSG,
    WHISPER,
    SYSTEM
};

struct TwitchIrcConnectionData
{
    TwitchIrcConnectionData( const QString &nick,
                    const QString &pass,
                    const QString &host = "irc.chat.twitch.tv",
                    const quint16 &port = 6667 )
    : nick{ nick }
    , pass{ pass }
    , host{ host }
    , port{ port }
    {
    }

    const QString nick;
    const QString pass;
    const QString host;
    const quint16 port;

    static TwitchIrcConnectionData getConnectionData( const QString &twitchIrcLoginFilePath )
    {
        QFile configFile{ twitchIrcLoginFilePath };

        if( configFile.open( QIODevice::ReadOnly | QIODevice::Text ) )
        {
            int lineCount = 0;

            QByteArray nick;
            QByteArray pass;

            while( !(configFile.atEnd()) )
            {
                ++lineCount;

                QByteArray line = configFile.readLine();
                if( line.endsWith( "\n") )
                {
                    line.remove( line.size()-1, 1 );
                }

                switch( lineCount )
                {
                case 1:
                    nick = line;
                    break;

                case 2:
                    pass = line;
                    break;
                }
            }

            return TwitchIrcConnectionData{ nick, pass };
        }

        return TwitchIrcConnectionData{ "", "" };
    }

    static QPair<QString,MESSAGE_TYPE> extractMessage( const QString &line, const QString &nick )
    {
        QRegularExpression reg{ R"(^.*?PRIVMSG #.*? :)",
                                QRegularExpression::DotMatchesEverythingOption };

        QString message = line;
        message.remove( reg );

        // could net extract PRIVMSG, mybe a WHISPER?
        if( message.size() == line.size() )
        {
            QRegularExpression reg{ R"(^.*?WHISPER )" + nick + R"(.*? :)",
                                    QRegularExpression::DotMatchesEverythingOption };

            message.remove( reg );

            if( message.size() == line.size() )
            {
                message.remove( "\r\n" );

                qDebug() << "[SYSTEM]" << message;

                return QPair<QString,MESSAGE_TYPE>{ message, MESSAGE_TYPE::SYSTEM };
            }
            else // is WHISPER
            {
                message.remove( "\r\n" );

                qDebug() << "[WHISPER-MESSAGE]" << message;

                return QPair<QString,MESSAGE_TYPE>{ message, MESSAGE_TYPE::WHISPER };
            }
        }
        else // is PRIVMSG
        {
            message.remove( "\r\n" );

            qDebug() << "[MESSAGE]" << message;

            return QPair<QString,MESSAGE_TYPE>{ message, MESSAGE_TYPE::PRIVMSG };
        }
    }

    static QString extractName( const QString &line )
    {
        QRegularExpression reg{ R"(:[a-zA-Z0-9_]+\!)" };

        QRegularExpressionMatchIterator it = reg.globalMatch( line );

        if( it.hasNext() )
        {
            const QRegularExpressionMatch match = it.next();
            QString username = match.captured(0);
            username.remove( 0, 1 );
            username.remove( username.size() - 1, 1 );

            qDebug() << "[USERNAME]" << username;

            return username;
        }
        else
        {
            return "ERROR_USERNAME";
        }
    }
};

#endif // CONNECTIONDATA_H
