#include "utils.h"
#include "error.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

extern int strins(char *buf, char *ins, int pos)
{
	int len;
	int buf_len = strlen(buf) + strlen(ins) + 2;
	char *tmp;

	if(!(tmp = calloc(buf_len, sizeof(char)))) {
		ERR_LOG(("Failed to allocate memory"));
		return -1;
	}

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
		if(!getcwd(exe_dir, sizeof(exe_dir))) {
			ERR_LOG(("Failed to get working-directory"));
			return NULL;
		}
	}
	else {
		*p = '\0';

		if(!getcwd(path_save, sizeof(path_save))) {
			ERR_LOG(("Failed to get working-directory"));
			return NULL;
		}

		if(chdir(pth) < 0) {
			ERR_LOG(("Failed to change directory"));
			return NULL;
		}

		if(!getcwd(exe_dir, sizeof(exe_dir))) {
			ERR_LOG(("Failed to get working-directory"));
			return NULL;
		}

		if(chdir(path_save) < 0) {
			ERR_LOG(("Failed to change directoy"));
			return NULL;
		}
	}

	if(!(ret_buf = malloc(strlen(exe_dir) + 1))) {
		ERR_LOG(("Failed to allocate memory"));
		return NULL;
	}

	strcpy(ret_buf, exe_dir);
	return ret_buf;
}

void join_paths(char *dst, char *pth1, char *pth2)
{
	int l;
	char *rm, *fn;
	char *dst_buf;

	if(!(dst_buf = malloc(strlen(pth1) + strlen(pth2) + 1))) {
		ERR_LOG(("Failed to allocate memory"));
		return;
	}

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


extern void buf_flip(void *a, void *b, int size)
{
	char *buf[32];

	memcpy(buf, a, size);
	memcpy(b, a, size);
	memcpy(a, buf, size);
}


extern surf_t *crop_surf(surf_t* in, rect_t *in_rect, SDL_Rect *out_rect)
{
	surf_t *out = SDL_CreateRGBSurface(in->flags,
			out_rect->w, out_rect->h,
			in->format->BitsPerPixel,
			in->format->Rmask,
			in->format->Gmask,
			in->format->Bmask,
			in->format->Amask);

	if(out == NULL) {
		ERR_LOG(("Failed to get directory"));
		return NULL;
	}

	SDL_BlitSurface(in, in_rect, out, out_rect);
	return out;
}
