#include "precompiled.h"
#include "misc.h"
#include "serializer.h"
#include "server/server_game.h"
#include "entity/asteroid.h"
#include "entity/ship.h"
#include "message.h"


/*------------------------------------------------------------------------------*
 *                                                                              *
 *                                 Message                                      *
 *                                                                              *
 *------------------------------------------------------------------------------*/

namespace MessageType
{

std::map<int, std::string> strings = boost::assign::map_list_of
    (CLI_INFO,              "CLI_INFO")
    (CLI_SHIP_CONTROL,      "CLI_SHIP_CONTROL")
    (SRV_GAMESTATE,         "SRV_GAMESTATE")
    (SRV_SHUTDOWN,          "SRV_SHUTDOWN")
    (SRV_ASTEROID_SPAWN,    "SRV_ASTEROID_SPAWN")
    (SRV_ASTEROID_STATE,    "SRV_ASTEROID_STATE")
    (SRV_SHIP_SPAWN,        "SRV_SHIP_SPAWN")
    (SRV_SHIP_STATE,        "SRV_SHIP_STATE")
    (SRV_SHIP_OWNER,        "SRV_SHIP_OWNER")
    (SRV_BULLET_SPAWN,      "SRV_BULLET_SPAWN")
    (SRV_BULLET_STATE,      "SRV_BULLET_STATE")
    (SRV_ENTITY_DELETE,     "SRV_ENTITY_DELETE")
    (SRV_SCORE,             "SRV_SCORE");

const std::string& getString(Enum type)
{
    return strings[type];
}

}

void Message::pack(MessagePtr msg, Serializer &data)
{
    data << msg->getType();
    msg->pack(data);
}

MessagePtr Message::unpack(Serializer &data)
{
    MessagePtr msg;

    int msg_type;
    data >> msg_type;

    switch(msg_type)
    {
    case MessageType::SRV_GAMESTATE:
        msg = MessagePtr(new GameStateMessage(data));
        break;
    case MessageType::SRV_ASTEROID_SPAWN:
        msg = MessagePtr(new AsteroidSpawnMessage(data));
        break;
    case MessageType::SRV_ASTEROID_STATE:
        msg = MessagePtr(new AsteroidStateMessage(data));
        break;
    case MessageType::SRV_SHIP_SPAWN:
        msg = MessagePtr(new ShipSpawnMessage(data));
        break;
    case MessageType::SRV_SHIP_STATE:
        msg = MessagePtr(new ShipStateMessage(data));
        break;
    case MessageType::SRV_SHIP_OWNER:
        msg = MessagePtr(new ShipOwnerMessage(data));
        break;
    case MessageType::SRV_BULLET_SPAWN:
        msg = MessagePtr(new BulletSpawnMessage(data));
        break;
    case MessageType::SRV_BULLET_STATE:
        msg = MessagePtr(new BulletStateMessage(data));
        break;
    case MessageType::SRV_ENTITY_DELETE:
        msg = MessagePtr(new EntityDeleteMessage(data));
        break;
    case MessageType::SRV_SCORE:
        msg = MessagePtr(new ScoreMessage(data));
        break;
    case MessageType::CLI_INFO:
        msg = MessagePtr(new ClientInfoMessage(data));
        break;
    case MessageType::CLI_SHIP_CONTROL:
        msg = MessagePtr(new ShipControlMessage(data));
        break;
    }

    return msg;
}

std::ostream& operator<<(std::ostream &o, const Message &m)
{
    m.print(o);
    return o;
}




/*------------------------------------------------------------------------------*
 *                                                                              *
 *                               EntityMessage                                  *
 *                                                                              *
 *------------------------------------------------------------------------------*/
EntityMessage::EntityMessage(int entity_id)
{
    this->entity_id = entity_id;
}

EntityMessage::EntityMessage(Serializer &in)
{
    in >> entity_id;
}

void EntityMessage::pack(Serializer &out) const
{
    out << entity_id;
}

int EntityMessage::getEntityID() const
{
    return entity_id;
}



/*------------------------------------------------------------------------------*
 *                                                                              *
 *                           ClientInfoMessage                                  *
 *                                                                              *
 *------------------------------------------------------------------------------*/
Serializer& operator<<(Serializer &out, const ClientInfoData &d)
{
    out << d.name;
    return out;
}

Serializer& operator>>(Serializer &in, ClientInfoData &d)
{
    in >> d.name;
    return in;
}

std::ostream& operator<<(std::ostream &o, const ClientInfoData &d)
{
    return o << d.name;
}



