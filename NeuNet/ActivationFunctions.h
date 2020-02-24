#ifndef ACTIVATIONFUNCTIONS_H
#define ACTIVATIONFUNCTIONS_H
#include <vector>
#include <math.h>
#include <cmath>
#include "enums.h"
#include "statisticfunctions.h"

const long double PI2 = 6.283185307179586476L;
const long double SQ2 = 1.414213562373095048L;

template <typename T> void BalanceWith(std::vector<T>& dataset, NeuralEnums::BalanceType BalancingMethod)
{
	switch (BalancingMethod)
	{
	case NeuralEnums::BalanceType::None: break;
	case NeuralEnums::BalanceType::GaussianStandartization:
	{
		Standartize(dataset);
		break;
	}
	case NeuralEnums::BalanceType::Normalization:
	{
		std::vector<int> tmp;
		tmp.resize(2);
		Compress(dataset, tmp);
		break;
	}

	default:
		break;
	}
}

template <typename T> void ActivateWith(
	std::vector<T>& inputs, 
	std::vector<T>& outputs, 
	std::vector<int>& indexVerctor, 
	bool batch, 
	std::vector<bool>& drops, 
	int& start, 
	int& end, 
	bool usingdrops, 
	NeuralEnums::ActivationFunction& function)
{
	if (function == NeuralEnums::ActivationFunction::Sigmoid)
	{
		Sigmoid(inputs, outputs, indexVerctor, batch, drops, start, end, usingdrops);
		return;
	}
	else if (function == NeuralEnums::ActivationFunction::ReLU)
	{
		ReLU(inputs, outputs, indexVerctor, batch, drops, start, end, usingdrops);
		return;
	}
	else if (function == NeuralEnums::ActivationFunction::MReLU)
	{
		MReLU(inputs, outputs, indexVerctor, batch, drops, start, end, usingdrops);
		return;
	}
	else if (function == NeuralEnums::ActivationFunction::Tanh)
	{
		Tanh(inputs, outputs, indexVerctor, batch, drops, start, end, usingdrops);
		return;
	}
	else if (function == NeuralEnums::ActivationFunction::GeLU)
	{
		GeLU(inputs, outputs, indexVerctor, batch, drops, start, end, usingdrops);
		return;
	}
	else if (function == NeuralEnums::ActivationFunction::SoftPlus)
	{
		SoftPlus(inputs, outputs, indexVerctor, batch, drops, start, end, usingdrops);
		return;
	}
	else if (function == NeuralEnums::ActivationFunction::SoftSign)
	{
		SoftSign(inputs, outputs, indexVerctor, batch, drops, start, end, usingdrops);
		return;
	}
	else
		throw runtime_error("ActivationFunction not assigned");
}

template <typename T> void GeLU(std::vector<T>& inputs, std::vector<T>& outputs, std::vector<int>& indexVerctor, bool batch, std::vector<bool>& drops, int& start, int& end, bool& usingdrops)
{
	if (batch)
	{
		for (int i : indexVerctor)
			outputs[i] = GeLU(inputs[i]);
	}
	else
	{
		for (size_t i = start; i < end; i++)
		{
			if (usingdrops && drops[i])
				continue;
			outputs[i] = GeLU(inputs[i]);
		}
	}
}
template <typename T> void Sigmoid(std::vector<T>& inputs, std::vector<T>& outputs, std::vector<int>& indexVerctor, bool batch, std::vector<bool>& drops, int& start, int& end, bool& usingdrops)
{
	if (batch)
	{
		for (int i : indexVerctor)
			outputs[i] = Sigmoid(inputs[i]);
	}
	else
	{
		for (size_t i = start; i < end; i++)
		{
			if (usingdrops && drops[i])
				continue;
			outputs[i] = Sigmoid(inputs[i]);
		}
	}
}
template <typename T> void Tanh(std::vector<T>& inputs, std::vector<T>& outputs, std::vector<int>& indexVerctor, bool batch, std::vector<bool>& drops, int& start, int& end, bool& usingdrops)
{
	if (batch)
	{
		for (int i : indexVerctor)
			outputs[i] = tanh(inputs[i]);
	}
	else
	{
		for (size_t i = start; i < end; i++)
		{
			if (usingdrops && drops[i])
				continue;
			outputs[i] = tanh(inputs[i]);
		}
	}
}
template <typename T> void MReLU(std::vector<T>& inputs, std::vector<T>& outputs, std::vector<int>& indexVerctor, bool batch, std::vector<bool>& drops, int& start, int& end, bool& usingdrops)
{
	if (batch)
	{
		for (int i : indexVerctor)
			outputs[i] = MReLU(inputs[i]);
	}
	else
	{
		for (size_t i = start; i < end; i++)
		{
			if (usingdrops && drops[i])
				continue;
			outputs[i] = MReLU(inputs[i]);
		}
	}
}
template <typename T> void ReLU(std::vector<T>& inputs, std::vector<T>& outputs, std::vector<int>& indexVerctor, bool batch, std::vector<bool>& drops, int& start, int& end, bool& usingdrops)
{
	if (batch)
	{
		for (int i : indexVerctor)
			outputs[i] = ReLU(inputs[i]);
	}
	else
	{
		for (size_t i = start; i < end; i++)
		{
			if (usingdrops && drops[i])
				continue;
			outputs[i] = ReLU(inputs[i]);
		}
	}
}
template <typename T> void SoftMax(std::vector<T>& inputs, std::vector<T>& inputsSoftMax, std::vector<T>& outputs, std::vector<int>& indexVerctor, bool batch, std::vector<bool>& drops, int& start, int& end, bool& usingdrops)
{
	//TODO მაინც კაი სანახავია როგორ მუშაობს
	for (int i : indexVerctor)
		outputs[i] = SoftMax(inputs[i], inputsSoftMax, dropoutNeurons);
}
template <typename T> void SoftPlus(std::vector<T>& inputs, std::vector<T>& outputs, std::vector<int>& indexVerctor, bool batch, std::vector<bool>& drops, int& start, int& end, bool& usingdrops)
{
	if (batch)
	{
		for (int i : indexVerctor)
			outputs[i] = SoftPlus(inputs[i]);
	}
	else
	{
		for (size_t i = start; i < end; i++)
		{
			if (usingdrops && drops[i])
				continue;
			outputs[i] = SoftPlus(inputs[i]);
		}
	}
}
template <typename T> void SoftSign(std::vector<T>& inputs, std::vector<T>& outputs, std::vector<int>& indexVerctor, bool batch, std::vector<bool>& drops, int& start, int& end, bool& usingdrops)
{
	if (batch)
	{
		for (int i : indexVerctor)
			outputs[i] = SoftSign(inputs[i]);
	}
	else
	{
		for (size_t i = start; i < end; i++)
		{
			if (usingdrops && drops[i])
				continue;
			outputs[i] = SoftSign(inputs[i]);
		}
	}
}

