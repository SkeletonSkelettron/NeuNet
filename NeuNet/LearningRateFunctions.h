#pragma once
#include <math.h>

template <typename T> T GetLearningRate(NeuralEnums::LearningRateType learningRateType,std::vector<T> &gradients, T gradient, int j, T &learningRate)
{
	switch (learningRateType)
	{
	case NeuralEnums::LearningRateType::Static: return learningRate;
	case NeuralEnums::LearningRateType::AdaGrad: return AdaGrad(gradients, gradient, j);
	case NeuralEnums::LearningRateType::RMSProp: return RMSProp(gradients, gradient, j);/*
	case NeuralEnums::LearningRateType.GuraMethod: return GuraMethod(NN, gradient, i, j, k);*/

	default:
		break;
	}
}

template <typename T> T AdaGrad(std::vector<T> &gradients, T Gradient, int j)
{
	const T epsilon = 0.00000001;
	const T StartingLearningRate = 0.01;

	gradients[j] += Gradient * Gradient;
	return StartingLearningRate / sqrt(gradients[j] + epsilon);
}

template <typename T> T RMSProp(std::vector<T> &gradients, T Gradient, int j)
{
	const T momentum = 0.9;
	const T epsilon = 0.0000001;
	const T StartingLearningRate = 0.001;

	gradients[j] = momentum * gradients[j] + (1 - momentum) * Gradient * Gradient;

	return StartingLearningRate / sqrt(gradients[j] + epsilon);
}
//
//template <typename T> T GuraMethod(NeuralNetwork<T> NN, double Gradient, int i, int j, int k)
//{
//	const T beta = 0.7d;
//
//	if ((NN.GradientsTensor[i][j][k] > 0 && Gradient < 0) || (NN.GradientsTensor[i][j][k] < 0 && Gradient > 0))
//	{
//		NN.LRTensor[i][j][k] = NN.LearningRate * beta;
//	}
//
//	if (NN.LRTensor[i][j][k] == 0)
//	{
//		NN.LRTensor[i][j][k] = NN.LearningRate;
//	}
//
//	NN.GradientsTensor[i][j][k] = Gradient;
//
//	return NN.LRTensor[i][j][k];
//}
//
//template <typename T> T Cyclic(this int LocalEpochs)
//{
//	const double momentum = 0.9d;
//	const double epsilon = 0.0000001d;
//	const double StartingLearningRate = 0.1d;
//
//
//	return StartingLearningRate / 1;
//}