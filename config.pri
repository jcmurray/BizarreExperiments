# Auto-generated by IDE. Any changes made by user will be lost!
BASEDIR =  $$quote($$_PRO_FILE_PWD_)

device {
    CONFIG(debug, debug|release) {
        SOURCES +=  $$quote($$BASEDIR/src/applicationui.cpp) \
                 $$quote($$BASEDIR/src/bluetooth/BluetoothHandler.cpp) \
                 $$quote($$BASEDIR/src/bluetooth/DeviceListing.cpp) \
                 $$quote($$BASEDIR/src/bluetooth/HrDataContainer.cpp) \
                 $$quote($$BASEDIR/src/bluetooth/LocalDeviceInfo.cpp) \
                 $$quote($$BASEDIR/src/bluetooth/RemoteDeviceInfo.cpp) \
                 $$quote($$BASEDIR/src/bluetooth/Utilities.cpp) \
                 $$quote($$BASEDIR/src/main.cpp) \
                 $$quote($$BASEDIR/src/soundmanager.cpp)

        HEADERS +=  $$quote($$BASEDIR/src/applicationui.hpp) \
                 $$quote($$BASEDIR/src/bluetooth/BluetoothHandler.hpp) \
                 $$quote($$BASEDIR/src/bluetooth/DeviceListing.hpp) \
                 $$quote($$BASEDIR/src/bluetooth/HrDataContainer.hpp) \
                 $$quote($$BASEDIR/src/bluetooth/HrTypes.hpp) \
                 $$quote($$BASEDIR/src/bluetooth/LocalDeviceInfo.hpp) \
                 $$quote($$BASEDIR/src/bluetooth/RemoteDeviceInfo.hpp) \
                 $$quote($$BASEDIR/src/bluetooth/Utilities.hpp) \
                 $$quote($$BASEDIR/src/soundmanager.h)
    }

    CONFIG(release, debug|release) {
        SOURCES +=  $$quote($$BASEDIR/src/applicationui.cpp) \
                 $$quote($$BASEDIR/src/bluetooth/BluetoothHandler.cpp) \
                 $$quote($$BASEDIR/src/bluetooth/DeviceListing.cpp) \
                 $$quote($$BASEDIR/src/bluetooth/HrDataContainer.cpp) \
                 $$quote($$BASEDIR/src/bluetooth/LocalDeviceInfo.cpp) \
                 $$quote($$BASEDIR/src/bluetooth/RemoteDeviceInfo.cpp) \
                 $$quote($$BASEDIR/src/bluetooth/Utilities.cpp) \
                 $$quote($$BASEDIR/src/main.cpp) \
                 $$quote($$BASEDIR/src/soundmanager.cpp)

        HEADERS +=  $$quote($$BASEDIR/src/applicationui.hpp) \
                 $$quote($$BASEDIR/src/bluetooth/BluetoothHandler.hpp) \
                 $$quote($$BASEDIR/src/bluetooth/DeviceListing.hpp) \
                 $$quote($$BASEDIR/src/bluetooth/HrDataContainer.hpp) \
                 $$quote($$BASEDIR/src/bluetooth/HrTypes.hpp) \
                 $$quote($$BASEDIR/src/bluetooth/LocalDeviceInfo.hpp) \
                 $$quote($$BASEDIR/src/bluetooth/RemoteDeviceInfo.hpp) \
                 $$quote($$BASEDIR/src/bluetooth/Utilities.hpp) \
                 $$quote($$BASEDIR/src/soundmanager.h)
    }
}

simulator {
    CONFIG(debug, debug|release) {
        SOURCES +=  $$quote($$BASEDIR/src/applicationui.cpp) \
                 $$quote($$BASEDIR/src/bluetooth/BluetoothHandler.cpp) \
                 $$quote($$BASEDIR/src/bluetooth/DeviceListing.cpp) \
                 $$quote($$BASEDIR/src/bluetooth/HrDataContainer.cpp) \
                 $$quote($$BASEDIR/src/bluetooth/LocalDeviceInfo.cpp) \
                 $$quote($$BASEDIR/src/bluetooth/RemoteDeviceInfo.cpp) \
                 $$quote($$BASEDIR/src/bluetooth/Utilities.cpp) \
                 $$quote($$BASEDIR/src/main.cpp) \
                 $$quote($$BASEDIR/src/soundmanager.cpp)

        HEADERS +=  $$quote($$BASEDIR/src/applicationui.hpp) \
                 $$quote($$BASEDIR/src/bluetooth/BluetoothHandler.hpp) \
                 $$quote($$BASEDIR/src/bluetooth/DeviceListing.hpp) \
                 $$quote($$BASEDIR/src/bluetooth/HrDataContainer.hpp) \
                 $$quote($$BASEDIR/src/bluetooth/HrTypes.hpp) \
                 $$quote($$BASEDIR/src/bluetooth/LocalDeviceInfo.hpp) \
                 $$quote($$BASEDIR/src/bluetooth/RemoteDeviceInfo.hpp) \
                 $$quote($$BASEDIR/src/bluetooth/Utilities.hpp) \
                 $$quote($$BASEDIR/src/soundmanager.h)
    }
}

INCLUDEPATH +=  $$quote($$BASEDIR/src/bluetooth) \
         $$quote($$BASEDIR/src)

CONFIG += precompile_header

PRECOMPILED_HEADER =  $$quote($$BASEDIR/precompiled.h)

lupdate_inclusion {
    SOURCES +=  $$quote($$BASEDIR/../src/*.c) \
             $$quote($$BASEDIR/../src/*.c++) \
             $$quote($$BASEDIR/../src/*.cc) \
             $$quote($$BASEDIR/../src/*.cpp) \
             $$quote($$BASEDIR/../src/*.cxx) \
             $$quote($$BASEDIR/../assets/*.qml) \
             $$quote($$BASEDIR/../assets/*.js) \
             $$quote($$BASEDIR/../assets/*.qs)

    HEADERS +=  $$quote($$BASEDIR/../src/*.h) \
             $$quote($$BASEDIR/../src/*.h++) \
             $$quote($$BASEDIR/../src/*.hh) \
             $$quote($$BASEDIR/../src/*.hpp) \
             $$quote($$BASEDIR/../src/*.hxx)
}

TRANSLATIONS =  $$quote($${TARGET}.ts)
