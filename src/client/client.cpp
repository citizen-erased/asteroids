#include "precompiled.h"
#include "client.h"
#include "network/network.h"
#include "network/message.h"
#include "misc.h"


int Client::TIMEOUT_TICKS = 300;


Client::Client(RenderWindowPtr win, const std::string &address_str, int port, const std::string &player_name)
{
    window = win;
    this->address_str = address_str;
    this->port = port;
    this->player_name = player_name;
}

Client::~Client()
{
    enet_host_destroy(net_host);
}

void Client::start()
{
    net_host = enet_host_create(NULL, 1, 1, 0, 0);
    
    if(net_host == NULL)
        fatal("failed to create net client");

    ENetAddress address;

    enet_address_set_host(&address, address_str.c_str());
    address.port = port;

    net_peer = enet_host_connect(net_host, &address, 1, 0);

    if(net_peer == NULL)
        fatal("no available peers");

    state = AWAITING_CONNECTION;
    tick_num = 0;
    net_idle_ticks = 0;
    last_server_ack = 0;
    last_server_tick_received = 0;


    //game = ClientGamePtr(new ClientGame(shared_from_this()));
}

void Client::tick()
{
    tick_num++;

    processNetEvents();
    processMessages();
    if(game) game->tick();
    sendMessages();

    //print(format("Client: sent %1%B received%2%B\n") % net_host->totalSentData % net_host->totalReceivedData);
    net_host->totalSentData = 0;
    net_host->totalReceivedData = 0;
}

void Client::processNetEvents()
{
    //print("-------------------------Client::processNetEvents---------------------------\n");
    ENetEvent evt;
    int num_events = 0;
    std::vector<PacketPtr> unreliable_packets;

    while(enet_host_service(net_host, &evt, 0) > 0)
    {
        num_events++;

        switch(evt.type)
        {
        case ENET_EVENT_TYPE_RECEIVE:
        {
            Serializer s;
            s.Append(evt.packet->data, evt.packet->dataLength);
            //print(format("client recieved %1%B\n") % evt.packet->dataLength);
            PacketPtr packet(new Packet(s));
            enet_packet_destroy(evt.packet);

            if(evt.channelID == NetChannel::UNRELIABLE)
            {
                if(compareTicks(packet->getTick(), last_server_tick_received))
                    unreliable_packets.push_back(packet);
            }
            else if(evt.channelID == NetChannel::CHAT)
            {
                packet->getMessages(&msgs_chat_in);
            }

        }
        break;

        case ENET_EVENT_TYPE_CONNECT:
        {
            print("Client: connect\n");
            if(state == AWAITING_CONNECTION)
            {
                //send client data
                state = AWAITING_GAMESTATE;

                print("Client: sending ClientInfoMessage\n");
                ClientInfoData d;
                d.name = player_name;
                queueReliableMessage(MessagePtr(new ClientInfoMessage(d)));
            }
        }
        break;

        case ENET_EVENT_TYPE_DISCONNECT:
        {
            print("Client: disconnect\n");
            state = DISCONNECTED;
        }
        break;

        default: break;
        }
    }

    
    //print(format("Client: received %1% packets\n") % unreliable_packets.size());
    if(!unreliable_packets.empty())
    {
        PacketPtr newest_packet = unreliable_packets[0];
        for(unsigned int i = 1; i < unreliable_packets.size(); i++)
            if(compareTicks(unreliable_packets[i]->getTick(), newest_packet->getTick()))
                newest_packet = unreliable_packets[i];

        newest_packet->getMessages(last_server_tick_received, &msgs_in);
        last_server_tick_received = newest_packet->getTick();
        last_server_ack = newest_packet->getAckTick();

        //print(format("%1%\n") % *newest_packet);
    }


    if(num_events == 0)
        net_idle_ticks++;
    else
        net_idle_ticks = 0;
}

void Client::processMessages()
{
    for(unsigned int i = 0; i < msgs_in.size(); i++)
    {
        MessagePtr msg = msgs_in[i];

        if(msg->getType() == MessageType::SRV_GAMESTATE)
        {
            print("gamestate received\n");
            GameStateMessage *state_msg = static_cast<GameStateMessage*>(msg.get());
            game = ClientGamePtr(new ClientGame(window, state_msg->getData(), shared_from_this()));
        }
        else if(msg->getType() == MessageType::SRV_SHUTDOWN)
        {

        }
        else
        {
            if(game) game->processMessage(msg);
        }
    }

    for(unsigned int i = 0; i < msgs_chat_in.size(); i++)
    {

    }

    msgs_in.clear();
    msgs_chat_in.clear();
}

void Client::sendMessages()
{
    //print("-------------------------Client::sendMessages---------------------------\n");
    if(state == AWAITING_CONNECTION)
        return;

    Serializer s;
    MessageBuffer buf = reliable_buffer;
    buf.addMessages(tick_num, unreliable_out);
    Packet p(tick_num, last_server_tick_received, buf);
    p.pack(s);

    //print(format("%1%\n") % p);
    ENetPacket *net_packet = enet_packet_create(s.GetData(), s.GetDataSize(), 0);

    //todo check null packet
    
    enet_peer_send(net_peer, NetChannel::UNRELIABLE, net_packet);
    enet_host_flush(net_host);

    unreliable_out.clear();
    reliable_buffer.removeMessages(last_server_ack);

    //print(format("client sent packet tick=%1% ack=%2% %3% bytes\n") % p.getTick() % p.getAckTick() % s.GetDataSize());
    //print(format("client sending to server:\n%1%\n") % p);
    //print(format("client sent tick_num=%1% last_server_tick_received=%2%\n") % tick_num % last_server_tick_received);
}

void Client::queueUnreliableMessage(MessagePtr msg)
{
    unreliable_out.push_back(msg);
}

void Client::queueReliableMessage(MessagePtr msg)
{
    reliable_buffer.addMessage(tick_num, msg);
}

void Client::draw()
{
    if(game)
        game->draw();
    else
    {
        glClear(GL_COLOR_BUFFER_BIT);
        sf::String str("connecting...\n", sf::Font::GetDefaultFont(), 30);
        int hw = window->GetWidth()/2;
        int hh = window->GetHeight()/2;
        int hsw = str.GetRect().GetWidth()/2;
        int hsh = str.GetRect().GetHeight()/2;

        str.SetPosition(hw-hsw, hh-hsh);

        window->Draw(str);
    }
}

void Client::event(const sf::Event e)
{
    if(game) game->event(e);
}
