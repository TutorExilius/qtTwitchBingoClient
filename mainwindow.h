#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <unordered_set>

#include "twitchircchat.h"
#include "twitchchatsubscriber.h"

namespace Ui
{
    class MainWindow;
}

class MainWindow : public QMainWindow, public TwitchChatSubscriber
{
    Q_OBJECT

public:
    explicit MainWindow( QWidget *parent = nullptr );
    virtual ~MainWindow() override ;

private slots:
    void onTwitchIrcConnection();

    void on_pushButton_login_clicked();

    void on_pushButton_reset_clicked();

private:
    void update() override;
    void readIrcChatData();
    void extractNumbers( const QString &message );
    void updateUi();
    QString getBingoBotName() const;

    Ui::MainWindow *ui;

    TwitchIrcChat *twitchChat;

    bool meJoinedBingoGame;
    std::unordered_set<int> numbers; // weil slarti sich das wünscht
};

#endif // MAINWINDOW_H
