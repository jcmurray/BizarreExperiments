#include "applicationui.hpp"
#include "bluetooth/HrDataContainer.hpp"

#include <bb/cascades/Application>
#include <bb/cascades/QmlDocument>
#include <bb/cascades/LocaleHandler>
#include <bb/cascades/Window>
#include <bb/cascades/DataModel>

#include <qdebug.h>
#include <pthread.h>
#include <errno.h>

using namespace bb::cascades;

ApplicationUI::ApplicationUI(bb::cascades::Application *app) :
		QObject(app) {

	//Stop the screen from timing out
	app->mainWindow()->setScreenIdleMode(ScreenIdleMode::KeepAwake);

	//Read the current scheduling parameters
	sched_param schedParam;
	int policy;
	int getParamsResult = pthread_getschedparam(pthread_self(), &policy,
			&schedParam);

	if (getParamsResult == EOK) {
		qDebug() << getSchedulingPolicyDescription(policy);
	}

	//Change the thread scheduling policy to FIFO
	int setScheduleResult = pthread_setschedparam(pthread_self(), SCHED_FIFO,
			&schedParam);
	if (setScheduleResult == EOK) {
		qDebug() << "Set scheduler to FIFO";
	} else {
		qDebug() << "Error setting scheduling policy to FIFO. Error: "
				<< setScheduleResult;
	}

	//Change the thread priority to the maximum permissible
	int priorityResult = pthread_setschedprio(pthread_self(), 63);

	if (priorityResult == EOK) {
		qDebug() << "Successfully set new thread priority";
	} else {
		qDebug() << "Failed to set new thread priority. Result: "
				<< priorityResult;
	}

	//Set up bluetooth
	m_pBluetoothHandler = new BluetoothHandler(app);

	//Do we have a HR monitor paired?
	DataModel* pHRDevices = m_pBluetoothHandler->deviceListing()->model();

	QVariantList indexPath;
	indexPath << 0;

	if (pHRDevices->hasChildren(indexPath)) {

		qDebug() << "YYYY Found one or more Heart Rate monitors";

		HrDataContainer* hrdc = HrDataContainer::getInstance();

		//get the first bluetooth device
		indexPath << 0;
		QMap<QString, QVariant> hrDevice = pHRDevices->data(indexPath).toMap();

		QString deviceAddress = hrDevice.value("deviceAddress").toString();
		QString deviceName = hrDevice.value("deviceName").toString();

		m_pBluetoothHandler->setRemoteDevice(deviceAddress);

		monitorHeartRate(deviceAddress, deviceName);

		QObject::connect(hrdc, SIGNAL(heartRateData(QVariant)), this,
				SLOT(logHeartRate(QVariant)), Qt::QueuedConnection);

	} else {

		qDebug() << "YYYY NO Heart Rate monitors found";
	}

	//Create the sound manager
	m_pSoundManager = new SoundManager("sounds/");
	m_tigerSourceId = 0;

	//INITIALISE THE UI

	// Create scene document from main.qml asset, the parent is set
	// to ensure the document gets destroyed properly at shut down.
	QmlDocument *qml = QmlDocument::create("asset:///main.qml").parent(this);
	qml->setContextProperty("app", this);

	// Create root object for the UI
	m_pRoot = qml->createRootObject<AbstractPane>();

	// Set created root object as the application scene
	app->setScene(m_pRoot);
}

ApplicationUI::~ApplicationUI() {

	//Clean up the sound manager
	delete (m_pSoundManager);
	m_pSoundManager = 0;
}

void ApplicationUI::monitorHeartRate(QString device_addr, QString device_name) {
	qDebug() << "YYYY monitoring heart rate using device " << device_addr;

	HrDataContainer* hrdc = HrDataContainer::getInstance();
	hrdc->setCurrentDeviceAddr(device_addr);
	hrdc->setCurrentDeviceName(device_name);

	_future = new QFuture<bool>;
	_watcher = new QFutureWatcher<bool>;
	qDebug() << "YYYY locking notification receiver thread mutex";
	_mutex.lock();
	qDebug() << "YYYY notification receiver thread mutex locked OK";
	*_future = QtConcurrent::run(m_pBluetoothHandler,
			&BluetoothHandler::receiveHrNotifications);
	_watcher->setFuture(*_future);
	QObject::connect(_watcher, SIGNAL(finished()), this,
			SLOT(finishedReceiving()));

}

void ApplicationUI::finishedReceiving() {
	qDebug() << "YYYY notification receiver thread has finished running";
	qDebug() << "YYYY notification receiver thread result: "
			<< _future->result();

	_mutex.unlock();
}

void ApplicationUI::logHeartRate(const QVariant &rate) {
	qDebug() << "YYYY logging heart rate now " << rate;
	QMetaObject::invokeMethod(m_pRoot, "logHeartRate", Q_ARG(QVariant, rate));
}

void ApplicationUI::onSystemLanguageChanged() {
	QCoreApplication::instance()->removeTranslator(m_pTranslator);
	// Initiate, load and install the application translation files.
	QString locale_string = QLocale().name();
	QString file_name = QString("Exp1Tiger_%1").arg(locale_string);
	if (m_pTranslator->load(file_name, "app/native/qm")) {
		QCoreApplication::instance()->installTranslator(m_pTranslator);
	}
}

void ApplicationUI::playTiger(float pitch) {

	m_tigerSourceId = m_pSoundManager->play("Tiger.wav", pitch);
}

void ApplicationUI::stopTiger() {

	//stop the song playing
	if (isTigerPlaying()) {
		m_pSoundManager->stop(m_tigerSourceId);
		m_tigerSourceId = 0;
	}
}

bool ApplicationUI::isTigerPlaying() {

	return (m_tigerSourceId != 0);
}

void ApplicationUI::connectToHRMonitor() {

}

void ApplicationUI::changeTigerPitch(float newPitch) {

	m_pSoundManager->changePitch(m_tigerSourceId, newPitch);
}

QString ApplicationUI::getSchedulingPolicyDescription(const int policy) {

	QString policyName;

	switch (policy) {
	case SCHED_FIFO:
		policyName = "FIFO";
		break;
	case SCHED_RR:
		policyName = "RR";
		break;
	default:
		policyName = policy;
		break;
	}

	return policyName;
}

