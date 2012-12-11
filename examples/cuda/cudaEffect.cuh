#ifndef CUDA_EFFECTS_H
#define CUDA_EFFECTS_H

#define effectsNumber 6;

char *effectsname[] = {"Pixelization", "CUDA processing blocks", "Motion blur", "Dilation", "Dilation + Blur", "Bloom effect"};

extern "C" void allocGPUMem( int width, int height, int BPP );

extern "C" void freeGPUMem();

extern "C" void executeCudaKernel( int effect, unsigned char *img, int width, int height, int BPP );

#endif