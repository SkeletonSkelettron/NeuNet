#ifndef ENUMS_H
#define ENUMS_H
namespace NeuralEnums {
	enum class ActivationFunction
	{
		None,
		Sigmoid,
		Tanh,
		ReLU,
		MReLU,
		SoftMax,
		GeLU,
		SoftPlus,
		SoftSign
    };
    enum class LayerType
    {
        InputLayer,
        HiddenLayer,
        OutputLayer
    };
    enum class BalanceType
    {
        None,
        // Mean = 0, StDev = 1
        GaussianStandartization,
        // x(i)/Range
        Normalization,
        //Probabilities are distributed normaly
        NormalDistrubution
    };
    enum class LossFunctionType
    {
        MeanSquaredLoss,
        CrossEntropyLoss,
    };

    enum class LearningRateType
        {
			AdaDelta,
            AdaGrad,
			Adam,
			AdamMod,
			AdaMax,
			AMSGrad,
			Cyclic,
			GuraMethod,
			Nadam,
            RMSProp,
			Static
        };

    enum class GradientType
        {
            Static,
            Momentum
        };

	enum class NetworkType
	{
		Normal,
		AutoEncoder
	};

	enum class Precision
	{
		Float,
		Double,
		LongDouble,
		BoostBinFloat50,
		BoostBinFloat100
	};
}
#endif // ENUMS_H


