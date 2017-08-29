# source files
set(HEADER_FILES
    syncthingapplet.h
)
set(SRC_FILES
    syncthingapplet.cpp
)
set(PLASMOID_FILES
    ../package/metadata.desktop
    ../package/contents/ui/CompactRepresentation.qml
    ../package/contents/ui/FullRepresentation.qml
    ../package/contents/ui/DirectoriesPage.qml
    ../package/contents/ui/DevicesPage.qml
    ../package/contents/ui/DownloadsPage.qml
    ../package/contents/ui/TopLevelView.qml
    ../package/contents/ui/TopLevelItem.qml
    ../package/contents/ui/DetailView.qml
    ../package/contents/ui/DetailItem.qml
    ../package/contents/ui/main.qml
)
list(APPEND QML_FILES ${PLASMOID_FILES})

# find c++utilities
find_package(c++utilities 4.6.0 REQUIRED)
list(APPEND CMAKE_MODULE_PATH ${CPP_UTILITIES_MODULE_DIRS})

# find qtutilities
find_package(qtutilities 5.0.0 REQUIRED)
use_qt_utilities()

# find backend libraries
find_package(syncthingconnector ${META_APP_VERSION} REQUIRED)
use_syncthingconnector()
find_package(syncthingmodel ${META_APP_VERSION} REQUIRED)
use_syncthingmodel()
find_package(syncthingwidgets ${META_APP_VERSION} REQUIRED)
use_syncthingwidgets()

# link also explicitely against the following Qt 5 modules
list(APPEND ADDITIONAL_QT_MODULES Network Qml)
list(APPEND ADDITIONAL_KF_MODULES Plasma)

include(BasicConfig)
include(QtGuiConfig)
include(QtConfig)
include(WindowsResources)
include(LibraryTarget)
include(ConfigHeader)

# what ever this does, it is done
kcoreaddons_desktop_to_json("${META_PROJECT_NAME}" ../package/metadata.desktop)