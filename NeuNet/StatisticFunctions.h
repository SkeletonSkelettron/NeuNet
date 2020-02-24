#ifndef STATISTICFUNCTIONS_H
#define STATISTICFUNCTIONS_H

#include <vector>
#include <math.h>

using namespace std;

template <typename T> static T Mean(vector<T> x)
{
	T sum = 0;
	int n = x.size();

	for (unsigned long int i = 0; i < n; i++)
		sum += x[i];

	return sum / n;
}


template <typename T> static void Standartize(vector<T>& dataset)
{
	const T Epsilon = 0.000001;
	auto mean = Mean(dataset);
	auto StDev = StandardDeviationOfPopulation(dataset);

	for (unsigned long int i = 0; i < dataset.size(); i++)
		dataset[i] = (dataset[i] - mean) / sqrt(StDev + Epsilon);
}

template <typename T> static void StandartizeLinearContract(vector<T>& dataset, std::vector<int>& minMax, T& start, T& end)
{
	T min = Min(dataset);
	T max = Max(dataset);
	minMax[0] = min;
	minMax[1] = max;
	T range = max - min;
	for (unsigned long int i = 0; i < dataset.size(); i++)
	{
		//if (dataset[i] != 0)
		dataset[i] = (end-start) * (dataset[i] - min) / range +start;
	}
}

template <typename T> static void DeStandartizeLinearContract(vector<T>& dataset, std::vector<int>& minMax)
{
	T min = minMax[0];
	T max = minMax[1];
	auto range = max - min;

	for (unsigned long int i = 0; i < dataset.size(); i++)
	{
		//if (dataset[i] != 0)
		dataset[i] = (int)(range * (dataset[i] + 1.0) / 2.0 + min);
	}
}

template <typename T> static T StandardDeviationOfPopulation(vector<T>& x)
{
	return sqrt(VarianceOfPopulation(x));
}


template <typename T> static T VarianceOfPopulation(vector<T>& x)
{
	T mean = Mean(x), sumSq = 0;
	int n = x.size();

	for (unsigned long int i = 0; i < n; i++)
	{
		T delta = x[i] - mean;

		sumSq += delta * delta;
	}

	return sumSq / n;
}

template <typename T> static void Normalize(vector<T>& dataset)
{
	const T Epsilon = 0.000001;
	T range = Max(dataset) - Min(dataset);
	for (unsigned long int i = 0; i < dataset.size(); i++)
	{
		dataset[i] = dataset[i] / (range + Epsilon);
	}
}

template <typename T> static T Max(vector<T>& x)
{
	T max = x[0];

	for (unsigned long int i = 0; i < x.size(); i++)
		if (x[i] > max)
			max = x[i];

	return max;
}

template <typename T> static T Min(vector<T>& x)
{
	T min = x[0];

	for (unsigned long int i = 0; i < x.size(); i++)
		if (x[i] < min)
			min = x[i];

	return min;
}

template <typename T> void NormalizeN(std::vector<T>& input, std::vector<int>& minMax)
{
	T min = input[0];
	T max = input[0];

	for (unsigned long int i = 0; i < input.size(); i++)
	{
		if (input[i] > max)
			max = input[i];
		if (input[i] < max)
			min = input[i];
	}
	minMax[0] = min;
	minMax[1] = max;

	T range = max - min;
	for (int i = 0; i < input.size(); i++)
		input[i] = (input[i] - min) / range;
}

template <typename T> void DeNormalizeN(std::vector<T>& input, std::vector<int>& minMax)
{
	T min = minMax[0];
	T max = minMax[1];


	T range = max - min;
	for (int i = 0; i < input.size(); i++)
		input[i] = (int)(input[i] * range + min);
}
template <typename T> void Compress(std::vector<T>& input, std::vector<int>& minMax)
{
	T min = input[0];
	T max = input[0];

	for (unsigned long int i = 0; i < input.size(); i++)
	{
		if (input[i] >= max)
			max = input[i];
		if (input[i] <= max)
			min = input[i];
	}
	minMax[0] = min;
	minMax[1] = max;

	T range = max - min;
	for (int i = 0; i < input.size(); i++)
		input[i] = input[i] / range;
}

template <typename T> void DeCompress(std::vector<T>& input, std::vector<int>& minMax)
{
	T min = minMax[0];
	T max = minMax[1];


	T range = max - min;
	for (int i = 0; i < input.size(); i++)
		input[i] = (int)(input[i] * range);
}
#endif // STATISTICFUNCTIONS_H
