#pragma once
#include <math.h>

template <typename T> T GetGradient(NeuralNetwork<T> &nn, T &gradient, long int &j, int &iterator)
{
	switch (nn.GradientType)
	{
	case NeuralEnums::GradientType::Static: return gradient;
	case NeuralEnums::GradientType::Momentum: return Momentum(nn.Layers[iterator].GradientsForGrads, gradient, j);

	default:
		break;
	}
}

template <typename T> T Momentum(std::vector<T> &gradients, T &gradient, long int &j)
{
	const T MomentumRate = 0.9;
	gradients[j] = MomentumRate * gradients[j] + (1 - MomentumRate) * gradient;

	return gradients[j];
}