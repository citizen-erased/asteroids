#ifndef AST_NETWORK_H
#define AST_NETWORK_H

namespace NetChannel
{
enum
{
    /*
     * Used for (non-game world) client server communication such as handshaking
     * and game changes.
     */
    //RELIABLE,

    /*
     * Used for any data that doesn't have to read its destination and doesn't have
     * to be in order. Used for game world messages and deltas.
     */
    UNRELIABLE,

    /*
     * Used for player chat only.
     */
    CHAT,

    NUM_CHANNELS
};
}

#endif /* AST_NETWORK_H */

