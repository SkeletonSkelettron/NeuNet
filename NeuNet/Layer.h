#include "enums.h"
#include <vector>

template<class T> class Layer
{
public:
	int Size;
	std::vector<T> Inputs;
	std::vector<T> Weights;
	std::vector<T> Outputs;
	std::vector<T> MultipliedSums;
	std::vector<T> Gradients;
	std::vector<T> Parameters;
	std::vector<T> GradientsForGrads;
	std::vector<T> LearningRates;
	float DropOutSize;
	bool UsingBias;
	std::vector<bool> DropoutNeurons;
	NeuralEnums::ActivationFunction ActivationFunction;
	NeuralEnums::LayerType LayerType;
	Layer() {}
	Layer(int size, NeuralEnums::LayerType layerType, NeuralEnums::ActivationFunction activationFunction, T bias, double dropoutSize = 0);

	void CalcInputsDelegate(std::vector<T> &prevLayerOutput, std::vector<bool> &prevLayerDropouts, bool usingDropouts, long  int start, long  int end);
	void CalcOutputsDelegate(long int start, long  int end, bool usingDropouts);
	void CalculateInputs(std::vector<T> &prevLayerOutput, std::vector<bool> &prevLayerDropouts, bool usingDropouts);
	void CalculateInputsThreaded(std::vector<T> &prevLayerOutput, std::vector<bool> &prevLayerDropouts, bool usingDropouts, long  int numThreads, std::vector<WorkerThread*> &_workers);
	void CalculateOutputs(bool usingDropouts);
	void CalculateOutputsThreaded(int numThreads, bool usingDropouts, std::vector<WorkerThread*> &_workers);
};

template<class T>
Layer<T>::Layer(int size, NeuralEnums::LayerType layerType, NeuralEnums::ActivationFunction activationFunction, T bias, double dropoutSize = 0)
{
	Size = size;
	LayerType = layerType;
	ActivationFunction = activationFunction;
	Inputs.resize(size);
	Outputs.resize(size);
	Gradients.resize(size);
	Parameters.resize(size);
	LearningRates.resize(size);
	GradientsForGrads.resize(size);
	DropOutSize = dropoutSize;
	UsingBias = !(bias == NULL);
	DropoutNeurons.resize(Size);
	for (long int i = 0; i < Size; i++)
		DropoutNeurons[i] = i < Size * DropOutSize;
	if (UsingBias)
		Inputs[0] = bias;
}

template<class T>
void Layer<T>::CalcInputsDelegate(std::vector<T> &prevLayerOutput, std::vector<bool> &prevLayerDropouts, bool usingDropouts, long  int start, long  int end)
{
	T result;
	bool drop;
	long int prevLayerSize = prevLayerOutput.size();
	int biasShift = UsingBias ? 1 : 0;
	start = start == 0 && UsingBias ? 1 : start;
	for (unsigned long int k = start; k < end; k++)
	{
		result = 0.0;
		if (usingDropouts)
		{
			if (DropoutNeurons[k])
				continue;
			for (unsigned long int i = 0; i < prevLayerSize; i++)
			{
				if (prevLayerDropouts[i])
					continue;
				result += prevLayerOutput[i] * Weights[(k - biasShift)* prevLayerSize + i];
			}
		}
		else
			for (unsigned long int i = 0; i < prevLayerSize; i++)
				result += prevLayerOutput[i] * Weights[(k - biasShift) * prevLayerSize + i];
		Inputs[k] = result;
	}
}

