#ifndef SND_H
#define SND_H

extern void open_sf(char *filename);
extern void close_sf();
extern int read_window(float* buffer, int n);


#endif
