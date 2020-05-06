#include "utils.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

extern int strins(char *buf, char *ins, int pos)
{
	int len;
	int buf_len = strlen(buf) + strlen(ins) + 2;
	char *tmp;

	if(!(tmp = calloc(buf_len, sizeof(char))))
		return -1;

	strncpy(tmp, buf, pos);
	len = strlen(tmp);
	strcpy(tmp + len, ins);
	len += strlen(ins);
	strcpy(buf + len, buf + pos);

	strcpy(buf, tmp);
	free(tmp);
	return 0;
}

extern char *get_bin_dir(char *pth)
{
	char path_save[512];
	char *p;
	char *ret_buf;
	char exe_dir[512];

	if(!(p = strrchr(pth, '/'))) {
		if(!getcwd(exe_dir, sizeof(exe_dir)))
			return NULL;
	}
	else {
		*p = '\0';

		if(!getcwd(path_save, sizeof(path_save)))
			return NULL;

		if(chdir(pth) < 0)
			return NULL;

		if(!getcwd(exe_dir, sizeof(exe_dir)))
			return NULL;

		if(chdir(path_save) < 0) {
			return NULL;
		}
	}
	
	if(!(ret_buf = malloc(strlen(exe_dir) + 1)))
		return NULL;

	strcpy(ret_buf, exe_dir);
	return ret_buf;
}

void join_paths(char *dst, char *pth1, char *pth2)
{
	int l;
	char *rm, *fn;
	char *dst_buf;

	if(!(dst_buf = malloc(strlen(pth1) + strlen(pth2) + 1)))
		return;

	if(!pth1 && !pth2) {
		strcpy(dst, "");
	}
	else if(!pth2 || strlen(pth2) == 0) {
		strcpy(dst, pth1);
	}
	else if(!pth1 || strlen(pth1) == 0) {
		strcpy(dst, pth2);
	}
	else {
		char directory_separator[] = "/";
#ifdef WIN32
		directory_separator[0] = '\\';
#endif
		int append_directory_separator = 0;
		char *last_char;

		last_char = pth1;
		while(*last_char != '\0') {
			last_char++;
		}
		if(strcmp(last_char, directory_separator) != 0) {
			append_directory_separator = 1;
		}
		strcpy(dst_buf, pth1);
		if(append_directory_separator) {
			strcat(dst_buf, directory_separator);
		}
		strcat(dst_buf, pth2);
	}

	while((rm = strstr (dst_buf, "/../")) != NULL) {
		for(fn = (rm - 1); fn >= dst_buf; fn--) {
			if(*fn == '/') {
				l = strlen(rm + 4);
				memcpy(fn + 1, rm + 4, l);
				*(fn + 1 + l) = 0;
				break;
			}
		}
	}

	strcpy(dst, dst_buf);
	free(dst_buf);
}

extern SDL_Surface *crop_surf(SDL_Surface* in, SDL_Rect *in_rect, 
		SDL_Rect *out_rect)
{
	SDL_Surface* out = SDL_CreateRGBSurface(in->flags,
			out_rect->w, out_rect->h,
			in->format->BitsPerPixel,
			in->format->Rmask,
			in->format->Gmask,
			in->format->Bmask,
			in->format->Amask);

	SDL_BlitSurface(in, in_rect, out, out_rect);
	return out;
}
