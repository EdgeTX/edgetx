/*
	MLDoubleSpinBox
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
	\brief MLDoubleSpinBox is a drop-in replacement for QtQuick Controls 2 SpinBox which can handle double-precision numbers and long integers.

			Supports SpinBox API of Controls v2.4 (Qt 5.11) but can be used with any version of QtQuick Controls 2 (Qt v5.2+).

	The possible value range is defined by ECMAScript \e [Number] limits: +/- \c Number.MAX_VALUE for reals and between
	\c Number.MIN_SAFE_INTEGER and \c Number.MAX_SAFE_INTEGER for integers. This includes being able to handle unsigned 32b ints,
	something the base \e SpinBox can't do.

	It has mostly the same properties, methods, and signal as \e SpinBox, except the \p value/from/to/stepSize properties are doubles.
  The only exception are the \c up and \c down group, which could be accessed directly via #spinBoxItem.
  It also has a number of other custom properties not found in the default SpinBox (see inline documentation below).

	Use the #decimals property to control precision (default is 2).

	In addition to the regular \e SpinBox controls (arrow keys/wheel-scroll), it reacts to Page Up/Down keys and CTRL-scroll for page-sized steps (see #pageSteps property).

	Individual property documentation can be found inline.

	\note About the #prefix and #suffix properties:
	\note
	This SpinBox does add \p prefix and \p suffix properties, but it is not in an ideal way. Specifically, in an
	editable spin box, the cursor is not placed in the correct position for editing the actual value (user can start typing
	before/after/in middle of prefix/suffix). It does still work and will auto-correct properly after editing is
	finished, but it is a bit awkward at best.  They do work nicely in a non-editable spin box.

	\note
	To achieve proper prefix/suffix support, the best way would be to use a custom QValidator, just like the Controls 1
	SpinBox does (or did... RIP). (search for QQuickSpinBoxValidator1) The other way is to use an input mask.
	Unfortunately Qt support for input masks is rather limited, and as a result one has to use a very specific mask (with
	the correct number of digits, notation type, localized, etc.). The number has to be typed/formatted exactly to the
	mask. This is not very user-friendly either, except in specific circumstances. Also the DoubleValidator does not work with
	input masks, so we need a custom validator in any case (see \e regExpValidator below).

	\note
	The accompanying tests.qml file in this folder has an example of a customized spin box using an input mask with prefix
	and suffix. It is pretty generalized, but for standard notation only.

	[Number]: https://developer.mozilla.org/en-US/docs/Web/JavaScript/Reference/Global_Objects/Number

*/

