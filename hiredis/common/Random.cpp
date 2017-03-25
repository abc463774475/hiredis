#include "Random.h"
#include <stdlib.h>
#include <time.h>
/* instance of random number generator */
RandomMersenne *g_generator;

uint32_t generate_seed()
{
//     uint32_t mstime = GetTickCount ();
// 	uint32_t stime = (uint32_t)time (0);
// 	uint32_t rnd[2];
// 	rnd[0] = rand()*rand()*rand();
// 	rnd[1] = rand()*rand()*rand();
// 
// 	uint32_t val = mstime ^ rnd[0];
// 	val += stime ^ rnd[1];
// 
// 	return val;
	return time(NULL);
}

void InitRandomNumberGenerator()
{
    srand (time(NULL));

    /// Make instance of random number generator
    g_generator = new RandomMersenne (generate_seed ());
}

void CleanupRandomNumberGenerators()
{	
    srand (time(NULL));

    delete g_generator;
    g_generator = 0;
}

uint32_t RandomUInt()
{
    return g_generator->IRandom (0, RAND_MAX);
}

uint32_t RandomUInt(uint32_t n)
{
    return g_generator->IRandom (0, n);
}

uint32_t RandomUInt(uint32_t min, uint32_t max)
{
    if (max <= min)
    {
        if (max == min) return min;
        else return 0x80000000;
    }

    return (min + g_generator->IRandom (0, (max - min)));
}

double RandomDouble()
{
    return g_generator->Random ();
}

double RandomDouble(double n)
{
	return RandomDouble () * n;
}

float RandomFloat()
{
	return float(RandomDouble ());
}

float RandomFloat(float n)
{
	return float(RandomDouble () * double(n));
}

//set<UINT>	RandomVector(const vector<UINT> &vSource , int needNum )
//{
//	if ( needNum <= vSource.size() )
//	{
//		return vSource;
//	}
//	vector<uint32_t>	vTotal = vSource ;
//
//	std::random_shuffle(vTotal.begin(),vTotal.end());
//
//	return ;
//}



void RandomMersenne::Init0(uint32_t seed) {
	// Detect computer architecture
	union {double f; uint32_t i[2];} convert;
	convert.f = 1.0;
	if (convert.i[1] == 0x3FF00000) Architecture = LITTLE_ENDIAN1;
	else if (convert.i[0] == 0x3FF00000) Architecture = BIG_ENDIAN1;
	else Architecture = NONIEEE;

	// Seed generator
	mt[0]= seed;
	for (mti=1; mti < MERS_N; mti++) {
		mt[mti] = (1812433253UL * (mt[mti-1] ^ (mt[mti-1] >> 30)) + mti);
	}
}

void RandomMersenne::RandomInit(uint32_t seed) {
	// Initialize and seed
	Init0(seed);

	// Randomize some more
	for (int i = 0; i < 37; i++) BRandom();
}


void RandomMersenne::RandomInitByArray(uint32_t seeds[], int length) {
	// Seed by more than 32 bits
	int i, j, k;

	// Initialize
	Init0(19650218);

	if (length <= 0) return;

	// Randomize mt[] using whole seeds[] array
	i = 1;  j = 0;
	k = (MERS_N > length ? MERS_N : length);
	for (; k; k--) {
		mt[i] = (mt[i] ^ ((mt[i-1] ^ (mt[i-1] >> 30)) * 1664525UL)) + seeds[j] + j;
		i++; j++;
		if (i >= MERS_N) {mt[0] = mt[MERS_N-1]; i=1;}
		if (j >= length) j=0;}
	for (k = MERS_N-1; k; k--) {
		mt[i] = (mt[i] ^ ((mt[i-1] ^ (mt[i-1] >> 30)) * 1566083941UL)) - i;
		if (++i >= MERS_N) {mt[0] = mt[MERS_N-1]; i=1;}}
	mt[0] = 0x80000000UL;  // MSB is 1; assuring non-zero initial array

	// Randomize some more
	mti = 0;
	for (int i = 0; i <= MERS_N; i++) BRandom();
}


