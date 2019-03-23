#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

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
    ~MainWindow();

private slots:
    void onTwitchIrcConnection();
    void on_pushButton_clicked();

private:
    void update() override;
    void readIrcChatData();

    Ui::MainWindow *ui;

    TwitchIrcChat *twitchChat;
};

#endif // MAINWINDOW_H
