#include "atcommand.h"
#include "navdata.h"

#include <QHostAddress>
#include <QUdpSocket>
#include <QByteArray>
#include <QTimer>

/*!
 Constructs the \c AtCommand.
 The \a parent parameter is is sent to the QThread constructor.
 */
AtCommand::AtCommand(QObject *parent) :
		QThread(parent)
{
	qDebug() << "[AtCommand] Constructor";
	mAbort = false;
	mARDroneState = 0;
	mPhi = 0;
	mTheta = 0;
	mGaz = 0;
	mYaw = 0;
	mAccel = false;
	mUserInput = (uint) ARDRONE_NO_TRIM;
	mCameraChange = false;

	//mAccelSensor = new QAccelerometer(this);
	// Whenever accelerometer reading has changed the function accelFilter() will be called.
	//connect(mAccelSensor, SIGNAL(readingChanged()), this, SLOT(accelFilter()));

#ifdef PRINT_INPUT_PER_SECOND
	mIpsMovCounter = 0;
	mIpsDataCounter = 0;
	mIpsSendCounter = 0;
	mIpsTimer = new QTimer(this);
	connect(mIpsTimer, SIGNAL(timeout()), this, SLOT(handleIPSTimer()));
	mIpsTimer->start(1000);
#endif
}

/*!
 Destructor.
 Waits for thread to abort before destroying the \c AtCommand object.
 */
AtCommand::~AtCommand()
{
	qDebug() << "[AtCommand] Destructor";
	mMutex.lock();
	mAbort = true;
	mCondition.wakeOne();
	mMutex.unlock();
	wait();
}

/*!
 Starts the thread with \c HighPriority unless it is already running.
 */
void AtCommand::startThread()
{
	qDebug() << "[AtCommand] Starting thread";
	QMutexLocker locker(&mMutex);
	qDebug() << "[AtCommand] Created QMutexLocker";

	if (!isRunning())
	{
		start(HighPriority);
	}
	else
	{
		mCondition.wakeOne();
	}
}

/*!
 Thread main function.
 First it sends initialization messages and then it sends user commands to the drone.
 */
