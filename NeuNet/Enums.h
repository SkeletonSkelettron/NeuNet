#ifndef ENUMS_H
#define ENUMS_H
namespace NeuralEnums {
	enum class ActivationFunction
	{
		None = 0,
		Sigmoid = 1,
		Tanh = 2,
		ReLU = 3,
		MReLU = 4,
		SoftMax = 5,
		GELU = 6
    };
    enum class LayerType
    {
        InputLayer = 1,
        HiddenLayer = 2,
        OutputLayer = 3
    };
    enum class BalanceType
    {
        None = 0,
        // Mean = 0, StDev = 1
        GaussianStandartization = 1,
        // x(i)/Range
        Normalization = 2,
        //Probabilities are distributed normaly
        NormalDistrubution = 3
    };
    enum class LossFunctionType
    {
        MeanSquaredLoss = 1,
        CrossEntropyLoss = 2,
    };

    enum class LearningRateType
        {
            Static = 1,
            AdaGrad = 2,
            AdaDelta = 3,
            RMSProp = 4,
			GuraMethod = 5,
            cyclic,
        };

    enum class GradientType
        {
            Static = 1,
            Momentum = 2,
            Adam = 3
        };

	enum class NetworkType
	{
		Normal,
		AutoEncoder
	};
}
#endif // ENUMS_H


