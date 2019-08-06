#include "NeuNet.h"
#include "readmnist.h"
#include <thread>
#include <ctime>
#include "textedit.h"
#include <QCompleter>
#include <qstringlistmodel.h>
#include <QtWidgets>
#include "treemodelcompleter.h"
#include <QStringList>
#include <qstring.h>
#include <QJsonDocument>
#include "NeuralQtThread.h"
#include <iomanip> 
#include "MnistData.h"
#include <sstream>
#include "Loss.h"
#include <boost/multiprecision/cpp_bin_float.hpp> 
//#include <boost/multiprecision/float128.hpp> 


//https://devblogs.microsoft.com/cppblog/new-code-optimizer/
NeuNet::NeuNet(QWidget *parent)
	: QMainWindow(parent)
{
	ui.setupUi(this);
	setupEditor();
}
void NeuNet::setupEditor()
{
	QFont font;
	font.setFamily("Source Code Pro");
	font.setFixedPitch(true);
	font.PreferAntialias;
	font.setPointSize(10);
	completingTextEdit = new TextEdit;
	highlighter = new Highlighter(completingTextEdit->document());
	completingTextEdit->setFont(font);

	completer = new TreeModelCompleter(this);
	completer->setModel(modelFromFile("../NeuNet/Resources/ModelTree.txt"));
	completer->setCaseSensitivity(Qt::CaseInsensitive);
	completer->setCompletionMode(QCompleter::PopupCompletion);

	////completingTextEdit->setCompleter(completer);

	completer->setSeparator(QLatin1String("."));
	////QObject::connect(completer, SIGNAL(highlighted(QModelIndex)),
	////	this, SLOT(highlight(QModelIndex)));
	// 
	//connect(completingTextEdit, SIGNAL(textChanged(QString)),
	//	completer, SLOT(setSeparator(QString)));

	//connect(completingTextEdit, SIGNAL(textChanged(QString)),
	//	this, SLOT(updateContentsLabel(QString)));
	completingTextEdit->setCompleter(completer);


	//lineEdit = new QLineEdit;
	//lineEdit->setCompleter(completer);

	//ui.tab->layout()->addWidget(completingTextEdit);

}
// The function we want to execute on the new thread.
void task1(string msg)
{
	std::string t = msg;
}

QAbstractItemModel *NeuNet::modelFromFile(const QString& fileName)
{
	QFile file(fileName);
	if (!file.open(QFile::ReadOnly))
		return new QStringListModel(completer);

#ifndef QT_NO_CURSOR
	QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));
#endif
	QStringList words;

	QStandardItemModel *model = new QStandardItemModel(completer);
	QVector<QStandardItem *> parents(10);
	parents[0] = model->invisibleRootItem();

	while (!file.atEnd()) {
		QString line = file.readLine();
		QString trimmedLine = line.trimmed();
		if (line.isEmpty() || trimmedLine.isEmpty())
			continue;

		QRegularExpression re("^\\s+");
		QRegularExpressionMatch match = re.match(line);
		int nonws = match.capturedStart();
		int level = 0;
		if (nonws == -1) {
			level = 0;
		}
		else {
			if (line.startsWith("\t")) {
				level = match.capturedLength();
			}
			else {
				level = match.capturedLength() / 4;
			}
		}

		if (level + 1 >= parents.size())
			parents.resize(parents.size() * 2);

		QStandardItem *item = new QStandardItem;
		item->setText(trimmedLine);
		parents[level]->appendRow(item);
		parents[level + 1] = item;
	}

#ifndef QT_NO_CURSOR
	QApplication::restoreOverrideCursor();
#endif

	return model;
}

