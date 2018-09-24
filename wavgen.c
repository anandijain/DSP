
/*Compiles with gcc -Wall -O2 -o wavwrite wavwrite.c*/

/*
	TODO 
SORT: iterate over wave data to 
define purpose of wavgen
create array of sound generators ie white noise
	sine tones etc, 
	then for a set length, randomize which file writes data
	array of pointers to functions and rand % funcCount to determine which function filles that frame 
*/


//#include "wavgen.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <limits.h>
#include <math.h>

/*Standard values for CD-quality audio*/
#define SUBCHUNK1SIZE   (16)
#define AUDIO_FORMAT    (1) /*For PCM*/
#define NUM_CHANNELS    (2)
#define SAMPLE_RATE     (44100)

#define BITS_PER_SAMPLE (16)

#define BYTE_RATE       (SAMPLE_RATE * NUM_CHANNELS * BITS_PER_SAMPLE/8)
#define BLOCK_ALIGN     (NUM_CHANNELS * BITS_PER_SAMPLE/8)

//#define RAND_MAX (5000)

/*
The header of a wav file Based on:
https://ccrma.stanford.edu/courses/422/projects/WaveFormat/
*/
typedef struct wavfile_header_s
{
    char    ChunkID[4];     /*  4   */
    int32_t ChunkSize;      /*  4   */
    char    Format[4];      /*  4   */

    char    Subchunk1ID[4]; /*  4   */
    int32_t Subchunk1Size;  /*  4   */
    int16_t AudioFormat;    /*  2   */
    int16_t NumChannels;    /*  2   */
    int32_t SampleRate;     /*  4   */
    int32_t ByteRate;       /*  4   */
    int16_t BlockAlign;     /*  2   */
    int16_t BitsPerSample;  /*  2   */

    char    Subchunk2ID[4];
    int32_t Subchunk2Size;
} wavfile_header_t;


/*Return 0 on success and -1 on failure*/
int write_PCM16_stereo_header(  FILE*   file_p,
                                int32_t SampleRate,
                                int32_t FrameCount)
{
    int ret;

    wavfile_header_t wav_header;
    int32_t subchunk2_size;
    int32_t chunk_size;

    size_t write_count;

    subchunk2_size  = FrameCount * NUM_CHANNELS * BITS_PER_SAMPLE/8;
    chunk_size      = 4 + (8 + SUBCHUNK1SIZE) + (8 + subchunk2_size);

    wav_header.ChunkID[0] = 'R';
    wav_header.ChunkID[1] = 'I';
    wav_header.ChunkID[2] = 'F';
    wav_header.ChunkID[3] = 'F';

    wav_header.ChunkSize = chunk_size;

    wav_header.Format[0] = 'W';
    wav_header.Format[1] = 'A';
    wav_header.Format[2] = 'V';
    wav_header.Format[3] = 'E';

    wav_header.Subchunk1ID[0] = 'f';
    wav_header.Subchunk1ID[1] = 'm';
    wav_header.Subchunk1ID[2] = 't';
    wav_header.Subchunk1ID[3] = ' ';

    wav_header.Subchunk1Size = SUBCHUNK1SIZE;
    wav_header.AudioFormat = AUDIO_FORMAT;
    wav_header.NumChannels = NUM_CHANNELS;
    wav_header.SampleRate = SampleRate;
    wav_header.ByteRate = BYTE_RATE;
    wav_header.BlockAlign = BLOCK_ALIGN;
    wav_header.BitsPerSample = BITS_PER_SAMPLE;

    wav_header.Subchunk2ID[0] = 'd';
    wav_header.Subchunk2ID[1] = 'a';
    wav_header.Subchunk2ID[2] = 't';
    wav_header.Subchunk2ID[3] = 'a';
    wav_header.Subchunk2Size = subchunk2_size;

    write_count = fwrite(   &wav_header, 
                            sizeof(wavfile_header_t), 1,
                            file_p);

    ret = (1 != write_count)? -1 : 0;

    return ret;
}

/*Data structure to hold a single frame with two channels*/
typedef struct PCM16_stereo_s
{
    int16_t left;
    int16_t right;
} PCM16_stereo_t;

PCM16_stereo_t *allocate_PCM16_stereo_buffer(   int32_t FrameCount)
{
    return (PCM16_stereo_t *)malloc(sizeof(PCM16_stereo_t) * FrameCount);
}


/*Return the number of audio frames sucessfully written*/
size_t  write_PCM16wav_data(FILE*           file_p,
                            int32_t         FrameCount,
                            PCM16_stereo_t  *buffer_p)
{
    size_t ret;

    ret = fwrite(   buffer_p, 
                    sizeof(PCM16_stereo_t), FrameCount,
                    file_p);

    return ret;
}

int generate_white_noise(double amplitude1, double amplitude2,
						 int32_t SampleRate, int32_t FrameCount, 
						 PCM16_stereo_t *buffer_p, int freqOffset, int kOffset);

