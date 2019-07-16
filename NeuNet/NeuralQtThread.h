#include <QThread>
#include <QTCore>
class NeuralQtThread : public QThread
{
	Q_OBJECT
public:
	std::function<void()> lambda;
	explicit NeuralQtThread(QObject *parent = 0) {};
	void setLambda(const std::function<void()>& t)
	{
		lambda = t;
	}
	void run() override
	{
		QMutex mutex;
		mutex.lock();
		lambda();
		emit resultReady("finished");
		mutex.unlock();
	};
	void innerWorker(int progress, int total)
	{
		emit workInProgress(progress, total);
	}
	void innerMessages(const QString &s)
	{
		emit resultReady(s);
	}
signals:
	void resultReady(const QString &s);
	void workInProgress(int progress, int total);
};