template<typename T> void NeuNet::initNeUnetFromJson(NeuralNetwork<T> &neuralNetwork)
{
	auto text = ui.textEdit->toPlainText();
	QJsonDocument jsonResponse = QJsonDocument::fromJson(text.toUtf8());

	QJsonObject jsonObject = jsonResponse.object();
	QVariantMap jsonMap = jsonObject.toVariantMap();

	neuralNetwork.ThreadCount = jsonMap["ThreadCount"].toInt();
	neuralNetwork.LearningRate = jsonMap["LearningRate"].toDouble();
	neuralNetwork.NeuralNetworkInit();
	QString lrType = jsonMap["LearningRateType"].toString();
	QString balance = jsonMap["Balance"].toString();
	QString lossFunction = jsonMap["LossFunction"].toString();
	QString gradient = jsonMap["Gradient"].toString();

	QString networkType = jsonMap["Type"].toString();

	if (networkType == "Normal")
		neuralNetwork.Type = NeuralEnums::NetworkType::Normal;
	if (networkType == "AutoEncoder")
		neuralNetwork.Type = NeuralEnums::NetworkType::AutoEncoder;


	if (lrType == "Static")
		neuralNetwork.LearningRateType = NeuralEnums::LearningRateType::Static;
	if (lrType == "AdaDelta")
		neuralNetwork.LearningRateType = NeuralEnums::LearningRateType::AdaDelta;
	if (lrType == "AdaGrad")
		neuralNetwork.LearningRateType = NeuralEnums::LearningRateType::AdaGrad;
	if (lrType == "Adam")
		neuralNetwork.LearningRateType = NeuralEnums::LearningRateType::Adam;
	if (lrType == "AdamMod")
		neuralNetwork.LearningRateType = NeuralEnums::LearningRateType::AdamMod;
	if (lrType == "cyclic")
		neuralNetwork.LearningRateType = NeuralEnums::LearningRateType::Cyclic;
	if (lrType == "RMSProp")
		neuralNetwork.LearningRateType = NeuralEnums::LearningRateType::RMSProp;
	if (lrType == "GuraMethod")
		neuralNetwork.LearningRateType = NeuralEnums::LearningRateType::GuraMethod;

	if (balance == "GaussianStandartization")
		neuralNetwork.BalanceType = NeuralEnums::BalanceType::GaussianStandartization;
	if (balance == "NormalDistrubution")
		neuralNetwork.BalanceType = NeuralEnums::BalanceType::NormalDistrubution;
	if (balance == "Normalization")
		neuralNetwork.BalanceType = NeuralEnums::BalanceType::Normalization;
	if (balance == "None")
		neuralNetwork.BalanceType = NeuralEnums::BalanceType::None;

	if (lossFunction == "CrossEntropyLoss")
		neuralNetwork.LossFunctionType = NeuralEnums::LossFunctionType::CrossEntropyLoss;
	if (lossFunction == "MeanSquaredLoss")
		neuralNetwork.LossFunctionType = NeuralEnums::LossFunctionType::MeanSquaredLoss;

	if (gradient == "Momentum")
		neuralNetwork.GradientType = NeuralEnums::GradientType::Momentum;
	if (gradient == "Static")
		neuralNetwork.GradientType = NeuralEnums::GradientType::Static;

	QJsonArray arr = jsonMap["Layers"].toJsonArray();
	for (int i = 0; i < arr.count(); ++i)
	{
		QJsonObject arrayObject = arr.at(i).toObject();

		QVariantMap arrayObjectMap = arrayObject.toVariantMap();
		NeuralEnums::LayerType layerType;
		NeuralEnums::ActivationFunction ActivationFunctionType;
		QString ActivationFunction = arrayObjectMap["ActivationFunction"].toString();
		QString LayerType = arrayObjectMap["Type"].toString();

		double DropuOutSize = arrayObjectMap["DropuOutSize"].toDouble();
		double bias = arrayObjectMap["Bias"].toDouble();
		int size = arrayObjectMap["Size"].toInt();
		if (ActivationFunction == "MReLU")
			ActivationFunctionType = NeuralEnums::ActivationFunction::MReLU;
		if (ActivationFunction == "None")
			ActivationFunctionType = NeuralEnums::ActivationFunction::None;
		if (ActivationFunction == "ReLU")
			ActivationFunctionType = NeuralEnums::ActivationFunction::ReLU;
		if (ActivationFunction == "Sigmoid")
			ActivationFunctionType = NeuralEnums::ActivationFunction::Sigmoid;
		if (ActivationFunction == "SoftMax")
			ActivationFunctionType = NeuralEnums::ActivationFunction::SoftMax;
		if (ActivationFunction == "Tanh")
			ActivationFunctionType = NeuralEnums::ActivationFunction::Tanh;
		if (ActivationFunction == "GeLU")
			ActivationFunctionType = NeuralEnums::ActivationFunction::GeLU;
		if (ActivationFunction == "SoftPlus")
			ActivationFunctionType = NeuralEnums::ActivationFunction::SoftPlus;
		if (ActivationFunction == "SoftSign")
			ActivationFunctionType = NeuralEnums::ActivationFunction::SoftSign;

		if (LayerType == "HiddenLayer")
			layerType = NeuralEnums::LayerType::HiddenLayer;
		if (LayerType == "InputLayer")
			layerType = NeuralEnums::LayerType::InputLayer;
		if (LayerType == "OutputLayer")
			layerType = NeuralEnums::LayerType::OutputLayer;
		neuralNetwork.Layers.push_back(*(new Layer<T>(size, layerType, ActivationFunctionType, bias, DropuOutSize)));
	}
	neuralNetwork.InitializeWeights();
}

template<typename T> void ReadData(std::vector<MnistData<T>> &trainingSet, std::vector<MnistData<T>> &testSet, bool hilbertCurve = false)
{
	std::vector<int> labels;
	std::vector<int> testLabels;
	std::vector<std::vector<T>> _trainingSet;
	std::vector<std::vector<T>> _testSet;
	std::vector<std::vector<T>> labeledTarget;
	std::vector<std::vector<T>> testLabeledTarget;

	ReadMNISTMod<T>(_trainingSet, labels, true, hilbertCurve);
	ReadMNISTMod<T>(_testSet, testLabels, false, hilbertCurve);

	long int total = _trainingSet.size();
	labeledTarget.resize(_trainingSet.size());
	for (long int i = 0; i < total; i++)
	{
		NormalizeN(_trainingSet[i]);
		labeledTarget[i].resize(10);
		for (unsigned int k = 0; k < 10; k++)
			labeledTarget[i][k] = labels[i] == k ? 1.0L : 0.0L;
	}

	long int totalTest = _testSet.size();
	testLabeledTarget.resize(totalTest);
	for (long int k = 0; k < totalTest; k++)
	{
		NormalizeN(_testSet[k]);
		testLabeledTarget[k].resize(10);
		for (unsigned int g = 0; g < 10; g++)
			testLabeledTarget[k][g] = testLabels[k] == g ? 1.0L : 0.0L;
	}
	trainingSet.resize(_trainingSet.size());
	for (long int i = 0; i < _trainingSet.size(); i++)
	{
		trainingSet[i].set = _trainingSet[i];
		trainingSet[i].labels = labeledTarget[i];
	}

	testSet.resize(_testSet.size());
	for (long int i = 0; i < _testSet.size(); i++)
	{
		testSet[i].set = _testSet[i];
		testSet[i].labels = testLabeledTarget[i];
	}
}

