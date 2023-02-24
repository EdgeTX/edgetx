/*
	AppDebugMessageHandler

	COPYRIGHT: (c)2017 Maxim Paperno; All Right Reserved.
	Contact: http://www.WorldDesign.com/contact

	LICENSE:

	Commercial License Usage
	Licensees holding valid commercial licenses may use this file in
	accordance with the terms contained in a written agreement between
	you and the copyright holder.

	GNU General Public License Usage
	Alternatively, this file may be used under the terms of the GNU
	General Public License as published by the Free Software Foundation,
	either version 3 of the License, or (at your option) any later version.

	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	A copy of the GNU General Public License is available at <http://www.gnu.org/licenses/>.
*/

#include "AppDebugMessageHandler.h"

#include <cstdlib>
#include <iostream>
#include <QFileDevice>

AppDebugMessageHandler::AppDebugMessageHandler() :
  QObject(),
  m_defaultHandler(nullptr)
{
	setAppDebugOutputLevel(APP_DBG_HANDLER_DEFAULT_LEVEL);
	setShowSourcePath(APP_DBG_HANDLER_SHOW_SRC_PATH);
	setSourceBasePath(QStringLiteral(APP_DBG_HANDLER_SRC_PATH));
	setShowFunctionDeclarations(APP_DBG_HANDLER_SHOW_FUNCTION_DECL);
	setShowTimestamp(APP_DBG_HANDLER_SHOW_TIMESTAMP);
	setTimestampFormat(QStringLiteral(APP_DBG_HANDLER_TIMESTAMP_FORMAT));
	defaultMessagePattern();  // preload
#if (QT_VERSION < QT_VERSION_CHECK(5, 4, 0))
	m_functionFilter = QRegularExpression("^.+?(\\w+\\().+$");
#endif
}

// static
AppDebugMessageHandler *AppDebugMessageHandler::instance()
{
	static AppDebugMessageHandler instance;
	return &instance;
}

void AppDebugMessageHandler::setAppDebugOutputLevel(quint8 appDebugOutputLevel)
{
	QWriteLocker locker(&m_mutex);
	m_appDebugOutputLevel = qMin<quint8>(appDebugOutputLevel, 4);
}

void AppDebugMessageHandler::setShowSourcePath(bool showSourcePath)
{
	QWriteLocker locker(&m_mutex);
	m_showSourcePath = showSourcePath;
	m_defaultPattern.clear();
}

void AppDebugMessageHandler::setSourceBasePath(const QString &path)
{
	QWriteLocker locker(&m_mutex);
	m_srcPathFilter = path.isEmpty() ? QRegularExpression() : QRegularExpression(QStringLiteral("^%1[\\\\\\/](.*?)$").arg(path), QRegularExpression::InvertedGreedinessOption);
}

void AppDebugMessageHandler::setShowFunctionDeclarations(bool showFunctionDeclarations)
{
	QWriteLocker locker(&m_mutex);
	m_showFunctionDeclarations = showFunctionDeclarations;
	m_defaultPattern.clear();
}

void AppDebugMessageHandler::setShowTimestamp(bool showTimestamp)
{
	QWriteLocker locker(&m_mutex);
	m_showTimestamp = showTimestamp;
	m_defaultPattern.clear();
}

void AppDebugMessageHandler::setTimestampFormat(const QString &timeFormat)
{
	QWriteLocker locker(&m_mutex);
	m_tsFormat = timeFormat;
	m_defaultPattern.clear();
}

void AppDebugMessageHandler::setMessagePattern(const QString &pattern)
{
	QWriteLocker locker(&m_mutex);
	m_msgPattern = pattern;
}

void AppDebugMessageHandler::addOutputDevice(QIODevice *device)
{
	QWriteLocker locker(&m_mutex);
	if (device && !m_outputDevices.contains(device))
		m_outputDevices.append(device);
}

void AppDebugMessageHandler::removeOutputDevice(QIODevice *device)
{
	if (!device)
		return;
	QWriteLocker locker(&m_mutex);
#if (QT_VERSION >= QT_VERSION_CHECK(5, 4, 0))
	m_outputDevices.removeAll(device);
#else
	int i = 0;
	foreach (QIODevice * d, m_outputDevices) {
		if (d == device)
			m_outputDevices.remove(i);
		++i;
	}
#endif
}

QString AppDebugMessageHandler::defaultMessagePattern() const
{
	if (!m_defaultPattern.isEmpty())
		return m_defaultPattern;

	QString msgPattern;
	if (m_showTimestamp)
		msgPattern.append(QStringLiteral("[%{time %1}] ").arg(m_tsFormat));

	msgPattern.append(QLatin1String("[%{short-type}] "));

	if (m_showSourcePath)
		msgPattern.append(QLatin1String("%{file}::"));

	if (m_showFunctionDeclarations)
		msgPattern.append(QLatin1String("%{full-function}"));
	else
		msgPattern.append(QLatin1String("%{function}()"));

	msgPattern.append(QLatin1String(":%{line} -%{if-category} [%{category}]%{endif} %{message}"));

#ifndef QT_NO_GLIB
	msgPattern.append("%{if-fatal}\nBACKTRACE:\n%{backtrace depth=12 separator=\"\n\"}%{endif}");
#endif

	m_defaultPattern = msgPattern;
	return msgPattern;
}

