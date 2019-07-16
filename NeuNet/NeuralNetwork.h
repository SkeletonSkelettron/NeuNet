#include <vector>
#include <random>
#include <math.h>
#include <thread>
#include <condition_variable>
#include "enums.h"
#include "utilityfunctions.h"
#include "WorkerThread.h"
#include "Layer.h"
#include "LearningRateFunctions.h"
#include "GradientFunctions.h"

//https://stackoverflow.com/questions/16350473/why-do-i-need-stdcondition-variable



template<class T> class NeuralNetwork
{
public:
	void doNothing();
	unsigned short ThreadCount;
	T LearningRate;
	NeuralEnums::NetworkType Type;
	std::vector<WorkerThread*> workers;
	NeuralEnums::LearningRateType LearningRateType;
	NeuralEnums::BalanceType BalanceType;
	NeuralEnums::LossFunctionType LossFunctionType;
	NeuralEnums::GradientType GradientType;
	std::vector<Layer<T>> Layers;
	void NeuralNetworkInit();
	void InitializeWeights();
	T PropageteForward(std::vector<T> &targetArray, bool usingDropouts);
	T PropageteForwardThreaded(std::vector<T> &targetArray, bool usingDropouts);
	void PropagateBack(std::vector<T> &/*, NeuralEnums::LearningRateType, NeuralEnums::GradientType*/);
	void PropagateBackThreaded(std::vector<T> &);
	void PopagateBackDelegate(std::vector<T> &targetArray, long  int i, long int start, long  int end);
	void ShuffleDropoutsPlain();
	void ShuffleDropoutsGuram();
};


template<class T>
void NeuralNetwork<T>::doNothing()
{
	int t = 0;
}

template<class T>
void NeuralNetwork<T>::NeuralNetworkInit()
{
	Layers.resize(0);
	for (int i = 0; i < ThreadCount; i++)
	{
		workers.push_back(new WorkerThread());
	}
}
template<class T>
void NeuralNetwork<T>::ShuffleDropoutsPlain()
{
	for (int k = 0; k < Layers.size(); k++)
	{
		if (Layers[k].DropOutSize > 0 && Layers[k].LayerType == NeuralEnums::LayerType::HiddenLayer)
		{
			unsigned long int seed = std::chrono::system_clock::now().time_since_epoch().count();
			shuffle(Layers[k].DropoutNeurons.begin(), Layers[k].DropoutNeurons.end(), std::default_random_engine(seed));
		}
	}
}
template<class T>
void NeuralNetwork<T>::ShuffleDropoutsGuram()
{
	for (int k = 0; k < Layers.size(); k++)
	{
		if (Layers[k].DropOutSize > 0 && Layers[k].LayerType == NeuralEnums::LayerType::HiddenLayer)
		{
			std::random_device rd; // obtain a random number from hardware
			std::mt19937 eng(rd()); // seed the generator
			std::uniform_int_distribution<> distr(0, Layers[k].Size - 1); //define the range

			for (int i = 0; i < Layers[k].Size; i++)
				Layers[k].DropoutNeurons[i] = false;

			for (int i = 0; i < Layers[k].DropOutSize * Layers[k].Size; i++)
				Layers[k].DropoutNeurons[distr(eng)] = true;

			unsigned long int seed = std::chrono::system_clock::now().time_since_epoch().count();
			shuffle(Layers[k].DropoutNeurons.begin(), Layers[k].DropoutNeurons.end(), std::default_random_engine(seed));
		}
	}
}
template<class T>
void NeuralNetwork<T>::InitializeWeights()
{
	std::random_device rd;
	// Will be used to obtain a seed for the random number engine
	std::mt19937 gen(rd());
	std::uniform_real_distribution<>dist(0.0, 100.0);
	for (long int i = 1; i < Layers.size(); i++)
	{
		auto StartIndex = Layers[i].UsingBias ? 0 : 1;
		Layers[i].Weights.resize(Layers[i - 1].Size * (Layers[i].Size - (Layers[i].UsingBias ? 1 : 0)));
		Layers[i].MultipliedSums.resize(Layers[i - 1].Size * (Layers[i].Size - (Layers[i].UsingBias ? 1 : 0)));
		Layers[i].Gradients.resize(Layers[i - 1].Size * (Layers[i].Size - (Layers[i].UsingBias ? 1 : 0)));
		Layers[i].GradientsForGrads.resize(Layers[i - 1].Size * (Layers[i].Size - (Layers[i].UsingBias ? 1 : 0)));
		for (long int ff = 0; ff < Layers[i - 1].Size * (Layers[i].Size - (Layers[i].UsingBias ? 1 : 0)); ff++)
		{
			Layers[i].MultipliedSums[ff] = 0L;
			Layers[i].Weights[ff] = 0L;
		}
		for (long int j = 0; j < Layers[i - 1].Size * (Layers[i].Size - (Layers[i].UsingBias ? 1 : 0)); j++)
		{
			Layers[i].Weights[j] = Layers[i].UsingBias && j % Layers[i - 1].Size == 0 ? 1L : static_cast<T>(dist(gen)) / 100L;
		}
		BalanceWith<T>(Layers[i].Weights, BalanceType);
		if (Layers[i - 1].UsingBias)
			for (long int j = 0; j < Layers[i - 1].Size * (Layers[i].Size - (Layers[i].UsingBias ? 1 : 0)); j++)
			{
				if (j % Layers[i - 1].Size == 0)
					Layers[i].Weights[j] = 1L;
			}
	}
}



