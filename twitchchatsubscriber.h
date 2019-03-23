#ifndef TWITCHCHATSUBSCRIBER_H
#define TWITCHCHATSUBSCRIBER_H

class TwitchChatSubscriber
{
public:
    TwitchChatSubscriber() = default;
    virtual ~TwitchChatSubscriber() = default;

    virtual void update() = 0;

private:
    // deletes ---
    TwitchChatSubscriber( const TwitchChatSubscriber& ) = delete;
    TwitchChatSubscriber( TwitchChatSubscriber&& ) = delete;
    TwitchChatSubscriber& operator=( const TwitchChatSubscriber& ) = delete;
    TwitchChatSubscriber& operator=( TwitchChatSubscriber&& ) = delete;
    // ---
};

#endif // TWITCHCHATSUBSCRIBER_H
