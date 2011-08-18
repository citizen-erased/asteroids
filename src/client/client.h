#ifndef AST_CLIENT_H
#define AST_CLIENT_H

#include <boost/enable_shared_from_this.hpp>
#include <enet/enet.h>
#include "network/message.h"
#include "client_game.h"

class Client : public boost::enable_shared_from_this<Client>
{
private:
    enum State
    {
        AWAITING_CONNECTION,
        AWAITING_GAMESTATE,
        PLAYING,
        DISCONNECTED,
    };

    static int              TIMEOUT_TICKS;

private:
    ClientGamePtr           game;
    RenderWindowPtr         window;

    std::string             address_str;
    int                     port;
    std::string             player_name;

    unsigned int            tick_num;
    int                     state;
    int                     uid;

    /* The last known client packet received by the server */
    unsigned int            last_server_ack;

    /* The server tick from the last packet received. */
    unsigned int            last_server_tick_received;

    ENetHost                *net_host;
    ENetPeer                *net_peer;

    std::vector<MessagePtr> msgs_in;
    std::vector<MessagePtr> msgs_chat_in;

    std::vector<MessagePtr> unreliable_out;
    MessageBuffer           reliable_buffer;

    int                     net_idle_ticks;


public:
                            Client(RenderWindowPtr win, const std::string &address_str, int port, const std::string &player_name);
                           ~Client();

    void                    start();

    void                    tick();
    void                    event(const sf::Event e);
    void                    draw();

    void                    processNetEvents();
    void                    processMessages();
    void                    sendMessages();

    void                    queueReliableMessage(MessagePtr msg);
    void                    queueUnreliableMessage(MessagePtr msg);
};

#endif /* AST_CLIENT_H */

