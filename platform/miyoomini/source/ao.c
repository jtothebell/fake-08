#include <stdio.h>
#include <unistd.h>
#include <stdint.h>
#include <pthread.h>
#include <linux/input.h>
#include <SDL/SDL.h>
#if UNIONTOOLCHAIN
#include <mi_sys.h>
#include <mi_ao.h>
#else
#include <sdkdir/mi_sys.h>
#include <sdkdir/mi_ao.h>
#endif

#define	NUM_FRAMES		2	// 512(samples)*2(16bit) *2 = 2048 bytes
#define	USLEEP_MIN		0	// Threshold time for reading ahead without usleep
					//	when the time remaining until the next frame is less
					// (usleep resolution is 10000us(10ms) in the case of miyoomini)

pthread_t 	audiothread_pt;
pthread_mutex_t	audiolock_mx;
uint32_t	audiothread_cancel;
SDL_AudioSpec	audiospec;
uint8_t		*audio_buffer;
int		audio_paused;

//
//	AO Audio playback thread
//
void* audiothread(void* param) {
	MI_AUDIO_Frame_t	AoSendFrame;
	struct timeval tod;
	int usleepclock;
	uint64_t startclock, clock_freqframes;
	uint32_t i, framecounter;

	void (*fill)(void*, uint8_t*, int) = audiospec.callback;
	void *udata = audiospec.userdata;

	memset(&AoSendFrame, 0, sizeof(AoSendFrame));
	AoSendFrame.apVirAddr[0] = audio_buffer;
	AoSendFrame.u32Len = audiospec.size;
	memset(audio_buffer, 0, audiospec.size);

	// Buffer initial frames
	for (i=NUM_FRAMES; i>0; i--) {
		MI_AO_SendFrame(0, 0, &AoSendFrame, 0);
	}

	clock_freqframes = audiospec.samples * 1000000;
	framecounter = 0;
	gettimeofday(&tod, NULL);
	startclock = tod.tv_usec + tod.tv_sec * 1000000;

	while(!audiothread_cancel) {
		// Wait until next frame
		framecounter++;
		if (framecounter == (uint32_t)audiospec.freq) {
			framecounter = 0;
			startclock += clock_freqframes;
		}
		gettimeofday(&tod, NULL);
		usleepclock = framecounter * clock_freqframes / audiospec.freq
				 + startclock - (tod.tv_usec + tod.tv_sec * 1000000);
		if (usleepclock > USLEEP_MIN) usleep(usleepclock - USLEEP_MIN);

		// Request filling audio_buffer to callback function
		if (!audio_paused) {
			pthread_mutex_lock(&audiolock_mx);
			(*fill)(udata, audio_buffer, audiospec.size);
			pthread_mutex_unlock(&audiolock_mx);
		}

		// Playback
		MI_AO_SendFrame(0, 0, &AoSendFrame, 0);

		// Clear Buffer , per SDL1.2 spec (SDL2 does not clear)
		memset(audio_buffer, 0,audiospec.size);
	}

	return 0;
}

//
//	Open AO Audio in place of SDL_OpenAudio
//		supports signed 16bit only
//
void AO_OpenAudio(SDL_AudioSpec *spec){
	MI_AUDIO_Attr_t	attr;

	memcpy(&audiospec, spec, sizeof(audiospec));
	memset(&attr, 0, sizeof(attr));
	attr.eSamplerate = (MI_AUDIO_SampleRate_e)audiospec.freq;
	attr.eSoundmode = (MI_AUDIO_SoundMode_e)(audiospec.channels - 1);
	attr.u32ChnCnt = audiospec.channels;
	attr.u32PtNumPerFrm = audiospec.samples;

	audiospec.size = audiospec.samples * audiospec.channels * 2;
	audio_buffer = (uint8_t*)malloc(audiospec.size);

	MI_AO_SetPubAttr(0,&attr);
	MI_AO_Enable(0);
	MI_AO_EnableChn(0,0);

	MI_AO_SetVolume(0,0);
	MI_AO_ClearChnBuf(0,0);

	audiolock_mx = (pthread_mutex_t)PTHREAD_MUTEX_INITIALIZER;

	audio_paused = 1;
	audiothread_cancel = 0;
	pthread_create(&audiothread_pt, NULL, &audiothread, NULL);
}

//
//	Close AO Audio in place of SDL_CloseAudio
//
void AO_CloseAudio(void){

	audiothread_cancel = 1;
	pthread_join(audiothread_pt, NULL);

	MI_AO_ClearChnBuf(0,0);
	MI_AO_DisableChn(0,0);
	MI_AO_Disable(0);

	free(audio_buffer);
}

//
//	Pause AO Audio in place of SDL_PauseAudio
//
void AO_PauseAudio(int pause_on){
	audio_paused = pause_on;
	// unnecessary if buffer clearing is of SDL1.2 spec
	// MI_AO_SetMute(0, (pause_on ? TRUE : FALSE));
}

//
//	Lock Audio Mutex in place of SDL_LockAudio
//
void AO_LockAudio(void){
	pthread_mutex_lock(&audiolock_mx);
}

//
//	Unlock Audio Mutex in place of SDL_UnlockAudio
//
void AO_UnlockAudio(void){
	pthread_mutex_unlock(&audiolock_mx);
}
