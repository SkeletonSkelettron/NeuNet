#include <vector>
#include <random>
#include <math.h>
#include <thread>
#include <condition_variable>
#include "enums.h"
#include "ActivationFunctions.h"
#include "WorkerThread.h"
#include "Layer.h"
#include "LearningRateFunctions.h"
#include "GradientFunctions.h"
#include "LossFunctions.h"
//https://stackoverflow.com/questions/16350473/why-do-i-need-stdcondition-variable



template<class T> class NeuralNetwork
{
public:
	unsigned short ThreadCount;
	T LearningRate;
	NeuralEnums::NetworkType Type;
	std::vector<WorkerThread*> workers;
	NeuralEnums::LearningRateType LearningRateType;
	NeuralEnums::BalanceType BalanceType;
	NeuralEnums::LossFunctionType LossFunctionType;
	NeuralEnums::GradientType GradientType;
	std::vector<std::vector<std::vector<T>>> GradientsTemp;
	NeuralEnums::Metrics Metrics;
	NeuralEnums::AutoEncoderType AutoEncoderType;
	NeuralEnums::LossCalculation LossCalculation;
	NeuralEnums::LogLoss LogLoss;
	int BatchSize;
	//std::vector<std::vector<std::unique_ptr<std::atomic<int>>>> gradients;
	std::vector<Layer<T>> Layers;
	std::vector<vector<T>> TempWeights;
	std::vector<T> lossesTmp;
	T ro;
	int iterations;
	T beta1Pow;
	T beta2Pow;
	T betaAELR;
	//weight decay parameter for sparce autoencoder
	T lambda = 0.7;
	void NeuralNetworkInit();
	void InitializeWeights();
	void StartTesting();
	T PropagateForward(bool training);
	T PropagateForwardThreaded(bool training, bool countingRohat);
	void PropagateBack();
	void PropagateBackThreaded();
	void PropagateBackDelegate(int i, int start, int end, std::vector<T>& outputs);
	void PropagateBackDelegateBatch(int start, int end, std::vector<std::vector<T>>& gradients);
	void ShuffleDropoutsPlain();
	void ShuffleDropoutsGuram();
	void CalculateWeightsBatch();
	void CalculateWeightsBatchSub(int i, std::vector<int>& prevLayerIndex, std::vector<int>& index);
	T CalculateLoss(bool& training);
	void CalculateLossSub(int start, int end, int klbstart, int  klbend, T& loss);
	void CalculateLossBatchSub(int start, int end, T& loss);
};


template<class T> void NeuralNetwork<T>::NeuralNetworkInit()
{
	iterations = 0;
	beta1Pow = 0.9;
	beta2Pow = 0.999;
	betaAELR = 0.001;
	ro = -0.9;
	for (int i = 0; i < ThreadCount; i++)
	{
		workers.push_back(new WorkerThread());
	}
	GradientsTemp.resize(ThreadCount);
	lossesTmp.resize(ThreadCount > Layers[Layers.size() - 1].Size ? Layers[Layers.size() - 1].Size : ThreadCount);
	lambda = 0.7;
}

