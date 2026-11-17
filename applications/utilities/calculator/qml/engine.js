// SPDX-License-Identifier: GPL-3.0-or-later
// Immediate-execution calculator state machine (pure functions, unit tested).
.pragma library

function initial() { return { display: "0", acc: null, pendingOp: "", fresh: true, error: false, parens: [] } }

function num(s) { return parseFloat(s.display) }
function fmt(v) {
    if (!isFinite(v)) return "Error"
    if (Math.abs(v) >= 1e15 || (Math.abs(v) < 1e-9 && v !== 0)) return v.toExponential(8).replace(/\.?0+e/, "e")
    return String(Math.round(v * 1e10) / 1e10)
}
function apply(a, op, b) {
    switch (op) {
    case "+": return a + b
    case "−": return a - b
    case "×": return a * b
    case "÷": return b === 0 ? NaN : a / b
    case "xʸ": return Math.pow(a, b)
    case "mod": return a % b
    case "EE": return a * Math.pow(10, b)
    }
    return b
}
function fact(n) { if (n < 0 || n > 170 || n !== Math.floor(n)) return NaN; let r = 1; for (let i = 2; i <= n; ++i) r *= i; return r }

function press(s, k) {
    s = Object.assign({}, s, { parens: s.parens.slice() })
    if (s.error && k !== "AC") return s
    if (/^[0-9]$/.test(k)) {
        s.display = (s.fresh || s.display === "0") ? k : (s.display.length < 16 ? s.display + k : s.display)
        s.fresh = false
        return s
    }
    switch (k) {
    case ".": if (s.fresh) { s.display = "0."; s.fresh = false } else if (s.display.indexOf(".") < 0) s.display += "."; return s
    case "AC": return initial()
    case "⌫": if (!s.fresh) s.display = s.display.length > 1 ? s.display.slice(0, -1) : "0"; return s
    case "±": s.display = fmt(-num(s)); return s
    case "%": s.display = fmt(s.acc !== null && (s.pendingOp === "+" || s.pendingOp === "−") ? s.acc * num(s) / 100 : num(s) / 100); s.fresh = true; return s
    case "+": case "−": case "×": case "÷": case "xʸ": case "mod": case "EE":
        if (s.acc !== null && s.pendingOp && !s.fresh) s.acc = apply(s.acc, s.pendingOp, num(s))
        else if (s.acc === null || !s.pendingOp) s.acc = num(s)
        s.pendingOp = k; s.fresh = true; s.display = fmt(s.acc)
        if (s.display === "Error") s.error = true
        return s
    case "=":
        if (s.pendingOp && s.acc !== null) { s.display = fmt(apply(s.acc, s.pendingOp, num(s))); s.acc = null; s.pendingOp = "" }
        s.fresh = true; if (s.display === "Error") s.error = true
        return s
    case "(": s.parens.push({ acc: s.acc, op: s.pendingOp }); s.acc = null; s.pendingOp = ""; s.fresh = true; s.display = "0"; return s
    case ")":
        if (s.parens.length) {
            let v = s.pendingOp && s.acc !== null ? apply(s.acc, s.pendingOp, num(s)) : num(s)
            const outer = s.parens.pop(); s.acc = outer.acc; s.pendingOp = outer.op; s.display = fmt(v); s.fresh = true
        }
        return s
    }
    const unary = { "x²": x => x * x, "√": Math.sqrt, "sin": Math.sin, "cos": Math.cos, "tan": Math.tan, "ln": Math.log, "log": Math.log10,
                    "1/x": x => 1 / x, "x!": fact, "eˣ": Math.exp, "10ˣ": x => Math.pow(10, x), "|x|": Math.abs }
    if (unary[k]) { s.display = fmt(unary[k](num(s))); s.fresh = true; if (s.display === "Error") s.error = true; return s }
    if (k === "π") { s.display = fmt(Math.PI); s.fresh = true; return s }
    if (k === "e") { s.display = fmt(Math.E); s.fresh = true; return s }
    if (k === "Rand") { s.display = fmt(Math.random()); s.fresh = true; return s }
    return s
}
function display(s) { return s.display }