/*------------------------------------------------------------------------------*
 *                                                                              *
 *                             Game State Message                               *
 *                                                                              *
 *------------------------------------------------------------------------------*/
Serializer& operator<<(Serializer &out, const GameStateData &s)
{
    out << s.game_bounds.getP0()[0];
    out << s.game_bounds.getP0()[1];
    out << s.game_bounds.getP1()[0];
    out << s.game_bounds.getP1()[1];
    return out;
}

Serializer& operator>>(Serializer &in, GameStateData &s)
{
    float p00, p01, p10, p11;
    in >> p00;
    in >> p01;
    in >> p10;
    in >> p11;
    s.game_bounds.set(cml::vector2f(p00, p01), cml::vector2f(p10, p11));
    return in;
}

std::ostream& operator<<(std::ostream& o, const GameStateData &s)
{
    //TODO
    return o;
}


/*------------------------------------------------------------------------------*
 *                                                                              *
 *                            Entity Delete Message                             *
 *                                                                              *
 *------------------------------------------------------------------------------*/
Serializer& operator<<(Serializer &out, const EntityDeleteData &d)
{
    return out << d.entity_id;
}

Serializer& operator>>(Serializer &in, EntityDeleteData &d)
{
    in >> d.entity_id;
    return in;
}

std::ostream& operator<<(std::ostream &o, const EntityDeleteData &d)
{
    return o << d.entity_id;
}


/*------------------------------------------------------------------------------*
 *                                                                              *
 *                                Ship Messages                                 *
 *                                                                              *
 *------------------------------------------------------------------------------*/

/*------------------------------------------------------------------------------*
 *                                ShipStateData                                 *
 *------------------------------------------------------------------------------*/
Serializer& operator<<(Serializer &out, const ShipStateData &s)
{
    out << s.position[0];
    out << s.position[1];
    out << s.velocity[0];
    out << s.velocity[1];
    out << s.rotation;
    out << s.angular_velocity;
    out << s.health;
    return out;
}

Serializer& operator>>(Serializer &in, ShipStateData &s)
{
    in >> s.position[0];
    in >> s.position[1];
    in >> s.velocity[0];
    in >> s.velocity[1];
    in >> s.rotation;
    in >> s.angular_velocity;
    in >> s.health;
    return in;
}

std::ostream& operator<<(std::ostream& o, const ShipStateData &s)
{
    return o << format("%1% %2% %3% %4% %5%") % s.position % s.velocity % s.rotation % s.angular_velocity % s.health;
}


/*------------------------------------------------------------------------------*
 *                                ShipSpawnData                                 *
 *------------------------------------------------------------------------------*/
Serializer& operator<<(Serializer &out, const ShipSpawnData &s)
{
    out << s.id;
    out << s.player_id;
    out << s.name;
    out << s.body_color[0] << s.body_color[1] << s.body_color[2] << s.body_color[3];
    out << s.state;
    return out;
}

Serializer& operator>>(Serializer &in, ShipSpawnData &s)
{
    in >> s.id;
    in >> s.player_id;
    in >> s.name;
    in >> s.body_color[0] >> s.body_color[1] >> s.body_color[2] >> s.body_color[3];
    in >> s.state;
    return in;
}

std::ostream& operator<<(std::ostream& o, const ShipSpawnData &s)
{
    //TODO
    return o << s.id << s.state;
}


/*------------------------------------------------------------------------------*
 *                                ShipControlData                                 *
 *------------------------------------------------------------------------------*/
Serializer& operator<<(Serializer &out, const ShipControlData &s)
{
    out << s.forward;
    out << s.reverse;
    out << s.left;
    out << s.right;
    out << s.shooting;
    return out;
}

Serializer& operator>>(Serializer &in, ShipControlData &s)
{
    in >> s.forward;
    in >> s.reverse;
    in >> s.left;
    in >> s.right;
    in >> s.shooting;
    return in;
}

std::ostream& operator<<(std::ostream& o, const ShipControlData &s)
{
    return o << format("forward=%1% reverse=%2% right=%3% left=%4% shooting=%5%") % s.forward % s.reverse % s.right % s.left % s.shooting;
}


/*------------------------------------------------------------------------------*
 *                                ShipOwnerData                                 *
 *------------------------------------------------------------------------------*/
Serializer& operator<<(Serializer &out, const ShipOwnerData &d)
{
    out << d.ship_id;
    return out;
}

Serializer& operator>>(Serializer &in, ShipOwnerData &d)
{
    in >> d.ship_id;
    return in;
}

std::ostream& operator<<(std::ostream &o, const ShipOwnerData &d)
{
    return o << d.ship_id;
}



