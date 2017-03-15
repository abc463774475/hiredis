#ifndef _Random_H_
#define _Random_H_
#include <stdint.h>

/**
 * \brief 
 *  tools function for random.
 */
void InitRandomNumberGenerator();
void ReseedRandomNumberGenerators();
void CleanupRandomNumberGenerators();
double RandomDouble();
double RandomDouble(double n);
float RandomFloat();
float RandomFloat(float n);
uint32_t RandomUInt();
uint32_t RandomUInt(uint32_t n);
uint32_t RandomUInt(uint32_t, uint32_t);

//set<UINT>	RandomVector(const	vector<UINT> &vSource , int needNum );
/**
 * @class RandomMersenne
 *
 * The Mersenne twister is a pseudorandom number generator,
 * The newer and more commonly used one is the MT19937, with 32-bit word length,
 * And we use the MT19937 also.
 */
class RandomMersenne
{
#if 0
    // Define constants for type MT11213A:
#define MERS_N   351
#define MERS_M   175
#define MERS_R   19
#define MERS_U   11
#define MERS_S   7
#define MERS_T   15
#define MERS_L   17
#define MERS_A   0xE4BD75F5
#define MERS_B   0x655E5280
#define MERS_C   0xFFD58000
#else    
    // or constants for type MT19937:
#define MERS_N   624
#define MERS_M   397
#define MERS_R   31
#define MERS_U   11
#define MERS_S   7
#define MERS_T   15
#define MERS_L   18
#define MERS_A   0x9908B0DF
#define MERS_B   0x9D2C5680
#define MERS_C   0xEFC60000
#endif

public:
    RandomMersenne(uint32_t seed) 
    {
        RandomInit(seed); LastInterval = 0;
    }

    void RandomInit(uint32_t seed);       // Re-seed
    void RandomInitByArray(uint32_t seeds[], int length); // Seed by more than 32 bits
    int IRandom (int min, int max);     // Output random integer
    int IRandomX(int min, int max);     // Output random integer, exact
    double Random();                    // Output random float
    uint32_t BRandom();                   // Output random bits

private:
    void Init0(uint32_t seed);            // Basic initialization procedure
    uint32_t mt[MERS_N];                  // State vector
    int mti;                            // Index into mt
    uint32_t LastInterval;                // Last interval length for IRandomX
    uint32_t RLimit;                      // Rejection limit used by IRandomX
    enum TArch {LITTLE_ENDIAN1, BIG_ENDIAN1, NONIEEE}; // Definition of architecture
    TArch Architecture;                 // Conversion to float depends on architecture
};    

#endif // _Random_H_
