#ifndef AST_AUDIO_H
#define AST_AUDIO_H

#include <string>

namespace Audio
{
void playSFX(const std::string &filename);
void playSFX(const std::string &filename, bool relative, const cml::vector2f &pos, float atteuation, float min_dist);
void playMusic(const std::string &filename);
bool isMusicPlaying();

float distToListener(const cml::vector2f &pos);
}

#endif /* AST_AUDIO_H */

