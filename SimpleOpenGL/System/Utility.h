#ifndef UTILITY
#define UTILITY

#include <random>

namespace Utility
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

	template <typename T>
	float Lerp(float a, float b, float f)
	{
		return a + f * (b - a);
	}

	inline int NumMipmap(int width, int height)
	{
		return static_cast<int>(floor(log2(std::max(width, height)))) + 1;
	}
}

#endif