void AtCommand::run()
{
	qDebug() << "[AtCommand] run";

	// Sequence number used for commands send to drone. The drone will ignore messages with sequence
	// numbers lower than a previously received message, so we need to keep iterating this for each
	// message. Sequence must start at 1 but initializations messages use the first 5.
	uint nbSequence = 5;

	// There a 2 different options for the camera, horizontal and vertical
	bool useHorizontalCamera = true;

	// Hostaddress of the drone.
	QHostAddress hostAddress = QHostAddress(DRONE_IP_ADDRESS);

	// UDP socket used for sending commands to the drone.
	QUdpSocket *udpSocketAT = new QUdpSocket();

	// This loop is responsible for sending commands to the drone throughout the life of the app.
	forever
	{
		// Do the stuff but check if we need to abort first...
		if (mAbort)
		{
			return;
		}

		// Copy values to variables that are local to the sending thread.
		mMutex.lock();
		uint tARDroneState = this->mARDroneState;
		int tPhi = this->mPhi;
		int tTheta = this->mTheta;
		int tGaz = this->mGaz;
		int tYaw = this->mYaw;
		uint tUserInput = this->mUserInput;
		bool tCameraChange = this->mCameraChange;
		mMutex.unlock();

		// Only send commands to the drone if we have received some data from it, meaning that it is
		// up and running.
		//if (tARDroneState != 0) {
		QByteArray datagram;

		if (get_mask_from_state(tARDroneState, ARDRONE_COM_WATCHDOG_MASK) && !get_mask_from_state(tARDroneState, ARDRONE_NAVDATA_BOOTSTRAP))
		{
			// Reset communication watchdog.
			qDebug() << "[AtCommand] Reset communication watchdog";
			datagram = "AT*COMWDG=" + NUM_STR(++nbSequence)+"\r";
		}
		else
		{
			datagram = "";

			if (tCameraChange)
			{
				//Switch the camera
					useHorizontalCamera = !useHorizontalCamera;

					datagram += "AT*CONFIG=" + NUM_STR(++nbSequence)
					+ ",\"video:video_channel\",\"" + NUM_STR((useHorizontalCamera) ? 0 : 1)
					+ "\"\r";
					qDebug() << "[AtCommand] Changing video. Datagram: " << datagram;
					mMutex.lock();
					mCameraChange = false;
					mMutex.unlock();
				}

				if (tPhi != 0 || tTheta != 0 || tGaz != 0 || tYaw != 0)
				{
					// The parameters of this command will make the drone move.
					datagram += "AT*PCMD=" + NUM_STR(++nbSequence)+",1," + NUM_STR(tPhi) + ","
					+ NUM_STR(tTheta) + "," + NUM_STR(tGaz) + "," + NUM_STR(tYaw)+"\r";
					qDebug() << "[AtCommand] Sending move datagram: " << datagram;
				}
				else
				{
					// This command will make the drone hover over the same spot.
					datagram += "AT*PCMD=" + NUM_STR(++nbSequence)+",0,0,0,0,0\r";
				}
				// The user input send here is Take-off/Land/Emergency/Reset commands.
				datagram += "AT*REF=" + NUM_STR(++nbSequence) + "," + NUM_STR(tUserInput) + "\r";
#ifdef PRINT_INPUT_PER_SECOND
					mMutex.lock();
					mIpsSendCounter++;
					mMutex.unlock();
#endif
					if (get_mask_from_state(tARDroneState, ARDRONE_EMERGENCY_MASK))
					{
						// If the drone is in the emergency state we clear the Emergency and Take-off
						// bits in the user input, so we are ready for Reset and Take-off again.
						mMutex.lock();
						mUserInput &= ~(1 << ARDRONE_UI_BIT_SELECT);
						mUserInput &= ~(1 << ARDRONE_UI_BIT_START);
						mMutex.unlock();
					}
				}

			// Send the datagram, with the commands assembled above, to the drone.
		int res = udpSocketAT->writeDatagram(datagram.data(), datagram.size(), hostAddress, AT_PORT);
		if (res == -1)
		{
			qDebug() << "[AtCommand] Error sending AT data!!";
		}
		//} else {
		//    qDebug()<<"[AtCommand] No NavData recieved yet";
		//}
		// Wait 33 milliseconds before sending the next command.
		mCondition.wait(&mMutex, 33);
	}
}

/*!
 Slot for receiving accelerometer values.
 */
void AtCommand::accelFilter()
{
	// Only read new accelerometer values if the sensor is still active and the user has activated
	// the joystick.
	/*    if (mAccelSensor->isActive() && mAccel) {
	 QAccelerometerReading *reading = mAccelSensor->reading();

	 #ifdef PRINT_INPUT_PER_SECOND
	 mIpsMovCounter++;
	 #endif
	 float_or_int_t _phi, _theta;

	 // Read the accelerometer values and divide by 20 to get some nice values.
	 _phi.f = reading->y()/20.0;
	 _theta.f = reading->x()/20.0;

	 // Save the floating point values in integer variables, so they are ready to be sent to
	 // the drone.
	 mMutex.lock();
	 mPhi= _phi.i;
	 mTheta= _theta.i;
	 mMutex.unlock();
	 }*/
}

/*!
 Slot for receiving user input regarding rotation and altitude change, from \c InputArea.

 The parameter \a gaz is altitude change and \a yaw is rotation speed.
 */
