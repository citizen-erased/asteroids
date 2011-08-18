#include "precompiled.h"
#include "audio.h"

typedef boost::shared_ptr<sf::Music> MusicPtr;
typedef boost::shared_ptr<sf::Sound> SoundPtr;
typedef boost::shared_ptr<sf::SoundBuffer> SoundBufferPtr;

namespace
{
std::map<std::string, SoundBufferPtr> buffers;
std::vector<SoundPtr> sounds;
MusicPtr music;

const SoundBufferPtr getSoundBuffer(const std::string &filename)
{
    if(buffers.find(filename) != buffers.end())
        return buffers[filename];

    SoundBufferPtr sb(new sf::SoundBuffer());
    buffers[filename] = sb;
    sb->LoadFromFile(filename);
    return sb;
}

} //end anonymous namespace

void Audio::playSFX(const std::string &filename)
{
    Audio::playSFX(filename, true, cml::vector2f(0.0f, 0.0f), 1.0f, 1.0f);
}

void Audio::playSFX(const std::string &filename, bool relative, const cml::vector2f &pos, float attenuation, float min_dist)
{
    if(sounds.empty())
        for(int i = 0; i < 32; i++)
        {
            SoundPtr s(new sf::Sound());
            s->SetVolume(60.0f);
            sounds.push_back(s);
        }


    SoundPtr s;

    for(unsigned int i = 0; i < sounds.size(); i++)
        if(sounds[i]->GetStatus() == sf::Sound::Stopped)
            s = sounds[i];

    if(!s)
    {
        warning("too many sounds");
        return;
    }
    
    SoundBufferPtr sb = getSoundBuffer(filename);
    s->SetRelativeToListener(relative);
    s->SetPosition(pos[0], pos[1], 0.0f);
    s->SetAttenuation(attenuation);
    s->SetMinDistance(min_dist);
    s->SetBuffer(*(sb.get()));
    s->Play();
}

void Audio::playMusic(const std::string &filename)
{
    if(!music)
        music = MusicPtr(new sf::Music());

    print(format("playing music %1%\n") % filename);
    music->OpenFromFile(filename);
    music->Play();
    print(format("music playing\n"));
}

bool Audio::isMusicPlaying()
{
    return music && music->GetStatus() == sf::Music::Playing;
}

float Audio::distToListener(const cml::vector2f &pos)
{
    sf::Vector3f lpos3 = sf::Listener::GetPosition();
    cml::vector2f lpos(lpos3.x, lpos3.y);

    return cml::length(pos - lpos);
}

