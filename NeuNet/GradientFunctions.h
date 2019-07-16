#pragma once
#include <math.h>

template <typename T> T GetGradient(NeuralEnums::GradientType gradientType, std::vector<T> &gradients, T gradient, int j)
{
	switch (gradientType)
	{
	case NeuralEnums::GradientType::Static: return gradient;
	case NeuralEnums::GradientType::Momentum: return Momentum(gradients, gradient, j);

	default:
		break;
	}
}

template <typename T> T Momentum(std::vector<T> &gradients, T gradient, int j)
{
	const T MomentumRate = 0.9;
	gradients[j] = MomentumRate * gradients[j] + (1 - MomentumRate) * gradient;

	return gradients[j];
}