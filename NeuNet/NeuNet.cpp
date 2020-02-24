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
#include <mmintrin.h>
#include <immintrin.h>
#include "AvxMath.h"
#include "StatisticFunctions.h"
//#include <boost/multiprecision/float128.hpp> 


//https://devblogs.microsoft.com/cppblog/new-code-optimizer/
NeuNet::NeuNet(QWidget* parent)
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

	completer->setSeparator(QLatin1String(":"));
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
	//setCentralWidget(completingTextEdit);
	//ui.tab_2->layout()->addWidget(completingTextEdit);

}
// The function we want to execute on the new thread.
void task1(string msg)
{
	std::string t = msg;
}

QAbstractItemModel* NeuNet::modelFromFile(const QString& fileName)
{
	QFile file(fileName);
	if (!file.open(QFile::ReadOnly))
		return new QStringListModel(completer);

#ifndef QT_NO_CURSOR
	QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));
#endif
	QStringList words;

	QStandardItemModel* model = new QStandardItemModel(completer);
	QVector<QStandardItem*> parents(10);
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

		QStandardItem* item = new QStandardItem;
		item->setText(trimmedLine);
		parents[level]->appendRow(item);
		parents[level + 1] = item;
	}

#ifndef QT_NO_CURSOR
	QApplication::restoreOverrideCursor();
#endif

	return model;
}

template<typename T> void NeuNet::initNeUnetFromJson(NeuralNetwork<T>& neuralNetwork)
{
	auto text = ui.textEdit->toPlainText();
	for (size_t i = 0; i < text.size(); i++)
	{
		auto current = text.at(i);
		if (current >= 65 && current <= 90 || current >= 97 && current <= 122)
		{
			auto prev = text.at(i - 1);
			if (prev < 65 || prev  > 122)
			{
				text.insert(i, "\"");
				i++;
				continue;
			}

		}
		else if (i != 0)
		{
			auto prev = text.at(i - 1);
			if (prev >= 65 && prev <= 90 || prev >= 97 && prev <= 122)
			{
				text.insert(i, "\"");
				i++;
				continue;
			}
		}
	}
	QJsonDocument jsonResponse = QJsonDocument::fromJson(text.toUtf8());

	QJsonObject jsonObject = jsonResponse.object();
	QVariantMap jsonMap = jsonObject.toVariantMap();

	neuralNetwork.ThreadCount = jsonMap["ThreadCount"].toInt();
	neuralNetwork.LearningRate = jsonMap["LearningRate"].toDouble();

	QString lrType = jsonMap["LearningRateType"].toString();
	QString balance = jsonMap["Balance"].toString();
	QString lossFunction = jsonMap["LossFunction"].toString();
	QString gradient = jsonMap["Gradient"].toString();
	QString metrics = jsonMap["Metrics"].toString();
	QString losscalc = jsonMap["LossCalculation"].toString();
	QString autoEncoderType = jsonMap["AutoEncoderType"].toString();
	QString logLoss = jsonMap["LogLoss"].toString();
	neuralNetwork.BatchSize = jsonMap["BatchSize"].toInt();

	QString networkType = jsonMap["Type"].toString();

	if (networkType == "Normal")
		neuralNetwork.Type = NeuralEnums::NetworkType::Normal;
	if (networkType == "AutoEncoder")
		neuralNetwork.Type = NeuralEnums::NetworkType::AutoEncoder;

	if (metrics == "None")
		neuralNetwork.Metrics = NeuralEnums::Metrics::None;
	if (metrics == "Test")
		neuralNetwork.Metrics = NeuralEnums::Metrics::TestSet;
	if (metrics == "Full")
		neuralNetwork.Metrics = NeuralEnums::Metrics::Full;

	if (logLoss == "None")
		neuralNetwork.LogLoss = NeuralEnums::LogLoss::None;
	if (logLoss == "Sparce")
		neuralNetwork.LogLoss = NeuralEnums::LogLoss::Sparce;
	if (logLoss == "Full")
		neuralNetwork.LogLoss = NeuralEnums::LogLoss::Full;

	if (autoEncoderType == "Contractive")
		neuralNetwork.AutoEncoderType = NeuralEnums::AutoEncoderType::Contractive;
	if (autoEncoderType == "Denoising")
		neuralNetwork.AutoEncoderType = NeuralEnums::AutoEncoderType::Denoising;
	if (autoEncoderType == "Sparce")
		neuralNetwork.AutoEncoderType = NeuralEnums::AutoEncoderType::Sparce;
	if (autoEncoderType == "UnderComplete")
		neuralNetwork.AutoEncoderType = NeuralEnums::AutoEncoderType::UnderComplete;

	if (losscalc == "None")
		neuralNetwork.LossCalculation = NeuralEnums::LossCalculation::None;
	if (losscalc == "Full")
		neuralNetwork.LossCalculation = NeuralEnums::LossCalculation::Full;

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

	if (lossFunction == "BinaryCrossentropy")
		neuralNetwork.LossFunctionType = NeuralEnums::LossFunctionType::BinaryCrossentropy;
	if (lossFunction == "MeanSquaredError")
		neuralNetwork.LossFunctionType = NeuralEnums::LossFunctionType::MeanSquaredError;
	if (lossFunction == "KullbackLeiblerDivergence")
		neuralNetwork.LossFunctionType = NeuralEnums::LossFunctionType::KullbackLeiblerDivergence;


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
		neuralNetwork.Layers.push_back(*(new Layer<T>(size, layerType, ActivationFunctionType, bias, DropuOutSize, neuralNetwork.BatchSize)));
	}
	neuralNetwork.NeuralNetworkInit();
	neuralNetwork.InitializeWeights();
}

