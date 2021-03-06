#ifdef QT_UTILITIES_SUPPORT_DBUS_NOTIFICATIONS
#include "./dbusstatusnotifier.h"

// use meta-data of syncthingtray application here
#include "resources/../../tray/resources/config.h"

#include <QCoreApplication>

using namespace Data;
using namespace MiscUtils;

namespace QtGui {

DBusStatusNotifier::DBusStatusNotifier(QObject *parent)
    : QObject(parent)
    , m_disconnectedNotification(QStringLiteral(APP_NAME), QStringLiteral("network-disconnect"), 5000)
    , m_internalErrorNotification(QStringLiteral(APP_NAME) + tr(" - internal error"), NotificationIcon::Critical, 5000)
    , m_syncthingNotification(tr("Syncthing notification"), NotificationIcon::Warning, 10000)
    , m_syncCompleteNotification(QStringLiteral(APP_NAME), NotificationIcon::Information, 5000)
{
    m_disconnectedNotification.setMessage(tr("Disconnected from Syncthing"));
    m_disconnectedNotification.setActions(QStringList({ QStringLiteral("reconnect"), tr("Try to reconnect") }));
    connect(&m_disconnectedNotification, &DBusNotification::actionInvoked, this, &DBusStatusNotifier::connectRequested);
    m_internalErrorNotification.setActions(QStringList({ QStringLiteral("details"), tr("View details") }));
    connect(&m_internalErrorNotification, &DBusNotification::actionInvoked, this, &DBusStatusNotifier::errorDetailsRequested);
    m_syncthingNotification.setActions(QStringList({ QStringLiteral("show"), tr("Show"), QStringLiteral("dismiss"), tr("Dismiss") }));
    connect(&m_syncthingNotification, &DBusNotification::actionInvoked, this, &DBusStatusNotifier::handleSyncthingNotificationAction);
}

void DBusStatusNotifier::handleSyncthingNotificationAction(const QString &action)
{
    if (action == QLatin1String("dismiss")) {
        emit dismissNotificationsRequested();
    } else if (action == QLatin1String("show")) {
        emit showNotificationsRequested();
    }
}
} // namespace QtGui

#endif