int generate_dual_sawtooth( double frequency1,
                            double amplitude1,
                            double frequency2,
                            double amplitude2,
                            int32_t SampleRate,
                            int32_t FrameCount,
                            PCM16_stereo_t  *buffer_p, int kOffset);


int main(int argc, char * argv[])
{
    int ret;
    FILE* file_p;
    double frequency1, amplitude1,
    frequency2, amplitude2, duration;

    if (argc == 1)
    {
    	frequency1 = 440.0; //concert A
    	frequency2 = 250.0;
    	duration = 5.0;
    }
    else{
    //frequency1 = atoi(argv[2]); 
    //frequency2 = atoi(argv[3]);
    duration = atof(argv[1]); /*seconds*/
    	if (duration > 1000)
    	{
    		goto error0;
    	}
	}	

	amplitude1 = 1.00 * (double)SHRT_MAX;
	amplitude2 = 0.75 * (double)SHRT_MAX;
    int32_t FrameCount = duration * SAMPLE_RATE;

    PCM16_stereo_t  *buffer_p = NULL;

    size_t written;

    /*Open the wav file*/
    file_p = fopen("./sound_test3.wav", "w");
    if(NULL == file_p)
    {
        perror("fopen failed in main");
        ret = -1;
        goto error0;
    }

    /*Allocate the data buffer*/ 	
    buffer_p = allocate_PCM16_stereo_buffer(FrameCount);
    if(NULL == buffer_p)
    {
        perror("fopen failed in main");
        ret = -1;
        goto error1;        
    }

    /*Fill the buffer while alternating which function is writing to the buffer*/
    
    for (int i = 0; i < FrameCount; i+=2)
    {
   	 //ret = generate_white_noise(amplitude1, amplitude2, SAMPLE_RATE, 1, buffer_p, 0, i);

     ret = generate_dual_sawtooth(frequency1, amplitude1, frequency2, amplitude2, SAMPLE_RATE, 1, buffer_p, i+1);	
                       

    }
                                    
  
    if(ret < 0)
    {
        fprintf(stderr, "generate_dual_sawtooth failed in main\n");
        ret = -1;
        goto error2;
    }

    /*Write the wav file header*/
    ret = write_PCM16_stereo_header(file_p,
                                    SAMPLE_RATE,
                                    FrameCount);
    if(ret < 0)
    {
        perror("write_PCM16_stereo_header failed in main");
        ret = -1;
        goto error2;
    }

    /*Write the data out to file*/
    written = write_PCM16wav_data(  file_p,
                                    FrameCount,
                                    buffer_p);
    if(written < FrameCount)
    {
        perror("write_PCM16wav_data failed in main");
        ret = -1;
        goto error2;
    }

    /*Free and close everything*/    
error2:
    free(buffer_p);
error1:
    fclose(file_p);
error0:
    return ret;    
}


int generate_white_noise(double amplitude1, double amplitude2,
						 int32_t SampleRate, int32_t FrameCount, 
						 PCM16_stereo_t *buffer_p, int freqOffset, int kOffset)
{
	int ret = 0;
	int32_t k;
    double SampleRate_d = (double)(SampleRate);
    double SamplePeriod = 1.0/SampleRate_d;

    double Period1, Period2;
    double phase1, phase2;
    double Slope1, Slope2;
    double f1, f2;
    double amp1 = amplitude1;
    double amp2 = amplitude2;

    //int RAND_MAX = 5000;
	for (k = 0, phase1= 0.0, phase2 = 0.0;
		k < FrameCount; k++)
	{
		//amp1 = amp1 * cos (amp2);
		//amp2 = amp2 * cos(amp1);
		f1 = (double) rand() - (k+kOffset); //% 10000) + freqOffset + k;
		f2 = (double) rand() - (k+kOffset); //% 5000) + freqOffset + k;
		/*
		if ((int) ceil(f1) % 2 == 0)
		{
			f1 = 440.0;
		}
		if ((int) floor(f2) % 3 == 0 || f2 < 700)
		{
			f2 = 261.6;
		}
		if (f1 < 500)
		{
			f1 = 440.0;
		}
		*/
		Period1 = 1.0/f1;
    	Period2 = 1.0/f2;

    	/*Compute the slope*/
    	Slope1  = amp1/Period1;
    	Slope2  = amp2/Period2;

    	phase1 = (phase1 > Period1)? (phase1 - Period1) : phase1;

        phase2 += SamplePeriod;
        phase2 = (phase2 > Period2)? (phase2 - Period2) : phase2;
        	if (kOffset % 2 == 0)
        	{
        		buffer_p[k+kOffset].left    = (int16_t)(phase1 * Slope1);
        		buffer_p[k+kOffset].right   = (int16_t)(phase2 * Slope2);
        	}
        	else 
        	{
        		buffer_p[k+kOffset].left    = (int16_t)(phase2 * Slope1);
        		buffer_p[k+kOffset].right   = (int16_t)(phase1 * Slope2);

        	}
			
	}

	error0:
		return ret;
}