uint32_t RandomMersenne::BRandom() {
	// Generate 32 random bits
	uint32_t y;

	if (mti >= MERS_N) {
		// Generate MERS_N words at one time
		const uint32_t LOWER_MASK = (1LU << MERS_R) - 1;       // Lower MERS_R bits
		const uint32_t UPPER_MASK = 0xFFFFFFFF << MERS_R;      // Upper (32 - MERS_R) bits
		static const uint32_t mag01[2] = {0, MERS_A};

		int kk;
		for (kk=0; kk < MERS_N-MERS_M; kk++) {    
			y = (mt[kk] & UPPER_MASK) | (mt[kk+1] & LOWER_MASK);
			mt[kk] = mt[kk+MERS_M] ^ (y >> 1) ^ mag01[y & 1];}

		for (; kk < MERS_N-1; kk++) {    
			y = (mt[kk] & UPPER_MASK) | (mt[kk+1] & LOWER_MASK);
			mt[kk] = mt[kk+(MERS_M-MERS_N)] ^ (y >> 1) ^ mag01[y & 1];}      

		y = (mt[MERS_N-1] & UPPER_MASK) | (mt[0] & LOWER_MASK);
		mt[MERS_N-1] = mt[MERS_M-1] ^ (y >> 1) ^ mag01[y & 1];
		mti = 0;
	}

	y = mt[mti++];

#if 1
	// Tempering (May be omitted):
	y ^=  y >> MERS_U;
	y ^= (y << MERS_S) & MERS_B;
	y ^= (y << MERS_T) & MERS_C;
	y ^=  y >> MERS_L;
#endif

	return y;
}


double RandomMersenne::Random() {
	// Output random float number in the interval 0 <= x < 1
	union {double f; uint32_t i[2];} convert;
	uint32_t r = BRandom();               // Get 32 random bits
	// The fastest way to convert random bits to floating point is as follows:
	// Set the binary exponent of a floating point number to 1+bias and set
	// the mantissa to random bits. This will give a random number in the 
	// interval [1,2). Then subtract 1.0 to get a random number in the interval
	// [0,1). This procedure requires that we know how floating point numbers
	// are stored. The storing method is tested in function RandomInit and saved 
	// in the variable Architecture.

	// This shortcut allows the compiler to optimize away the following switch
	// statement for the most common architectures:
	convert.i[0] =  r << 20;
	convert.i[1] = (r >> 12) | 0x3FF00000;
	return convert.f - 1.0;
	// This somewhat slower method works for all architectures, including 
	// non-IEEE floating point representation:
	//return (double)r * (1./((double)(uint32_t)(-1L)+1.));
}


int RandomMersenne::IRandom(int min, int max) {
	// Output random integer in the interval min <= x <= max
	// Relative error on frequencies < 2^-32
	if (max <= min) {
		if (max == min) return min; else return 0x80000000;
	}
	// Multiply interval with random and truncate
	int r = int((max - min + 1) * Random()) + min; 
	if (r > max) r = max;
	return r;
}


int RandomMersenne::IRandomX(int min, int max) {
	// Output random integer in the interval min <= x <= max
	// Each output value has exactly the same probability.
	// This is obtained by rejecting certain bit values so that the number
	// of possible bit values is divisible by the interval length
	if (max <= min) {
		if (max == min) return min; else return 0x80000000;
	}

	// 64 bit integers available. Use multiply and shift method
	uint32_t interval;                    // Length of interval
	uint64_t longran;                     // Random bits * interval
	uint32_t iran;                        // Longran / 2^32
	uint32_t remainder;                   // Longran % 2^32

	interval = uint32_t(max - min + 1);
	if (interval != LastInterval) {
		// Interval length has changed. Must calculate rejection limit
		// Reject when remainder = 2^32 / interval * interval
		// RLimit will be 0 if interval is a power of 2. No rejection then
		RLimit = uint32_t(((uint64_t)1 << 32) / interval) * interval - 1;
		LastInterval = interval;
	}
	do { // Rejection loop
		longran  = (uint64_t)BRandom() * interval;
		iran = (uint32_t)(longran >> 32);
		remainder = (uint32_t)longran;
	} while (remainder > RLimit);
	// Convert back to signed and return result
	return (int)iran + min;
}

//vector<uint32_t>	RandVecotr(uint32_t _startPos,uint32_t _totalSize , uint32_t _needSize)
//{
//	
//}