QString AppDebugMessageHandler::messagePattern() const
{
	return m_msgPattern.isEmpty() ? defaultMessagePattern() : m_msgPattern;
}

void AppDebugMessageHandler::installAppMessageHandler()
{
#if APP_DBG_HANDLER_ENABLE
	m_defaultHandler = qInstallMessageHandler(g_appDebugMessageHandler);
#else
	qInstallMessageHandler(nullptr);
#endif
}

void AppDebugMessageHandler::messageHandler(QtMsgType type, const QMessageLogContext &context, const QString &msg)
{
	// normalize types, QtDebugMsg stays 0, QtInfoMsg becomes 1, rest are QtMsgType + 1
	quint8 lvl = type;
	if (type == QtInfoMsg)
		lvl = 1;
	else if (type > QtDebugMsg)
		++lvl;

	QReadLocker locker(&m_mutex);

	if (lvl < m_appDebugOutputLevel)
		return;

#if defined(Q_OS_LINUX) && (QT_VERSION < QT_VERSION_CHECK(5, 3, 0))
	// Filter out lots of QPainter warnings from undocked QDockWidgets... hackish but effective (only workaround found so far)
	if (lvl == 2 && QString(context.function).contains("QPainter::"))
		return;
#endif

	QString msgPattern = messagePattern();

	QString file = context.file;
	if (m_srcPathFilter.isValid())
		file.replace(m_srcPathFilter, "\\1");

	const bool hasDevices = !m_outputDevices.isEmpty();

	locker.unlock();

	msgPattern.replace(QLatin1String("%{short-type}"), shortTypeNames().value(type, QStringLiteral("?")));
	msgPattern.replace(QLatin1String("%{full-function}"), context.function);

	QMessageLogContext newContext(qPrintable(file), context.line, context.function, context.category);

	qSetMessagePattern(msgPattern);

	if (!m_defaultHandler || hasDevices || receivers(SIGNAL(messageOutput(quint8, const QString &)))) {
#if (QT_VERSION >= QT_VERSION_CHECK(5, 4, 0))
		msgPattern = qFormatLogMessage(type, newContext, msg);
#else
		msgPattern.replace("%{type}", fullTypeNames().value(type, "unknown"));
		msgPattern.replace("%{line}", QString::number(context.line));
		msgPattern.replace("%{if-category} [%{category}]%{endif}", QString(context.category));
		msgPattern.replace("%{message}", msg);
		msgPattern.replace("%{function}", QString(context.function).replace(m_functionFilter, "\\1)"));
#endif

		emit messageOutput(lvl, msgPattern);

		if (hasDevices) {
			locker.relock();
			const QVector<QIODevice *> devices(m_outputDevices);
			locker.unlock();
			for (QIODevice * d : devices) {
				if (d && d->isWritable() && (!d->property("level").isValid() || d->property("level").toInt() <= lvl)) {
					d->write(qPrintable(msgPattern + "\n"));
					if (QFileDevice * fd = qobject_cast<QFileDevice *>(d))
						fd->flush();
				}
			}
		}
	}

	// if (QThread::currentThread() == qApp->thread())  // gui thread

	if (m_defaultHandler) {
		m_defaultHandler(type, newContext, msg);
	}
	else {
		fprintf(stderr, "%s", qPrintable(msgPattern));
		if (type == QtFatalMsg)
			abort();
	}
}

// static
QHash<int, QString> &AppDebugMessageHandler::shortTypeNames()
{
	static QHash<int, QString> symbols({
	  {QtDebugMsg,    tr("D")},
	  {QtWarningMsg,  tr("W")},
	  {QtCriticalMsg, tr("C")},
	  {QtFatalMsg,    tr("F")},
	  {QtInfoMsg,     tr("I")}
	});
	return symbols;
}

#if (QT_VERSION < QT_VERSION_CHECK(5, 4, 0))
// static
QHash<int, QString> &AppDebugMessageHandler::fullTypeNames()
{
	static QHash<int, QString> symbols({
	  {QtDebugMsg,    tr("debug")},
	  {QtWarningMsg,  tr("warning")},
	  {QtCriticalMsg, tr("critical")},
	  {QtFatalMsg,    tr("fatal")},
	  {QtInfoMsg,     tr("info")}
	});
	return symbols;
}
#endif

// Message handler which is installed using qInstallMessageHandler. This needs to be global.
void g_appDebugMessageHandler(QtMsgType type, const QMessageLogContext &context, const QString &msg)
{
	if (AppDebugMessageHandler::instance())
		AppDebugMessageHandler::instance()->messageHandler(type, context, msg);
}
