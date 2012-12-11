#include <cuda_runtime.h>
#include <iostream>

// The CUDA code is not optimized

using namespace std;

unsigned char *srcd = 0;
unsigned char *dstd = 0;
unsigned char *tmpd = 0;

__device__ static void calculateRanges( int indexX, int indexY, int size, int width, int height, int &numPixels, int &beginX, int &endX, int &beginY, int &endY )
{
	beginX = indexX - size;
	if ( beginX < 0 )
		beginX = 0;
	endX = indexX + size;
	if ( endX >= width )
		endX = width - 1;
	beginY = indexY - size;
	if ( beginY < 0 )
		beginY = 0;
	endY = indexY + size;
	if ( endY >= height )
		endY = height - 1;
	numPixels = ( endX - beginX + 1 ) * ( endY - beginY + 1 );
}

__global__ void pixelizationKernel( unsigned char *src, unsigned char *dst, int width, int height, int BPP )
{
	const int index_x = blockIdx.x * blockDim.x + threadIdx.x;
	const int index_y = blockIdx.y * blockDim.y + threadIdx.y;

	const int pixIndex = index_y * width + index_x;

	__shared__ int pixelVal[3];
	if ( threadIdx.x == 0 && threadIdx.y == 0 )
	{		
		for ( int c = 0; c < 3; ++c )
			pixelVal[c] = src[pixIndex * BPP + c];
	}

	__syncthreads();
	
	for ( int c = 0; c < 3; ++c )
		dst[pixIndex*BPP+c] = pixelVal[c];//src[pixIndex0*BPP+c];
}

__global__ void blurKernel( unsigned char *src, unsigned char *dst, int width, int height, int BPP )
{
	const int index_x = blockIdx.x * blockDim.x + threadIdx.x;
	const int index_y = blockIdx.y * blockDim.y + threadIdx.y;

	const int pixIndex = index_y * width + index_x;
	
	int beginX, endX, beginY, endY, numPixels;
	calculateRanges( index_x, index_y, 3, width, height, numPixels, beginX, endX, beginY, endY );

	int sum[] = {0,0,0};
	for ( int f = beginY; f <= endY; ++f )
	{
		for ( int c = beginX; c <= endX; ++c )
		{
			const int index = ( f * width + c ) * BPP;
			sum[0] += src[index];
			sum[1] += src[index + 1];
			sum[2] += src[index + 2];			
		}
	}
	for ( int c = 0; c < 3; ++c )
		dst[pixIndex*BPP+c] = sum[c] / numPixels;
}

__global__ void dilationKernel( unsigned char *src, unsigned char *dst, int width, int height, int BPP )
{
	const int index_x = blockIdx.x * blockDim.x + threadIdx.x;
	const int index_y = blockIdx.y * blockDim.y + threadIdx.y;
	const int pixIndex = index_y * width + index_x;

	int max = 0;//[] = {0,0,0};
	int fMax = index_y - 2;
	int cMax = index_x - 1;
	int beginX, endX, beginY, endY, numPixels;
	calculateRanges( index_x, index_y, 2, width, height, numPixels, beginX, endX, beginY, endY );
	for ( int f = beginY; f < endY; ++f )
	{
		for ( int c = beginX; c < endX; ++c )
		{
			const int index = ( f * width + c ) * BPP;	
			for ( int channel = 0; channel < 3; ++channel )
			{
				const int aux = src[index + channel];
				if( aux > max )
				{
					max = aux;
					cMax = c;					
					fMax = f;
				}
			}			
		}
	}
	const int index = ( fMax * width + cMax ) * BPP;
	for ( int channel = 0; channel < 3; ++channel )	
		dst[pixIndex*BPP+channel] = src[index + channel];
}

__global__ void blocksKernel( unsigned char *src, unsigned char *dst, int width, int height, int BPP )
{
	const int index_x = blockIdx.x * blockDim.x + threadIdx.x;
	const int index_y = blockIdx.y * blockDim.y + threadIdx.y;

	const int pixIndex = index_y * width + index_x;
	
	dst[pixIndex*BPP+2] = 255 * blockIdx.x / gridDim.x;//255 * threadIdx.x / blockDim.x;
	dst[pixIndex*BPP+1] = src[pixIndex*BPP+2] ;//255 * threadIdx.x / blockDim.x;
	dst[pixIndex*BPP] = 255 * blockIdx.y / gridDim.y;//255 * threadIdx.y / blockDim.y;
}

__global__ void ghostKernel( unsigned char *src, unsigned char *dst, int width, int height, int BPP )
{
	const int index_x = blockIdx.x * blockDim.x + threadIdx.x;
	const int index_y = blockIdx.y * blockDim.y + threadIdx.y;

	const int pixIndex = index_y * width + index_x;
	
	//ghost effect
	for ( int c = 0; c < 3; ++c )
	{
		const float aux = src[pixIndex*BPP+c];
		float sum = ( 0.25f * aux + 0.75f * (float)dst[pixIndex*BPP+c] );
		if ( sum > 255 ) sum = 255;
		src[pixIndex*BPP+c] = sum;
		dst[pixIndex*BPP+c] = sum;
	}
}

