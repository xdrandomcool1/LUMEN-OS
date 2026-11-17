// SPDX-License-Identifier: GPL-3.0-or-later
// JavaScript engines: Spotlight calculator and the Calculator app.
import QtQuick
import QtTest
import "../../shell/qml/calc.js" as Calc
import "../../applications/utilities/calculator/qml/engine.js" as Engine

TestCase {
    name: "Engines"

    function test_spotlight_calculator() {
        compare(Calc.evaluate("2+3*4"), 14)
        compare(Calc.evaluate("(2+3)*4"), 20)
        compare(Calc.evaluate("2^10"), 1024)
        compare(Calc.evaluate("-3+5"), 2)
        fuzzyCompare(Calc.evaluate("2pi"), 2 * Math.PI, 1e-9)
        fuzzyCompare(Calc.evaluate("sqrt(16)+1"), 5, 1e-9)
        compare(Calc.evaluate("12*(3+4)"), 84)
        compare(Calc.evaluate("hello"), null)        // words are not calculations
        compare(Calc.evaluate("42"), null)           // plain numbers are not either
        compare(Calc.evaluate("1/0"), null)          // non-finite rejected
        compare(Calc.evaluate("2+"), null)           // syntax error
        compare(Calc.evaluate("alert(1)"), null)     // no code execution
    }
    function press(keys) { let s = Engine.initial(); for (const k of keys) s = Engine.press(s, k); return Engine.display(s) }
    function test_calculator_engine() {
        compare(press(["1", "2", "+", "3", "="]), "15")
        compare(press(["2", "+", "3", "×", "4", "="]), "20")       // immediate execution
        compare(press(["9", "÷", "0", "="]), "Error")
        compare(press(["5", "±"]), "-5")
        compare(press(["2", "0", "0", "+", "1", "0", "%"]), "20")   // percent of accumulator
        compare(press(["3", "x²"]), "9")
        compare(press(["5", "x!"]), "120")
        compare(press(["1", "2", "⌫"]), "1")
        compare(press(["(", "2", "+", "3", ")", "×", "2", "="]), "10")
        compare(press(["1", ".", ".", "5"]), "1.5")
    }
}
