#ifndef AST_MESSAGE_H
#define AST_MESSAGE_H

#include "types.h"
#include "aabb.h"
#include "misc.h"
#include "serializer.h"

class ostream;

/*------------------------------------------------------------------------------*
 *                                                                              *
 *                              Message Types                                   *
 *                                                                              *
 *------------------------------------------------------------------------------*/
namespace MessageType
{
enum Enum
{
    /*
     * Messages from the client.
     */
    CLI_INFO,
    CLI_SHIP_CONTROL,


    /*
     * Messages from the server.
     */
    SRV_GAMESTATE,
    SRV_SHUTDOWN,

    SRV_ASTEROID_SPAWN,
    SRV_ASTEROID_STATE,
    SRV_SHIP_SPAWN,
    SRV_SHIP_STATE,
    SRV_SHIP_OWNER,
    SRV_BULLET_SPAWN,
    SRV_BULLET_STATE,

    SRV_ENTITY_DELETE,

    SRV_SCORE,
};

const std::string& getString(Enum type);
}





/*------------------------------------------------------------------------------*
 *                                                                              *
 *                                 Message                                      *
 *                                                                              *
 *------------------------------------------------------------------------------*/
/*
 * Message is an immutable class. All subclasses should probably be immutable too.
 */
class Message
{
public:
    static void             pack(MessagePtr msg, Serializer &data);
    static MessagePtr       unpack(Serializer &data);

public:
    virtual                ~Message(){};

    virtual void            pack(Serializer &data) const = 0;
    virtual MessageType::Enum getType() const = 0;

    virtual void            print(std::ostream&) const{}
    friend std::ostream& operator<<(std::ostream &o, const Message &m);
};



/*
 * A message meant for a specific entity
 */
class EntityMessage : public Message
{
private:
    int entity_id;

public:
    EntityMessage(int entity_id);
    EntityMessage(Serializer &in);

    int getEntityID() const;

    virtual void pack(Serializer &out) const;
};



/*------------------------------------------------------------------------------*
 *                                                                              *
 *                            GenericDataMessage                                *
 *                                                                              *
 *------------------------------------------------------------------------------*/
template<class DATA_T, MessageType::Enum MESSAGE_T> class GenericDataMessage : public Message
{
protected:
    DATA_T data;

public:
    GenericDataMessage(const DATA_T &data)
    {
        this->data = data;
    }

    GenericDataMessage(Serializer &in)
    {
        in >> data;
    }

    void pack(Serializer &out) const
    {
        out << data;
    }

    const DATA_T& getData() const
    {
        return data;
    }

    MessageType::Enum getType() const
    {
        return MESSAGE_T;
    }

    void print(std::ostream &o) const
    {
        o << format("type=%1%, %2%") % MessageType::getString(MESSAGE_T) % data;
    }
};




/*------------------------------------------------------------------------------*
 *                                                                              *
 *                          GenericEntityDataMessage                            *
 *                                                                              *
 *------------------------------------------------------------------------------*/
template<class DATA_T, MessageType::Enum MESSAGE_T> class GenericEntityDataMessage : public EntityMessage
{
private:
    DATA_T data;

public:
    GenericEntityDataMessage(int entity_id, const DATA_T &data)
    : EntityMessage(entity_id)
    {
        this->data = data;
    }

    GenericEntityDataMessage(Serializer &in)
    : EntityMessage(in)
    {
        in >> data;
    }

    void pack(Serializer &out) const
    {
        EntityMessage::pack(out);
        out << data;
    }

    const DATA_T& getData() const
    {
        return data;
    }

    MessageType::Enum getType() const
    {
        return MESSAGE_T;
    }

    void print(std::ostream &o) const
    {
        o << format("type=%1%, entity_id=%2% %3%") % MessageType::getString(MESSAGE_T) % getEntityID() % data;
    }
};



/*------------------------------------------------------------------------------*
 *                                                                              *
 *                              ClientInfoMessage                               *
 *                                                                              *
 *------------------------------------------------------------------------------*/
struct ClientInfoData
{
    std::string name;
};

Serializer&   operator<<(Serializer   &out, const ClientInfoData &d);
Serializer&   operator>>(Serializer   &in,        ClientInfoData &d);
std::ostream& operator<<(std::ostream &o,   const ClientInfoData &d);

typedef GenericDataMessage<ClientInfoData, MessageType::CLI_INFO> ClientInfoMessage;



/*------------------------------------------------------------------------------*
 *                                                                              *
 *                             Game State Message                               *
 *                                                                              *
 *------------------------------------------------------------------------------*/