__global__ void screeenBlendKernel( unsigned char *src, unsigned char *blend, unsigned char *dst, int width, int height, int BPP )
{
	const int index_x = blockIdx.x * blockDim.x + threadIdx.x;
	const int index_y = blockIdx.y * blockDim.y + threadIdx.y;

	const int pixIndex = ( index_y * width + index_x ) * BPP;
	
	for ( int c = 0; c < 3; ++c )
		dst[pixIndex + c] = 255 - ( ( ( 255 - blend[pixIndex + c] ) * ( 255 - src[pixIndex + c] ) ) ) / 255;
}

__global__ void aberrationKernel( unsigned char *src, unsigned char *dst, int width, int height, int BPP )
{
	const int index_x = blockIdx.x * blockDim.x + threadIdx.x;
	const int index_y = blockIdx.y * blockDim.y + threadIdx.y;

	//const int pixIndex = index_y * width + index_x;	
	dst[( index_y * width + index_x ) * BPP + 0] = src[(( index_y - 5 )* width + index_x -3)* BPP];
	dst[( index_y * width + index_x ) * BPP + 1] = src[(( index_y + 5 )* width + index_x +3)* BPP + 1];
	dst[( index_y * width + index_x ) * BPP + 2] = src[( index_y * width + index_x - 7)* BPP + 2];
	
}

extern "C"
void allocGPUMem( int width, int height, int BPP )
{
	const int sizeBytes = sizeof(unsigned char) * width * height * BPP;
	cudaError_t err = cudaMalloc( (void**)&srcd, sizeBytes );
	err = cudaMalloc( (void**)&dstd, sizeBytes );	
	err = cudaMalloc( (void**)&tmpd, sizeBytes );		
	err = cudaMemset( srcd, 0, sizeBytes );
	err = cudaMemset( dstd, 0, sizeBytes );
	err = cudaMemset( tmpd, 0, sizeBytes );	
	if(err != cudaSuccess)
	{
		// print the CUDA error message and exit
		printf("CUDA error: %s\n", cudaGetErrorString(err));
		exit(-1);
	}        
}

extern "C"
void freeGPUMem()
{
	cudaError_t err = cudaFree( srcd );
	err = cudaFree( dstd );
	err = cudaFree( tmpd );	
	if(err != cudaSuccess)
	{
		// print the CUDA error message and exit
		printf("CUDA error: %s\n", cudaGetErrorString(err));
		exit(-1);
	}        
}

extern "C"
void executeCudaKernel( int effect, unsigned char *img, int width, int height, int BPP )
{
	const int sizeBytes = sizeof(unsigned char) * width * height * BPP;
	cudaError_t err = cudaMemcpy( srcd, img, sizeBytes, cudaMemcpyHostToDevice );
	const int threads = 16;
	dim3 dimGrid( width / (float)threads, height / (float)threads );	
	dim3 dimBlock( threads,  threads );
	switch(effect)
	{
		case 0:
		{	
			pixelizationKernel<<< dimGrid, dimBlock >>>( srcd, dstd, width, height, BPP );
			break;
		}		
		case 1:
		{	
			blocksKernel<<< dimGrid, dimBlock >>>( srcd, dstd, width, height, BPP );
			break;
		}
		case 2:
		{	
			ghostKernel<<< dimGrid, dimBlock >>>( srcd, dstd, width, height, BPP );
			break;
		}
		case 3:
		{	
			dilationKernel<<< dimGrid, dimBlock >>>( srcd, dstd, width, height, BPP );
			break;
		}
		case 4:
		{	
			dilationKernel<<< dimGrid, dimBlock >>>( srcd, tmpd, width, height, BPP );
			blurKernel<<< dimGrid, dimBlock >>>( tmpd, dstd, width, height, BPP );
			break;
		}
		case 5:
		{
			dilationKernel<<< dimGrid, dimBlock >>>( srcd, dstd, width, height, BPP );
			blurKernel<<< dimGrid, dimBlock >>>( dstd, tmpd, width, height, BPP );
			screeenBlendKernel<<< dimGrid, dimBlock >>>( srcd, tmpd, dstd, width, height, BPP );
			break;
		}		
	}
	cudaMemcpy( img, dstd, sizeBytes, cudaMemcpyDeviceToHost );
	err = cudaGetLastError();
	if(err != cudaSuccess)
	{
		// print the CUDA error message and exit
		printf("CUDA error: %s\n", cudaGetErrorString(err));
		exit(-1);
	}                                     
}
