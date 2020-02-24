#include "NeuralNetwork.h"

template<class T> void NeuralNetwork<T>::NeuralNetworkInit()
{
	Layers.resize(0);
	iterations = 0;
	beta1Pow = 0.9;
	beta2Pow = 0.999;
	for (int i = 0; i < ThreadCount; i++)
	{
		workers.push_back(new WorkerThread());
	}
	GradientsTemp.resize(ThreadCount);
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

			Layers[k].IndexVector.clear();

			for (size_t i = biasShift; i < Layers[k].DropoutNeurons.size(); i++)
				if (!Layers[k].DropoutNeurons[i])
					Layers[k].IndexVector.push_back(i);
		}
		else if (Layers[k].LayerType == NeuralEnums::LayerType::HiddenLayer)
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
		auto StartIndex = Layers[i].UsingBias ? 0 : 1;
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
			Layers[i].Weights[j] = Layers[i].UsingBias && j % Layers[i - 1].Size == 0 ? 1 : dist(gen) / 100;
		}
		BalanceWith<T>(Layers[i].Weights, BalanceType);
		if (Layers[i - 1].UsingBias)
			for (int j = 0; j < Layers[i - 1].Size * (Layers[i].Size - (Layers[i].UsingBias ? 1 : 0)); j++)
			{
				if (j % Layers[i - 1].Size == 0)
					Layers[i].Weights[j] = 1L;
			}
		//ifstream inData;
		//inData.open("weights" + std::to_string(i) + ".txt");
		//for (int count = 0; count < Layers[i].Weights.size(); count++) {
		//	inData >> std::setprecision(100) >> Layers[i].Weights[count];
		//}
	}
}



template<class T> T NeuralNetwork<T>::PropageteForward(bool training)
{
	ShuffleDropoutsGuram();
	for (int k = 1; k < Layers.size(); k++)
	{
		Layers[k].CalculateInputs(Layers[k - 1].OutputsBatch, Layers[k - 1].Outputs, Layers[k - 1].DropoutNeurons, usingDropouts, training);
		Layers[k].CalculateOutputs(usingDropouts, training);
	}
	return -1;//  CalculateLoss<T>(LossFunctionType, false, ThreadCount, workers);
}