void AtCommand::setThrottleAndYaw(qreal gaz, qreal yaw)
{
	// Only accept move commands if the joystick is active.
	//if (mAccel)
	//{
#ifdef PRINT_INPUT_PER_SECOND
	mIpsDataCounter++;
#endif
	float_or_int_t _gaz;
	float_or_int_t _yaw;

	_gaz.f = gaz;
	_yaw.f = yaw;

	// The received floating point values are saved in integer variables, so they are ready to
	// be sent to the drone.
	mMutex.lock();
	mGaz = _gaz.i;
	mYaw = _yaw.i;

	qDebug() << "[AtCommand] Gaz (throttle) set to: " << mGaz << " yaw set to: " << mYaw;
	mMutex.unlock();
	//}
}

/*!
 Slot for receiving user input regarding pitch and roll., from \c InputArea.

 Range is -1 to 1 for both parameters

 */
void AtCommand::setPitchAndRoll(qreal pitch, qreal roll)
{

#ifdef PRINT_INPUT_PER_SECOND
	mIpsDataCounter++;
#endif
	float_or_int_t _pitch;
	float_or_int_t _roll;

	_pitch.f = pitch;
	_roll.f = roll;

	// The received floating point values are saved in integer variables, so they are ready to
	// be sent to the drone.
	mMutex.lock();
	mTheta = _pitch.i;
	mPhi = _roll.i;

	qDebug() << "[AtCommand] Pitch set to: " << _pitch.f << " roll set to: " << _roll.f;
	mMutex.unlock();

}

/*!
 Slot for receiving AR.Drone status change, from \c NavDataHandler.

 The parameter \a state is the AR.Drone status.
 */
void AtCommand::updateARDroneState(uint state)
{
	qDebug() << "[AtCommand] New state = " << state;
	mMutex.lock();
	mARDroneState = state;
	mMutex.unlock();
	// Wake thread, since we want to react to the new drone state right away.
	if (isRunning())
	{
		mCondition.wakeOne();
	}
	else
	{
		// This is the first drone state we recieve. Start the thread.
		startThread();
	}
}

/*!
 Slot for receiving camera change commands, from \c InputArea.
 */
void AtCommand::changeCamera()
{
	mMutex.lock();
	mCameraChange = true;
	mMutex.unlock();
	// Wake thread, since we want to change camera right away.
	if (isRunning())
	{
		mCondition.wakeOne();
	}
}

/*!
 Slot for receiving indication that the emergency button was pressed in \c InputArea.
 */
void AtCommand::emergencyPressed()
{
	mMutex.lock();
	mUserInput |= (1 << ARDRONE_UI_BIT_SELECT);
	mMutex.unlock();
	// Wake thread, since we want to react to the user pressing Emergency right away.
	if (isRunning())
	{
		mCondition.wakeOne();
	}
}

/*!
 Slot for receiving indication that the reset button was pressed in \c InputArea.
 */
void AtCommand::resetPressed()
{
	mMutex.lock();
	mUserInput |= (1 << ARDRONE_UI_BIT_SELECT);
	mMutex.unlock();
	// Wake thread, since we want to react to the user pressing Reset right away.
	if (isRunning())
	{
		mCondition.wakeOne();
	}
}

/*!
 Slot for receiving indication that the take off button was pressed in \c InputArea.
 */
void AtCommand::startPressed()
{
	qDebug() << "[AtCommand] startPressed";
	mMutex.lock();
	mUserInput |= (1 << ARDRONE_UI_BIT_START);
	mMutex.unlock();
}

/*!
 Slot for receiving indication that the land button was pressed in \c InputArea.
 */
void AtCommand::stopPressed()
{
	mMutex.lock();
	mUserInput &= ~(1 << ARDRONE_UI_BIT_START);
	mMutex.unlock();
}

/*!
 Slot for receiving indication that the user has activated the joystick in \c InputArea.
 */
void AtCommand::accelPressed()
{
	// Set mAccel to true, since user has activated the joystick.
	//mAccel = true;
	// Start the accelerometer.
	//mAccelSensor->start();
}

