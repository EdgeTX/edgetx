/*
	MLHexSpinBox
	https://github.com/mpaperno/maxLibQt

	COPYRIGHT: (c)2018 Maxim Paperno; All Right Reserved.
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

import QtQuick 2.10
import QtQuick.Controls 2.3

/*!
	\brief MLHexSpinBox allows editing integers using hexadecimal notation.

	It uses MLDoubleSpinBox as the base class because this allows a wider range of values, including signed integers.
  The basic Controls 2 SpinBox is limited to signed int range only.

	Individual property documentation can be found inline.

	\sa MLDoubleSpinBox
*/

MLDoubleSpinBox {
	id: control
	objectName: "MLHexSpinBox"

	property bool upperCase: true     //!< Whether to force upper-case formatting for letters.
	property bool zeroPad: true       //!< Whether to pad numbers with leading zeros up to \p digits length.
	property bool showPrefix: true    //!< Whether to show the "0x" prefix.
	property int digits: Math.abs(topValue).toString(16).length   //!< Number of digits expected, used in validator, input mask, and for zero-padding. Default is based on maximum value.

	from: 0
	to: 0xFFFFFFFF
	decimals: 0
	inputMethodHints: Qt.ImhNoPredictiveText | (upperCase ? Qt.ImhPreferUppercase : 0)
	inputMask: (value < 0 ? "-" : "") + (showPrefix ? "\\0\\x" : "") + "H".repeat(digits)

	validator: RegExpValidator {
		regExp: new RegExp("-?(0x)?[0-9A-Fa-f]{1," + control.digits + "}")
	}

	function textFromValue(value, locale) {
		var ret = Math.abs(Number(value)).toString(16);
		if (zeroPad && digits > ret.length)
			ret = "0".repeat(digits - ret.length) + ret;
		if (upperCase)
			ret = ret.toUpperCase();
		if (showPrefix)
			ret = "0x" + ret;

		return ret;
	}

	function valueFromText(text, locale) {
		return parseInt(text, 16);
	}

}
