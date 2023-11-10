#ifndef __USEFUL_STUFF_H__
#define __USEFUL_STUFF_H__

#include <random>

namespace UsefulStuff
{
	template <typename T>
	inline T RandomNumber()
	{
		static std::uniform_real_distribution<T> distribution(0.0, 1.0);
		static std::random_device rd;
		static std::mt19937 generator(rd());
		return distribution(generator);
	}

	// Returns a random real in [min,max)
	template <typename T>
	inline T RandomNumber(T min, T max)
	{
		return min + (max - min) * RandomNumber<T>();
	}
}

#endif
