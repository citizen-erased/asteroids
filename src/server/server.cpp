#include "precompiled.h"
#include "network/network.h"
#include "network/message.h"
#include "entity/asteroid.h"
#include "misc.h"
#include "server_game.h"
#include "server.h"

Server::Server(const std::string &address_str, int port, int max_players)
{
    this->address_str = address_str;
    this->port = port;
    max_clients = max_players;

    if(max_clients < 1) max_clients = 1;
    if(max_clients > 128) max_clients = 128;

    host = NULL;
    running = false;
    server_error = ERROR_NONE;
    tick_num = 0;
}

Server::~Server()
{
    stop();
}

void Server::start()
{
    ENetAddress address;

    if(enet_address_set_host(&address, address_str.c_str()) != 0)
        fatal("failed to find host\n");

    address.port = port;
    
    char host_ip[32];
    enet_address_get_host_ip(&address, host_ip, 31);
    print(format("creating host on %1%:%2%\n") % host_ip % address.port);

    host = enet_host_create(&address, max_clients, NetChannel::NUM_CHANNELS, 0, 0);

    if(host == NULL)
        fatal("failed to create ENet server\n");

    game = ServerGamePtr(new ServerGame(shared_from_this()));
    running = true;
}

void Server::stop()
{
    if(host != NULL) enet_host_destroy(host);

    host = NULL;
    running = false;
    peers.clear();
}

bool Server::isRunning()
{
    return running;
}

int Server::getError()
{
    return server_error;
}

void Server::tick()
{
    tick_num++;

    processNetEvents();
    processMessages();

    game->tick();

    sendMessages();

    //print(format("Server: sent %1%B received%2%B\n") % host->totalSentData % host->totalReceivedData);
    host->totalSentData = 0;
    host->totalReceivedData = 0;
}

void Server::processNetEvents()
{
    //print("-------------------------Server::processNetEvents---------------------------\n");
    ENetEvent evt;
    std::map<int, std::vector<PacketPtr> > peer_unreliable_packets;

    while(enet_host_service(host, &evt, 0) > 0)
    {
        switch(evt.type)
        {
        case ENET_EVENT_TYPE_RECEIVE:
        {
            Serializer s;
            s.Append(evt.packet->data, evt.packet->dataLength);
            enet_packet_destroy(evt.packet);

            PacketPtr packet(new Packet(s));

            PeerPtr srv_peer;
            for(unsigned int i = 0; i < peers.size(); i++)
                if(peers[i]->net_peer == evt.peer)
                {
                    srv_peer = peers[i];
                    break;
                }

            if(!srv_peer)
            {
                warning("received a packet from an unknown peer\n");
                continue;
            }

            if(evt.channelID == NetChannel::UNRELIABLE)
            {
                if(compareTicks(packet->getTick(), srv_peer->last_client_tick_received))
                    peer_unreliable_packets[srv_peer->id].push_back(packet);
            }
            else if(evt.channelID == NetChannel::CHAT)
            {
                print("server chat\n");
                packet->getMessages(&srv_peer->chat_in);
            }
        }
        break;
        case ENET_EVENT_TYPE_CONNECT:
        {
            char host_ip[32];
            enet_address_get_host_ip(&evt.peer->address, host_ip, 31);
            print(format("%1%:%2% connected\n") % host_ip % evt.peer->address.port);

            PeerPtr peer(new Peer());
            peer->id = genPeerID();
            peer->state = AWAITING_PEER_INFO;
            peer->net_peer = evt.peer;
            peer->last_client_ack = tick_num-1;
            peer->last_client_tick_received = 0;

            peers.push_back(peer);
            id_to_peer[peer->id] = peer;
        }
        break;
        case ENET_EVENT_TYPE_DISCONNECT:
        {
            print(format("%1%:%2% disconnected\n") % evt.peer->address.host % evt.peer->address.port);

            //TODO put this (and the copy above) into a function
            PeerPtr peer;
            for(unsigned int i = 0; i < peers.size(); i++)
                if(peers[i]->net_peer == evt.peer)
                {
                    peer = peers[i];
                    break;
                }

            if(!peer)
            {
                warning("disconnect message from an unknown peer\n");
                continue;
            }

            game->removePlayer(peer->id);
            id_to_peer.erase(peer->id);
            for(std::vector<PeerPtr>::iterator it = peers.begin(); it != peers.end(); it++)
                if(*it == peer)
                {
                    peers.erase(it);
                    break;
                }
        }
        break;
        default: break;
        }
    }

    for(unsigned int i = 0; i < peers.size(); i++)
    {
        PeerPtr peer = peers[i];

        // no packets recieved from the peer
        if(peer_unreliable_packets.find(peer->id) == peer_unreliable_packets.end())
            continue;

        std::vector<PacketPtr> unreliable_packets = peer_unreliable_packets[peer->id];

        //find the most recent packet
        PacketPtr newest_packet = unreliable_packets[0];
        for(unsigned int i = 1; i < unreliable_packets.size(); i++)
        {
            PacketPtr p = unreliable_packets[i];
            if(compareTicks(p->getTick(), newest_packet->getTick()))
                newest_packet = p;
        }

        //print(format("received from %2%:\n%1%\n") % *newest_packet % peer->id);
        //print(format("server got packet tick=%1% ack=%2% from peer %3% ") % newest_packet->getTick() % newest_packet->getAckTick() % peer->id);
        newest_packet->getMessages(peer->last_client_tick_received, &peer->msgs_in);
        peer->last_client_tick_received = newest_packet->getTick();
        peer->last_client_ack = newest_packet->getAckTick();
        //print(format("%1% messages\n") % peer->msgs_in.size());
        //print(format("server got packet last_client_ack=%1% last_client_received_tick=%2% for peer%3% (%4% messages)\n") % peer.last_client_ack % peer.last_client_tick_received % peer.id % peer.msgs_in.size());
    }
}

