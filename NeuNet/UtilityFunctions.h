#ifndef UTILITYFUNCTIONS_H
#define UTILITYFUNCTIONS_H
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

template <typename T> T CalculateLoss(NeuralEnums::LossFunctionType Function, std::vector<T> &output, std::vector<T> &target)
{
	switch (Function)
	{
	case NeuralEnums::LossFunctionType::MeanSquaredLoss: return MSL(output, target);
		//case NeuralEnums::LossFunctionType::CrossEntropyLoss: return CEL(output, target);

	default:
		break;
	}
}

template <typename T> T MSL(std::vector<T> &output, std::vector<T> &target)
{
	if (output.size() != target.size())
		throw runtime_error("შემავალი მასივების სიგრძეები განსხვავდება");
	T Sum = static_cast<T>(0);
	T n = output.size();
	for (unsigned long int i = 0; i < output.size(); i++)
	{
		Sum += MSL(target[i], output[i]) / static_cast<T>(n);
	}
	return Sum;
}
template <typename T> T MSL(T output, T target)
{
	return pow((target - output), 2) / static_cast<T>(2);
}

template <typename T> T DifferentiateLossWith(T Output, T Target, NeuralEnums::LossFunctionType Function)
{
	switch (Function)
	{
	case NeuralEnums::LossFunctionType::MeanSquaredLoss: return Output - Target;
		//case NeuralEnums::LossFunctionType::CrossEntropyLoss: return CELDerevative(Output, Target);

	default:
		break;
	}
}

template <typename T> T CELDerevative(T Output, T Target)
{
	return -Target / Output + (static_cast<T>(1) - Target) / (static_cast<T>(1) - Output);
}

template <typename T> T DifferentiateWith(T x, std::vector<T> &layerInputs, NeuralEnums::ActivationFunction Function)
{
	switch (Function)
	{
	case NeuralEnums::ActivationFunction::SoftMax: return GetSoftMaxDerivative(x, layerInputs);

	default:
		break;
	}
}

template <typename T> T DifferentiateWith(T x, NeuralEnums::ActivationFunction Function, std::vector<T> &layerInputs)
{
	switch (Function)
	{
	case NeuralEnums::ActivationFunction::SoftMax: return GetSoftMaxDerivative(x, layerInputs);
	case NeuralEnums::ActivationFunction::Sigmoid: return GetSigmoidDerivative(x);
	case NeuralEnums::ActivationFunction::ReLU: return GetReLUDerivative(x);
	case NeuralEnums::ActivationFunction::MReLU: return GetMReLUDerivative(x);
	case NeuralEnums::ActivationFunction::Tanh: return GetTanhDerivative(x);
	case NeuralEnums::ActivationFunction::GELU: return GetGELUDerivative(x);
	default:
		break;
	}
}

template <typename T> T SoftMax(T x, std::vector<T> &layerInputs)
{
	T sum = static_cast<T>(0);
	for (unsigned long int i = 0; i < layerInputs.size(); i++)
	{
		sum += exp(layerInputs[i]);
	}
	return exp(x) / sum;
}

template <typename T> T GetSoftMaxDerivative(T x, std::vector<T> &layerInputs)
{
	auto y = SoftMax(x, layerInputs);
	return y * (static_cast<T>(1) - y);
}

template <typename T> T Sigmoid(T x)
{
	return  static_cast<T>(1) / (static_cast<T>(1) + exp(-x));
}

template <typename T> T GetSigmoidDerivative(T x)
{
	T sigm = Sigmoid(x);
	return sigm * (static_cast<T>(1) - sigm);
}

template <typename T> T ReLU(T x)
{
	return x <= 0 ? static_cast<T>(0) : x;
}

template <typename T> T GetReLUDerivative(T x)
{
	return x == 0 ? static_cast<T>(0) : static_cast<T>(1);
}

template <typename T> T Tanh(T x)
{
	return static_cast<T>(1) / tan(x);
}

template <typename T> T GetTanhDerivative(T x)
{
	return static_cast<T>(1) / (static_cast<T>(1) + x * x);
}

template <typename T> T MReLU(T x)
{
	return x < 0 ? -static_cast<T>(0.0005)*x : x;
}

template <typename T> T GetMReLUDerivative(T x)
{
	return x < 0 ? -static_cast<T>(0.0005) : static_cast<T>(1);
}

template <typename T> T GELU(T x)
{
	return 0.5L * x * (1.0L + erf(x / SQ2));
}

template <typename T> T GetGELUDerivative(T x)
{
	return 0.5L + 0.5L * erf(x / SQ2) +  x   / (exp(-(x * x) / 2.0L) * pow(PI2, 0.5L));
}

template <typename T> T GetGradient(NeuralEnums::GradientType gradientType, T gradTen, T gradient)
{
	switch (gradientType)
	{
	case NeuralEnums::GradientType::Momentum: return Momentum(gradTen, gradient);

	default:
		break;
	}
}

template <typename T> T Momentum(T gradTen, T gradient)
{
	const T MomentumRate = static_cast<T>(0.9);
	gradTen = MomentumRate * gradTen + (static_cast<T>(1) - MomentumRate) * gradient;

	return gradTen;
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
	int min = input[0];
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

//template <typename T> T GetLearningRate(NeuralEnums::LearningRateType learningRateType, T gradient, T learningRate = 0.1)
//{
//	switch (learningRateType)
//	{
//	case NeuralEnums::LearningRateType::AdaGrad: return AdaGrad(NN, gradient);
//	case NeuralEnums::LearningRateType::RMSProp: return RMSProp(NN, gradient);
//
//	default:
//		break;
//	}
//}

//template <typename T> T AdaGrad(this NeuralNetwork NN, double Gradient, int i, int j, int k)
//{
//    const T epsilon = 0.00000001; /// TODO
//    const T StartingLearningRate = 0.1;

//    NN.GradientsTensor[i][j][k] += Gradient * Gradient;
//    return StartingLearningRate/Math.Sqrt(NN.GradientsTensor[i][j][k] + epsilon);
//}

//template <typename T> T RMSProp(this NeuralNetwork NN, T Gradient)
//{
//    const T momentum = 0.9;
//    const T epsilon = 0.0000001;
//    const T StartingLearningRate = 0.1;

//    NN.GradientsTensor[i][j][k] = momentum * NN.GradientsTensor[i][j][k] + (1-momentum) * Gradient * Gradient;

//    return StartingLearningRate / Math.Sqrt(NN.GradientsTensor[i][j][k] + epsilon);
//}

//template <typename T> T Cyclic(int LocalEpochs)
//{
//    const double momentum = 0.9;
//    const double epsilon = 0.0000001;
//    const double StartingLearningRate = 0.1;


//    return StartingLearningRate /1;
//}
#endif // UTILITYFUNCTIONS_H
























