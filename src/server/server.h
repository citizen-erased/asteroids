#ifndef AST_SERVER_H
#define AST_SERVER_H

#include <boost/enable_shared_from_this.hpp>
#include <enet/enet.h>
#include "types.h"
#include "network/message.h"

class Server : public boost::enable_shared_from_this<Server>
{
private:
    enum Error
    {
        ERROR_NONE,
        ERROR_FAILED_TO_START,
    };

    enum PeerState
    {
        AWAITING_PEER_INFO,
        //AWAITING_GAME_START,
        PLAYING,
    };

    struct Peer
    {
        int id;
        int state;
        std::string name;

        /* The last known server packet received by the peer */
        unsigned int last_client_ack;

        /* The client tick from the last packet received from the peer */
        unsigned int last_client_tick_received;

        ENetPeer *net_peer;

        std::vector<MessagePtr> msgs_in;
        std::vector<MessagePtr> chat_in;

        std::vector<MessagePtr> unreliable_out;
        std::vector<MessagePtr> chat_out;

        MessageBuffer           reliable_buffer;
    };

    typedef boost::shared_ptr<Peer> PeerPtr;

private:
    std::string address_str;
    int port;

    ENetHost *host;
    int max_clients;
    bool running;
    int server_error;
    int tick_num;

    std::vector<PeerPtr> peers;
    std::map<int, PeerPtr> id_to_peer;

    boost::shared_ptr<ServerGame> game;

public:
    Server(const std::string &address_str, int port, int max_players);
    ~Server();
    
    void start();
    void stop();
    bool isRunning();
    int  getError();

    void tick();


    void queueReliableMessage(int peer_id, MessagePtr msg);
    void queueReliableMessageAll(MessagePtr msg);
    void queueUnreliableMessage(int peer_id, MessagePtr msg);
    void queueUnreliableMessageAll(MessagePtr msg);


private:
    void processNetEvents();
    void processMessages();
    void sendMessages();
    int genPeerID() const;
    bool peerIDExists(int id) const;
};

#endif /* AST_SERVER_H */