template<class T>
T NeuralNetwork<T>::PropageteForward(std::vector<T> &targetArray, bool usingDropouts)
{
	ShuffleDropoutsGuram();
	for (int k = 0; k < Layers.size(); k++)
	{
		if (k != 0)
			Layers[k].CalculateInputs(Layers[k - 1].Outputs, Layers[k - 1].DropoutNeurons, usingDropouts);
		Layers[k].CalculateOutputs(usingDropouts);
	}
	return CalculateLoss<T>(NeuralEnums::LossFunctionType::MeanSquaredLoss, Layers[Layers.size() - 1].Outputs, targetArray);
}

template<class T>
T NeuralNetwork<T>::PropageteForwardThreaded(std::vector<T> &targetArray, bool usingDropouts)
{
	ShuffleDropoutsGuram();
	for (int k = 0; k < Layers.size(); k++)
	{
		if (k != 0)
			Layers[k].CalculateInputsThreaded(Layers[k - 1].Outputs, Layers[k - 1].DropoutNeurons, usingDropouts, ThreadCount, workers);
		Layers[k].CalculateOutputsThreaded(ThreadCount, usingDropouts, workers);
	}
	return CalculateLoss<T>(NeuralEnums::LossFunctionType::MeanSquaredLoss, Layers[Layers.size() - 1].Outputs, targetArray);
}