void NeuNet::on_pushButton_clicked()
{
	NeuralQtThread *nthread = new NeuralQtThread(this);
	nthread->setLambda([this]
	{
		boost::multiprecision::cpp_bin_float_100 b = 2;

		NeuralNetwork<long double> neuralNetwork;

		std::vector<MnistData<long double>> trainingSet;

		std::vector<MnistData<long double>> testSet;

		std::vector<long double> losses;

		LossVector lossVector;

		initNeUnetFromJson(neuralNetwork);
		ui.textEdit->append(QString::fromStdString("start reading training+test data "));
		clock_t begin = clock();
		ReadData(trainingSet, testSet, false);
		clock_t end = clock();
		ui.textEdit->append(QString::fromStdString("...done in " + std::to_string(float(end - begin) / CLOCKS_PER_SEC) + " seconds"));
		long int total = trainingSet.size();
		long int testTotal = testSet.size();

		ui.textEdit->append(QString::fromStdString("start training"));
		begin = clock();
		int globalEpochs = 300;
		long int totalcounter = 0;
		unsigned long int TotalEpochs = trainingSet.size() * globalEpochs, range;
		for (size_t g = 0; g < globalEpochs; g++)
		{
			try {
				clock_t beginInside = clock();
				unsigned seed = std::chrono::system_clock::now().time_since_epoch().count();
				shuffle(trainingSet.begin(), trainingSet.end(), std::default_random_engine(seed));
				for (long int i = 0; i < total; i++)
				{

					int biasShift = neuralNetwork.Layers[0].UsingBias ? 1 : 0;
					for (unsigned long int l = 0; l < trainingSet[i].set.size(); l++)
						neuralNetwork.Layers[0].Inputs[l + biasShift] = trainingSet[i].set[l];

					auto loss = neuralNetwork.PropageteForwardThreaded(neuralNetwork.Type == NeuralEnums::NetworkType::Normal ? trainingSet[i].labels : trainingSet[i].set, true);
					neuralNetwork.PropagateBackThreaded(neuralNetwork.Type == NeuralEnums::NetworkType::Normal ? trainingSet[i].labels : trainingSet[i].set);
					if (neuralNetwork.LearningRateType == NeuralEnums::LearningRateType::Adam)
					{
						neuralNetwork.iterations++;
						neuralNetwork.beta1Pow = pow(0.9, neuralNetwork.iterations);
						neuralNetwork.beta2Pow = pow(0.999, neuralNetwork.iterations);
					}

					//neuralNetwork.LearningRate -= neuralNetwork.LearningRate / (long double)TotalEpochs;
					//if (g == 20)
					//{
					//	neuralNetwork.LearningRate = 0.5L;
					//}
					//if (g == 40)
					//{
					//	neuralNetwork.LearningRate = 0.45L;
					//}
					//if (g == 60)
					//{
					//	neuralNetwork.LearningRate = 0.5L;
					//}
					//neuralNetwork.LearningRate -= (neuralNetwork.LearningRate - static_cast<long double>(0.5)) / static_cast<long double>(total * globalEpochs);
					losses.push_back(loss);
					//neuralNetwork.LearningRate -= (neuralNetwork.LearningRate - 0.05L) / (long double)(total * globalEpochs);
					if (i % 1000 == 0 && i != 0)
					{
						totalcounter += 1000;
						ui.pushButton->setText(QString::fromStdString(std::to_string(totalcounter) + "/" + std::to_string(total * globalEpochs)));
						//ui.textEdit->append(QString::fromStdString("loss: " + std::to_string(loss) + " ;"));
					}

				}
				clock_t endInside = clock();
				/*auto loopComplete = std::to_string(g + 1) + " of " + std::to_string(globalEpochs) + " done in " + std::to_string(float(endInside - beginInside) / CLOCKS_PER_SEC) + " seconds. ";*/

				long int counter = 0;
				long int digitCounter = 0;
				//beginInside = clock();
				if (neuralNetwork.Type == NeuralEnums::NetworkType::Normal)
				{
					for (long int i = 0; i < trainingSet[i].set.size(); i++)
					{

						int biasShift = neuralNetwork.Layers[0].UsingBias ? 1 : 0;
						for (unsigned long int l = 0; l < testSet[i].set.size(); l++)
							neuralNetwork.Layers[0].Inputs[l + biasShift] = trainingSet[i].set[l];

						auto loss = neuralNetwork.PropageteForwardThreaded(trainingSet[i].labels, false);
						if (GetMaxIndex(neuralNetwork.Layers[neuralNetwork.Layers.size() - 1].Outputs) == GetMaxIndex(trainingSet[i].labels))
							counter++;
						digitCounter++;
					}
					long double result = (long double)counter / (long double)digitCounter;
					auto testComplete = "training-set result: " + std::to_string(result);

					//ui.textEdit->append(QString::fromStdString(loopComplete + testComplete /* + "...testing done in " + std::to_string(float(endInside - beginInside) / CLOCKS_PER_SEC) + " seconds")*/));

					counter = 0;
					digitCounter = 0;
					for (long int i = 0; i < testTotal; i++)
					{

						int biasShift = neuralNetwork.Layers[0].UsingBias ? 1 : 0;
						for (unsigned long int l = 0; l < testSet[i].set.size(); l++)
							neuralNetwork.Layers[0].Inputs[l + biasShift] = testSet[i].set[l];

						auto loss = neuralNetwork.PropageteForwardThreaded(testSet[i].labels, false);
						if (GetMaxIndex(neuralNetwork.Layers[neuralNetwork.Layers.size() - 1].Outputs) == GetMaxIndex(testSet[i].labels))
							counter++;
						digitCounter++;
					}
					result = (long double)counter / (long double)digitCounter;
					auto testComplete2 = "; test-set result: " + std::to_string(result);
					//endInside = clock();
					auto loopComplete = std::to_string(g + 1) + " of " + std::to_string(globalEpochs) + " done in " + std::to_string(float(endInside - beginInside) / CLOCKS_PER_SEC) + " seconds. ";
					ui.textEdit->append(QString::fromStdString(loopComplete + testComplete + testComplete2 /* + "...testing done in " + std::to_string(float(endInside - beginInside) / CLOCKS_PER_SEC) + " seconds")*/));


				}
				losses.clear();
				//if (neuralNetwork.Type == NeuralEnums::NetworkType::AutoEncoder)
				//{
				//	for (long int i = 0; i < testTotal; i++)
				//	{

				//		int biasShift = neuralNetwork.Layers[0].UsingBias ? 1 : 0;
				//		for (unsigned long int l = 0; l < testSet[i].set.size(); l++)
				//			neuralNetwork.Layers[0].Inputs[l + biasShift] = testSet[i].set[l];

				//		auto loss = neuralNetwork.PropageteForwardThreaded(testSet[i].set, false);
				//		losses.push_back(loss);
				//	}
				//	auto result = 0.0;
				//	for (unsigned long int v = 0; v < losses.size(); v++)
				//	{
				//		result += losses[v];
				//	}
				//	auto testComplete = "test-set average loss: " + std::to_string(result / losses.size());
				//	endInside = clock();
				//	//ui.textEdit->append(QString::fromStdString(loopComplete + testComplete /* + "...testing done in " + std::to_string(float(endInside - beginInside) / CLOCKS_PER_SEC) + " seconds")*/));
				//}
			}
			catch (exception e)
			{
				ui.textEdit->append(QString::fromStdString(e.what()));
			}
		}


		end = clock();
		ui.textEdit->append(QString::fromStdString("training done in " + std::to_string(float(end - begin) / CLOCKS_PER_SEC) + " seconds"));

		QString Hfilename = "./losses.txt";
		QFile fileH(Hfilename);
		if (fileH.open(QIODevice::ReadWrite))
		{
			std::string str;
			QTextStream stream(&fileH);
			for (long i = 0; i < losses.size(); i++)
			{
				std::ostringstream out;
				out.precision(64);
				out << std::fixed << losses[i];

				stream << QString::fromStdString(out.str()) << endl;
			}
		}
		//begin = clock();
		//long int counter2 = 0;
		//long int digitCounter2 = 0;
		//for (long int i = 0; i < total; i++)
		//{
		//	int biasShift = neuralNetwork.Layers[0].UsingBias ? 1 : 0;
		//	for (unsigned long int l = 0; l < trainingSet[i].set.size(); l++)
		//		neuralNetwork.Layers[0].Inputs[l + biasShift] = trainingSet[i].set[l];

		//	auto loss = neuralNetwork.PropageteForwardThreaded(trainingSet[i].labels, false);
		//	if (GetMaxIndex(neuralNetwork.Layers[neuralNetwork.Layers.size() - 1].Outputs) == GetMaxIndex(trainingSet[i].labels))
		//		counter2++;
		//	digitCounter2++;
		//}
		//end = clock();
		//long double result2 = (long double)counter2 / (long double)digitCounter2;
		//ui.textEdit->append(QString::fromStdString("training-set result: " + std::to_string(result2)));
		//ui.textEdit->append(QString::fromStdString("testing done in " + std::to_string(float(end - begin) / CLOCKS_PER_SEC) + " seconds"));

		NeuralQtThread *nthread = new NeuralQtThread(this);
	});
	nthread->start();


}

