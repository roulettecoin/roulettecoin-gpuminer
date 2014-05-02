// Copyright (c) 2014 Roulettecoin developers

#ifndef ROULETTE_H
#define ROULETTE_H

#include "sph_blake.h"
#include "sph_bmw.h"
#include "sph_cubehash.h"
#include "sph_echo.h"
#include "sph_fugue.h"
#include "sph_groestl.h"
#include "sph_hamsi.h"
#include "sph_jh.h"
#include "sph_keccak.h"
#include "sph_luffa.h"
#include "sph_sha2.h"
#include "sph_shabal.h"
#include "sph_shavite.h"
#include "sph_simd.h"
#include "sph_skein.h"
#include "sph_whirlpool.h"
#include <string.h>

void RouletteHash(unsigned char* output, unsigned char* input)
{
    sph_blake512_context    ctx_blake;
    sph_bmw512_context      ctx_bmw;
    sph_cubehash512_context ctx_cubehash;
    sph_echo512_context     ctx_echo;
    sph_fugue512_context    ctx_fugue;
    sph_groestl512_context  ctx_groestl;
    sph_hamsi512_context    ctx_hamsi;
    sph_jh512_context       ctx_jh;
    sph_keccak512_context   ctx_keccak;
    sph_luffa512_context    ctx_luffa;
    sph_sha512_context      ctx_sha;
    sph_shabal512_context   ctx_shabal;
    sph_shavite512_context  ctx_shavite;
    sph_simd512_context     ctx_simd;
    sph_skein512_context    ctx_skein;
    sph_whirlpool_context   ctx_whirlpool1;

    unsigned char hash[64];  
    
    sph_sha512_init(&ctx_sha);
    sph_sha512(&ctx_sha, input, 88);
    sph_sha512_close(&ctx_sha, hash);    

    for(unsigned i = 0; i < 16; i++)
    {
        unsigned hdec = hash[0] & 0xf;
        switch(hdec)
        {
            case 0:
                sph_blake512_init(&ctx_blake);   
                sph_blake512(&ctx_blake, hash, 64);
                sph_blake512_close(&ctx_blake, hash);        
                break;
            case 1:
                sph_bmw512_init(&ctx_bmw);   
                sph_bmw512(&ctx_bmw, hash, 64);
                sph_bmw512_close(&ctx_bmw, hash);        
                break;
            case 2:
                sph_cubehash512_init(&ctx_cubehash);
                sph_cubehash512(&ctx_cubehash, hash, 64);   
                sph_cubehash512_close(&ctx_cubehash, hash);  
                break;
            case 3:
                sph_echo512_init(&ctx_echo);
                sph_echo512(&ctx_echo, hash, 64);   
                sph_echo512_close(&ctx_echo, hash);    
                break;
            case 4:
                sph_fugue512_init(&ctx_fugue);
                sph_fugue512(&ctx_fugue, hash, 64);   
                sph_fugue512_close(&ctx_fugue, hash);    
                break;
            case 5:
                sph_groestl512_init(&ctx_groestl);   
                sph_groestl512(&ctx_groestl, hash, 64); 
                sph_groestl512_close(&ctx_groestl, hash);
                break;
            case 6:
                sph_hamsi512_init(&ctx_hamsi);   
                sph_hamsi512(&ctx_hamsi, hash, 64); 
                sph_hamsi512_close(&ctx_hamsi, hash);
                break;
            case 7:
                sph_jh512_init(&ctx_jh);     
                sph_jh512(&ctx_jh, hash, 64); 
                sph_jh512_close(&ctx_jh, hash);
                break;
            case 8:
                sph_keccak512_init(&ctx_keccak);     
                sph_keccak512(&ctx_keccak, hash, 64); 
                sph_keccak512_close(&ctx_keccak, hash);
                break;
            case 9:
                sph_luffa512_init(&ctx_luffa);
                sph_luffa512(&ctx_luffa, hash, 64);
                sph_luffa512_close(&ctx_luffa, hash);    
                break;
            case 10:
                sph_sha512_init(&ctx_sha);
                sph_sha512(&ctx_sha, hash, 64);
                sph_sha512_close(&ctx_sha, hash);    
                break;
            case 11:
                sph_shabal512_init(&ctx_shabal);
                sph_shabal512(&ctx_shabal, hash, 64);
                sph_shabal512_close(&ctx_shabal, hash);    
                break;
            case 12:
                sph_shavite512_init(&ctx_shavite);
                sph_shavite512(&ctx_shavite, hash, 64);   
                sph_shavite512_close(&ctx_shavite, hash);  
                break;
            case 13:
                sph_simd512_init(&ctx_simd);
                sph_simd512(&ctx_simd, hash, 64);   
                sph_simd512_close(&ctx_simd, hash); 
                break;
            case 14:
                sph_skein512_init(&ctx_skein);   
                sph_skein512(&ctx_skein, hash, 64); 
                sph_skein512_close(&ctx_skein, hash); 
                break;
            case 15:
                sph_whirlpool_init(&ctx_whirlpool1);
                sph_whirlpool(&ctx_whirlpool1, hash, 64);
                sph_whirlpool_close(&ctx_whirlpool1, hash);    
                break;
            default:
                break;
        }
    }

    memcpy(output, hash, 32);
}

#endif

