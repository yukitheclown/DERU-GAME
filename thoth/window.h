#ifndef WINDOW_DEF
#define WINDOW_DEF

// #define WINDOW_INIT_WIDTH 			300
// #define WINDOW_INIT_HEIGHT 			160
#ifndef LIBRARY_COMPILE
#define WINDOW_TITLE "zim"
#define WINDOW_INIT_WIDTH 			960
#define WINDOW_INIT_HEIGHT 			540
#include <SDL2/SDL_events.h>
void Window_Swap();
void Window_Close();
int Window_GetTicks();
int Window_Open();
void Window_PollEvent(void (*callback)(SDL_Event ev));
#endif


#endif