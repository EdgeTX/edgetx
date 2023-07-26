/*
	TimerEdit
	https://github.com/mpaperno/maxLibQt

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

#ifndef TIMEREDIT_H
#define TIMEREDIT_H

#include <QLineEdit>
#include <QRegularExpression>
#include <QRegularExpressionValidator>
#include <QKeyEvent>
#include <QWheelEvent>

/**
	\class TimerEdit
	\version 1.0.0

	\brief A time value line editor which accepts negative and large times (> 23:59:59), suitable for a timer, etc.

	Qt's \c QTimeEdit has a limitation in that it can only accept "valid" times, in the range of 0:00:00 to 23:59:59.
	The TimerEdit control aims to address that.  It allows any amount of time to be entered, as well as negative times.
	It provides a mask for data entry, a validator with settable min/max times, and increment/decrement via keyboard
	or mouse wheel in configurable steps.

	Data I/O is in seconds only, that is you set the time value in total seconds and also read it back in seconds
	(there is no conversion from/to a \e QTime object).

*/
class TimerEdit : public QLineEdit
{
		Q_OBJECT
		Q_PROPERTY(int minimumTime READ minimumTime WRITE setMinimumTime)
		Q_PROPERTY(int maximumTime READ maximumTime WRITE setMaximumTime)
		Q_PROPERTY(bool showSeconds READ showSeconds WRITE setShowSeconds)
		Q_PROPERTY(unsigned int singleStep READ singleStep WRITE setSingleStep)
		Q_PROPERTY(unsigned int pageStep READ pageStep WRITE setPageStep)


	public:
		enum TimeParts {Whole, Polarity, Hours, Minutes, Seconds};

		TimerEdit(QWidget *parent = Q_NULLPTR);

		int timeInSeconds() const;
		int timepart(const QString &input, TimeParts part) const;

		bool showSeconds() const { return m_showSeconds; }
		int minimumTime() const { return m_minTime; }
		int maximumTime() const { return m_maxTime; }
		unsigned int singleStep() const { return m_singleStep; }
		unsigned int pageStep() const { return m_pageStep; }

	public slots:
		void setTime(int seconds);
		void incrDecr(int seconds);
		void setTimeRange(int minSeconds, int maxSeconds);
		void setMinimumTime(int minSeconds);
		void setMaximumTime(int maxSeconds);
		void setShowSeconds(bool showSeconds);

		void setSingleStep(unsigned int step) { m_singleStep = step; }
		void setPageStep(unsigned int pageStep) { m_pageStep = pageStep; }

	protected:
		void textEditedHandler();
		virtual void setupFormat();
		void emitValueChanged();
		void keyPressEvent(QKeyEvent *event) Q_DECL_OVERRIDE;
		void wheelEvent(QWheelEvent *event) Q_DECL_OVERRIDE;

		bool m_showSeconds;
		int m_minTime;  // seconds
		int m_maxTime;  // seconds
		unsigned m_singleStep;  // seconds
		unsigned m_pageStep;    // seconds
		short m_hourDigits;
		QRegularExpressionValidator * m_validator;
};

#endif // TIMEREDIT_H
