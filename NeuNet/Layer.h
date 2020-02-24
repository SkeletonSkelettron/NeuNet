#include "enums.h"
#include <vector>

template<class T> class Layer
{
public:
	int Size;
	std::vector<T> Inputs;
	std::vector<vector<T>> InputsBatch;
	std::vector<T> Weights;
	std::vector<T> Outputs;
	std::vector<T> RoHat;
	std::vector< std::vector<T>> RoHatBatch;
	std::vector<vector<T>> OutputsBatch;
	std::vector<T> Gradients;
	std::vector<T> GradientsLR;
	std::vector<vector<T>> GradientsBatch;
	std::vector<T> Parameters;
	std::vector<T> GradientsForGrads;
	std::vector<T> LearningRates;
	std::vector<T> Target;
	std::vector<vector<T>> TargetsBatch;
	std::vector<int> IndexVector;
	float DropOutSize;
	bool UsingBias;
	std::vector<bool> DropoutNeurons;
	NeuralEnums::ActivationFunction ActivationFunction;
	NeuralEnums::LayerType LayerType;
	Layer() {}
	Layer(int size, NeuralEnums::LayerType layerType, NeuralEnums::ActivationFunction activationFunction, T bias, float dropoutSize = 0, int batchSize = 1);

	void CalcInputsDelegate(std::vector<T>& prevLayerOutput, std::vector<vector<T>>& prevLayerOutputBatch, std::vector<int>& prevLayerIndexes, std::vector<bool>& prevLayerDropouts, bool training, int start, int end);
	void CalcOutputsDelegate(int start, int end, bool training, bool countingRohat);
	void CalculateInputs(std::vector<T>& prevLayerOutput, std::vector<vector<T>>& prevLayerOutputBatch, std::vector<bool>& prevLayerDropouts, bool training);
	void CalculateInputsThreaded(
		std::vector<T>& prevLayerOutput,
		std::vector<vector<T>>& prevLayerOutputBatch,
		std::vector<int>& prevLayerIndex,
		std::vector<bool>& prevLayerDropouts,
		bool& prevLayerUsingBias,
		bool training,
		int numThreads,
		std::vector<WorkerThread*>& _workers);
	void CalculateOutputs(bool training, bool countingRohat);
	void CalculateOutputsThreaded(int numThreads, bool training, bool countingRohat, std::vector<WorkerThread*>& _workers);
};

template<class T> Layer<T>::Layer(int size, NeuralEnums::LayerType layerType, NeuralEnums::ActivationFunction activationFunction, T bias, float dropoutSize, int batchSize)
{
	Size = size;
	LayerType = layerType;
	ActivationFunction = activationFunction;
	Inputs.resize(size);
	Outputs.resize(size);
	Parameters.resize(size);
	RoHat.resize(size);
	LearningRates.resize(size);
	GradientsForGrads.resize(size);
	DropOutSize = dropoutSize;
	UsingBias = !(bias == NULL);
	DropoutNeurons.resize(Size);
	InputsBatch.resize(batchSize > 1 ? batchSize : 0);
	OutputsBatch.resize(batchSize > 1 ? batchSize : 0);
	GradientsBatch.resize(batchSize > 1 ? batchSize : 0);

	if (LayerType == NeuralEnums::LayerType::InputLayer || LayerType == NeuralEnums::LayerType::OutputLayer)
	{
		DropOutSize = 0;
		UsingBias = false;
		if (batchSize > 1)
		{
			for (size_t i = 0; i < Size; i++)
				IndexVector.push_back(i);
		}
	}

	if (LayerType == NeuralEnums::LayerType::OutputLayer)
		TargetsBatch.resize(batchSize);
	if (batchSize > 1)
		for (int i = 0; i < batchSize; i++)
		{
			InputsBatch[i].resize(size);
			OutputsBatch[i].resize(size);
			if (LayerType == NeuralEnums::LayerType::OutputLayer)
				TargetsBatch[i].resize(size);
		}
	for (int i = 0; i < Size; i++)
		DropoutNeurons[i] = i < Size * DropOutSize;
	if (UsingBias)
	{
		Inputs[0] = bias;
		if (batchSize > 1)
			for (int i = 0; i < batchSize; i++)
				InputsBatch[i][0] = bias;
	}
	//if (batchSize > 1)
	//{

	//}
}

//---------------------------------------------------
template<class T> void Layer<T>::CalculateInputs(std::vector<T>& prevLayerOutput, std::vector<vector<T>>& prevLayerOutputBatch, std::vector<bool>& prevLayerDropouts, bool training)
{
	CalcInputsDelegate(prevLayerOutput, prevLayerOutputBatch, prevLayerDropouts, training, 0, Size);
}