/*------------------------------------------------------------------------------*
 *                                                                              *
 *                              Asteroid Messages                               *
 *                                                                              *
 *------------------------------------------------------------------------------*/

/*------------------------------------------------------------------------------*
 *                              AsteroidStateData                               *
 *------------------------------------------------------------------------------*/
Serializer& operator<<(Serializer &out, const AsteroidStateData &s)
{
    out << s.position[0];
    out << s.position[1];
    out << s.velocity[0];
    out << s.velocity[1];
    out << s.rotation;
    out << s.angular_velocity;
    out << s.health;
    return out;
}

Serializer& operator>>(Serializer &in, AsteroidStateData &s)
{
    in >> s.position[0];
    in >> s.position[1];
    in >> s.velocity[0];
    in >> s.velocity[1];
    in >> s.rotation;
    in >> s.angular_velocity;
    in >> s.health;
    return in;
}

std::ostream& operator<<(std::ostream& o, const AsteroidStateData &s)
{
    //TODO
    return o << s.position << s.velocity << s.rotation;
}


/*------------------------------------------------------------------------------*
 *                              AsteroidSpawnData                               *
 *------------------------------------------------------------------------------*/
Serializer& operator<<(Serializer &out, const AsteroidSpawnData &s)
{
    out << s.id;
    out << s.asteroid_type;
    out << s.state;
    return out;
}

Serializer& operator>>(Serializer &in, AsteroidSpawnData &s)
{
    in >> s.id;
    in >> s.asteroid_type;
    in >> s.state;
    return in;
}

std::ostream& operator<<(std::ostream& o, const AsteroidSpawnData &s)
{
    //TODO
    return o << s.id << s.asteroid_type << s.state;
}



/*------------------------------------------------------------------------------*
 *                                                                              *
 *                              Bullet Messages                                 *
 *                                                                              *
 *------------------------------------------------------------------------------*/
 
/*------------------------------------------------------------------------------*
 *                              BulletStateData                                 *
 *------------------------------------------------------------------------------*/
Serializer& operator<<(Serializer &out, const BulletStateData &d)
{
    out << d.position[0];
    out << d.position[1];
    out << d.velocity[0];
    out << d.velocity[1];
    out << d.rotation;
    return out;
}

Serializer& operator>>(Serializer &in, BulletStateData &d)
{
    in >> d.position[0];
    in >> d.position[1];
    in >> d.velocity[0];
    in >> d.velocity[1];
    in >> d.rotation;
    return in;
}

std::ostream& operator<<(std::ostream &o, const BulletStateData &d)
{
    //TODO
    return o << d.position << d.velocity << d.rotation;
}



/*------------------------------------------------------------------------------*
 *                                BulletSpawnData                               *
 *------------------------------------------------------------------------------*/
Serializer& operator<<(Serializer &out, const BulletSpawnData &d)
{
    out << d.id;
    out << d.shooter_id;
    out << d.state;
    return out;
}

Serializer& operator>>(Serializer &in, BulletSpawnData &d)
{
    in >> d.id;
    in >> d.shooter_id;
    in >> d.state;
    return in;
}

std::ostream& operator<<(std::ostream &o, const BulletSpawnData &d)
{
    //TODO
    return o << d.state;
}

/*------------------------------------------------------------------------------*
 *                                  ScoreData                                   *
 *------------------------------------------------------------------------------*/
Serializer& operator<<(Serializer &out, const ScoreData &d)
{
    out << d.names.size();
    for(unsigned int i = 0; i < d.names.size(); i++)
        out << d.names[i] << d.scores[i];
    return out;
}

Serializer& operator>>(Serializer &in, ScoreData &d)
{
    unsigned int size;
    in >> size;
    d.names.resize(size);
    d.scores.resize(size);
    for(unsigned int i = 0; i < d.names.size(); i++)
        in >> d.names[i] >> d.scores[i];
    return in;
}

std::ostream& operator<<(std::ostream &o, const ScoreData &d)
{
    //TODO
    return o;
}



/*------------------------------------------------------------------------------*
 *                                                                              *
 *                              Message Buffer                                  *
 *                                                                              *
 *------------------------------------------------------------------------------*/

MessageBuffer::MessageBuffer(Serializer &data)
{
    unsigned int num_entries;
    data >> num_entries;

    for(unsigned int i = 0; i < num_entries; i++)
    {
        EntryPtr e(new Entry());
        data >> e->tick;

        unsigned int num_msgs;
        data >> num_msgs;
        
        for(unsigned int j = 0; j < num_msgs; j++)
        {
            MessagePtr msg = Message::unpack(data);
            if(msg) e->msgs.push_back(msg);
        }

        entries.push_back(e);
    }
}

