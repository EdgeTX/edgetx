/*
	Image Viewer example application
	https://github.com/mpaperno/maxLibQt

	COPYRIGHT: (c)2019 Maxim Paperno; All Rights Reserved.
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

#include "ImageViewer.h"
#include <QApplication>
#include <QCommandLineParser>
#include <QPixmapCache>

#define RESOURCES_PATH  ":"           // use for embedded CSS resource
//#define RESOURCES_PATH  SOURCE_DIR  // use for local CSS file, SOURCE_DIR should be set in build script

/*!
	\ingroup examples
	\defgroup examples_imageviewer Image Viewer example application
	Main application file for the Image Viewer example.

	```
	Usage: ImageViewer [options] [path]

	Options:
		-m, --maxcache <MBytes>  Maximum size of image previews cache (default: 1000 MB).
		-?, -h, --help           Displays this help.

	Arguments:
		path                     Start with this image folder or file.
	```

	\note The global \c QPixmapCache is used to store image previews in the \c ImageGrid class. It should be suitably "large."

*/
int main(int argc, char *argv[])
{
	QApplication::setStyle("Fusion");  // things style better with Fusion
	QApplication a(argc, argv);
	QCoreApplication::setOrganizationName("maxLibQt");
	QCoreApplication::setApplicationName("maxLibQt Image Viewer Example");

	const int maxCache = QPixmapCache::cacheLimit() * 100 / 1024;

	QCommandLineParser clp;
	clp.setApplicationDescription(QCoreApplication::applicationName());
	clp.setSingleDashWordOptionMode(QCommandLineParser::ParseAsLongOptions);
	clp.addPositionalArgument(QStringLiteral("path"), QStringLiteral("Start with this image folder or file."), QStringLiteral("[path]"));
	clp.addOption({ {QStringLiteral("m"), QStringLiteral("maxcache")},
	                QStringLiteral("Maximum size of image previews cache (default: %1 MB).").arg(maxCache),
	                QStringLiteral("MBytes"), QString::number(maxCache) });
	clp.addHelpOption();
	clp.process(a);

	QPixmapCache::setCacheLimit(clp.value(QStringLiteral("m")).toInt() * 1024);
	ImageViewer w(RESOURCES_PATH "/imageviewer.css", clp.positionalArguments().value(0));
	return a.exec();
}