/*Generate two saw-tooth signals at two frequencies and amplitudes*/
int generate_dual_sawtooth( double frequency1,
                            double amplitude1,
                            double frequency2,
                            double amplitude2,
                            int32_t SampleRate,
                            int32_t FrameCount,
                            PCM16_stereo_t  *buffer_p,
                            int kOffset)
{
    int ret = 0;
    double SampleRate_d = (double)SampleRate;
    double SamplePeriod = 1.0/SampleRate_d;

    double Period1, Period2;
    double phase1, phase2;
    double Slope1, Slope2;

    int32_t k;

    /*Check for the violation of the Nyquist limit*/
    if( (frequency1*2 >= SampleRate_d) || (frequency2*2 >= SampleRate_d) )
    {
        ret = -1;
        goto error0;
    }

    for(k = 0, phase1 = 0.0, phase2 = 0.0; 
        k < FrameCount; 
        k++)
    {

	    /*Compute the period*/
	    Period1 = 1.0/(frequency1 - ((k+kOffset) % 1000));
	    Period2 = 1.0/(frequency2 - floor((k+kOffset) * cos((k+kOffset))));
	    	//(k % (rand() % 1000)))

	    /*Compute the slope*/
	    Slope1  = amplitude1/Period1;
	    Slope2  = amplitude2/Period2;


        phase1 += SamplePeriod;
        phase1 = (phase1 > Period1)? (phase1 - Period1) : phase1;

        phase2 += SamplePeriod;
        phase2 = (phase2 > Period2)? (phase2 - Period2) : phase2;

        	if (kOffset % 2 == 0)
        	{
        		buffer_p[k+kOffset].left    = (int16_t)(phase1 * Slope1);
        		buffer_p[k+kOffset].right   = (int16_t)(phase2 * Slope2);
        	}
        	else 
        	{
        		buffer_p[k+kOffset].left    = (int16_t)(phase2 * Slope2);
        		buffer_p[k+kOffset].right   = (int16_t)(phase1 * Slope1);

        	}
    }

error0:
    return ret;
}


/*

int nyquistCheck(double f1, double f2, double sampleRate)
{
	int ret = 0;
	    //Check for the violation of the Nyquist limit
    if( (frequency1*2 >= SampleRate_d) || (frequency2*2 >= SampleRate_d) )
    {
        ret = -1;
    }
	return ret;
}

*/

/*
#include <stdio.h>


typedef struct Wave {
    WaveHeader header;
    char* data;
    long long int index;
    long long int size;
    long long int nSamples;
} Wave;

int main (int argc, char * argv[])
{
	Wave w = (Wave) malloc(sizeof(Wave));
	w->size = (long long int) atoi(argv[1]);
	return 1; 

}

Wave makeWave(int const sampleRate, short int const numChannels, short int const bitsPerSample){
    Wave myWave;
    myWave.header = makeWaveHeader(sampleRate,numChannels,bitsPerSample);
    return myWave;
}

WaveHeader makeWaveHeader(int const sampleRate,
        short int const numChannels,
        short int const bitsPerSample ){
    WaveHeader myHeader;

    // RIFF WAVE Header
    myHeader.chunkId[0] = 'R';
    myHeader.chunkId[1] = 'I';
    myHeader.chunkId[2] = 'F';
    myHeader.chunkId[3] = 'F';
    myHeader.format[0] = 'W';
    myHeader.format[1] = 'A';
    myHeader.format[2] = 'V';
    myHeader.format[3] = 'E';

    // Format subchunk
    myHeader.subChunk1Id[0] = 'f';
    myHeader.subChunk1Id[1] = 'm';
    myHeader.subChunk1Id[2] = 't';
    myHeader.subChunk1Id[3] = ' ';
    myHeader.audioFormat = 1; // FOR PCM
    myHeader.numChannels = numChannels; // 1 for MONO, 2 for stereo
    myHeader.sampleRate = sampleRate; // ie 44100 hertz, cd quality audio
    myHeader.bitsPerSample = bitsPerSample; // 
    myHeader.byteRate = myHeader.sampleRate * myHeader.numChannels * myHeader.bitsPerSample / 8;
    myHeader.blockAlign = myHeader.numChannels * myHeader.bitsPerSample/8;

    // Data subchunk
    myHeader.subChunk2Id[0] = 'd';
    myHeader.subChunk2Id[1] = 'a';
    myHeader.subChunk2Id[2] = 't';
    myHeader.subChunk2Id[3] = 'a';

    // All sizes for later:
    // chuckSize = 4 + (8 + subChunk1Size) + (8 + subChubk2Size)
    // subChunk1Size is constanst, i'm using 16 and staying with PCM
    // subChunk2Size = nSamples * nChannels * bitsPerSample/8
    // Whenever a sample is added:
    //    chunkSize += (nChannels * bitsPerSample/8)
    //    subChunk2Size += (nChannels * bitsPerSample/8)
    myHeader.chunkSize = 4+8+16+8+0;
    myHeader.subChunk1Size = 16;
    myHeader.subChunk2Size = 0;
    
    return myHeader;
}

*/