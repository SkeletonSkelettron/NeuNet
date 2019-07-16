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

    return sum / static_cast<T>(n);
}


template <typename T> static void Standartize(vector<T> &dataset)
{
    const T Epsilon = static_cast<T>(0.000001);
    auto mean = Mean(dataset);
    auto StDev = StandardDeviationOfPopulation(dataset);

    for (unsigned long int i = 0; i < dataset.size(); i++)
        dataset[i] = (dataset[i] - mean) / sqrt(StDev + Epsilon);
}

template <typename T> static T StandardDeviationOfPopulation(vector<T> &x)
{
    return sqrt(VarianceOfPopulation(x));
}


template <typename T> static T VarianceOfPopulation(vector<T> &x)
{
    T mean = Mean(x), sumSq = static_cast<T>(0);
    int n = x.size();

    for (unsigned long int i = 0; i < n; i++)
    {
        double delta = x[i] - mean;

        sumSq += delta * delta;
    }

    return sumSq / static_cast<T>(n);
}

template <typename T> static void Normalize(vector<T> &dataset)
{
    const T Epsilon = static_cast<T>(0.000001);
    T range = Max(dataset) - Min(dataset);
    for (unsigned long int i = 0; i < dataset.size(); i++)
    {
        dataset[i] = dataset[i] / (range + Epsilon);
    }
}

template <typename T> static T Max(vector<T> &x)
{
    T max = x[0];

    for (unsigned long int i = 0; i < x.size(); i++)
        if (x[i] > max)
            max = x[i];

    return max;
}

template <typename T> static T Min(vector<T> &x)
{
    T min = x[0];

    for (unsigned long int i = 0; i < x.size(); i++)
        if (x[i] < min)
            min = x[i];

    return min;
}
#endif // STATISTICFUNCTIONS_H