//---------------------------------------------------
template<class T> void Layer<T>::CalculateInputsThreaded(
	std::vector<T>& prevLayerOutput,
	std::vector<vector<T>>& prevLayerOutputBatch,
	std::vector<int>& prevLayerIndex,
	std::vector<bool>& prevLayerDropouts,
	bool& prevLayerUsingBias,
	bool training,
	int numThreads,
	std::vector<WorkerThread*>& _workers)
{

	if (InputsBatch.size() > 1 && training)
	{
		//თუკი წინა ლეიერი იყენებს ბიასს, მაშინ მისი ინდექსი აქ არ იქნება და უნდა ჩავამატოთ.
		//If previous layer uses bias, then its index won't be here and it shouldf be added at the beggining. 
		if (prevLayerUsingBias)
			prevLayerIndex.insert(prevLayerIndex.begin(), 0);

		int chunkSize = InputsBatch.size() / numThreads;
		int idx = 0;
		for (int i = 0; i < numThreads; i++)
		{
			idx = chunkSize * i;
			_workers[i]->doAsync(std::bind(&Layer::CalcInputsDelegate, this,
				std::ref(prevLayerOutput), std::ref(prevLayerOutputBatch), std::ref(prevLayerIndex), std::ref(prevLayerDropouts),training, idx, idx + chunkSize));
		}
		for (int k = 0; k < numThreads; k++)
			_workers[k]->wait();
	}
	else
	{
		int chunkSize = Size / numThreads == 0 ? 1 : Size / numThreads;
		int iterator = numThreads > Size ? Size : numThreads;

		for (int i = 0; i < iterator; i++)
		{
			int start = i * chunkSize;
			int end = (i + 1) == iterator ? Size : (i + 1) * chunkSize;

			_workers[i]->doAsync(std::bind(&Layer::CalcInputsDelegate, this, std::ref(prevLayerOutput), std::ref(prevLayerOutputBatch), std::ref(prevLayerIndex), std::ref(prevLayerDropouts), training, start, end));
		}
		for (int k = 0; k < iterator; k++)
			_workers[k]->wait();
	}
}

//---------------------------------------------------
template<class T> void Layer<T>::CalculateOutputs(bool training, bool countingRohat)
{
	CalcOutputsDelegate(0, Size, training, countingRohat);
}

//---------------------------------------------------
template<class T> void Layer<T>::CalculateOutputsThreaded(int numThreads, bool training, bool countingRohat, std::vector<WorkerThread*>& _workers)
{
	if (InputsBatch.size() > 1 && training)
	{
		bool miniBatch = InputsBatch.size() > 1 && training;
		int chunkSize = InputsBatch.size() / numThreads;
		int idx = 0;

		for (int i = 0; i < numThreads; i++)
		{
			idx = chunkSize * i;
			_workers[i]->doAsync(std::bind(&Layer::CalcOutputsDelegate, this, idx, idx + chunkSize, training, countingRohat));
		}
		for (int k = 0; k < numThreads; k++)
			_workers[k]->wait();
	}
	else
	{
		int chunkSize = Size / numThreads == 0 ? 1 : Size / numThreads;
		int iterator = numThreads > Size ? Size : numThreads;

		for (int i = 0; i < iterator; i++)
		{
			int start = i * chunkSize;
			int end = (i + 1) == iterator ? Size : (i + 1) * chunkSize;
			_workers[i]->doAsync(std::bind(&Layer::CalcOutputsDelegate, this, start, end, training, countingRohat));
		}
		for (int k = 0; k < iterator; k++)
			_workers[k]->wait();
	}
}
template<class T> void Layer<T>::CalcInputsDelegate(std::vector<T>& prevLayerOutput, std::vector<vector<T>>& prevLayerOutputBatch, std::vector<int>& prevLayerIndexes, std::vector<bool>& prevLayerDropouts, bool training, int start, int end)
{
	T result;
	bool drop;
	int prevLayerSize = prevLayerOutput.size();
	int biasShift = UsingBias ? 1 : 0;

	if (InputsBatch.size() > 1 && training)
	{
		for (int batch = start; batch < end; batch++)
		{
			for (int k : IndexVector)
			{
				result = 0.0;
				for (int i : prevLayerIndexes)
					result += prevLayerOutputBatch[batch][i] * Weights[(k - biasShift) * prevLayerSize + i];
				InputsBatch[batch][k] = result;
			}
		}
	}
	else
	{
		start = start == 0 && UsingBias ? 1 : start;
		for (int k = (start == 0 && UsingBias) ? 1 : start; k < end; k++)
		{
			if (training && DropoutNeurons[k])
				continue;
			result = 0.0;
			for (int i = 0; i < prevLayerOutput.size(); i++)
			{
				if (training && prevLayerDropouts[i])
					continue;
				result += prevLayerOutput[i] * Weights[(k - biasShift) * prevLayerSize + i];
			}
			Inputs[k] = result;
		}
	}
}

//---------------------------------------------------
template<class T> void Layer<T>::CalcOutputsDelegate(int start, int end, bool training, bool countingRohat)
{
	//TODO ჩასამატებელია სოფტმაქსის რეალიზაცია 
	if (InputsBatch.size() > 1 && training)
	{
		for (int batch = start; batch < end; batch++)
		{
			if (UsingBias)
				OutputsBatch[batch][0] = InputsBatch[batch][0];
			ActivateWith(InputsBatch[batch], OutputsBatch[batch], IndexVector, true, DropoutNeurons, start, end, false, ActivationFunction);
		}
	}
	else
	{
		if (start == 0 && UsingBias)
			Outputs[0] = Inputs[0];

		if (training)
		{
			ActivateWith(Inputs, Outputs, IndexVector, false, DropoutNeurons, start, end, true, ActivationFunction);

			if (countingRohat)
				for (size_t i = start; i < end; i++)
					RoHat[i] += Outputs[i];
		}
		else
		{
			ActivateWith(Inputs, Outputs, IndexVector, false, DropoutNeurons, start, end, false, ActivationFunction);
		}
	}
}

//Layer.h