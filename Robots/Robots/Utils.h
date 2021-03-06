#pragma once
#include <cstdlib>


// returns a random integer between x and y
inline int RandInt(int x, int y) { return rand() % (y - x + 1) + x; }

// returns a random float between zero and 1
inline double RandFloat() { return (double)rand() / (RAND_MAX + 1.0); }

// returns a random float in the range -1 < n < 1
inline double RandomClamped() { return RandFloat() - RandFloat(); }