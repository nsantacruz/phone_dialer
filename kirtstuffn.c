//Noah Santacruz
//Prof Kirtman 2013 ECE 161

#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/soundcard.h>

#include <math.h>

#define NUM_ROWS 4
#define NUM_COLS 4

#define TONE_LEN 48000

#define SAMP_RATE 48000

#define PI (3.14159265)
int dsp_fd;		// File descr for sound card; used by several functions

void dsp_open(char *dsp_name, int mode, int samp_fmt, int n_chan, int samp_rate)
{
	int	arg;

	// Open the sound card
	if( (dsp_fd = open(dsp_name, mode)) < 0 )
	{
		printf("Cannot open sound card '%s'\n", dsp_name);
		exit(1);
	}


	// According to article cited above, settings should
	// take place in the following order...
	
	// Set sample format (num. of bits, little/big endian, encoding, etc.)
	arg = samp_fmt;
	if( ioctl(dsp_fd, SNDCTL_DSP_SETFMT, &arg) == -1 )
	{
		printf("Ioctl error setting sound card sample format\n");
		exit(1);
	}
	if( arg != samp_fmt )
	{
		printf("Sound card cannot be set to %d\n", samp_fmt);
		exit(1);
	}


	// Set number of channels (e.g. 1 (mono) or 2 (stereo))
	arg = n_chan;
	if( ioctl(dsp_fd, SNDCTL_DSP_CHANNELS, &arg) == -1 )
	{
		printf("Ioctl error setting number of channels\n");
		exit(1);
	}
	if( arg != n_chan )
	{
		printf("Could not set number of channels to %d\n", n_chan);
		exit(1);
	}


	// Set sample rate (num. of samps. per sec.)
	arg = samp_rate;
	if( ioctl(dsp_fd, SNDCTL_DSP_SPEED, &arg) == -1 )
	{
		printf("Ioctl error setting sample rate\n");
		exit(1);
	}
	if( arg != samp_rate )
	{
		printf("Could not set sample rate to %d\n", samp_rate);
		exit(1);
	}
}

//*data = short with required freqs
//samp_size = sizeof short used in *data
//n_samps = number of samples in *data
void dsp_write(void *data, size_t samp_size, size_t n_samps)
{
	size_t	tmp;

	tmp = samp_size * n_samps;

	if( write(dsp_fd, data, tmp) != tmp )
	{
		printf("Write error\n");
		exit(1);
	}
}


void dsp_sync(void)
{
	int	tmp;

	tmp = 0;
	if( ioctl(dsp_fd, SNDCTL_DSP_SYNC, &tmp) == -1 )
	{
		printf("Ioctl error syncing dsp\n");
		exit(1);
	}
}


void dsp_close(void)
{
	if( close(dsp_fd) < 0 )
	{
		printf("Error closing sound card\n");
		exit(1);
	}
}


void numToFreq(char input, int* freqs) {
	int i,j;
	int num_pad[4][4] = {{'1','2','3','A'},
						 {'4','5','6','B'},
						 {'7','8','9','C'},
						 {'*','0','#','D'}};
	int col_freqs[4] = {1209,1336,1477,1633};
	int row_freqs[4] = {697,770,852,941};
	int found = 0;
	for (i = 0;i < NUM_ROWS && !found; ++i) {
		for (j = 0;j < NUM_COLS && !found; ++j) {
			if (tolower(input) == tolower(num_pad[i][j])) {
				found = 1;
				i--; //account for +1
				j--;
			}
		}
	}
	*freqs = row_freqs[i];
	*(freqs+1) = col_freqs[j];
}

void getSineWave(int freq, unsigned short* sine, int time) {
	int i;
	for(i=0; i < TONE_LEN; i++)
		sine[i] = (32767/2)*sin(2.0*PI*freq*i/(time*SAMP_RATE));
}

void getInterpolation(int* freqs, unsigned short* interpolatedSine,unsigned short* sine1,unsigned short* sine2) {
	int i;
	for (i = 0; i < TONE_LEN; i++) {
		interpolatedSine[i] = sine1[i] + sine2[i];
	}
}
// Test program- plays a sine wave

int isValidInput(char c) {
	switch(c) {
		case 'A'...'D':
			return 1;
		case 'a'...'d':
			return 1;
		case '1'...'9':
			return 1;
		case '*':
			return 1;
		case '#':
			return 1;
		case 'p':
			return 1;
		case 'P':
			return 1;
	}
	printf("you suck\n");
	return 0;
}

int main()
{
	printf("type some numbers and see what happens...\n");
	int	i;
	// Open connection to the sound card
	dsp_open("/dev/dsp", O_WRONLY, AFMT_S16_NE, 1, SAMP_RATE);
	char c ;
	int count = 0;
	while ( ( c = getchar() ) != '\n' && count < 256) {
		if (isValidInput(c)) {
			int freqs[2];
			unsigned short sine1[TONE_LEN];
			unsigned short sine2[TONE_LEN];
			unsigned short interpolatedSine[TONE_LEN];
			if (c == 'p') {
				getSineWave(0,sine1,1);
				dsp_write(sine1,sizeof(short), TONE_LEN);
			} else if (c == 'P') {
				unsigned short sine1[2*TONE_LEN];
				getSineWave(0,sine1,2);
				dsp_write(sine1,sizeof(short), 2*TONE_LEN);
			} else {
			
				numToFreq(c,freqs);
				
				getSineWave(freqs[0],sine1,1);
				getSineWave(freqs[1],sine2,1);
				//printf("%d - %d\n",freqs[0],freqs[1]);
				
				getInterpolation(freqs,interpolatedSine,sine1,sine2);
	 
				dsp_write(interpolatedSine, sizeof(short), TONE_LEN);
				getSineWave(0,sine1,1);
				dsp_write(sine1,sizeof(short), TONE_LEN);
			}
		} else {
			break;
		}
		count++;
	}
	
	// May be necessary to flush the buffers
	dsp_sync();

	// Close connection to the sound card
	dsp_close();

	// Successful exit
	exit(0);
}
