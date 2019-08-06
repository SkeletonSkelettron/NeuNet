#ifndef LEARNINGRATEFUNCTIONS_H
#define LEARNINGRATEFUNCTIONS_H
#include <math.h>
template<class T> class NeuralNetwork;
const float momentum = 0.9;
const float epsilon = 0.0000001;
const float startingLearningRate = 0.001;
const float beta1 = 0.9;
const float beta2 = 0.999;

template <typename T> T GetLearningRate(NeuralNetwork<T> &nn, T &gradient, int &iterator, long int &j)
{
	switch (nn.LearningRateType)
	{
	case NeuralEnums::LearningRateType::Static: return nn.LearningRate * gradient;
	case NeuralEnums::LearningRateType::AdaGrad: return AdaGrad(nn.Layers[iterator].Gradients, gradient, j) * gradient;
	case NeuralEnums::LearningRateType::AdaDelta: return AdaDelta(nn.Layers[iterator].Gradients, nn.Layers[iterator].Parameters, gradient, j) * gradient;
	case NeuralEnums::LearningRateType::Adam: return Adam(nn, gradient, j, iterator);
	case NeuralEnums::LearningRateType::AdaMax: return AdaMax(nn, gradient, j, iterator);
	case NeuralEnums::LearningRateType::AdamMod: return AdamMod(nn, gradient, j, iterator);
	case NeuralEnums::LearningRateType::RMSProp: return RMSProp(nn.Layers[iterator].Gradients, gradient, j) * gradient;
	case NeuralEnums::LearningRateType::GuraMethod: return GuraMethod(nn.Layers[iterator].Gradients, nn.Layers[iterator].LearningRates, gradient, j, nn.LearningRate) * gradient;
	default:
		break;
	}
}
template <typename T> T AdaGrad(std::vector<T> &gradients, T &gradient, long int &j)
{
	gradients[j] += gradient * gradient;
	return 0.01 / sqrt(gradients[j] + epsilon);
}


template <typename T> T AdaDelta(std::vector<T> &gradients, std::vector<T> &parameters, T &Gradient, long int &j)
{
	T result, param;
	gradients[j] = momentum * gradients[j] + (1 - momentum) * Gradient * Gradient;
	result = sqrt(parameters[j] + epsilon) / sqrt(gradients[j] + epsilon);
	param = result * Gradient;
	parameters[j] = momentum * parameters[j] + (1 - momentum) * param * param;
	return result;
}

template <typename T> T AdamMod(NeuralNetwork<T> &nn, T &Gradient, long int &j, int &iterator)
{
	T result, param;
	T prelim = (1 - momentum) * Gradient;

	nn.Layers[iterator].Gradients[j] = momentum * nn.Layers[iterator].Gradients[j] + prelim * Gradient;
	nn.Layers[iterator].Parameters[j] = momentum * nn.Layers[iterator].Parameters[j] + prelim;

	return (nn.LearningRate * nn.Layers[iterator].Parameters[j] / (1 - beta1)) / (sqrt(nn.Layers[iterator].Gradients[j] / (1 - beta2)) + epsilon);
}

template <typename T> T Adam(NeuralNetwork<T> &nn, T &gradient, long int &j, int &iterator)
{
	T result, param;

	//mt
	nn.Layers[iterator].Parameters[j] = beta1 * nn.Layers[iterator].Parameters[j] + (1 - beta1) * gradient;
	//vt
	nn.Layers[iterator].Gradients[j] = beta2 * nn.Layers[iterator].Gradients[j] + (1 - beta2) * gradient* gradient;


	return (nn.LearningRate * nn.Layers[iterator].Parameters[j]) / ((1 - nn.beta1Pow) * (sqrt(nn.Layers[iterator].Gradients[j] / (1 - nn.beta2Pow)) + epsilon));
}

template <typename T> T AdaMax(NeuralNetwork<T> &nn, T &gradient, long int &j, int &iterator)
{
	T result, param;

	//mt
	nn.Layers[iterator].Parameters[j] = beta1 * nn.Layers[iterator].Parameters[j] + (1 - beta1) * gradient;
	//vt
	nn.Layers[iterator].Gradients[j] = max(beta2 * nn.Layers[iterator].Gradients[j],abs(gradient));


	return (nn.LearningRate * nn.Layers[iterator].Parameters[j]) / ((1 - nn.beta1Pow) * nn.Layers[iterator].Gradients[j]);
}

template <typename T> T RMSProp(std::vector<T> &gradients, T &gradient, long int &j)
{
	gradients[j] = momentum * gradients[j] + (1 - momentum) * gradient * gradient;
	return startingLearningRate / sqrt(gradients[j] + epsilon);
}

template <typename T> T GuraMethod(std::vector<T> &gradients, std::vector<T> &lrTensor, T &gradient, long int &j, T &learningrate)
{
	const T beta = 0.7;

	if ((gradients[j] > 0 && gradient < 0) || (gradients[j] < 0 && gradient > 0))
	{
		lrTensor[j] = learningrate * beta;
	}

	if (lrTensor[j] == 0)
	{
		lrTensor[j] = learningrate;
	}

	gradients[j] = gradient;

	return lrTensor[j];
}
#endif