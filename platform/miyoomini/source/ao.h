#ifndef __AO_H__
#define __AO_H__

#include <SDL/SDL.h>

void	AO_OpenAudio(SDL_AudioSpec *spec);
void	AO_CloseAudio(void);
void	AO_PauseAudio(int pause_on);
void	AO_LockAudio(void);
void	AO_UnlockAudio(void);

#endif