template<class T> void NeuralNetwork<T>::ShuffleDropoutsPlain()
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
template<class T> void NeuralNetwork<T>::ShuffleDropoutsGuram()
{
	for (int k = 1; k < Layers.size(); k++)
	{
		int biasShift = Layers[k].UsingBias ? 1 : 0;

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

			if (BatchSize > 1)
			{
				Layers[k].IndexVector.clear();

				for (size_t i = biasShift; i < Layers[k].DropoutNeurons.size(); i++)
					if (!Layers[k].DropoutNeurons[i])
						Layers[k].IndexVector.push_back(i);
			}
		}
		else if (Layers[k].LayerType == NeuralEnums::LayerType::HiddenLayer && BatchSize > 1)
		{
			Layers[k].IndexVector.clear();
			for (size_t i = biasShift; i < Layers[k].Size; i++)
				Layers[k].IndexVector.push_back(i);
		}
	}
}
template<class T> void NeuralNetwork<T>::InitializeWeights()
{
	std::random_device rd;
	// Will be used to obtain a seed for the random number engine
	std::mt19937 gen(rd());
	std::uniform_real_distribution<>dist(0.0, 100.0);
	for (size_t t = 0; t < ThreadCount; t++)
	{
		GradientsTemp[t].resize(Layers.size());
	}

	for (int i = 1; i < Layers.size(); i++)
	{
		int StartIndex = Layers[i].UsingBias ? 0 : 1;
		Layers[i].Weights.resize(Layers[i - 1].Size * (Layers[i].Size - (Layers[i].UsingBias ? 1 : 0)));


		if (BatchSize > 1)
			for (int v = 0; v < ThreadCount; v++)
				GradientsTemp[v][i].resize(Layers[i - 1].Size * (Layers[i].Size - (Layers[i].UsingBias ? 1 : 0)));
		else
			Layers[i].Gradients.resize(Layers[i - 1].Size * (Layers[i].Size - (Layers[i].UsingBias ? 1 : 0)));
		Layers[i].GradientsLR.resize(Layers[i - 1].Size * (Layers[i].Size - (Layers[i].UsingBias ? 1 : 0)));
		Layers[i].Parameters.resize(Layers[i - 1].Size * (Layers[i].Size - (Layers[i].UsingBias ? 1 : 0)));
		Layers[i].LearningRates.resize(Layers[i - 1].Size * (Layers[i].Size - (Layers[i].UsingBias ? 1 : 0)));
		for (int ff = 0; ff < Layers[i - 1].Size * (Layers[i].Size - (Layers[i].UsingBias ? 1 : 0)); ff++)
		{
			Layers[i].Weights[ff] = 0L;
		}
		for (int j = 0; j < Layers[i - 1].Size * (Layers[i].Size - (Layers[i].UsingBias ? 1 : 0)); j++)
		{
			Layers[i].Weights[j] = Layers[i].UsingBias && j % Layers[i - 1].Size == 0
				? 1.0
				: dist(gen);
		}
		if (Layers[i].ActivationFunction == NeuralEnums::ActivationFunction::Sigmoid)
		{
			std::vector<int> tmp;
			tmp.resize(2);
			T start = -1.0;
			T end = 1.0;
			StandartizeLinearContract(Layers[i].Weights, tmp, start, end);
		}
		else
		{
			std::vector<int> tmp;
			tmp.resize(2);
			T start = -0.07;
			T end = 0.07;
			StandartizeLinearContract(Layers[i].Weights, tmp, start, end);
		}
		if (Layers[i - 1].UsingBias)
			for (int j = 0; j < Layers[i - 1].Size * (Layers[i].Size - (Layers[i].UsingBias ? 1 : 0)); j++)
			{
				if (j % Layers[i - 1].Size == 0)
					Layers[i].Weights[j] = 1L;
			}
		//ofstream oData;
		//oData.open("weights" + std::to_string(i) + ".txt");
		//for (int count = 0; count < Layers[i].Weights.size(); count++) {
		//	oData << std::setprecision(100) << Layers[i].Weights[count] << endl;
		//}

		/*ifstream inData;
		inData.open("weights" + std::to_string(i) + ".txt");
		for (int count = 0; count < Layers[i].Weights.size(); count++) {
			inData >> std::setprecision(100) >> Layers[i].Weights[count];
		}*/
	}
}



template<class T> T NeuralNetwork<T>::PropagateForward(bool training)
{
	ShuffleDropoutsGuram();
	for (int k = 1; k < Layers.size(); k++)
	{
		Layers[k].CalculateInputs(Layers[k - 1].OutputsBatch, Layers[k - 1].Outputs, Layers[k - 1].DropoutNeurons, usingDropouts, training);
		Layers[k].CalculateOutputs(usingDropouts, training);
	}
	return -1;//  CalculateLoss<T>(LossFunctionType, false, ThreadCount, workers);
}

template<class T> T NeuralNetwork<T>::PropagateForwardThreaded(bool training, bool countingRohat)
{
	if (training)
		ShuffleDropoutsGuram();
	for (int k = 1; k < Layers.size() - (countingRohat ? 1 : 0); k++)
	{
		Layers[k].CalculateInputsThreaded(Layers[k - 1].Outputs, Layers[k - 1].OutputsBatch, Layers[k - 1].IndexVector, Layers[k - 1].DropoutNeurons, Layers[k - 1].UsingBias, training, ThreadCount, workers);
		Layers[k].CalculateOutputsThreaded(ThreadCount, training, countingRohat, workers);
	}
	if (this->LossCalculation == NeuralEnums::LossCalculation::Full && !countingRohat)
		return CalculateLoss(training);
	return -1;
}