void MessageBuffer::addMessage(unsigned int tick, MessagePtr msg)
{
    for(unsigned int i = 0; i < entries.size(); i++)
        if(entries[i]->tick == tick)
        {
            entries[i]->msgs.push_back(msg);
            return;
        }

    EntryPtr e(new Entry());
    e->tick = tick;
    e->msgs.push_back(msg);

    //keep entries sorted
    entries.push_back(e);
    std::sort(entries.begin(), entries.end());
}

void MessageBuffer::addMessages(unsigned int tick, const std::vector<MessagePtr> &vec)
{
    for(unsigned int i = 0; i < vec.size(); i++)
        addMessage(tick, vec[i]);
}

void MessageBuffer::addBuffer(const MessageBuffer &buf)
{
    for(unsigned int i = 0; i < buf.entries.size(); i++)
    {
        const EntryPtr &entry = buf.entries[i];
        for(unsigned int j = 0; j < entry->msgs.size(); j++)
            addMessage(entry->tick, entry->msgs[j]);
    }
}

void MessageBuffer::getMessages(std::vector<MessagePtr> *out) const
{
    for(unsigned int i = 0; i < entries.size(); i++)
        out->insert(out->end(), entries[i]->msgs.begin(), entries[i]->msgs.end());
}

void MessageBuffer::getMessages(unsigned int tick, std::vector<MessagePtr> *out) const
{
    for(unsigned int i = 0; i < entries.size(); i++)
        if(compareTicks(entries[i]->tick, tick))
            out->insert(out->end(), entries[i]->msgs.begin(), entries[i]->msgs.end());
}

void MessageBuffer::removeMessages(unsigned int tick)
{
    std::vector<EntryPtr> old_entries = entries;
    entries.clear();

    for(unsigned int i = 0; i < old_entries.size(); i++)
        if(compareTicks(old_entries[i]->tick, tick))
            entries.push_back(old_entries[i]);
}

bool MessageBuffer::empty() const
{
    return entries.empty();
}

void MessageBuffer::pack(Serializer &data) const
{
    data << entries.size();

    for(unsigned int i = 0; i < entries.size(); i++)
    {
        const EntryPtr &e = entries[i];

        data << e->tick;
        data << e->msgs.size();
        
        for(unsigned int j = 0; j < e->msgs.size(); j++)
            Message::pack(e->msgs[j], data);
    }
}

std::ostream& operator<<(std::ostream &o, const MessageBuffer &mb)
{
    o << format("%1% entries\n") % mb.entries.size();
    for(unsigned int i = 0; i < mb.entries.size(); i++)
    {
        const MessageBuffer::EntryPtr &e = mb.entries[i];
        o << format("Entry:%1% tick=%2%\n") % i % e->tick;

        for(unsigned int j = 0; j < e->msgs.size(); j++)
        {
            o << format("Message %1%: %2%\n") % j % *e->msgs[j];
        }
    }
    return o;
}




/*------------------------------------------------------------------------------*
 *                                                                              *
 *                                  Packet                                      *
 *                                                                              *
 *------------------------------------------------------------------------------*/

Packet::Packet(unsigned int tick, unsigned int ack_tick, const MessageBuffer &buf)
{
    this->tick = tick;
    this->ack_tick = ack_tick;
    this->msg_buffer = buf;
}

Packet::Packet(Serializer &data)
{
    data >> tick;
    data >> ack_tick;
    msg_buffer = MessageBuffer(data);
}

/*
void Packet::addMessage(unsigned int tick, MessagePtr m)
{
    msg_buffer.addMessage(tick, m);
}
*/

void Packet::getMessages(std::vector<MessagePtr> *msg_list) const
{
    return msg_buffer.getMessages(msg_list);
}

void Packet::getMessages(unsigned int tick, std::vector<MessagePtr> *msg_list) const
{
    return msg_buffer.getMessages(tick, msg_list);
}

unsigned int Packet::getTick() const
{
    return tick;
}

unsigned int Packet::getAckTick() const
{
    return ack_tick;
}

void Packet::pack(Serializer &data) const
{
    data << tick;
    data << ack_tick;
    msg_buffer.pack(data);
}

std::ostream& operator<<(std::ostream &o, const Packet &p)
{
    o << format("tick=%1% ack_tick=%2%\n") % p.tick % p.ack_tick;
    o << p.msg_buffer;

    return o;
}

