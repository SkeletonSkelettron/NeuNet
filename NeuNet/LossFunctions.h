#include "enums.h"
#include <vector>
#include <math.h>
#include <cmath>

template <typename T> T CalculateLossFunction(NeuralEnums::LossFunctionType& function, std::vector<T>& output, std::vector<T>& target, int start, int end)
{
	switch (function)
	{
	case NeuralEnums::LossFunctionType::MeanSquaredError: return MSL(output, target, start, end);
	case NeuralEnums::LossFunctionType::BinaryCrossentropy: return BinaryCrossentropy(output, target);
	//case NeuralEnums::LossFunctionType::KullbackLeiblerDivergence: return KullbackLeiblerDivergence(output, target);
	default:
		break;
	}
}
template <typename T> T DifferentiateLossWith(T& output, T& target, NeuralEnums::LossFunctionType& function, int size)
{
	switch (function)
	{
	case NeuralEnums::LossFunctionType::MeanSquaredError: return output - target;
	case NeuralEnums::LossFunctionType::BinaryCrossentropy: return BinaryCrossentropyDerivative(output, target, size);
	case NeuralEnums::LossFunctionType::KullbackLeiblerDivergence: return KullbackLeiblerDivergenceDerivative(output, target);
	default:
		break;
	}
}


template <typename T> T  KullbackLeiblerDivergence(std::vector<T>& roHat, T& ro, int start, int end)
{
	T sum = 0.0;
	for (size_t i = start; i < end; i++)
		sum += ro * log(ro / roHat[i]) + (1 - ro) * log((1 - ro) / (1 - roHat[i]));
	return sum;
}

template <typename T> T  KullbackLeiblerDivergenceDerivative(T& output, T& target)
{
	//TODO არ მუშაობს
	return log(output / target) + 1 / target;
}

template <typename T> T  BinaryCrossentropy(std::vector<T>& output, std::vector<T>& target)
{
	T sum = 0;
	for (int i = 0; i < target.size(); i++)
	{
		sum += target[i] * log(output[i]) - (1 - target[i]) * log(1 - output[i]);
	}
	return -sum / target.size();
}

template <typename T> T  BinaryCrossentropyDerivative(T& output, T& target, int size)
{
	return (-target / output + (1 - target) / (1 - output)) / size;
}

template <typename T> T  _CEL(T& output, T& target)
{
	return -target * log(output) - (1 - target) * log(1 - output);
}
template <typename T> T CEL(std::vector<T>& output, std::vector<T>& target)
{
	if (output.size() != target.size())
		throw runtime_error("input arrays have different lengths");
	int n = output.size();
	T sum = 0;
	for (int i = 0; i < output.size(); i++)
	{
		sum += _CEL(output[i], target[i]);
	}
	return sum / output.size();
}
template <typename T> T MSL(std::vector<T>& output, std::vector<T>& target, int start, int end)
{
	if (output.size() != target.size())
		throw runtime_error("input arrays have different lengths");
	T Sum = 0;
	T n = output.size();
	for (unsigned long int i = start; i < end; i++)
	{
		Sum += MSL(target[i], output[i]) / n;
	}
	return Sum;
}
template <typename T> T MSL(T& output, T& target)
{
	return pow((target - output), 2) / 2;
}

template <typename T> T CELDerevative(T& output, T& target)
{
	return -target / output + (1 - target) / (1 - output);
}
//
//template <typename T> T CalculateRegularisationCost(NeuralNetwork<T>& nn, T weightDecayParameter)
//{
//	T result = 0.0;
//	for (size_t i = nn.Layers.size() - 1; i > 1; i--)
//	{
//		for (size_t j = 0; j < nn.Layers[i].Weights.size(); j++)
//		{
//			result += nn.Layers[i].Weights[j] * nn.Layers[i].Weights[j];
//		}
//	}
//	return (result * weightDecayParameter) / 2.0;
//}