template<typename T> void ReadData(std::vector<MnistData<T>>& trainingSet, std::vector<MnistData<T>>& testSet, bool hilbertCurve = false)
{
	std::vector<int> labels;
	std::vector<int> testLabels;
	std::vector<std::vector<T>> _trainingSet;
	std::vector<std::vector<T>> _testSet;
	std::vector<std::vector<T>> labeledTarget;
	std::vector<std::vector<T>> testLabeledTarget;

	ReadMNISTMod<T>(_trainingSet, labels, true, hilbertCurve);
	ReadMNISTMod<T>(_testSet, testLabels, false, hilbertCurve);

	trainingSet.resize(_trainingSet.size());
	testSet.resize(_testSet.size());

	int total = _trainingSet.size();
	labeledTarget.resize(_trainingSet.size());
	for (int i = 0; i < total; i++)
	{
		trainingSet[i].minMax.resize(2);
		Compress(_trainingSet[i], trainingSet[i].minMax);
		labeledTarget[i].resize(10);
		for (int k = 0; k < 10; k++)
			labeledTarget[i][k] = labels[i] == k ? 1.0L : 0.0L;
	}

	int totalTest = _testSet.size();
	testLabeledTarget.resize(totalTest);
	for (int k = 0; k < totalTest; k++)
	{
		testSet[k].minMax.resize(2);
		Compress(_testSet[k], testSet[k].minMax);
		testLabeledTarget[k].resize(10);
		for (int g = 0; g < 10; g++)
			testLabeledTarget[k][g] = testLabels[k] == g ? 1.0L : 0.0L;
	}

	for (int i = 0; i < _trainingSet.size(); i++)
	{
		trainingSet[i].set = _trainingSet[i];
		trainingSet[i].labels = labeledTarget[i];
	}


	for (int i = 0; i < _testSet.size(); i++)
	{
		testSet[i].set = _testSet[i];
		testSet[i].labels = testLabeledTarget[i];
	}
}