template<class T> void NeuralNetwork<T>::PropagateBack()
{
	std::vector<T> dummy;
	//CloneWeights();
	for (int i = Layers.size() - 1; i >= 1; i--)
	{
		if (BatchSize > 1)
		{
			for (size_t idx = 0; idx < BatchSize; idx++)
			{
				//PopagateBackDelegateBatch(0, Layers[i].Size);
			}
		}
		else
		{
			PopagateBackDelegate(i, 0, Layers[i].Size);
		}
	}
}
template<class T> void NeuralNetwork<T>::PropagateBackThreaded()
{//https://hmkcode.github.io/ai/backpropagation-step-by-step/
	std::vector<T> dummy;
	//ClearNetwork();
	// PopagateBackDelegateBatch2(0, 1, vector);
	if (LearningRateType == NeuralEnums::LearningRateType::Adam)
	{
		iterations++;
		beta1Pow = pow(0.9, iterations);
		beta2Pow = pow(0.999, iterations);
	}
	//TODO ეს აქ არ უნდა იყოს
	if (BatchSize > 1)
	{
		int chunkSize = Layers[0].InputsBatch.size() / ThreadCount;
		int idx = 0;
		for (int i = Layers.size() - 1; i >= 1; i--)
		{
			if (Layers[i].UsingBias)
				Layers[i].IndexVector.erase(std::remove(Layers[i].IndexVector.begin(), Layers[i].IndexVector.end(), 0), Layers[i].IndexVector.end());
		}
		for (int i = 0; i < ThreadCount; i++)
		{
			idx = chunkSize * i;

			workers[i]->doAsync(std::bind(&NeuralNetwork::PropagateBackDelegateBatch, this, idx, idx + chunkSize, std::ref(GradientsTemp[i])));
		}
		for (int k = 0; k < ThreadCount; k++)
			workers[k]->wait();
		CalculateWeightsBatch();
	}
	else
	{
		for (unsigned int i = Layers.size() - 1; i >= 1; i--)
		{

			int Size = Layers[i].Size;
			int chunkSize = Size / ThreadCount == 0 ? 1 : Size / ThreadCount;
			int iterator = ThreadCount > Size ? Size : ThreadCount;
			std::vector<vector<T>> tmpOutputs;
			tmpOutputs.resize(iterator);

			for (int l = 0; l < iterator; l++)
			{
				int start = l * chunkSize;
				int end = (l + 1) == iterator ? Size : (l + 1) * chunkSize;
				tmpOutputs[l].resize(Layers[i - 1].Size);
				workers[l]->doAsync(std::bind(&NeuralNetwork::PropagateBackDelegate, this, i, start, end, std::ref(tmpOutputs[l])));
			}

			for (int k = 0; k < iterator; k++)
				workers[k]->wait();
			if (i != 1)
			{
				for (size_t f = 0; f < Layers[i - 1].Size; f++)
					Layers[i - 1].Outputs[f] = 0;

				for (size_t g = 0; g < iterator; g++)
					for (size_t f = 0; f < Layers[i - 1].Size; f++)
					{
						Layers[i - 1].Outputs[f] += tmpOutputs[g][f];
						tmpOutputs[g][f] = 0;
					}
			}
		}
	}
}

template<class T> void NeuralNetwork<T>::PropagateBackDelegate(int i, int start, int end, std::vector<T>& outputs)
{
	int numberIndex = 0;
	int pLS = 0;
	int biasShift = 0;
	T gradient;
	T gradientTemp;
	start = start == 0 && Layers[i].UsingBias ? 1 : start;
	pLS = Layers[i - 1].Size;
	biasShift = Layers[i].UsingBias ? 1 : 0;

	for (int j = start; j < end; j++)
	{
		if (Layers[i].DropoutNeurons[j])
			continue;
		// Output ლეიერი
		if (i == Layers.size() - 1)
			Layers[i].Outputs[j] = DifferentiateLossWith(Layers[i].Outputs[j], Layers[i].Target[j], LossFunctionType, Layers[i].Target.size());
		Layers[i].Inputs[j] = Layers[i].Outputs[j] * DifferentiateWith(Layers[i].Inputs[j], Layers[i].ActivationFunction, Layers[i].Inputs, Layers[i].DropoutNeurons);

		for (int p = 0; p < pLS; p++)
		{
			if (Layers[i - 1].DropoutNeurons[p])
				continue;
			numberIndex = pLS * (j - biasShift) + p;
			if (i != 1)
				outputs[p] += Layers[i].Inputs[j] * Layers[i].Weights[numberIndex];
			gradient = Layers[i].Inputs[j] * Layers[i - 1].Outputs[p] /*+ 0.7 * Layers[i].Weights[numberIndex]/60000*/;  //Layers[i].Gradients[numberIndex] = ... if gradient optimization is needed
			Layers[i].Weights[numberIndex] -= GetLearningRateMultipliedByGrad(*this, gradient/*Layers[i].Gradients[numberIndex]*/, i, numberIndex);
		}
		//
	}
}