struct GameStateData
{
    AABB game_bounds;
};

Serializer&   operator<<(Serializer   &out, const GameStateData &d);
Serializer&   operator>>(Serializer   &in,        GameStateData &d);
std::ostream& operator<<(std::ostream &o,   const GameStateData &d);

typedef GenericDataMessage<GameStateData, MessageType::SRV_GAMESTATE> GameStateMessage;



/*------------------------------------------------------------------------------*
 *                                                                              *
 *                            Entity Delete Message                             *
 *                                                                              *
 *------------------------------------------------------------------------------*/
struct EntityDeleteData
{
    int entity_id;
};

Serializer&   operator<<(Serializer   &out, const EntityDeleteData &d);
Serializer&   operator>>(Serializer   &in,        EntityDeleteData &d);
std::ostream& operator<<(std::ostream &o,   const EntityDeleteData &d);

typedef GenericDataMessage<EntityDeleteData, MessageType::SRV_ENTITY_DELETE> EntityDeleteMessage;



/*------------------------------------------------------------------------------*
 *                                                                              *
 *                                Ship Messages                                 *
 *                                                                              *
 *------------------------------------------------------------------------------*/
struct ShipStateData
{
    cml::vector2f   position;
    cml::vector2f   velocity;
    float           rotation;
    float           angular_velocity;
    int             health;
};

Serializer&   operator<<(Serializer   &out, const ShipStateData &d);
Serializer&   operator>>(Serializer   &in,        ShipStateData &d);
std::ostream& operator<<(std::ostream &o,   const ShipStateData &d);


struct ShipSpawnData
{
    int           id;
    int           player_id;
    cml::vector4f body_color;
    std::string   name;
    ShipStateData state;
};

Serializer&   operator<<(Serializer   &out, const ShipSpawnData &d);
Serializer&   operator>>(Serializer   &in,        ShipSpawnData &d);
std::ostream& operator<<(std::ostream &o,   const ShipSpawnData &d);


struct ShipControlData
{
    bool forward, reverse, left, right, shooting;
};

Serializer&   operator<<(Serializer   &out, const ShipControlData &d);
Serializer&   operator>>(Serializer   &in,        ShipControlData &d);
std::ostream& operator<<(std::ostream &o,   const ShipControlData &d);


struct ShipOwnerData
{
    int ship_id;
};

Serializer&   operator<<(Serializer   &out, const ShipOwnerData &d);
Serializer&   operator>>(Serializer   &in,        ShipOwnerData &d);
std::ostream& operator<<(std::ostream &o,   const ShipOwnerData &d);

typedef GenericDataMessage<ShipSpawnData, MessageType::SRV_SHIP_SPAWN> ShipSpawnMessage;
typedef GenericEntityDataMessage<ShipStateData, MessageType::SRV_SHIP_STATE> ShipStateMessage;
typedef GenericDataMessage<ShipControlData, MessageType::CLI_SHIP_CONTROL> ShipControlMessage;
typedef GenericDataMessage<ShipOwnerData, MessageType::SRV_SHIP_OWNER> ShipOwnerMessage;



/*------------------------------------------------------------------------------*
 *                                                                              *
 *                              Asteroid Messages                               *
 *                                                                              *
 *------------------------------------------------------------------------------*/
struct AsteroidStateData
{
    cml::vector2f   position;
    cml::vector2f   velocity;
    float           rotation;
    float           angular_velocity;
    int             health;
};

Serializer&   operator<<(Serializer   &out, const AsteroidStateData &d);
Serializer&   operator>>(Serializer   &in,        AsteroidStateData &d);
std::ostream& operator<<(std::ostream &o,   const AsteroidStateData &d);


struct AsteroidSpawnData
{
    int               id;
    int               asteroid_type;
    AsteroidStateData state;
};

Serializer&   operator<<(Serializer   &out, const AsteroidSpawnData &d);
Serializer&   operator>>(Serializer   &in,        AsteroidSpawnData &d);
std::ostream& operator<<(std::ostream &o,   const AsteroidSpawnData &d);


typedef GenericDataMessage<AsteroidSpawnData, MessageType::SRV_ASTEROID_SPAWN> AsteroidSpawnMessage;
typedef GenericEntityDataMessage<AsteroidStateData, MessageType::SRV_ASTEROID_STATE> AsteroidStateMessage;



/*------------------------------------------------------------------------------*
 *                                                                              *
 *                               Bullet Messages                                *
 *                                                                              *
 *------------------------------------------------------------------------------*/
struct BulletStateData
{
    cml::vector2f   position;
    cml::vector2f   velocity;
    float           rotation;
};

