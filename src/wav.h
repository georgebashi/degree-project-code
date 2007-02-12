#ifndef SND_H
#define SND_H

extern void wav_open(char *filename);
extern void wav_close();
extern int wav_read(float* buffer, int n);


#endif
