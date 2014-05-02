// Copyright (c) 2014 Roulettecoin developers

#include "roulette.h"

#include "cpuminer-config.h"
#include "miner.h"

#include <string.h>
#include <stdint.h>

int scanhash_roulette(int thr_id, uint32_t *pdata, const uint32_t *ptarget,
	uint32_t max_nonce, unsigned long *hashes_done)
{
	uint32_t n = pdata[19] - 1;
	const uint32_t first_nonce = pdata[19];
	const uint32_t Htarg = ptarget[7];

	uint32_t hash64[8] __attribute__((aligned(32)));
	uint32_t endiandata[32];
	
	//we need bigendian data...
	int kk=0;
	for (; kk < 32; kk++)
	{
		be32enc(&endiandata[kk], ((uint32_t*)pdata)[kk]);
	};

	do {
		pdata[19] = ++n;
		be32enc(&endiandata[19], n); 
		RouletteHash((unsigned char*) hash64, (unsigned char*) &endiandata);
		if (((hash64[7] < ptarget[7]) || ((hash64[7] == ptarget[7]) && (hash64[6] < ptarget[6]))) &&
				fulltest(hash64, ptarget)) {
			*hashes_done = n - first_nonce + 1;
			return true;
		}
	} while (n < max_nonce && !work_restart[thr_id].restart);	
	
	*hashes_done = n - first_nonce + 1;
	pdata[19] = n;
	return 0;
}

