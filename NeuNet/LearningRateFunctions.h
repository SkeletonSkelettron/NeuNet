#ifndef LEARNINGRATEFUNCTIONS_H
#define LEARNINGRATEFUNCTIONS_H
#include <math.h>
template<class T> class NeuralNetwork;
const float momentum = 0.9;
const float epsilon = 0.0000001;
const float startingLearningRate = 0.001;
const float beta1 = 0.9;
const float beta2 = 0.999;

template <typename T> T GetLearningRateMultipliedByGrad(NeuralNetwork<T>& nn, T& gradient, int& iterator, int& j)
{
	//if (nn.LearningRateType == NeuralEnums::LearningRateType::Static)
	//	return nn.LearningRate * gradient;
	//else if (nn.LearningRateType == NeuralEnums::LearningRateType::AdaGrad)
	//	return AdaGrad(nn.Layers[iterator].GradientsLR, gradient, j) * gradient;
	//else if (nn.LearningRateType == NeuralEnums::LearningRateType::AdaDelta)
	//	return AdaDelta(nn.Layers[iterator].GradientsLR, nn.Layers[iterator].Parameters, gradient, j) * gradient;
	//else if (nn.LearningRateType == NeuralEnums::LearningRateType::Adam)
	//	return Adam(nn, gradient, j, iterator);
	//else if (nn.LearningRateType == NeuralEnums::LearningRateType::AdaMax)
	//	return AdaMax(nn, gradient, j, iterator);
	//else if (nn.LearningRateType == NeuralEnums::LearningRateType::AdamMod)
	//	return AdamMod(nn, gradient, j, iterator);
	//else if (nn.LearningRateType == NeuralEnums::LearningRateType::RMSProp)
	//	return RMSProp(nn.Layers[iterator].GradientsLR, gradient, j) * gradient;
	//else if (nn.LearningRateType == NeuralEnums::LearningRateType::GuraMethod)
	//	return GuraMethod(nn.Layers[iterator].GradientsLR, nn.Layers[iterator].LearningRates, gradient, j, nn.LearningRate) * gradient;
		switch (nn.LearningRateType)
		{
		case NeuralEnums::LearningRateType::Static: 
			return nn.LearningRate * gradient;
		case NeuralEnums::LearningRateType::AdaGrad: 
			return AdaGrad(nn.Layers[iterator].GradientsLR, gradient, j) * gradient;
		case NeuralEnums::LearningRateType::AdaDelta: 
			return AdaDelta(nn.Layers[iterator].GradientsLR, nn.Layers[iterator].Parameters, gradient, j) * gradient;
			//following 3 methods does not require gradient multiplication
		case NeuralEnums::LearningRateType::Adam: 
			return Adam(nn, gradient, j, iterator);
		case NeuralEnums::LearningRateType::AdaMax: 
			return AdaMax(nn, gradient, j, iterator);
		case NeuralEnums::LearningRateType::AdamMod: 
			return AdamMod(nn, gradient, j, iterator);
		case NeuralEnums::LearningRateType::RMSProp: 
			return RMSProp(nn.Layers[iterator].GradientsLR, gradient, j) * gradient;
		case NeuralEnums::LearningRateType::GuraMethod: 
			return GuraMethod(nn.Layers[iterator].GradientsLR, nn.Layers[iterator].LearningRates, gradient, j, nn.LearningRate) * gradient;
		default:
		{
			throw runtime_error("learning rate function not defined");
		}
		}
}


template <typename T> T Adam(NeuralNetwork<T>& nn, T& gradient, int& j, int& iterator)
{
	T result, param;

	//mt
	nn.Layers[iterator].Parameters[j] = beta1 * nn.Layers[iterator].Parameters[j] + (1 - beta1) * gradient;
	//vt
	nn.Layers[iterator].GradientsLR[j] = beta2 * nn.Layers[iterator].GradientsLR[j] + (1 - beta2) * gradient * gradient;


	return (nn.LearningRate * nn.Layers[iterator].Parameters[j]) / ((1 - nn.beta1Pow) * (sqrt(nn.Layers[iterator].GradientsLR[j] / (1 - nn.beta2Pow)) + epsilon));
}

template <typename T> T AdaGrad(std::vector<T>& gradients, T& gradient, int& j)
{
	gradients[j] += gradient * gradient;
	return 0.01 / sqrt(gradients[j] + epsilon);
}


template <typename T> T AdaDelta(std::vector<T>& gradients, std::vector<T>& parameters, T& Gradient, int& j)
{
	T result, param;
	gradients[j] = momentum * gradients[j] + (1 - momentum) * Gradient * Gradient;
	result = sqrt(parameters[j] + epsilon) / sqrt(gradients[j] + epsilon);
	param = result * Gradient;
	parameters[j] = momentum * parameters[j] + (1 - momentum) * param * param;
	return result;
}

template <typename T> T AdamMod(NeuralNetwork<T>& nn, T& Gradient, int& j, int& iterator)
{
	T result, param;
	T prelim = (1 - momentum) * Gradient;

	nn.Layers[iterator].GradientsLR[j] = momentum * nn.Layers[iterator].GradientsLR[j] + prelim * Gradient;
	nn.Layers[iterator].Parameters[j] = momentum * nn.Layers[iterator].Parameters[j] + prelim;

	return (nn.LearningRate * nn.Layers[iterator].Parameters[j] / (1 - beta1)) / (sqrt(nn.Layers[iterator].GradientsLR[j] / (1 - beta2)) + epsilon);
}


template <typename T> T AdaMax(NeuralNetwork<T>& nn, T& gradient, int& j, int& iterator)
{
	T result, param;

	//mt
	nn.Layers[iterator].Parameters[j] = beta1 * nn.Layers[iterator].Parameters[j] + (1 - beta1) * gradient;
	//vt
	nn.Layers[iterator].GradientsLR[j] = max(beta2 * nn.Layers[iterator].GradientsLR[j], abs(gradient));


	return (nn.LearningRate * nn.Layers[iterator].Parameters[j]) / ((1 - nn.beta1Pow) * nn.Layers[iterator].GradientsLR[j]);
}

template <typename T> T RMSProp(std::vector<T>& gradients, T& gradient, int& j)
{
	gradients[j] = momentum * gradients[j] + (1 - momentum) * gradient * gradient;
	return startingLearningRate / sqrt(gradients[j] + epsilon);
}

template <typename T> T GuraMethod(std::vector<T>& gradients, std::vector<T>& lrTensor, T& gradient, int& j, T& learningrate)
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