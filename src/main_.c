#include <stdio.h>
#include <stdint.h>
#include <assert.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_opengl.h>
#include <GL/gl.h>

/* Window flags */
uint32_t win_flags = SDL_WINDOW_OPENGL;
/* Screen dimension */
int scn_w = 640;
int scn_h = 480;
SDL_Window *win;
/* OpenGL context */
SDL_GLContext ctx;

char running = 0;
int32_t fullscreen = 0;

/* Define prototypes */
void handle_input();

int main(int argc, char **argv)
{
	printf("Start game:\n");
	
	if(SDL_Init(SDL_INIT_VIDEO) < 0) {
		printf("Failed to init SDL.\n (%s)", SDL_GetError());
		goto close;
	}	

	win = SDL_CreateWindow("Vasall", 
			SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, 
			scn_w, scn_h, 
			win_flags);

	ctx = SDL_GL_CreateContext(win);

	running = 1;

	while (running) {
		handle_input();

		glViewport(0, 0, scn_w, scn_h);
		glClearColor(0.09f, 0.09f, 0.09f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT);

		glBegin(GL_QUADS);
        	glColor3d(1,1,1);
        	glVertex3f(-0.5,-0.5,-10);
        	glColor3d(1,1,0);
        	glVertex3f(0.5,-0.5,-10);
        	glColor3d(1,1,1);
        	glVertex3f(0.5,0.5,-10);
        	glColor3d(0,1,1);
        	glVertex3f(-0.5,0.5,-10);
    		glEnd();

		SDL_GL_SwapWindow(win);
	}

close:
	return (0);
}


void handle_input()
{
	SDL_Event evt;
	while (SDL_PollEvent(&evt)) {
		if (evt.type == SDL_KEYDOWN) {
			switch (evt.key.keysym.sym) {
				case (SDLK_ESCAPE):
					running = 0;
					break;
				case ('f'):
					fullscreen = !fullscreen;
					if (fullscreen) {
						SDL_SetWindowFullscreen(win, win_flags | SDL_WINDOW_FULLSCREEN_DESKTOP);
					}
					else {
						SDL_SetWindowFullscreen(win, win_flags);
					}
					break;
				default:
					break;
			}
		}
		else if (evt.type == SDL_QUIT) {
			running = 0;
		}
	}
}
