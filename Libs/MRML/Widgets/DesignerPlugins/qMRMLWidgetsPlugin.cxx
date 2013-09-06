#include "qMRMLWidgetsPlugin.h"

#include <QtPlugin>

#if QT_VERSION < QT_VERSION_CHECK(5,0,0)
Q_EXPORT_PLUGIN2(customwidgetplugin, qMRMLWidgetsPlugin);
#endif
