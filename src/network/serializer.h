#ifndef AST_SERIALIZER_H
#define AST_SERIALIZER_H

#include <SFML/Network.hpp>
typedef sf::Packet Serializer;

#if 0
class Serializer
{
private:
    std::vector<char> data;
    unsigned int read_pos;

public:
    Serializer(){read_pos = 0;};

    const char* getDataPtr() const{return data.empty() ? NULL : &data[0];};
    unsigned int getSize() const{return data.size(); };

    template<class T> bool put(const T &d);
    template<class T> bool get();
};

template<class T> bool Serializer::put(const T &d)
{
    if(sizeof(d) > 0)
    {
        unsigned int pos = data.size();
        data.resize(pos + sizeof(d));
        memcpy(&data[pos], &d, sizeof(d));
        return true;
    }

    return false;
}

template<class T> bool Serializer::get()
{
    if(read_pos + sizeof(T) < data.size())
    {
        T d = *(reinterpret_cast<T*>(&data[read_pos]));
        read_pos += sizeof(T);
        return d;
    }

    return T();
}
#endif

#endif /* AST_SERIALIZER_H */