void Server::processMessages()
{
    for(unsigned int j = 0; j < peers.size(); j++)
    {
        PeerPtr p = peers[j];

        for(unsigned int i = 0; i < p->msgs_in.size(); i++)
        {
            MessagePtr msg = p->msgs_in[i];
            //print(format("server: message type = %1%\n") % msg->getType());

            if(msg->getType() == MessageType::CLI_INFO)
            {
                print("server: client info received\n");
                if(p->state == AWAITING_PEER_INFO)
                {
                    ClientInfoMessage *info_msg = static_cast<ClientInfoMessage*>(msg.get());

                    //set player's state first so messages game::addPlayer sends will work
                    p->state = PLAYING;
                    game->addPlayer(p->id, info_msg->getData().name);
                }
            }
            else
            {
                game->processPlayerMessage(p->id, msg);
            }
        }

        p->msgs_in.clear();
        p->chat_in.clear();
    }
}

void Server::sendMessages()
{
    //print("-------------------------Server::sendMessages---------------------------\n");
    for(unsigned int j = 0; j < peers.size(); j++)
    {
        PeerPtr peer = peers[j];

        Serializer s;
        MessageBuffer buf = peer->reliable_buffer;
        buf.addMessages(tick_num, peer->unreliable_out);

        Packet packet(tick_num, peer->last_client_tick_received, buf);
        packet.pack(s);

        ENetPacket *net_packet = enet_packet_create(s.GetData(), s.GetDataSize(), 0);

        //todo check null packet
        
        enet_peer_send(peer->net_peer, NetChannel::UNRELIABLE, net_packet);

        //print(format("server sent packet tick=%1% ack=%2% to peer %3%  %4% bytes\n") % packet.getTick() % packet.getAckTick() % peer->id % s.GetDataSize());
        //print(format("sending to %2%:\n%1%\n") % packet % peer->id);
        //print(format("server sent tick_num=%1% last_client_received_tick=%2%\n") % tick_num % p.last_client_tick_received);
    }

    for(unsigned int i = 0; i < peers.size(); i++)
    {
        peers[i]->unreliable_out.clear();
        peers[i]->reliable_buffer.removeMessages(peers[i]->last_client_ack);
    }

    enet_host_flush(host);
}

void Server::queueReliableMessage(int peer_id, MessagePtr msg)
{
    if(peerIDExists(peer_id))
        id_to_peer[peer_id]->reliable_buffer.addMessage(tick_num, msg);
    else
        print(format("Server::queueReliableMessage: peer_id %1% doesn't exist\n") % peer_id);
}

void Server::queueReliableMessageAll(MessagePtr msg)
{
    for(unsigned int i = 0; i < peers.size(); i++)
        if(peers[i]->state == PLAYING)
            peers[i]->reliable_buffer.addMessage(tick_num, msg);
}

void Server::queueUnreliableMessage(int peer_id, MessagePtr msg)
{
    if(peerIDExists(peer_id))
        id_to_peer[peer_id]->unreliable_out.push_back(msg);
    else
        print(format("Server::queueUnreliableMessage: peer_id %1% doesn't exist\n") % peer_id);
}

void Server::queueUnreliableMessageAll(MessagePtr msg)
{
    for(unsigned int i = 0; i < peers.size(); i++)
        if(peers[i]->state == PLAYING)
            peers[i]->unreliable_out.push_back(msg);
}

int Server::genPeerID() const
{
    //TODO generate random id?
    int id = 0;
    do
    {
        id++;
    }
    while(peerIDExists(id));

    return id;
}

bool Server::peerIDExists(int id) const
{
    return id_to_peer.find(id) != id_to_peer.end();
}
