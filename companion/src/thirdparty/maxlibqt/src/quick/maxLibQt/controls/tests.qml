/*
	This file is part of maxLibQt
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
import QtQuick.Layouts 1.3

/*
	Testing page for maxLibQt QtQuick Controls components.
*/

ScrollView {
	id: root
	width: 400
	height: 650
	padding: 9

	ColumnLayout {
		id: sbTests
		spacing: 6
		clip: true

		property bool editable: true
		property bool wrap: true
		property bool useLocaleFormat: true
		property bool showGroupSeparator: true

		function setupSb(sb) {
			sb.editable = Qt.binding(function() { return sbTests.editable; });
			sb.wrap = Qt.binding(function() { return sbTests.wrap; });
			sb.useLocaleFormat = Qt.binding(function() { return sbTests.useLocaleFormat; });
			sb.showGroupSeparator = Qt.binding(function() { return sbTests.showGroupSeparator; });
			//sb.Layout.fillWidth = true
		}

		RowLayout {
			CheckBox {
				text: "Editable"
				checked: sbTests.editable
				onToggled: sbTests.editable = !sbTests.editable
			}
			CheckBox {
				text: "Wrap"
				checked: sbTests.wrap
				onToggled: sbTests.wrap = !sbTests.wrap
			}
			CheckBox {
				text: "Group Sep."
				checked: sbTests.showGroupSeparator
				onToggled: sbTests.showGroupSeparator = !sbTests.showGroupSeparator
			}
			CheckBox {
				text: "Localize"
				checked: sbTests.useLocaleFormat
				onToggled: sbTests.useLocaleFormat = !sbTests.useLocaleFormat
			}
		}

		GroupBox {
			title: "MLDoubleSpinBox"
			Layout.fillWidth: true

			GridLayout {
				anchors.fill: parent
				columns: 2

				Label { text: "Default" }
				MLDoubleSpinBox {
					Component.onCompleted: sbTests.setupSb(this)
				}

				Label { text: "+/-9e6.4: s:.01; loc: en" }
				MLDoubleSpinBox {
					to: 9e6; from: -to
					decimals: 4
					value: 1234567.1234
					stepSize: 0.01
					locale: Qt.locale("en_US")
					Component.onCompleted: sbTests.setupSb(this)
				}

				Label { text: "+/-9e6.4; s:.01; loc: fr" }
				MLDoubleSpinBox {
					to: 9e6; from: -to
					decimals: 4
					value: 1234567.1234
					stepSize: 0.01
					locale: Qt.locale("fr")
					Component.onCompleted: sbTests.setupSb(this)
				}

				Label { text: "+/-9e6.4; s:.01; loc: de" }
				MLDoubleSpinBox {
					to: 9e6; from: -to
					decimals: 4
					value: 1234567.1234
					stepSize: 0.01
					locale: Qt.locale("de")
					Component.onCompleted: sbTests.setupSb(this)
				}

				Label { text: "+/-9e6.0" }
				MLDoubleSpinBox {
					to: 9e6; from: -to
					decimals: 0
					value: 10234567.123456  // invalid value, should auto-correct to 9e6
					Component.onCompleted: sbTests.setupSb(this)
				}

				Label { text: "+/-9e6.6; exp" }
				MLDoubleSpinBox {
					to: 9e6; from: -to
					decimals: 6
					value: 12.3456789
					notation: DoubleValidator.ScientificNotation
					stepSize: 0.1
					Component.onCompleted: sbTests.setupSb(this)
				}

				Label { text: "+/-1e4.2; s:.01; pfx/sfx basic" }
				MLDoubleSpinBox {
					to: 10000; from: -to
					value: 55.5
					stepSize: 0.01
					prefix: "foo: "
					suffix: " (bars)"
					Component.onCompleted: sbTests.setupSb(this)
				}

				Label { text: "+/-1e4.2; s:.01; pfx/sfx inpMask" }
				/* This is an example of using a custom input mask to create an editable spin box with prefix/suffix.
					 It is somewhat generalized, but only supports standard notation, with no group separators. */
				MLDoubleSpinBox {
					to: 10000; from: -to
					value: 55.5
					stepSize: 0.01
					notation: DoubleValidator.StandardNotation   // inputMask and textFromValue do not work with exp. notation
					prefix: "foo: "
					suffix: "[bar" + (Math.abs(value) !== 1 ? "s" : "") + "]"
					inputMask: getInputMask()
					// We need to use the "experimental" RegExpValidator since DoubleValidator does not support masks.
					validator: regExpValidator
					Component.onCompleted: sbTests.setupSb(this)

					function textFromValue(value, locale) {
						locale = locale || effectiveLocale;
						var text = Math.abs(value).toLocaleString(locale, 'f', Math.max(decimals, 0)),
								dig = Math.ceil(Math.abs(topValue)).toString(10).length,
								len = Math.floor(Math.abs(value)).toString(10).length;

						text = Math.abs(value).toLocaleString(locale, 'f', Math.max(decimals, 0));

						if (dig > len)
							text = "0".repeat(dig - len) + text;
						if (botValue < 0)
							text = (value < 0 ? "-" : "+") + text;

						if (prefix)
							text = prefix + text;
						if (suffix)
							text = text + suffix;

						return text;
					}

					function getInputMask() {
						var dig = Math.abs(Math.round(topValue)).toString(10).length - 1,
							mask = (botValue < 0 ? "#" : "") + "0".repeat(dig) + "9" + (decimals > 0 ? effectiveLocale.decimalPoint + "0".repeat(decimals) : "");
						if (prefix)
							mask = escapeInputMaskChars(prefix) + mask;
						if (suffix)
							mask = mask + escapeInputMaskChars(suffix);
						return mask;
					}
				}

				RowLayout {
					Label { text: "+/-9e4.%1: regExp".arg(reBox.decimals) }
					CheckBox { id: exp; text: "exp"; checked: false }
				}
				// This version allows entry in both scientific and standard formats, and will convert to the specified notation after editing is finished.
				MLDoubleSpinBox {
					id: reBox
					to: 9e4; from: -to
					decimals: exp.checked ? 4 + 4 : 4
					value: 12345.1234
					notation: exp.checked ? DoubleValidator.ScientificNotation : DoubleValidator.StandardNotation
					validator: regExpValidator
					Component.onCompleted: sbTests.setupSb(this)
				}

			}  // GridLayout
		}  //  GroupBox MLDoubleSpinBox

		GroupBox {
			title: "MLHexSpinBox"
			Layout.fillWidth: true

			GridLayout {
				anchors.fill: parent
				columns: 2

				Label { text: "Default" }
				MLHexSpinBox {
					Component.onCompleted: sbTests.setupSb(this)
				}

				Label { text: "to: 0xFFFF; pfx: false" }
				MLHexSpinBox {
					to: 0xFFFF
					showPrefix: false
					Component.onCompleted: sbTests.setupSb(this)
				}

				Label { text: "+/-0xFFFF; noPad, LCase" }
				MLHexSpinBox {
					to: 0xFFFF
					from: -to
					zeroPad: false
					upperCase: false
					Component.onCompleted: sbTests.setupSb(this)
				}

			}  // GridLayout
		}  // GroupBox MLHexSpinBox

	}  // ColumnLayout
}