template<class T> void NeuralNetwork<T>::PropagateBackDelegateBatch(int start, int end, std::vector<std::vector<T>>& gradients)
{
	int numberIndex = 0;
	std::vector<T> outputsTemp;
	std::vector<T> inputs;
	int pLS = 0;
	int biasShift = 0;
	T gradient;
	T gradientTemp;

	std::vector<int>indexVector;

	for (int batch = start; batch < end; batch++)
	{
		for (int i = Layers.size() - 1; i >= 1; i--)
		{

			pLS = Layers[i - 1].Size;
			biasShift = Layers[i].UsingBias ? 1 : 0;
			outputsTemp.clear();
			outputsTemp.resize(Layers[i - 1].OutputsBatch[batch].size());

			for (int j : Layers[i].IndexVector)

			{
				// Output ლეიერი
				if (i == Layers.size() - 1)
					Layers[i].OutputsBatch[batch][j] = DifferentiateLossWith(Layers[i].OutputsBatch[batch][j], Layers[i].TargetsBatch[batch][j],
						LossFunctionType, Layers[i].TargetsBatch[batch].size());
				Layers[i].InputsBatch[batch][j] = Layers[i].OutputsBatch[batch][j] * DifferentiateWith(Layers[i].InputsBatch[batch][j], Layers[i].ActivationFunction,
					Layers[i].InputsBatch[batch], Layers[i].DropoutNeurons);

				for (int p : Layers[i - 1].IndexVector)
				{
					numberIndex = pLS * (j - biasShift) + p;
					if (i != 1)
						outputsTemp[p] += Layers[i].InputsBatch[batch][j] * Layers[i].Weights[numberIndex];
					gradients[i][numberIndex] += Layers[i].InputsBatch[batch][j] * Layers[i - 1].OutputsBatch[batch][p];// gradient;
				}
				//
			}
			if (i != 1) //ამის ოპტიმიზაცია შეიძლება 
				for (int p = Layers[i - 1].UsingBias ? 1 : 0; p < pLS; p++)
				{
					if (Layers[i - 1].DropoutNeurons[p])
						continue;
					Layers[i - 1].OutputsBatch[batch][p] = outputsTemp[p];
				}
			gradient = 0;
			gradientTemp = 0;
			outputsTemp.clear();
		}
	}
}


template<class T> void  NeuralNetwork<T>::CalculateWeightsBatch()
{
	for (unsigned int i = Layers.size() - 1; i >= 1; i--)
	{

		int Size = Layers[i].IndexVector.size();
		int chunkSize = Size / ThreadCount == 0 ? 1 : Size / ThreadCount;
		int iterator = ThreadCount > Size ? Size : ThreadCount;

		for (int l = 0; l < iterator; l++)
		{

			int start = l * chunkSize;
			int end = (l + 1) == iterator ? Size : (l + 1) * chunkSize;
			std::vector<int> tmp;
			for (size_t ii = start == 0 && Layers[i].UsingBias ? 1 : start; ii < end; ii++)
				tmp.push_back(ii);
			workers[l]->doAsync(std::bind(&NeuralNetwork::CalculateWeightsBatchSub, this, i, std::ref(Layers[i - 1].IndexVector), tmp));
		}
		for (int k = 0; k < iterator; k++)
			workers[k]->wait();
	}
}
template<class T> void  NeuralNetwork<T>::CalculateWeightsBatchSub(int i, std::vector<int>& prevLayerIndex, std::vector<int>& index)
{
	int biasShift;
	T gradient = 0;
	int numberIndex = 0;
	int pLS = Layers[i - 1].Size;
	biasShift = Layers[i].UsingBias ? 1 : 0;
	for (int j : index)
	{
		for (int p : prevLayerIndex)
		{
			numberIndex = pLS * (j - biasShift) + p;
			for (size_t t = 0; t < ThreadCount; t++)
			{
				gradient += GradientsTemp[t][i][numberIndex];
				GradientsTemp[t][i][numberIndex] = 0;
			}
			gradient /= BatchSize;
			Layers[i].Weights[numberIndex] -= GetLearningRateMultipliedByGrad(*this, gradient, i, numberIndex);
			gradient = 0;
		}
	}
	gradient = 0;
}

