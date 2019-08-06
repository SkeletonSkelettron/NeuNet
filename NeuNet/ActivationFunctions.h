#ifndef ACTIVATIONFUNCTIONS_H
#define ACTIVATIONFUNCTIONS_H
#include <vector>
#include <math.h>
#include <cmath>
#include "enums.h"
#include "statisticfunctions.h"

const long double PI2 = 6.283185307179586476L;
const long double SQ2 = 1.414213562373095048L;

template <typename T> void BalanceWith(std::vector<T> &dataset, NeuralEnums::BalanceType BalancingMethod)
{
	switch (BalancingMethod)
	{
	case NeuralEnums::BalanceType::None: break;
	case NeuralEnums::BalanceType::GaussianStandartization:
	{
		Standartize(dataset);
		break;
	}
	case NeuralEnums::BalanceType::Normalization: Normalize(dataset);

	default:
		break;
	}
}

template <typename T> T CalculateLoss(NeuralEnums::LossFunctionType &function, std::vector<T> &output, std::vector<T> &target)
{
	switch (function)
	{
	case NeuralEnums::LossFunctionType::MeanSquaredLoss: return MSL(output, target);
	case NeuralEnums::LossFunctionType::CrossEntropyLoss: return CEL(output, target);

	default:
		break;
	}
}

template <typename T> T  _CEL(T &output, T &target)
{
	return -target * log(output) - (1 - target)*log(1 - output);
}
template <typename T> T CEL(std::vector<T> &output, std::vector<T> &target)
{
	if (output.size() != target.size())
		throw runtime_error("შემავალი მასივების სიგრძეები განსხვავდება");
	int n = output.size();
	T sum = 0;
	for (int i = 0; i < output.size(); i++)
	{
		sum += _CEL(output[i], target[i]);
	}
	return sum;
}
template <typename T> T MSL(std::vector<T> &output, std::vector<T> &target)
{
	if (output.size() != target.size())
		throw runtime_error("შემავალი მასივების სიგრძეები განსხვავდება");
	T Sum = 0;
	T n = output.size();
	for (unsigned long int i = 0; i < output.size(); i++)
	{
		Sum += MSL(target[i], output[i]) / n;
	}
	return Sum;
}
template <typename T> T MSL(T &output, T &target)
{
	return pow((target - output), 2) / 2;
}

template <typename T> T DifferentiateLossWith(T &output, T &target, NeuralEnums::LossFunctionType &function)
{
	switch (function)
	{
	case NeuralEnums::LossFunctionType::MeanSquaredLoss: return output - target;
	case NeuralEnums::LossFunctionType::CrossEntropyLoss: return CELDerevative(output, target);

	default:
		break;
	}
}

template <typename T> T CELDerevative(T &output, T &target)
{
	return -target / output + (1 - target) / (1 - output);
}


template <typename T> T DifferentiateWith(T &x, NeuralEnums::ActivationFunction &function/*, std::vector<T> &layerInputs*/)
{
	switch (function)
	{
		//case NeuralEnums::ActivationFunction::SoftMax: return GetSoftMaxDerivative(x, layerInputs);
	case NeuralEnums::ActivationFunction::Sigmoid: return SigmoidDerivative(x);
	case NeuralEnums::ActivationFunction::ReLU: return ReLUDerivative(x);
	case NeuralEnums::ActivationFunction::MReLU: return MReLUDerivative(x);
	case NeuralEnums::ActivationFunction::Tanh: return TanhDerivative(x);
	case NeuralEnums::ActivationFunction::GeLU: return GeLUDerivative(x);
	case NeuralEnums::ActivationFunction::SoftPlus: return SoftPlusDerivative(x);
	default:
		break;
	}
}

template <typename T> T SoftSign(T &x)
{
	return x / (abs(x) + 1);
}
template <typename T> T SoftSignDerivative(T &x)
{
	return  1 /pow(1 + abs(x),2);
}

template <typename T> T SoftPlus(T &x)
{
	return log(1 + exp(x));
}
template <typename T> T SoftPlusDerivative(T &x)
{
	return  1 / (1 + exp(-x));
}
template <typename T> T SoftMax(T x, std::vector<T> &layerInputs)
{
	T sum = 0;
	for (unsigned long int i = 0; i < layerInputs.size(); i++)
	{
		sum += exp(layerInputs[i]);
	}
	return exp(x) / sum;
}

template <typename T> T SoftMaxDerivative(T x, std::vector<T> &layerInputs)
{
	auto y = SoftMax(x, layerInputs);
	return y * (1.0 - y);
}

template <typename T> T Sigmoid(T &x)
{
	return  1 / (1 + exp(-x));
}

template <typename T> T SigmoidDerivative(T &x)
{
	T sigm = Sigmoid(x);
	return sigm * (1 - sigm);
}

template <typename T> T ReLU(T &x)
{
	return x <= 0 ? 0 : x;
}

template <typename T> T ReLUDerivative(T &x)
{
	return x == 0 ? 0 : 1;
}

template <typename T> T Tanh(T &x)
{
	return tanh(x);
}

template <typename T> T TanhDerivative(T &x)
{
	return 1  - tanh(x) * tanh(x);
}

template <typename T> T MReLU(T &x)
{
	return x < 0 ? -0.0005*x : x;
}

template <typename T> T MReLUDerivative(T &x)
{
	return x < 0 ? -0.0005 : 1;
}

template <typename T> T GeLU(T &x)
{
	return 0.5 * x * (1 + erf(x / SQ2));
}

template <typename T> T GeLUDerivative(T &x)
{
	return 0.5 + 0.5 * erf(x / SQ2) + x / (exp(-(x * x) / 2.0L) * pow(PI2, 0.5));
}


template <typename T> int GetMaxIndex(std::vector<T> &outPut)
{
	int index = 0;
	T val = outPut[0];
	for (unsigned long int i = 0; i < outPut.size(); i++)
	{
		if (outPut[i] > val)
		{
			val = outPut[i];
			index = i;
		}
	}
	return index;
}

template <typename T> void NormalizeN(std::vector<T> &input)
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
	T range = max - min;
	for (unsigned long int i = 0; i < input.size(); i++)
		input[i] = (input[i] - min) / range;
}
#endif // UTILITYFUNCTIONS_H
