void NeuNet::on_pushButton_2_clicked()
{

	/*long double *k;
	k = new long double[200000000];
std:vector<long double>f;
	f.resize(200000000);
	for (long int  i = 0; i < 200000000; i++)
	{
		k[i] = i;
		f[i] = i;
	}

	clock_t begin = clock();
	long int g = 0;
	for (long int i = 0; i < 200000000; i++)
	{
		k[i] = i;
		//g++;
	}
	clock_t end = clock();
	ui.textEdit->append(QString::fromStdString("training done in " + std::to_string(float(end - begin) / CLOCKS_PER_SEC) + " seconds;" + std::to_string(float(g))));*/


	std::vector<long double> ar;
	//bias test case
	NeuralNetwork<long double> neuralNetwork;
	ar.push_back(1);
	ar.push_back(0.9);
	ar.push_back(0.4);
	ar.push_back(0.7);
	std::vector<long double> labels;
	std::vector<long double> targetArray;
	std::vector<long double> losses;
	targetArray.resize(2);
	targetArray[0] = long double(1.0);
	targetArray[1] = long double(0.01);
	Layer<long double> layerInput(4, NeuralEnums::LayerType::InputLayer, NeuralEnums::ActivationFunction::None, 1, 0);
	Layer<long double> layerHidden1(4, NeuralEnums::LayerType::HiddenLayer, NeuralEnums::ActivationFunction::Sigmoid, 1, 0);
	Layer<long double> layerHidden2(3, NeuralEnums::LayerType::HiddenLayer, NeuralEnums::ActivationFunction::Sigmoid, 1, 0);
	Layer<long double> layerOutput(2, NeuralEnums::LayerType::OutputLayer, NeuralEnums::ActivationFunction::Sigmoid, 0, 0);
	std::vector<Layer<long double>> vecs;

	vecs.push_back(layerInput);
	vecs.push_back(layerHidden1);
	vecs.push_back(layerHidden2);
	vecs.push_back(layerOutput);
	neuralNetwork.LearningRate = long double(0.6);
	neuralNetwork.ThreadCount = 6;

	neuralNetwork.BalanceType = NeuralEnums::BalanceType::GaussianStandartization;
	neuralNetwork.LossFunctionType = NeuralEnums::LossFunctionType::MeanSquaredLoss;
	neuralNetwork.NeuralNetworkInit();
	neuralNetwork.Layers = vecs;
	neuralNetwork.Layers[1].Weights.push_back(long double(1));
	neuralNetwork.Layers[1].Weights.push_back(long double(0.6));
	neuralNetwork.Layers[1].Weights.push_back(long double(0.7));
	neuralNetwork.Layers[1].Weights.push_back(long double(-0.4));
	neuralNetwork.Layers[1].Weights.push_back(long double(1));
	neuralNetwork.Layers[1].Weights.push_back(long double(-0.8));
	neuralNetwork.Layers[1].Weights.push_back(long double(0.4));
	neuralNetwork.Layers[1].Weights.push_back(long double(0.1));
	neuralNetwork.Layers[1].Weights.push_back(long double(1));
	neuralNetwork.Layers[1].Weights.push_back(long double(0.23));
	neuralNetwork.Layers[1].Weights.push_back(long double(0.17));
	neuralNetwork.Layers[1].Weights.push_back(long double(0.16));
	neuralNetwork.Layers[1].MultipliedSums.resize(12);

	neuralNetwork.Layers[2].Weights.push_back(long double(1));
	neuralNetwork.Layers[2].Weights.push_back(long double(-0.5));
	neuralNetwork.Layers[2].Weights.push_back(long double(0.5));
	neuralNetwork.Layers[2].Weights.push_back(long double(-0.2));
	neuralNetwork.Layers[2].Weights.push_back(long double(1));
	neuralNetwork.Layers[2].Weights.push_back(long double(0.3));
	neuralNetwork.Layers[2].Weights.push_back(long double(-0.46));
	neuralNetwork.Layers[2].Weights.push_back(long double(0.76));
	neuralNetwork.Layers[2].MultipliedSums.resize(8);

	neuralNetwork.Layers[3].Weights.push_back(long double(1));
	neuralNetwork.Layers[3].Weights.push_back(long double(0.3));
	neuralNetwork.Layers[3].Weights.push_back(long double(0.4));
	neuralNetwork.Layers[3].Weights.push_back(long double(1));
	neuralNetwork.Layers[3].Weights.push_back(long double(0.7));
	neuralNetwork.Layers[3].Weights.push_back(long double(0.92));
	neuralNetwork.Layers[3].MultipliedSums.resize(6);
	neuralNetwork.Layers[0].Inputs = ar;


	for (unsigned int i = 0; i < 100; i++)
	{

		auto loss = neuralNetwork.PropageteForwardThreaded(targetArray, false); // პირველი loss უნდა იყოს 0.20739494219121993
		neuralNetwork.PropagateBackThreaded(targetArray);

		//auto loss = neuralNetwork.PropageteForwardThreaded(targetArray); // პირველი loss უნდა იყოს 0.20739494219121993
		//neuralNetwork.PropagateBackThreaded(targetArray);

			//1.0002170802724326
			//0.60019537224518926
			//0.70008683210897293
			//- 0.39984804380929723
			//0.99984718866787936
			//- 0.80013753019890865
			//0.39993887546715173
			//0.099893032067515528
			//0.99955724383635436
			//0.22960151945271889
			//0.16982289753454174
			//0.15969007068544802


		losses.push_back(loss);
	}


	/*std::vector<long double> ar;

	NeuralNetwork<long double> neuralNetwork;
	ar.push_back(1);
	ar.push_back(0.4);
	ar.push_back(0.7);
	std::vector<long double> labels;
	std::vector<long double> targetArray;
	std::vector<long double> losses;
	targetArray.resize(2);
	targetArray[0] = long double(1.0);
	targetArray[1] = long double(0.01);
	Layer<long double> layerInput(3, NeuralEnums::LayerType::InputLayer, NeuralEnums::ActivationFunction::None, 0, 0);
	Layer<long double> layerHidden1(3, NeuralEnums::LayerType::HiddenLayer, NeuralEnums::ActivationFunction::Sigmoid, 0.0, 0);
	Layer<long double> layerHidden2(2, NeuralEnums::LayerType::HiddenLayer, NeuralEnums::ActivationFunction::Sigmoid, 0.0, 0);
	Layer<long double> layerOutput(2, NeuralEnums::LayerType::OutputLayer, NeuralEnums::ActivationFunction::Sigmoid, 0, 0);
	std::vector<Layer<long double>> vecs;

	vecs.push_back(layerInput);
	vecs.push_back(layerHidden1);
	vecs.push_back(layerHidden2);
	vecs.push_back(layerOutput);
	neuralNetwork.LearningRate = long double(0.5);
	neuralNetwork.ThreadCount = 6;

	neuralNetwork.BalanceType = NeuralEnums::BalanceType::GaussianStandartization;
	neuralNetwork.LossFunctionType = NeuralEnums::LossFunctionType::MeanSquaredLoss;
	neuralNetwork.NeuralNetworkInit();
	neuralNetwork.Layers = vecs;
	neuralNetwork.Layers[1].Weights.push_back(long double(0.5));
	neuralNetwork.Layers[1].Weights.push_back(long double(0.6));
	neuralNetwork.Layers[1].Weights.push_back(long double(0.7));
	neuralNetwork.Layers[1].Weights.push_back(long double(-0.4));
	neuralNetwork.Layers[1].Weights.push_back(long double(0.1));
	neuralNetwork.Layers[1].Weights.push_back(long double(-0.8));
	neuralNetwork.Layers[1].Weights.push_back(long double(0.4));
	neuralNetwork.Layers[1].Weights.push_back(long double(0.1));
	neuralNetwork.Layers[1].Weights.push_back(long double(0.8));
	neuralNetwork.Layers[1].Gradients.resize(9);

	neuralNetwork.Layers[2].Weights.push_back(long double(0.4));
	neuralNetwork.Layers[2].Weights.push_back(long double(-0.5));
	neuralNetwork.Layers[2].Weights.push_back(long double(0.5));
	neuralNetwork.Layers[2].Weights.push_back(long double(-0.2));
	neuralNetwork.Layers[2].Weights.push_back(long double(-0.3));
	neuralNetwork.Layers[2].Weights.push_back(long double(0.3));
	neuralNetwork.Layers[2].Gradients.resize(6);

	neuralNetwork.Layers[3].Weights.push_back(long double(0.2));
	neuralNetwork.Layers[3].Weights.push_back(long double(0.3));
	neuralNetwork.Layers[3].Weights.push_back(long double(0.4));
	neuralNetwork.Layers[3].Weights.push_back(long double(0.5));
	neuralNetwork.Layers[3].Gradients.resize(4);
	neuralNetwork.Layers[0].Inputs = ar;

	neuralNetwork.Layers[0].DropoutNeurons = { false,false,false };
	neuralNetwork.Layers[1].DropoutNeurons = { false,false,false };
	neuralNetwork.Layers[2].DropoutNeurons = { false,false };
	neuralNetwork.Layers[3].DropoutNeurons = { false,false };

	for (unsigned int i = 0; i < 100; i++)
	{

		auto loss = neuralNetwork.PropageteForward(targetArray, false); // პირველი loss უნდა იყოს 2.256
		neuralNetwork.PropagateBack(targetArray);

		//auto loss = neuralNetwork.PropageteForwardThreaded(targetArray); // პირველი loss უნდა იყოს 2.256
		//neuralNetwork.PropagateBackThreaded(targetArray);

		//0.49987896854397323
		//	0.59995158741758925
		//	0.69991527798078124
		//	-0.39926114723068823
		//	0.10029554110772472
		//	-0.79948280306148178
		//	0.39928705591562424
		//	0.099714822366249706
		//	0.79950093914093701
		losses.push_back(loss);
	}*/



	/*unsigned short tn = std::thread::hardware_concurrency();

   std::vector<std::thread> threads;
   threads.resize(2);

   threads[0] = std::thread(task1,"hello");
   threads[1] = std::thread(task1, "zd");

   int numThreads = std::thread::hardware_concurrency();

   std::vector<float> ar;
   ar.push_back(5);
   ar.push_back(6);
   ar.push_back(7);
   ar.push_back(8);

   std::vector<float> labels;
   std::vector<float> targetArray;
   std::vector<float> losses;

   targetArray.resize(2);
   targetArray[0] = 1;
   targetArray[1] = 0.01;

   Layer<float> layerInput(4, NeuralEnums::LayerType::InputLayer, NeuralEnums::ActivationFunction::None, numThreads);
   Layer<float> layerHidden1(3, NeuralEnums::LayerType::HiddenLayer, NeuralEnums::ActivationFunction::Sigmoid, numThreads);
   Layer<float> layerHidden2(3, NeuralEnums::LayerType::HiddenLayer, NeuralEnums::ActivationFunction::Sigmoid, numThreads);
   Layer<float> layerOutput(2, NeuralEnums::LayerType::OutputLayer, NeuralEnums::ActivationFunction::Sigmoid, numThreads);
   std::vector<Layer<float>> vecs;

   vecs.push_back(layerInput);
   vecs.push_back(layerHidden1);
   vecs.push_back(layerHidden2);
   vecs.push_back(layerOutput);
   NeuralNetwork<float> neuralNetwork;
   neuralNetwork.ThreadCount = 8;
   neuralNetwork.LearningRate = 0.5;
   neuralNetwork.NeuralNetworkInit(vecs);
   neuralNetwork.InitializeWeights(NeuralEnums::BalanceType::GaussainStandartization);

   neuralNetwork.Layers[0].Inputs = ar;

   for (unsigned int i = 0; i < 10; i++)
   {

	   auto loss = neuralNetwork.PropageteForwardThreaded(targetArray); // პირველი loss უნდა იყოს 2.256
	   //auto loss2 = neuralNetwork.PropageteForward(targetArray); // პირველი loss უნდა იყოს 2.256
	   losses.push_back(loss);
	   // neuralNetwork.PropagateBack(targetArray, NeuralEnums::LearningRateType::Static, NeuralEnums::GradientType::Static);
   } */
   /*
	   std::vector<long double> ar;

	   NeuralNetwork<long double> neuralNetwork;
	   ar.push_back(1);
	   ar.push_back(0.4);
	   ar.push_back(0.7);
	   std::vector<long double> labels;
	   std::vector<long double> targetArray;
	   std::vector<long double> losses;
	   targetArray.resize(2);
	   targetArray[0] = long double(1.0);
	   targetArray[1] = long double(0.01);
	   Layer<long double> layerInput(3, NeuralEnums::LayerType::InputLayer, NeuralEnums::ActivationFunction::None, 0, 0);
	   Layer<long double> layerHidden1(5, NeuralEnums::LayerType::HiddenLayer, NeuralEnums::ActivationFunction::Sigmoid, 0.4, 0);
	   Layer<long double> layerHidden2(4, NeuralEnums::LayerType::HiddenLayer, NeuralEnums::ActivationFunction::Sigmoid, 0.5, 0);
	   Layer<long double> layerOutput(2, NeuralEnums::LayerType::OutputLayer, NeuralEnums::ActivationFunction::Sigmoid, 0, 0);
	   std::vector<Layer<long double>> vecs;

	   vecs.push_back(layerInput);
	   vecs.push_back(layerHidden1);
	   vecs.push_back(layerHidden2);
	   vecs.push_back(layerOutput);
	   neuralNetwork.LearningRate = long double(0.5);
	   neuralNetwork.ThreadCount = 6;
	   neuralNetwork.NeuralNetworkInit();
	   neuralNetwork.Layers = vecs;
	   neuralNetwork.Layers[1].Weights.push_back(long double(0.1));
	   neuralNetwork.Layers[1].Weights.push_back(long double(0.2));
	   neuralNetwork.Layers[1].Weights.push_back(long double(0.3));

	   neuralNetwork.Layers[1].Weights.push_back(long double(0.4));
	   neuralNetwork.Layers[1].Weights.push_back(long double(0.5));
	   neuralNetwork.Layers[1].Weights.push_back(long double(0.6));

	   neuralNetwork.Layers[1].Weights.push_back(long double(0.7));
	   neuralNetwork.Layers[1].Weights.push_back(long double(0.8));
	   neuralNetwork.Layers[1].Weights.push_back(long double(0.9));

	   neuralNetwork.Layers[1].Weights.push_back(long double(0.91));
	   neuralNetwork.Layers[1].Weights.push_back(long double(0.92));
	   neuralNetwork.Layers[1].Weights.push_back(long double(0.93));

	   neuralNetwork.Layers[1].Weights.push_back(long double(0.94));
	   neuralNetwork.Layers[1].Weights.push_back(long double(0.95));
	   neuralNetwork.Layers[1].Weights.push_back(long double(0.96));
	   neuralNetwork.Layers[1].Gradients.resize(15);

	   neuralNetwork.Layers[2].Weights.push_back(long double(0.01));
	   neuralNetwork.Layers[2].Weights.push_back(long double(0.02));
	   neuralNetwork.Layers[2].Weights.push_back(long double(0.03));
	   neuralNetwork.Layers[2].Weights.push_back(long double(0.04));
	   neuralNetwork.Layers[2].Weights.push_back(long double(0.05));

	   neuralNetwork.Layers[2].Weights.push_back(long double(0.06));
	   neuralNetwork.Layers[2].Weights.push_back(long double(0.07));
	   neuralNetwork.Layers[2].Weights.push_back(long double(0.08));
	   neuralNetwork.Layers[2].Weights.push_back(long double(0.09));
	   neuralNetwork.Layers[2].Weights.push_back(long double(0.29));

	   neuralNetwork.Layers[2].Weights.push_back(long double(0.11));
	   neuralNetwork.Layers[2].Weights.push_back(long double(0.12));
	   neuralNetwork.Layers[2].Weights.push_back(long double(0.13));
	   neuralNetwork.Layers[2].Weights.push_back(long double(0.14));
	   neuralNetwork.Layers[2].Weights.push_back(long double(0.15));

	   neuralNetwork.Layers[2].Weights.push_back(long double(0.16));
	   neuralNetwork.Layers[2].Weights.push_back(long double(0.17));
	   neuralNetwork.Layers[2].Weights.push_back(long double(0.18));
	   neuralNetwork.Layers[2].Weights.push_back(long double(0.19));
	   neuralNetwork.Layers[2].Weights.push_back(long double(0.20));
	   neuralNetwork.Layers[2].Gradients.resize(20);

	   neuralNetwork.Layers[3].Weights.push_back(long double(0.2));
	   neuralNetwork.Layers[3].Weights.push_back(long double(0.3));
	   neuralNetwork.Layers[3].Weights.push_back(long double(0.4));
	   neuralNetwork.Layers[3].Weights.push_back(long double(0.5));

	   neuralNetwork.Layers[3].Weights.push_back(long double(0.6));
	   neuralNetwork.Layers[3].Weights.push_back(long double(0.7));
	   neuralNetwork.Layers[3].Weights.push_back(long double(0.8));
	   neuralNetwork.Layers[3].Weights.push_back(long double(0.9));
	   neuralNetwork.Layers[3].Gradients.resize(8);

	   neuralNetwork.BalanceType = NeuralEnums::BalanceType::GaussianStandartization;
	   neuralNetwork.LossFunctionType = NeuralEnums::LossFunctionType::MeanSquaredLoss;

	   //neuralNetwork.InitializeWeights();

	   neuralNetwork.Layers[0].Inputs = ar;

	   neuralNetwork.Layers[1].DropoutNeurons = { true,false,false,true,false };
	   neuralNetwork.Layers[2].DropoutNeurons = { true,false,true,false };

	   NeuralNetwork<long double> neuralNetwork2;
	   Layer<long double> layerInput_(3, NeuralEnums::LayerType::InputLayer, NeuralEnums::ActivationFunction::None, 0, 0);
	   Layer<long double> layerHidden1_(3, NeuralEnums::LayerType::HiddenLayer, NeuralEnums::ActivationFunction::Sigmoid, 0, 0);
	   Layer<long double> layerHidden2_(2, NeuralEnums::LayerType::HiddenLayer, NeuralEnums::ActivationFunction::Sigmoid, 0, 0);
	   Layer<long double> layerOutput_(2, NeuralEnums::LayerType::OutputLayer, NeuralEnums::ActivationFunction::Sigmoid, 0, 0);
	   std::vector<Layer<long double>> vecs_;

	   vecs_.push_back(layerInput_);
	   vecs_.push_back(layerHidden1_);
	   vecs_.push_back(layerHidden2_);
	   vecs_.push_back(layerOutput_);
	   neuralNetwork2.LearningRate = long double(0.5);
	   neuralNetwork2.ThreadCount = 6;
	   neuralNetwork2.NeuralNetworkInit();
	   neuralNetwork2.Layers = vecs_;

	   neuralNetwork2.Layers[1].Weights.push_back(long double(0.4));
	   neuralNetwork2.Layers[1].Weights.push_back(long double(0.5));
	   neuralNetwork2.Layers[1].Weights.push_back(long double(0.6));

	   neuralNetwork2.Layers[1].Weights.push_back(long double(0.7));
	   neuralNetwork2.Layers[1].Weights.push_back(long double(0.8));
	   neuralNetwork2.Layers[1].Weights.push_back(long double(0.9));

	   neuralNetwork2.Layers[1].Weights.push_back(long double(0.94));
	   neuralNetwork2.Layers[1].Weights.push_back(long double(0.95));
	   neuralNetwork2.Layers[1].Weights.push_back(long double(0.96));
	   neuralNetwork2.Layers[1].Gradients.resize(9);


	   //neuralNetwork2.Layers[2].Weights.push_back(long double(0.01));
	   //neuralNetwork2.Layers[2].Weights.push_back(long double(0.02));
	   //neuralNetwork2.Layers[2].Weights.push_back(long double(0.03));
	   //neuralNetwork2.Layers[2].Weights.push_back(long double(0.04));
	   //neuralNetwork2.Layers[2].Weights.push_back(long double(0.05));

	   //neuralNetwork2.Layers[2].Weights.push_back(long double(0.06));
	   neuralNetwork2.Layers[2].Weights.push_back(long double(0.07));
	   neuralNetwork2.Layers[2].Weights.push_back(long double(0.08));
	   //neuralNetwork2.Layers[2].Weights.push_back(long double(0.09));
	   neuralNetwork2.Layers[2].Weights.push_back(long double(0.29));

	   //neuralNetwork2.Layers[2].Weights.push_back(long double(0.11));
	   //neuralNetwork2.Layers[2].Weights.push_back(long double(0.12));
	   //neuralNetwork2.Layers[2].Weights.push_back(long double(0.13));
	   //neuralNetwork2.Layers[2].Weights.push_back(long double(0.14));
	   //neuralNetwork2.Layers[2].Weights.push_back(long double(0.15));

	   //neuralNetwork2.Layers[2].Weights.push_back(long double(0.16));
	   neuralNetwork2.Layers[2].Weights.push_back(long double(0.17));
	   neuralNetwork2.Layers[2].Weights.push_back(long double(0.18));
	   //neuralNetwork2.Layers[2].Weights.push_back(long double(0.19));
	   neuralNetwork2.Layers[2].Weights.push_back(long double(0.20));
	   neuralNetwork2.Layers[2].Gradients.resize(6);

	   neuralNetwork2.Layers[3].Weights.push_back(long double(0.3));
	   neuralNetwork2.Layers[3].Weights.push_back(long double(0.5));

	   neuralNetwork2.Layers[3].Weights.push_back(long double(0.7));
	   neuralNetwork2.Layers[3].Weights.push_back(long double(0.9));
	   neuralNetwork2.Layers[3].Gradients.resize(4);

	   neuralNetwork2.BalanceType = NeuralEnums::BalanceType::GaussianStandartization;
	   neuralNetwork2.LossFunctionType = NeuralEnums::LossFunctionType::MeanSquaredLoss;

	   //neuralNetwork.InitializeWeights();

	   neuralNetwork2.Layers[0].Inputs = ar;



	   for (unsigned int i = 0; i < 100; i++)
	   {

		   auto loss = neuralNetwork.PropageteForwardThreaded(targetArray, true); // პირველი loss უნდა იყოს 2.256
		   neuralNetwork.PropagateBackThreaded(targetArray);

		   auto loss2 = neuralNetwork2.PropageteForward(targetArray, false); // პირველი loss უნდა იყოს 2.256
		   neuralNetwork2.PropagateBack(targetArray);
		   //auto loss = neuralNetwork.PropageteForwardThreaded(targetArray); // პირველი loss უნდა იყოს 2.256
		   //neuralNetwork.PropagateBackThreaded(targetArray);

		   losses.push_back(loss);
	   }*/
}

void NeuNet::updateContentsLabel(const QString& sep)
{
	contentsLabel->setText(tr("Type path from model above with items at each level separated by a '%1'").arg(sep));
}