void NeuNet::on_pushButton_clicked()
{

	NeuralQtThread* nthread = new NeuralQtThread(this);
	nthread->setLambda([this]
		{
			boost::multiprecision::cpp_bin_float_100 b = 2;
			NeuralNetwork<float> neuralNetwork;

			std::vector<MnistData<float>> trainingSet;

			std::vector<MnistData<float>> testSet;

			std::vector<float> losses;

			LossVector lossVector;

			__m256d c = _mm256_set_pd(1L, 1L, 1L, 1L);

			__m256d a = _mm256_sqrt_pd(c);

			initNeUnetFromJson(neuralNetwork);


			ui.textEdit->append(QString::fromStdString("start reading training+test data "));
			clock_t begin = clock();
			ReadData(trainingSet, testSet, false);
			clock_t end = clock();
			ui.textEdit->append(QString::fromStdString("...done in " + std::to_string(float(end - begin) / CLOCKS_PER_SEC) + " seconds"));
			int total = trainingSet.size();

			ui.textEdit->append(QString::fromStdString("start training"));
			begin = clock();
			int globalEpochs = 300;
			int totalcounter = 0;
			int TotalEpochs = trainingSet.size() * globalEpochs, range;
			float loss = 0;

			for (size_t g = 0; g < globalEpochs; g++)
			{
				try {
					unsigned seed = std::chrono::system_clock::now().time_since_epoch().count();
					shuffle(trainingSet.begin(), trainingSet.end(), std::default_random_engine(seed));
					clock_t beginInside = clock();
					if (neuralNetwork.Type == NeuralEnums::NetworkType::Normal)
					{
						for (int i = 0; i < total / neuralNetwork.BatchSize; i++)
						{
							int biasShift = neuralNetwork.Layers[0].UsingBias ? 1 : 0;
							if (neuralNetwork.BatchSize == 1)
							{
								neuralNetwork.Layers[0].Outputs = trainingSet[i].set;
								if (neuralNetwork.Type == NeuralEnums::NetworkType::Normal)
									neuralNetwork.Layers[neuralNetwork.Layers.size() - 1].Target = trainingSet[i].labels;
								else
									neuralNetwork.Layers[neuralNetwork.Layers.size() - 1].Target = trainingSet[i].set;
							}
							else
							{
								for (int batch = 0; batch < neuralNetwork.BatchSize; batch++)
								{
									neuralNetwork.Layers[0].OutputsBatch[batch] = trainingSet[i * neuralNetwork.BatchSize + batch].set;
									if (neuralNetwork.Type == NeuralEnums::NetworkType::Normal)
										neuralNetwork.Layers[neuralNetwork.Layers.size() - 1].TargetsBatch[batch] = trainingSet[i * neuralNetwork.BatchSize + batch].labels;
									else
										neuralNetwork.Layers[neuralNetwork.Layers.size() - 1].TargetsBatch[batch] = trainingSet[i * neuralNetwork.BatchSize + batch].set;
								}
							}

							//main learning sequence
							loss = neuralNetwork.PropagateForwardThreaded(true, false);
							neuralNetwork.PropagateBackThreaded();
							losses.push_back(loss);

							if (neuralNetwork.BatchSize == 1)
							{
								if (i % 1000 == 0 && i != 0)
								{
									totalcounter += 1000;
									ui.pushButton->setText(QString::fromStdString(std::to_string(totalcounter) + "/" + std::to_string(total * globalEpochs)));
									//ui.textEdit->append(QString::fromStdString("loss: " + std::to_string(loss) + " ;"));
								}
							}
							else
							{
								totalcounter++;
								if (i % 100 == 0 && i != 0)
									ui.pushButton->setText(QString::fromStdString(std::to_string(totalcounter) + "/" + std::to_string(total * globalEpochs / neuralNetwork.BatchSize)));
							}

						}
					}
					if (neuralNetwork.Type == NeuralEnums::NetworkType::AutoEncoder)
					{
						for (int i = 0; i < total / neuralNetwork.BatchSize; i++)
						{
							int biasShift = neuralNetwork.Layers[0].UsingBias ? 1 : 0;
							if (neuralNetwork.BatchSize == 1)
							{
								neuralNetwork.Layers[0].Outputs = trainingSet[i].set;
								if (neuralNetwork.Type == NeuralEnums::NetworkType::Normal)
									neuralNetwork.Layers[neuralNetwork.Layers.size() - 1].Target = trainingSet[i].labels;
								else
									neuralNetwork.Layers[neuralNetwork.Layers.size() - 1].Target = trainingSet[i].set;
							}
							else
							{
								for (int batch = 0; batch < neuralNetwork.BatchSize; batch++)
								{
									neuralNetwork.Layers[0].OutputsBatch[batch] = trainingSet[i * neuralNetwork.BatchSize + batch].set;
									if (neuralNetwork.Type == NeuralEnums::NetworkType::Normal)
										neuralNetwork.Layers[neuralNetwork.Layers.size() - 1].TargetsBatch[batch] = trainingSet[i * neuralNetwork.BatchSize + batch].labels;
									else
										neuralNetwork.Layers[neuralNetwork.Layers.size() - 1].TargetsBatch[batch] = trainingSet[i * neuralNetwork.BatchSize + batch].set;
								}
							}

							//main learning sequence
							neuralNetwork.PropagateForwardThreaded(true, true);
							//losses.push_back(loss);
							if (neuralNetwork.BatchSize == 1)
							{
								if (i % 1000 == 0 && i != 0)
								{
									totalcounter += 1000;
									ui.pushButton->setText(QString::fromStdString(std::to_string(totalcounter) + "/" + std::to_string(total * globalEpochs)));
									//ui.textEdit->append(QString::fromStdString("loss: " + std::to_string(loss) + " ;"));
								}
							}
							else
							{
								totalcounter++;
								if (i % 100 == 0 && i != 0)
									ui.pushButton->setText(QString::fromStdString(std::to_string(totalcounter) + "/" + std::to_string(total * globalEpochs / neuralNetwork.BatchSize)));
							}
						}
						// rohat average
						for (size_t l = 1; l < neuralNetwork.Layers.size(); l++)
						{
							for (size_t f = 0; f < neuralNetwork.Layers[l].RoHat.size(); f++)
							{
								neuralNetwork.Layers[l].RoHat[f] /= total;
							}
						}
					}
					clock_t endInside = clock();

					int counter = 0;
					int digitCounter = 0;
					ui.textEdit->append(QString::fromStdString(std::to_string(g + 1) + " of " + std::to_string(globalEpochs) + " done in " + std::to_string(float(endInside - beginInside) / CLOCKS_PER_SEC) + " seconds. "
						+ ". loss: " + std::to_string(losses.size() > 0 ? losses[losses.size() - 1] : 0)));

					if (neuralNetwork.BatchSize > 1)
						neuralNetwork.StartTesting();
					long double result = 0;
					if (neuralNetwork.Type == NeuralEnums::NetworkType::Normal)
					{
						if (neuralNetwork.Metrics == NeuralEnums::Metrics::Full)
						{
							beginInside = clock();
							for (int i = 0; i < trainingSet.size(); i++)
							{
								neuralNetwork.Layers[0].Outputs = trainingSet[i].set;
								neuralNetwork.Layers[neuralNetwork.Layers.size() - 1].Target = trainingSet[i].labels;
								auto loss = neuralNetwork.PropagateForwardThreaded(false, false);
								if (GetMaxIndex(neuralNetwork.Layers[neuralNetwork.Layers.size() - 1].Outputs) == GetMaxIndex(trainingSet[i].labels))
									counter++;
								digitCounter++;
							}
							result = (long double)counter / (long double)digitCounter;
							auto testComplete = "training-set result: " + std::to_string(result);
							endInside = clock();
							ui.textEdit->append(QString::fromStdString("...training set testing done in " + std::to_string(float(endInside - beginInside) / CLOCKS_PER_SEC) + " seconds. Result: " + std::to_string(result)));
						}
						if (neuralNetwork.Metrics == NeuralEnums::Metrics::TestSet || neuralNetwork.Metrics == NeuralEnums::Metrics::Full)
						{
							beginInside = clock();
							counter = 0;
							digitCounter = 0;
							for (int i = 0; i < testSet.size(); i++)
							{
								neuralNetwork.Layers[0].Outputs = testSet[i].set;
								neuralNetwork.Layers[neuralNetwork.Layers.size() - 1].Target = testSet[i].labels;
								auto loss = neuralNetwork.PropagateForwardThreaded(false, false);
								if (GetMaxIndex(neuralNetwork.Layers[neuralNetwork.Layers.size() - 1].Outputs) == GetMaxIndex(testSet[i].labels))
									counter++;
								digitCounter++;
							}
							result = (long double)counter / (long double)digitCounter;
							auto testComplete2 = "; test-set result: " + std::to_string(result);
							endInside = clock();
							bool checksum = true;
							int val = 0;
							ui.textEdit->append(QString::fromStdString("...testing set testing done in " + std::to_string(float(endInside - beginInside) / CLOCKS_PER_SEC) + " seconds. Result: " + std::to_string(result) + ". loss: " + std::to_string(losses[losses.size() - 2])));

						}
					}
					if (neuralNetwork.LogLoss == NeuralEnums::LogLoss::Full || neuralNetwork.LogLoss == NeuralEnums::LogLoss::Sparce)
					{
						ofstream oData;
						oData.open("loss.txt");
						for (int count = 0; count < losses.size(); count++) {
							if (neuralNetwork.LogLoss == NeuralEnums::LogLoss::Sparce && count % 10 == 0)
								oData << std::setprecision(100) << losses[count] << endl;
							else
								oData << std::setprecision(100) << losses[count] << endl;
						}
					}
					//if (neuralNetwork.Type == NeuralEnums::NetworkType::AutoEncoder)
					//{
					//	std::vector<float> tmp;
					//	tmp.resize(trainingSet[0].set.size());
					//	for (size_t cnt = 0; cnt < trainingSet[0].set.size(); cnt++)
					//	{
					//		tmp[cnt] = trainingSet[0].set[cnt];
					//	}
					//	DeCompress(tmp, trainingSet[0].minMax);
					//	QByteArray arr;
					//	for (size_t cnt = 0; cnt < tmp.size(); cnt++)
					//	{
					//		arr.push_back(tmp[cnt]);
					//	}
					//	QImage image((const uchar*)arr.data(), 28, 28, QImage::Format_Indexed8);
					//	QGraphicsScene* scene = new QGraphicsScene();
					//	QGraphicsView* view = new QGraphicsView(scene);
					//	QGraphicsPixmapItem* item = new QGraphicsPixmapItem(QPixmap::fromImage(image));
					//	scene->addItem(item);

					//	ui.graphicsView->setScene(scene);
					//	ui.graphicsView->fitInView(scene->sceneRect(), Qt::KeepAspectRatio);
					//	ui.graphicsView->show();

					//	neuralNetwork.Layers[0].Outputs = trainingSet[0].set;
					//	neuralNetwork.Layers[neuralNetwork.Layers.size() - 1].Target = trainingSet[0].set;
					//	auto loss = neuralNetwork.PropageteForwardThreaded(false, false);

					//	std::vector<float> tmp2;
					//	tmp2.resize(trainingSet[0].set.size());
					//	for (size_t cnt = 0; cnt < trainingSet[0].set.size(); cnt++)
					//	{
					//		tmp2[cnt] = neuralNetwork.Layers[neuralNetwork.Layers.size() - 1].Outputs[cnt];
					//	}
					//	DeCompress(tmp2, trainingSet[0].minMax);
					//	QByteArray arr2;
					//	for (size_t cnt = 0; cnt < tmp2.size(); cnt++)
					//	{
					//		arr2.push_back(tmp2[cnt]);
					//	}
					//	QImage image2((const uchar*)arr2.data(), 28, 28, QImage::Format_Indexed8);
					//	QGraphicsScene* scene2 = new QGraphicsScene();
					//	QGraphicsView* view2 = new QGraphicsView(scene2);
					//	QGraphicsPixmapItem* item2 = new QGraphicsPixmapItem(QPixmap::fromImage(image2));
					//	scene2->addItem(item2);
					//	ui.graphicsView_2->setScene(scene2);
					//	ui.graphicsView_2->fitInView(scene2->sceneRect(), Qt::KeepAspectRatio);
					//	ui.graphicsView_2->show();
					//}
					losses.clear();
				}
				catch (exception e)
				{
					QMessageBox::information(this, "Image Viewer", QString::fromStdString(e.what()));
					ui.textEdit->append(QString::fromStdString(e.what()));
				}
			}


			end = clock();
			ui.textEdit->append(QString::fromStdString("training done in " + std::to_string(float(end - begin) / CLOCKS_PER_SEC) + " seconds"));
			NeuralQtThread* nthread = new NeuralQtThread(this);
		});
	nthread->start();


}