/*!
 Slot for receiving indication that the user has released the joystick in \c InputArea.
 */
void AtCommand::accelReleased()
{
	// The user no longer wants to send control commands to fly the drone, so we stop the
	// accelerometer and reset all movement variables.
	mAccel = false;
	//mAccelSensor->stop();
	mMutex.lock();
	mPhi = 0;
	mTheta = 0;
	mGaz = 0;
	mYaw = 0;
	mMutex.unlock();
	// Wake thread, so we can command the drone to stop moving right away.
	if (isRunning())
	{
		mCondition.wakeOne();
	}
}

void AtCommand::sendInitSequence(int step)
{
	qDebug() << "[AtCommand] sendInitSequence(" << step << ")";
	// Hostaddress of the drone.
	QHostAddress hostAddress = QHostAddress(DRONE_IP_ADDRESS);

	// UDP socket used for sending commands to the drone.
	QUdpSocket *udpSocketInit = new QUdpSocket();

	// A series of commands are send to the drone in order to make it start up in the correct state.
	QByteArray datagramInit;

	switch (step)
	{
	case 1:
	{
		// We would like to receive a limited amount of feedback data from the drone. We do not need
		// debug data.
		datagramInit = "AT*CONFIG=1,\"general:navdata_demo\",\"TRUE\"\r";
		int res = udpSocketInit->writeDatagram(datagramInit.data(), datagramInit.size(), hostAddress, AT_PORT);
		if (res == -1)
		{
			qDebug() << "[AtCommand] Error sending navdata_demo config message!";
		}

		break;
	}
	case 2:
	{
		datagramInit = "AT*CTRL=2,5,0\r";
		int res = udpSocketInit->writeDatagram(datagramInit.data(), datagramInit.size(), hostAddress, AT_PORT);
		if (res == -1)
		{
			qDebug() << "[AtCommand] Error sending AT data!!";
		}
		else
		{
			qDebug() << "[AtCommand] ACK_CONTROL_MODE sent succesfully";
		}

		// This will make the drone set it's offset values. It is important that the drone is standing
		// on a plain surface during startup!
		datagramInit = "AT*FTRIM=3\r";
		res = udpSocketInit->writeDatagram(datagramInit.data(), datagramInit.size(), hostAddress, AT_PORT);
		if (res == -1)
		{
			qDebug() << "[AtCommand] Error sending AT data!!";
		}

		// Disable bitrate control mode for the video feed, which is introduced in firmware 1.5.1.
		datagramInit.clear();
		datagramInit = "AT*CONFIG=4,\"video:bitrate_control_mode\",\"0\"\r";
		res = udpSocketInit->writeDatagram(datagramInit.data(), datagramInit.size(), hostAddress, AT_PORT);
		if (res == -1)
		{
			qDebug() << "[AtCommand] Error sending AT data!!";
		}

		// Select the frontcamera as the initial camera to receive video from.
		datagramInit.clear();
		datagramInit = "AT*CONFIG=5,\"video:video_channel\",\"0\"\r";
		res = udpSocketInit->writeDatagram(datagramInit.data(), datagramInit.size(), hostAddress, AT_PORT);
		if (res == -1)
		{
			qDebug() << "[AtCommand] Error sending AT data!!";
		}

		break;
	}
	default:
		break;
	}
}

#ifdef PRINT_INPUT_PER_SECOND
/*!
 Prints debug messages on the amount of commands received and send.
 */
void AtCommand::handleIPSTimer()
{
	qDebug() << "[AtCommand] Movement/second: " << mIpsMovCounter;
	qDebug() << "[AtCommand] Data/second:     " << mIpsDataCounter;
	mMutex.lock();
	qDebug() << "[AtCommand] Send/second:     " << mIpsSendCounter;
	mIpsSendCounter = 0;
	mMutex.unlock();
	mIpsMovCounter = 0;
	mIpsDataCounter = 0;
}
#endif