Control {
	id: control
	objectName: "MLDoubleSpinBox"

	// Standard SpinBox API properties (v2.4)
	property double value: 0.0
	property double from: 0.0
	property double to: 100.0
	property double stepSize: 1.0
	property bool editable: true
	property bool wrap: true
	property QtObject validator: doubleValidator    //!< There are 2 validators available, see notes for each below.
	property int inputMethodHints: Qt.ImhFormattedNumbersOnly
	readonly property string displayText: textFromValue(value, effectiveLocale)
	readonly property bool inputMethodComposing: textInputItem ? textInputItem.inputMethodComposing : false

	// Custom properties
	property int decimals: 2                  //!< Desired precision
	property int notation: DoubleValidator.StandardNotation   //!< For validator and text formatting
	property string inputMask                 //!< Input mask for the text edit control (\sa TextInput::inputMask).
	property bool selectByMouse: true         //!< Whether to allow selection of text (bound to the text editor of the spinbox control).
	property bool useLocaleFormat: true       //!< Whether to format numbers according to the current locale. If false, use standard "C" format.
	property bool showGroupSeparator: true    //!< Whether to format numbers with the thousands separator visible (using current locale if useLocaleFormat is true).
	property bool trimExtraZeros: true        //!< Whether to remove trailing zeros from decimals.
	property string prefix                    //!< Optional string to display before the value. See notes in the main comments above.
	property string suffix                    //!< Optional string to display after the value. See notes in the main comments above.
	property int pageSteps: 10                //!< How many steps in a "page" step (PAGE UP/DOWN keys or CTRL-Wheel).
	property int buttonRepeatDelay: 300       //!< Milliseconds to delay before held +/- button repeat is activated.
	property int buttonRepeatInterval: 100    //!< +/- button repeat interval while held (in milliseconds).

	readonly property string cleanText: getCleanText(displayText)                            //!< Holds the text of the spin box excluding any prefix, suffix, or leading or trailing whitespace.
	readonly property bool acceptableInput: textInputItem && textInputItem.acceptableInput   //!< Indicates if input is valid (it would be nicer if the validator would expose an "isValid" prop/method!).
	readonly property real topValue: Math.max(from, to)                                      //!< The effective maximum value
	readonly property real botValue: Math.min(from, to)                                      //!< The effective minimum value

	//! The SpinBox item. To use a custom one, replace the \p contentItem with a class derived from Controls 2.x SpinBox.
	//! Or use any other \p contentItem (or even \e null) and (optionally) set the #textInputItem to some \e Item with a \c text property for a custom display.
	readonly property SpinBox spinBoxItem: contentItem
	//! Use the "native" text editor of the SpinBox to preserve look/feel. If you use a custom SpinBox, you may need to set this property also. If defined, it must have a \e text property.
	property Item textInputItem: spinBoxItem ? spinBoxItem.contentItem : null

	//! Default numeric validator, strictly enforces \p notation type, does not allow for custom \p inputMask with non-numeric components.
	readonly property QtObject doubleValidator: DoubleValidator {
		top: control.topValue
		bottom: control.botValue
		decimals: Math.max(control.decimals, 0)
		notation: control.notation
		locale: control.effectiveLocale.name
	}

	/*! This is an experimental validator using a RegExp instead of numerical validation (which we do anyway).
		The advantage of using it is that:
			\li It allows for scientific notation entry even if std. notation is specified (and vice versa) and will then re-format the entry as necessary after editing is finished.
      \li Could also be used with a custom input mask, as demonstrated in tests.qml
		To use it, just set \c validator: regExpValidator  */
	readonly property QtObject regExpValidator: RegExpValidator { regExp: control.doubleValidationRegEx(); }

	// signals

	signal valueModified()   //!< Mimic SpinBox API (interactive change only, NOT emitted if \e value property is set directly).

	// QtQuick Control properties

	// By default wheel is enabled only if editor has active focus or item is not editable.
	wheelEnabled: !editable || (textInputItem && textInputItem.activeFocus)

	// The spin box itself... it's really only here for its buttons and overall formatting, we ignore its actual value/etc.
	contentItem: SpinBox {
		width: control.availableWidth
		height: control.availableHeight
		editable: control.editable
		inputMethodHints: control.inputMethodHints
		validator: control.validator
		from: -0x7FFFFFFF; to: 0x7FFFFFFF;  // prevent interference with our real from/to values
		// wrap peroperty is set below as a Binding in case SpinBox vesion is < 2.3 (Qt 5.10).
	}

	// Public function API

	//! Increment value by one #stepSize
	function increase() {
		stepBy(1);
	}

	//! Decrement value by one #stepSize
	function decrease() {
		stepBy(-1);
	}

	/*! Adjust value by number of \p steps. (Each step size is determined by the spin box #stepSize property.)
		\param type:int steps Number of steps to adjust by. Can be negative to decrement the value.
		\param type:bool noWrap (optional) If true will prevent wrapping even if the spin box #wrap property is true. Default is false.
	*/
	function stepBy(steps, noWrap) {
		// always use current editor value in case user has changed it w/out losing focus
		setValue(textValue() + (stepSize * steps), noWrap);
	}

	/*! Set the spin box value to \p newValue. This is generally preferable to setting the #value spin box property directly, but not required.
		\param type:real newValue The value to set.
		\param type:bool noWrap (optional) If true will prevent wrapping even if the spin box #wrap property is true. Default is false.
		\param type:bool notModified (optional) If true will prevent the \e valueModified() signal from being emitted. Default is false.
		\return type:bool True if value was updated (that is, it did not equal the old value), false otherwise.
	*/
	function setValue(newValue, noWrap, notModified)
	{
		if (!wrap || noWrap)
			newValue = Math.max(Math.min(newValue, control.topValue), control.botValue);
		else if (newValue < control.botValue)
			newValue = control.topValue;
		else if (newValue > control.topValue)
			newValue = control.botValue;

		newValue = Number(newValue.toFixed(Math.max(decimals, 0)));  // round

		if (value !== newValue) {
			isValidated = true;
			value = newValue;
			isValidated = false;
			if (!notModified)
				valueModified();
			if (spinBoxItem)
				spinBoxItem.value = 0;  // reset this to prevent it from disabling the buttons or other weirdness
			//console.log("setValue:", newValue.toFixed(control.decimals));
			return true;
		}
		return false;
	}

	//! Reimplimented from SpinBox
	function textFromValue(value, locale)
	{
		if (!locale)
			locale = effectiveLocale;

		var text = value.toLocaleString(locale, (notation === DoubleValidator.StandardNotation ? 'f' : 'e'), Math.max(decimals, 0));

		if (!showGroupSeparator && locale.name !== "C")
			text = text.replace(new RegExp("\\" + locale.groupSeparator, "g"), "");
		if (trimExtraZeros) {
			var pt = locale.decimalPoint;
			var ex = new RegExp("\\" + pt + "0*$|(\\" + pt + "\\d*[1-9])(0+)$").exec(text);
			if (ex)
				text = text.replace(ex[0], ex[1] || "");
		}

		if (prefix)
			text = prefix + text;
		if (suffix)
			text = text + suffix;

		return text;
	}

	//! Reimplimented from SpinBox
	function valueFromText(text, locale)
	{
		if (!locale)
			locale = effectiveLocale;
		// strip prefix/suffix, or custom pre-processor
		text = getCleanText(text, locale);
		// We need to clean the string before using Number::fromLocaleString because it throws errors when the input format isn't valid, eg. thousands separator in the wrong place. D'oh.
		var re = "[^\\+\\-\\d\\" + locale.decimalPoint + locale.exponential + "]+";
		text = text.replace(new RegExp(re, "gi"), "");
		if (!text.length)
			text = "0";
		//console.log("valueFromText:", text, locale.name, Number.fromLocaleString(locale, text));
		return Number.fromLocaleString(locale, text);
	}

	/*! Return \p text stripped of any \e prefix or \e suffix and trimmed. Same as \e cleanText property.
			Called by #valueFromText() before other cleanup operations. Could be reimplemented for custom replacements. */
	function getCleanText(text, locale)
	{
		text = String(text);
		if (prefix)
			text = text.replace(prefixRegEx, "");
		if (suffix)
			text = text.replace(suffixRegEx, "");
		return text.trim();
	}

	//! Make \p string safe for use in RegExp as a literal.
	function escapeRegExpChars(string) {
		return string.replace(/[.*+?^${}()|[\]\\]/g, '\\$&');
	}

	//! Make \p string safe for use in Qt input mask as a literal. \sa QLineEdit::inputMask
	function escapeInputMaskChars(string) {
		return string.replace(/[{}\[\]\\><!#09anxdhb]/gi, '\\$&');
	}

	//! Return a RegExp object to validate numeric entry according to the current formatting & locale specs and accounting for any prefix/suffix.
	function doubleValidationRegEx()
	{
		var locale = effectiveLocale,
				pnt = locale.decimalPoint,
				grp = locale.groupSeparator,
				exp = locale.exponential,
				pfx = escapeRegExpChars(prefix),
				sfx = escapeRegExpChars(suffix),
				expRe = "(?:" + exp + "[+-]?[\\d]+)?",
				re = "^" + pfx + "[+-]?(?:[\\d]{1,3}\\" + grp + "?)+\\" + pnt + "?[\\d]*" + expRe + sfx + "$";
		// ^[+-]?(?:[\d]{1,3},?)+\.?[\d]*(?:e[+-]?[\d]+)?$
		return new RegExp(re, "i");
	}

	// internals

	property bool isValidated: false  //!< \private
	property bool completed: false    //!< \private
	readonly property var defaultLocale: Qt.locale("C")  //!< \private
	readonly property var effectiveLocale: useLocaleFormat ? locale : defaultLocale  //!< \private
	readonly property var prefixRegEx: new RegExp("^" + escapeRegExpChars(prefix))  //!< \private
	readonly property var suffixRegEx: new RegExp(escapeRegExpChars(suffix) + "$")  //!< \private


	//! \private Get numeric value from current text
	function textValue() {
		return textInputItem ? valueFromText(textInputItem.text, effectiveLocale) : 0;
	}

	//! \private Update the current value and/or formatting of the displayed text. In mnost cases one would use \e setValue() .
	function updateValueFromText() {
		if (!setValue(textValue(), true))
			updateUi();  // make sure the text is formatted anyway
	}

	//! \private
	function handleKeyEvent(event)
	{
		var steps = 0;
		if (event.key === Qt.Key_Up)
			steps = 1;
		else if (event.key === Qt.Key_Down)
			steps = -1;
		else if (event.key === Qt.Key_PageUp)
			steps = control.pageSteps;
		else if (event.key === Qt.Key_PageDown)
			steps = -control.pageSteps;
		else if (event.key !== Qt.Key_Enter && event.key !== Qt.Key_Return)
			return;

		event.accepted = true;

		if (steps)
			stepBy(steps);
		else
			updateValueFromText();
	}

	//! \private
	function toggleButtonPress(press, increment)
	{
		if (!press) {
			btnRepeatTimer.stop();
			return;
		}

		if (increment)
			increase();
		else
			decrease();
		btnRepeatTimer.increment = increment;
		btnRepeatTimer.start();
	}

	//! \private
	function updateUi()
	{
		if (!completed)
			return;

		if (textInputItem)
			textInputItem.text = textFromValue(value, effectiveLocale);

		if (spinBoxItem) {
			if (spinBoxItem.up && spinBoxItem.up.indicator)
				spinBoxItem.up.indicator.enabled = (wrap || value < topValue);
			if (spinBoxItem.down && spinBoxItem.down.indicator)
				spinBoxItem.down.indicator.enabled = (wrap || value > botValue);
		}
	}

	onValueChanged: {
		if (!completed)
			return;
		if (!isValidated)
			setValue(value, true, true);
		updateUi();
	}

	// We need to override spin box arrow key events to distinguish from +/- button presses, otherwise we get double repeats.
	onSpinBoxItemChanged: {
		if (spinBoxItem)
			spinBoxItem.Keys.forwardTo = [control];
	}

	Component.onCompleted: {
		completed = true;
		// An initial value may have been set, but not validated. Do that now.
		if (!setValue(value, true, true))
			updateUi();  // in case it hasn't changed
	}

	onWrapChanged: updateUi()
	onNotationChanged: updateUi()
	onTrimExtraZerosChanged: updateUi()
	onShowGroupSeparatorChanged: updateUi()
	onEffectiveLocaleChanged: updateUi()
	Keys.onPressed: handleKeyEvent(event)

	Connections {
		target: control.spinBoxItem ? control.spinBoxItem.up : null
		onPressedChanged: control.toggleButtonPress(control.spinBoxItem.up.pressed, true)
	}

	Connections {
		target: control.spinBoxItem ? control.spinBoxItem.down : null
		onPressedChanged: control.toggleButtonPress(control.spinBoxItem.down.pressed, false)
	}

	Connections {
		target: control.textInputItem
		// Checking active focus works better than onEditingFinished because the latter doesn't fire if input is invalid (nor does it fix it up automatically).
		onActiveFocusChanged: {
			if (!control.textInputItem.activeFocus)
				control.updateValueFromText();
		}
	}

	// We use a binding here just in case the resident SpinBox is older than v2.3
	Binding {
		target: control.spinBoxItem
		when: control.spinBoxItem && typeof control.spinBoxItem.wrap !== "undefined"
		property: "wrap"
		value: control.wrap
	}

	Binding {
		target: control.textInputItem
		property: "selectByMouse"
		value: control.selectByMouse
	}

	Binding {
		target: control.textInputItem
		property: "inputMask"
		value: control.inputMask
	}

	// Timer for firing the +/- button repeat events while they're held down.
	Timer {
		id: btnRepeatTimer
		property bool delay: true
		property bool increment: true
		interval: delay ? control.buttonRepeatDelay : control.buttonRepeatInterval
		repeat: true
		onRunningChanged: delay = true
		onTriggered: {
			if (delay)
				delay = false;
			else if (increment)
				control.increase();
			else
				control.decrease();
		}
	}

	// Wheel/scroll action detection area
	MouseArea {
		anchors.fill: control
		z: control.contentItem.z + 1
		acceptedButtons: Qt.NoButton
		enabled: control.wheelEnabled
		onWheel: {
			var delta = (wheel.angleDelta.y === 0.0 ? -wheel.angleDelta.x : wheel.angleDelta.y) / 120;
			if (wheel.inverted)
				delta *= -1;
			if (wheel.modifiers & Qt.ControlModifier)
				delta *= control.pageSteps;
			control.stepBy(delta);
		}
	}

}