void NeuNet::on_pushButton_2_clicked()
{

	//QImage image("1.bmp");
	//
	////image.fromData(arr);
	//auto format = image.format();
	//

	//QByteArray arr = QByteArray::fromRawData((const char*)image.bits(), image.byteCount());
	//QImage image2((const uchar*)arr.data(), 28, 28, QImage::Format_Indexed8);
	//

	//if (image2.isNull())
	//{
	//	QMessageBox::information(this, "Image Viewer", "Error Displaying image");
	//	return;
	//}

	//QGraphicsScene* scene = new QGraphicsScene();
	//QGraphicsView* view = new QGraphicsView(scene);
	//QGraphicsPixmapItem* item = new QGraphicsPixmapItem(QPixmap::fromImage(image2));
	//scene->addItem(item);

	//ui.graphicsView->setScene(scene);
	//ui.graphicsView->show();
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
	neuralNetwork.ThreadCount = 1;
	neuralNetwork.BatchSize = 1;
	neuralNetwork.LogLoss = NeuralEnums::LogLoss::Full;
	neuralNetwork.LearningRateType = NeuralEnums::LearningRateType::Static;
	neuralNetwork.BalanceType = NeuralEnums::BalanceType::GaussianStandartization;
	neuralNetwork.LossFunctionType = NeuralEnums::LossFunctionType::MeanSquaredError;
	neuralNetwork.LossCalculation = NeuralEnums::LossCalculation::Full;
	neuralNetwork.NeuralNetworkInit();
	neuralNetwork.Layers = vecs;
	neuralNetwork.Layers[1].GradientsBatch.resize(1);
	neuralNetwork.Layers[1].GradientsBatch[0].resize(12);
	neuralNetwork.Layers[1].Gradients.resize(12);
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

	neuralNetwork.Layers[2].GradientsBatch.resize(1);
	neuralNetwork.Layers[2].GradientsBatch[0].resize(8);
	neuralNetwork.Layers[2].Gradients.resize(8);
	neuralNetwork.Layers[2].Weights.push_back(long double(1));
	neuralNetwork.Layers[2].Weights.push_back(long double(-0.5));
	neuralNetwork.Layers[2].Weights.push_back(long double(0.5));
	neuralNetwork.Layers[2].Weights.push_back(long double(-0.2));
	neuralNetwork.Layers[2].Weights.push_back(long double(1));
	neuralNetwork.Layers[2].Weights.push_back(long double(0.3));
	neuralNetwork.Layers[2].Weights.push_back(long double(-0.46));
	neuralNetwork.Layers[2].Weights.push_back(long double(0.76));

	neuralNetwork.Layers[3].GradientsBatch.resize(1);
	neuralNetwork.Layers[3].GradientsBatch[0].resize(6);
	neuralNetwork.Layers[3].Gradients.resize(6);
	neuralNetwork.Layers[3].Weights.push_back(long double(1));
	neuralNetwork.Layers[3].Weights.push_back(long double(0.3));
	neuralNetwork.Layers[3].Weights.push_back(long double(0.4));
	neuralNetwork.Layers[3].Weights.push_back(long double(1));
	neuralNetwork.Layers[3].Weights.push_back(long double(0.7));
	neuralNetwork.Layers[3].Weights.push_back(long double(0.92));
	neuralNetwork.Layers[3].Target = targetArray;

	neuralNetwork.Layers[3].Target = targetArray;
	neuralNetwork.Layers[0].Outputs = ar;


	for (unsigned int i = 0; i < 100; i++)
	{

		auto loss = neuralNetwork.PropagateForwardThreaded(true, false); // პირველი loss უნდა იყოს 0.20739494219121993
		neuralNetwork.PropagateBackThreaded();

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