template<class T> void  NeuralNetwork<T>::StartTesting()
{
	for (int k = 0; k < Layers.size(); k++)
	{
		if (Layers[k].LayerType == NeuralEnums::LayerType::HiddenLayer && Layers[k].DropOutSize > 0)
		{
			Layers[k].IndexVector.clear();
			int biasShift = Layers[k].UsingBias ? 1 : 0;
			for (size_t i = biasShift; i < Layers[k].DropoutNeurons.size(); i++)
				Layers[k].IndexVector.push_back(i);
		}
	}
}

template<class T> T  NeuralNetwork<T>::CalculateLoss(bool& training)
{
	std::vector<T> losses;
	if (Layers[Layers.size() - 1].OutputsBatch.size() > 1 && training)
	{
		int chunkSize = Layers[0].OutputsBatch.size() / ThreadCount;
		int idx = 0;

		losses.resize(ThreadCount);
		T result;
		for (int i = 0; i < ThreadCount; i++)
		{
			idx = chunkSize * i;

			workers[i]->doAsync(std::bind(&NeuralNetwork::CalculateLossBatchSub, this, idx, idx + chunkSize, std::ref(losses[i])));
		}
		for (int k = 0; k < ThreadCount; k++)
			workers[k]->wait();
		return std::accumulate(losses.begin(), losses.end(), 0.0);
	}
	if (Layers[Layers.size() - 1].OutputsBatch.size() == 0 || training)
	{
		int chunkSize = Layers[Layers.size() - 1].Size / ThreadCount == 0 ? 1 : Layers[Layers.size() - 1].Size / ThreadCount;
		int hidenchunkSize = 0;
		if (Type == NeuralEnums::NetworkType::AutoEncoder && AutoEncoderType == NeuralEnums::AutoEncoderType::Sparce)
		{
			int hidenchunkSize = Layers[Layers.size() - 2].Size / ThreadCount == 0 ? 1 : Layers[Layers.size() - 2].Size / ThreadCount;
		}
		int iterator = ThreadCount > Layers[Layers.size() - 1].Size ? Layers[Layers.size() - 1].Size : ThreadCount;

		for (size_t i = 0; i < lossesTmp.size(); i++)
		{
			lossesTmp[i] = 0;
		};

		for (int i = 0; i < iterator; i++)
		{
			int start = i * chunkSize;
			int end = (i + 1) == iterator ? Layers[Layers.size() - 1].Size : (i + 1) * chunkSize;
			int klbstart = i * hidenchunkSize;
			int klbend = (i + 1) == iterator ? Layers[Layers.size() - 2].Size : (i + 1) * hidenchunkSize;
			workers[i]->doAsync(std::bind(&NeuralNetwork::CalculateLossSub, this, start, end, klbstart, klbend, std::ref(lossesTmp[i])));
		}
		for (int k = 0; k < iterator; k++)
			workers[k]->wait();

		T result = 0.0;
		for (size_t i = 0; i < lossesTmp.size(); i++)
			result += lossesTmp[i];

		T regularizerCost = 0.0;
		if (Type == NeuralEnums::NetworkType::AutoEncoder && AutoEncoderType == NeuralEnums::AutoEncoderType::Sparce)
		{
			for (size_t w = 1; w < Layers.size(); w++)
			{
				for (size_t y = 0; y < Layers[w].Weights.size(); y++)
				{
					regularizerCost += Layers[w].Weights[y] * Layers[w].Weights[y];
				}
			}
		}
		return result + regularizerCost * lambda / 2.0;
	}
}

template<class T> void NeuralNetwork<T>::CalculateLossBatchSub(int start, int end, T& loss)
{
	T result = 0.0;
	for (size_t i = start; i < end; i++)
	{
		result += CalculateLossFunction(LossFunctionType, Layers[Layers.size() - 1].OutputsBatch[i], Layers[Layers.size() - 1].TargetsBatch[i], 0, Layers[Layers.size() - 1].TargetsBatch[i].size());
	}
	loss = result;
}

template<class T> void NeuralNetwork<T>::CalculateLossSub(int start, int end, int klbstart, int  klbend, T& loss)
{
	T result = 0.0;
	T klbResult = 0.0;
	for (size_t i = start; i < end; i++)
	{
		result += CalculateLossFunction(LossFunctionType, Layers[Layers.size() - 1].Outputs, Layers[Layers.size() - 1].Target, start, end);
	}

	if (Type == NeuralEnums::NetworkType::AutoEncoder && AutoEncoderType == NeuralEnums::AutoEncoderType::Sparce)
	{
		klbResult += KullbackLeiblerDivergence(Layers[1].RoHat, ro, klbstart, klbend);
	}
	loss = result + klbResult;
}