template<class T> T NeuralNetwork<T>::PropageteForwardThreaded(bool training)
{
	if (training)
		ShuffleDropoutsGuram();
	for (int k = 1; k < Layers.size(); k++)
	{
		Layers[k].CalculateInputsThreaded(Layers[k - 1].Outputs, Layers[k - 1].OutputsBatch, Layers[k - 1].IndexVector, Layers[k - 1].UsingBias, training, ThreadCount, workers);
		Layers[k].CalculateOutputsThreaded(ThreadCount, training, workers);
	}
	if (this->LossCalculation == NeuralEnums::LossCalculation::Full)
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
	for (int i = Layers.size() - 1; i >= 1; i--)
	{
		if (Layers[i].UsingBias)
			Layers[i].IndexVector.erase(std::remove(Layers[i].IndexVector.begin(), Layers[i].IndexVector.end(), 0), Layers[i].IndexVector.end());
	}
	if (BatchSize > 1)
	{
		int chunkSize = Layers[0].InputsBatch.size() / ThreadCount;
		int idx = 0;

		for (int i = 0; i < ThreadCount; i++)
		{
			idx = chunkSize * i;

			workers[i]->doAsync(std::bind(&NeuralNetwork::PopagateBackDelegateBatch, this, idx, idx + chunkSize, std::ref(GradientsTemp[i])));
		}
		for (int k = 0; k < ThreadCount; k++)
			workers[k]->wait();

	}
	else
	{
		for (unsigned int i = Layers.size() - 1; i >= 1; i--)
		{

			int Size = Layers[i].Size;
			int chunkSize = Size / ThreadCount == 0 ? 1 : Size / ThreadCount;
			int iterator = ThreadCount > Size ? Size : ThreadCount;

			for (int l = 0; l < iterator; l++)
			{
				int start = l * chunkSize;
				int end = (l + 1) == iterator ? Size : (l + 1) * chunkSize;

				workers[l]->doAsync(std::bind(&NeuralNetwork::PopagateBackDelegate, this, i, start, end));
			}
			for (int k = 0; k < iterator; k++)
				workers[k]->wait();
		}
	}
	CalculateWeightsBatch();
}
template<class T> void NeuralNetwork<T>::PopagateBackDelegate(int i, int start, int end)
{
	int numberIndex = 0;
	int pLS = Layers[i - 1].Size;
	int biasShift = Layers[i].UsingBias ? 1 : 0;
	std::vector<T> inputs;
	start = start == 0 && Layers[i].UsingBias ? 1 : start;
	std::vector<T> outputsTemp;
	outputsTemp.resize(Layers[i - 1].Outputs.size());
	T gradient;
	T gradientTemp;
	T inpOutp;

	for (int j = start; j < end; j++)
	{
		if (Layers[i].DropoutNeurons[j])
			continue;
		if (Layers[i].ActivationFunction == NeuralEnums::ActivationFunction::SoftMax)
			inputs = Layers[i].Inputs;
		// Output ლეიერი
		if (i == Layers.size() - 1)
			Layers[i].Outputs[j] = DifferentiateLossWith(Layers[i].Outputs[j], Layers[i].Target[j], LossFunctionType, Layers[i].Target.size());
		Layers[i].Inputs[j] = DifferentiateWith(Layers[i].Inputs[j], Layers[i].ActivationFunction, inputs, Layers[i].DropoutNeurons);

		if (i != 1)
			for (int p = 0; p < pLS; p++)
			{
				if (Layers[i - 1].DropoutNeurons[p])
					continue;
				numberIndex = pLS * (j - biasShift) + p;
				inpOutp = Layers[i].Outputs[j] * Layers[i].Inputs[j];
				outputsTemp[p] += inpOutp * TempWeights[i][numberIndex];
				gradient = inpOutp * Layers[i - 1].Outputs[p];
				gradient = GetGradient(*this, gradient, numberIndex, i);
				Layers[i].Weights[numberIndex] = TempWeights[i][numberIndex] - GetLearningRate(*this, gradient, i, numberIndex);
			}
		else
			for (int p = 0; p < pLS; p++)
			{
				if (Layers[i - 1].DropoutNeurons[p])
					continue;
				numberIndex = pLS * (j - biasShift) + p;
				gradient = Layers[i].Outputs[j] * Layers[i].Inputs[j] * Layers[i - 1].Outputs[p];
				gradient = GetGradient(*this, gradient, numberIndex, i);
				Layers[i].Weights[numberIndex] = TempWeights[i][numberIndex] - GetLearningRate(*this, gradient, i, numberIndex);
			}
	}
	if (i != 1)
		for (int p = Layers[i - 1].UsingBias ? 1 : 0; p < pLS; p++)
		{
			if (Layers[i - 1].DropoutNeurons[p])
				continue;
			Layers[i - 1].Outputs[p] = outputsTemp[p];
		}
}


template<class T> void NeuralNetwork<T>::PopagateBackDelegateBatch(int start, int end, std::vector<std::vector<T>>& vec)
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
					vec[i][numberIndex] += Layers[i].InputsBatch[batch][j] * Layers[i - 1].OutputsBatch[batch][p];// gradient;
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
			Layers[i].Weights[numberIndex] -= GetLearningRate(*this, gradient, i, numberIndex);
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

			workers[i]->doAsync(std::bind(&NeuralNetwork::CalculateLossSub, this, idx, idx + chunkSize, losses[i]));
		}
		for (int k = 0; k < ThreadCount; k++)
			workers[k]->wait();
		return std::accumulate(losses.begin(), losses.end(), 0.0);
	}
	if (!training)
	{
		return CalculateLossFunction(LossFunctionType, Layers[Layers.size() - 1].Outputs, Layers[Layers.size() - 1].Target);
	}
	throw runtime_error("Loss function bad case!!");
}

template<class T> void NeuralNetwork<T>::CalculateLossSub(int start, int end, T& loss)
{
	T result = 0.0;
	for (size_t i = start; i < end; i++)
	{
		result += CalculateLossFunction(LossFunctionType, Layers[Layers.size() - 1].OutputsBatch[i], Layers[Layers.size() - 1].TargetsBatch[i]);
	}
	loss = result;
}