template <typename T> void Assign(std::vector<T>& inputs, std::vector<T>& outputs, std::vector<int>& indexVerctor, bool batch, std::vector<bool>& drops, int& start, int& end, bool& usingdrops)
{
	if (batch)
	{
		for (int i : indexVerctor)
			outputs[i] = inputs[i];
	}
	else
	{
		for (size_t i = start; i < end; i++)
		{
			if (usingdrops && drops[i])
				continue;
			outputs[i] = inputs[i];
		}
	}
}

template <typename T> T DifferentiateWith(T& x, NeuralEnums::ActivationFunction& function, std::vector<T>& inputs, std::vector<bool>& dropouts)
{
	if (function == NeuralEnums::ActivationFunction::Sigmoid)
		return SigmoidDerivative(x);
	else if (function == NeuralEnums::ActivationFunction::ReLU)
		return ReLUDerivative(x);
	else if (function == NeuralEnums::ActivationFunction::MReLU)
		return MReLUDerivative(x);
	else if (function == NeuralEnums::ActivationFunction::Tanh)
		return TanhDerivative(x);
	else if (function == NeuralEnums::ActivationFunction::GeLU)
		return GeLUDerivative(x);
	else if (function == NeuralEnums::ActivationFunction::SoftPlus)
		return SoftPlusDerivative(x);
	else
		throw runtime_error("ActivationFunction not assigned");
}

template <typename T> inline T SoftSign(T& x)
{
	return x / (abs(x) + 1);
}
template <typename T> inline T SoftSignDerivative(T& x)
{
	return  1.0 / pow(1.0 + abs(x), 2);
}

template <typename T> T SoftPlus(T& x)
{
	return log(1.0 + exp(x));
}
template <typename T> inline T SoftPlusDerivative(T& x)
{
	return  1.0 / (1.0 + exp(-x));
}
template <typename T> inline T SoftMax(T x, std::vector<T>& layerInputs, std::vector<int>& indexVector)
{
	T sum = 0.0;
	for (int i : indexVector)
	{
		sum += exp(layerInputs[i]);
	}
	return exp(x) / sum;
}

template <typename T> inline T SoftMaxDerivative(T& x, std::vector<T>& inputs, std::vector<int>& indexVector)
{
	T y = SoftMax(x, inputs, indexVector);
	return y * (1.0 - y);
}

template <typename T> inline T Sigmoid(T& x)
{
	return  1.0 / (1.0 + exp(-x));
}

template <typename T> inline T SigmoidDerivative(T& x)
{
	T sigm = Sigmoid(x);
	return sigm * (1.0 - sigm);
}

template <typename T> inline T ReLU(T& x)
{
	return x <= 0.0 ? 0.0 : x;
}

template <typename T> inline T ReLUDerivative(T& x)
{
	return x == 0.0 ? 0.0 : 1.0;
}

template <typename T> inline T Tanh(T& x)
{
	return tanh(x);
}

template <typename T> inline T TanhDerivative(T& x)
{
	return 1.0 - tanh(x) * tanh(x);
}

template <typename T> inline T MReLU(T& x)
{
	return x < 0.0 ? 0.0005 * x : x;
}

template <typename T> inline T MReLUDerivative(T& x)
{
	return x < 0.0 ? 0.0005 : 1.0;
}

template <typename T> inline T GeLU(T& x)
{
	return 0.5 * x * (1.0 + erf(x / SQ2));
}

template <typename T> inline T GeLUDerivative(T& x)
{
	return 0.5 + 0.5 * erf(x / SQ2) + x / (exp(-(x * x) / 2.0) * pow(PI2, 0.5));
}


template <typename T> int GetMaxIndex(std::vector<T>& outPut)
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
//


template <typename T> T exp1024(T x)
{
	x = 1.0 + x / 256.0;
	x *= x; x *= x; x *= x; x *= x;
	x *= x; x *= x; x *= x; x *= x;
	return x;
}

#endif ACTIVATIONFUNCTIONS_H