//---------------------------------------------------
template<class T>
void Layer<T>::CalcOutputsDelegate(long int start, long  int end, bool usingDropouts)
{
	if (start == 0 && UsingBias)
		Outputs[0] = Inputs[0];

	start = start == 0 && UsingBias ? 1 : start;
	int biasShift = UsingBias ? 1 : 0;
	switch (ActivationFunction)
	{
	case NeuralEnums::ActivationFunction::GeLU:
	{
		if (usingDropouts)
			for (int i = start; i < end; i++)
			{
				if (DropoutNeurons[i])
					continue;
				Outputs[i] = GeLU(Inputs[i]);
			}
		else
			for (int i = start; i < end; i++)
				Outputs[i] = GeLU(Inputs[i]);
		break;
	}
	case NeuralEnums::ActivationFunction::Sigmoid:
	{
		if (usingDropouts)
			for (int i = start; i < end; i++)
			{
				if (DropoutNeurons[i])
					continue;
				Outputs[i] = Sigmoid(Inputs[i]);
			}
		else
			for (int i = start; i < end; i++)
				Outputs[i] = Sigmoid(Inputs[i]);
		break;
	}
	case NeuralEnums::ActivationFunction::Tanh:
	{
		if (usingDropouts)
			for (int i = start; i < end; i++)
			{
				if (DropoutNeurons[i])
					continue;
				Outputs[i] = Tanh(Inputs[i]);
			}
		else
			for (int i = start; i < end; i++)
				Outputs[i] = Tanh(Inputs[i]);
		break;
	}
	case NeuralEnums::ActivationFunction::MReLU:
	{
		if (usingDropouts)
			for (int i = start; i < end; i++)
			{
				if (DropoutNeurons[i])
					continue;
				Outputs[i] = MReLU(Inputs[i]);
			}
		else
			for (int i = start; i < end; i++)
				Outputs[i] = MReLU(Inputs[i]);
		break;
	}
	case NeuralEnums::ActivationFunction::ReLU:
	{
		if (usingDropouts)
			for (int i = start; i < end; i++)
			{
				if (DropoutNeurons[i])
					continue;
				Outputs[i] = ReLU(Inputs[i]);
			}
		else
			for (int i = start; i < end; i++)
				Outputs[i] = ReLU(Inputs[i]);
		break;
	}
	case NeuralEnums::ActivationFunction::SoftMax:
	{
		if (usingDropouts)
			for (int i = start; i < end; i++)
			{
				if (DropoutNeurons[i])
					continue;
				Outputs[i] = SoftMax(Inputs[i], Inputs);
			}
		else
			for (int i = start; i < end; i++)
				Outputs[i] = SoftMax(Inputs[i], Inputs);
		break;
	}
	case NeuralEnums::ActivationFunction::SoftPlus:
	{
		if (usingDropouts)
			for (int i = start; i < end; i++)
			{
				if (DropoutNeurons[i])
					continue;
				Outputs[i] = SoftPlus(Inputs[i]);
			}
		else
			for (int i = start; i < end; i++)
				Outputs[i] = SoftPlus(Inputs[i]);
		break;
	}
	case NeuralEnums::ActivationFunction::SoftSign:
	{
		if (usingDropouts)
			for (int i = start; i < end; i++)
			{
				if (DropoutNeurons[i])
					continue;
				Outputs[i] = SoftSign(Inputs[i]);
			}
		else
			for (int i = start; i < end; i++)
				Outputs[i] = SoftSign(Inputs[i]);
		break;
	}
	default:
	{
		if (usingDropouts)
			for (int i = start; i < end; i++)
			{
				if (DropoutNeurons[i])
					continue;
				Outputs[i] = Inputs[i];
			}
		else
			for (int i = start; i < end; i++)
				Outputs[i] = Inputs[i];
		break;
	}
	}
}

//---------------------------------------------------
template<class T>
void Layer<T>::CalculateInputs(std::vector<T> &prevLayerOutput, std::vector<bool> &prevLayerDropouts, bool usingDropouts)
{
	CalcInputsDelegate(prevLayerOutput, prevLayerDropouts, usingDropouts, 0, Size);
}

//---------------------------------------------------
template<class T>
void Layer<T>::CalculateInputsThreaded(std::vector<T> &prevLayerOutput, std::vector<bool> &prevLayerDropouts, bool usingDropouts, long  int numThreads, std::vector<WorkerThread*> &_workers)
{
	int chunkSize = Size / numThreads == 0 ? 1 : Size / numThreads;
	int iterator = numThreads > Size ? Size : numThreads;

	for (int i = 0; i < iterator; i++)
	{
		long int start = i * chunkSize;
		long int end = (i + 1) == iterator ? Size : (i + 1) * chunkSize;

		_workers[i]->doAsync([this, &prevLayerOutput, &prevLayerDropouts, usingDropouts, start, end]
		{
			CalcInputsDelegate(prevLayerOutput, prevLayerDropouts, usingDropouts, start, end);
		});
	}
	for (int k = 0; k < iterator; k++)
		_workers[k]->wait();
}

//---------------------------------------------------
template<class T>
void Layer<T>::CalculateOutputs(bool usingDropouts)
{
	CalcOutputsDelegate(0, Size, usingDropouts);
}

//---------------------------------------------------
template<class T>
void Layer<T>::CalculateOutputsThreaded(int numThreads, bool usingDropouts, std::vector<WorkerThread*> &_workers)
{
	int chunkSize = Size / numThreads == 0 ? 1 : Size / numThreads;
	int iterator = numThreads > Size ? Size : numThreads;

	for (long int i = 0; i < iterator; i++)
	{
		long int start = i * chunkSize;
		long int end = (i + 1) == iterator ? Size : (i + 1) * chunkSize;
		_workers[i]->doAsync([this, start, end, usingDropouts]
		{
			CalcOutputsDelegate(start, end, usingDropouts);
		});
	}
	for (int k = 0; k < iterator; k++)
		_workers[k]->wait();
}
//Layer.h