Serializer&   operator<<(Serializer   &out, const BulletStateData &d);
Serializer&   operator>>(Serializer   &in,        BulletStateData &d);
std::ostream& operator<<(std::ostream &o,   const BulletStateData &d);


struct BulletSpawnData
{
    int             id;
    int             shooter_id;
    BulletStateData state;
};

Serializer&   operator<<(Serializer   &out, const BulletSpawnData &d);
Serializer&   operator>>(Serializer   &in,        BulletSpawnData &d);
std::ostream& operator<<(std::ostream &o,   const BulletSpawnData &d);

typedef GenericDataMessage<BulletSpawnData, MessageType::SRV_BULLET_SPAWN> BulletSpawnMessage;
typedef GenericEntityDataMessage<BulletStateData, MessageType::SRV_BULLET_STATE> BulletStateMessage;


/*------------------------------------------------------------------------------*
 *                                                                              *
 *                                Score Message                                 *
 *                                                                              *
 *------------------------------------------------------------------------------*/
struct ScoreData
{
    std::vector<std::string> names;
    std::vector<int> scores;
};

Serializer&   operator<<(Serializer   &out, const ScoreData &d);
Serializer&   operator>>(Serializer   &in,        ScoreData &d);
std::ostream& operator<<(std::ostream &o,   const ScoreData &d);

typedef GenericDataMessage<ScoreData, MessageType::SRV_SCORE> ScoreMessage;




/*------------------------------------------------------------------------------*
 *                                                                              *
 *                              Message Buffer                                  *
 *                                                                              *
 *------------------------------------------------------------------------------*/
class MessageBuffer
{
private:
    struct Entry
    {
        unsigned int tick;
        std::vector<MessagePtr> msgs;

        bool operator<(const Entry &e)
        {
            return compareTicks(e.tick, tick);
        }
    };

    typedef boost::shared_ptr<Entry> EntryPtr;

private:
    //TODO store EntryPtr since we're sorting them
    std::vector<EntryPtr> entries;

public:
                            MessageBuffer(){};
                            MessageBuffer(Serializer &data);

    void                    addMessage(unsigned int tick, MessagePtr msg);
    void                    addMessages(unsigned int tick, const std::vector<MessagePtr> &vec);
    void                    addBuffer(const MessageBuffer &buf);

    void                    getMessages(std::vector<MessagePtr> *out) const;
    /*
     * Gets all messages > tick.
     */
    void                    getMessages(unsigned int tick, std::vector<MessagePtr> *out) const;
    /*
     * Removes all messages with a tick <= tick
     */
    void                    removeMessages(unsigned int tick);

    bool                    empty() const;
    void                    pack(Serializer &data) const;

    friend std::ostream& operator<<(std::ostream &o, const MessageBuffer &mb);
};




/*------------------------------------------------------------------------------*
 *                                                                              *
 *                                  Packet                                      *
 *                                                                              *
 *------------------------------------------------------------------------------*/
/*
 * Packet is an immutable class.
 */
class Packet
{
private:
    /*
     * ticks allow a host to know which messages have been confirmed as received
     * by peers. all unconfirmed messages are sent along with any new messages in
     * the next packet send out. peers then ignore any messages they've already
     * processed (messages with a tick that compares as happening before their last
     * received tick time).
     *
     * Message from server to client:
     * tick = server tick message was sent at
     * ack_tick = (client) tick from the most recent packet received by the server
     *
     * Message from client to server:
     * tick     = client tick message was sent at
     * ack_tick = (server) tick from the most recent packet received by the client
     */

    /* The tick on the client/server the packet was sent at */
    unsigned int tick;

    /* The tick of the last packet received by the client/server */
    unsigned int ack_tick;

    MessageBuffer msg_buffer;

public:
    Packet(unsigned int tick, unsigned int ack_tick, const MessageBuffer &buf);
    Packet(Serializer &data);

    //void addMessage(const MessagePtr &m);
    //void getMessages(std::vector<MessagePtr> *msg_list) const;
    //void addMessage(unsigned int tick, MessagePtr m);
    //void addMessageBuffer(const MessageBuffer &buf);

    void getMessages(std::vector<MessagePtr> *msg_list) const;
    void getMessages(unsigned int tick, std::vector<MessagePtr> *msg_list) const;

    unsigned int getTick() const;
    unsigned int getAckTick() const;

    void pack(Serializer &data) const;

    friend std::ostream& operator<<(std::ostream &o, const Packet &p);
};

#endif /* AST_MESSAGE_H */

