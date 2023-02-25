/*
	RoundedMessageBox
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

#ifndef ROUNDEDMESSAGEBOX_H
#define ROUNDEDMESSAGEBOX_H

#include <QMessageBox>
#include <QPainter>
#include <QPalette>
#include <QStyle>
#include <QStyleOption>

/*!
	\brief The RoundedMessageBox class is a frameless \c QMessageBox implementation.
	It can be styled using using either QSS (CSS) or by using a custom \c QPalette
	in combination with the regular \c QWidget::setForegroundRole() and \c QWidget::setBackgroundRole().

	This was originally created as an answer to a [StackOverflow question][1]. It is very simple, only
	re-implementing the \c paintEvent(). There is no way for the user to move the message box around
	on the screen (allowing to drag it by the contents area would be a nice TODO).

	Here is an example of using the message box with both ways of styling it, also from the SO answer (see link for screenshot).
	\code
	int main(int argc, char *argv[])
	{
		//QApplication::setStyle("Fusion");
		QApplication app(argc, argv);

		// Dialog setup
		RoundedMessageBox *msgBox = new RoundedMessageBox();
		msgBox->setAttribute(Qt::WA_DeleteOnClose);
		msgBox->setMinimumSize(300, 300);
		msgBox->setWindowTitle("Frameless window test");
		msgBox->setText("<h3>Frameless rounded message box.</h3>");
		msgBox->setInformativeText("Lorem ipsum dolor sit amet, consectetur ....");

		// Styling: two options with the same (garish) result.
		if (1) {
			// Use QSS style
			app.setStyleSheet(QStringLiteral(
				"QMessageBox { "
					"border-radius: 12px; "
					"border: 3.5px solid; "
					"border-color: qlineargradient(x1: 1, y1: 1, x2: 0, y2: 0, stop: 0 #ffeb7f, stop: 1 #d09d1e); "
					"background: qlineargradient(x1: 0, y1: 0, x2: 1, y2: 1, stop: 0 #ffeb7f, stop: 1 #d09d1e); "
					"color: #003200; "
				"}"
			));
		}
		else {
			// Use "native" styling
			msgBox->radius = 12.0;
			msgBox->borderWidth = 3.5;

			QLinearGradient bgGrad(0, 0, 1, 1);
			bgGrad.setCoordinateMode(QGradient::ObjectMode);
			bgGrad.setColorAt(0.0, QColor("gold").lighter());
			bgGrad.setColorAt(1.0, QColor("goldenrod").darker(105));
			QLinearGradient fgGrad(bgGrad);
			fgGrad.setStart(bgGrad.finalStop());
			fgGrad.setFinalStop(bgGrad.start());

			QPalette pal;
			pal.setBrush(QPalette::Window, QBrush(bgGrad));
			pal.setBrush(QPalette::Mid, QBrush(fgGrad));
			pal.setBrush(QPalette::WindowText, QColor("darkgreen").darker());
			msgBox->setPalette(pal);

			msgBox->setForegroundRole(QPalette::Mid);     // default is WindowText
			msgBox->setBackgroundRole(QPalette::Window);  // this is actually the default already
		}

		msgBox->show();
		return app.exec();
	}
	\endcode

	[1]: https://stackoverflow.com/questions/58145272/qdialog-with-rounded-corners-have-black-corners-instead-of-being-translucent/58151965#58151965
*/
class RoundedMessageBox : public QMessageBox
{
		Q_OBJECT
	public:
		explicit RoundedMessageBox(QWidget *parent = nullptr) :
		  QMessageBox(parent)
		{
			// The FramelessWindowHint flag and WA_TranslucentBackground attribute are vital.
			setWindowFlags(windowFlags() | Qt::FramelessWindowHint | Qt::WindowSystemMenuHint);
			setAttribute(Qt::WA_TranslucentBackground);
		}

		qreal radius = 0.0;        //!< desired radius in absolute pixels
		qreal borderWidth = -1.0;  //!< -1 = use style hint frame width;  0 = no border;  > 0 = use this width.

	protected:
		void paintEvent(QPaintEvent *) override
		{
			if (!(windowFlags() & Qt::FramelessWindowHint) && !testAttribute(Qt::WA_TranslucentBackground))
				return;  // nothing to do

			QPainter p(this);
			p.setRenderHint(QPainter::Antialiasing);

			// Have style sheet?
			if (testAttribute(Qt::WA_StyleSheetTarget)) {
				// Let QStylesheetStyle have its way with us.
				QStyleOption opt;
				opt.initFrom(this);
				style()->drawPrimitive(QStyle::PE_Widget, &opt, &p, this);
				p.end();
				return;
			}

			// Paint thyself.
			QRectF rect(QPointF(0, 0), size());
			// Check for a border size.
			qreal penWidth = borderWidth;
			if (penWidth < 0.0) {
				QStyleOption opt;
				opt.initFrom(this);
				penWidth = style()->pixelMetric(QStyle::PM_DefaultFrameWidth, &opt, this);
			}
			// Got pen?
			if (penWidth > 0.0) {
				p.setPen(QPen(palette().brush(foregroundRole()), penWidth));
				// Ensure border fits inside the available space.
				const qreal dlta = penWidth * 0.5;
				rect.adjust(dlta, dlta, -dlta, -dlta);
			}
			else {
				// QPainter comes with a default 1px pen when initialized on a QWidget.
				p.setPen(Qt::NoPen);
			}
			// Set the brush from palette role.
			p.setBrush(palette().brush(backgroundRole()));
			// Got radius?  Otherwise draw a quicker rect.
			if (radius > 0.0)
				p.drawRoundedRect(rect, radius, radius, Qt::AbsoluteSize);
			else
				p.drawRect(rect);

			// C'est finí
			p.end();
		}
};

#endif // ROUNDEDMESSAGEBOX_H