template<class T>
void NeuralNetwork<T>::PropagateBack(std::vector<T> &targetArray/*, NeuralEnums::LearningRateType learningRateType = NeuralEnums::LearningRateType::AdaGrad, NeuralEnums::GradientType gradientType = NeuralEnums::GradientType::Static*/)
{//https://hmkcode.github.io/ai/backpropagation-step-by-step/
	for (long int i = Layers.size() - 1; i >= 1; i--)
	{
		PopagateBackDelegate(targetArray, i, 0, Layers[i].Size);
	}
}
template<class T>
void NeuralNetwork<T>::PropagateBackThreaded(std::vector<T> &targetArray)
{//https://hmkcode.github.io/ai/backpropagation-step-by-step/
	for (unsigned int i = Layers.size() - 1; i >= 1; i--)
	{

		int Size = Layers[i].Size;
		int chunkSize = Size / ThreadCount == 0 ? 1 : Size / ThreadCount;
		int iterator = ThreadCount > Size ? Size : ThreadCount;

		for (int l = 0; l < iterator; l++)
		{
			long int start = l * chunkSize;
			long int end = (l + 1) == iterator ? Size : (l + 1) * chunkSize;

			workers[l]->doAsync([this, &targetArray, i, start, end]
			{
				PopagateBackDelegate(targetArray, i, start, end);
			});
		}
		for (int k = 0; k < iterator; k++)
			workers[k]->wait();
	}
}
template<class T>
void NeuralNetwork<T>::PopagateBackDelegate(std::vector<T> &targetArray, long  int i, long int start, long  int end)
{
	int pLS = Layers[i - 1].Size;
	int biasShift = Layers[i].UsingBias ? 1 : 0;
	start = start == 0 && Layers[i].UsingBias ? 1 : start;
	T gradient;
	for (long int j = start; j < end; j++)
	{
		if (Layers[i].DropoutNeurons[j])
			continue;
		// Output ლეიერი
		if (i == Layers.size() - 1)
		{
			Layers[i].Outputs[j] = DifferentiateLossWith(Layers[i].Outputs[j], targetArray[j], NeuralEnums::LossFunctionType::MeanSquaredLoss);

			//SoftMax ის შემთხვევაში ეს არ იმუშავებს რადგან ინფუთში მნიშვნელობები იცვლება. TODO
			Layers[i].Inputs[j] = DifferentiateWith(Layers[i].Inputs[j], Layers[i].ActivationFunction, Layers[i].Inputs);

			// Output ლეიერში წონების დაკორექტირება
			for (long int p = 0; p < pLS; p++)
			{
				if (Layers[i - 1].DropoutNeurons[p])
					continue;
				Layers[i].MultipliedSums[pLS * j + p] = Layers[i].Weights[pLS * j + p] * Layers[i].Inputs[j] * Layers[i].Outputs[j];
				gradient = GetGradient(GradientType, Layers[i].GradientsForGrads, Layers[i].Outputs[j] * Layers[i].Inputs[j] * Layers[i - 1].Outputs[p], pLS * j + p);
				Layers[i].Weights[pLS * j + p] -= GetLearningRate(LearningRateType, Layers[i].Gradients, Layers[i].Outputs[j] * Layers[i].Inputs[j] * Layers[i - 1].Outputs[p]
					, pLS * j + p, LearningRate)
					* Layers[i].Outputs[j] * Layers[i].Inputs[j] * Layers[i - 1].Outputs[p];
			}
		}
		else
		{
			//SoftMax ის შემთხვევაში ეს არ იმუშავებს რადგან ინფუთში მნიშვნელობები იცვლება. TODO
			Layers[i].Inputs[j] = DifferentiateWith(Layers[i].Inputs[j], Layers[i].ActivationFunction, Layers[i].Inputs);

			T sum = 0;

			int nextLayerBiasShift = Layers[i + 1].UsingBias ? 1 : 0;

			//შემდეგი ლეიერიდან უნდა აიღოს შესაბამისი ჯამები
			for (long int n = Layers[i + 1].UsingBias ? 1 : 0; n < Layers[i + 1].Size; n++)
			{
				if (Layers[i + 1].DropoutNeurons[n])
					continue;
				sum += Layers[i + 1].MultipliedSums[(n - nextLayerBiasShift) * Layers[i].Size + j];
			}

			//მიმდინარე ნეირონის შესაბამისი წონების განახლება
			int numberIndex = 0;
			T mult = Layers[i].Inputs[j] * sum;
			
			if (i != 1)
				for (long int n = 0; n < pLS; n++)
				{
					// mult * Layers[i - 1].Outputs[n] არის gradient
					if (Layers[i - 1].DropoutNeurons[n])
						continue;
					numberIndex = pLS * (j - biasShift) + n;
					Layers[i].MultipliedSums[numberIndex] = Layers[i].Weights[numberIndex] * mult;
					gradient = GetGradient(GradientType, Layers[i].GradientsForGrads, mult * Layers[i - 1].Outputs[n], numberIndex);
					Layers[i].Weights[numberIndex] -= GetLearningRate(LearningRateType, Layers[i].Gradients, gradient, numberIndex, LearningRate)
						*gradient;
				}
			else
				for (long int n = 0; n < pLS; n++)
				{
					if (Layers[i - 1].DropoutNeurons[n])
						continue;
					numberIndex = pLS * (j - biasShift) + n;
					gradient = GetGradient(GradientType, Layers[i].GradientsForGrads, mult * Layers[i - 1].Outputs[n], numberIndex);
					Layers[i].Weights[numberIndex] -= GetLearningRate(LearningRateType, Layers[i].Gradients, gradient, numberIndex, LearningRate)
						* gradient;
				}
		}
	}
}





//NeuralNetwork.h

