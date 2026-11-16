// SPDX-License-Identifier: GPL-3.0-or-later
// Safe arithmetic evaluator for Spotlight (recursive descent, no eval()).
// Supports + - * / % ^, parentheses, unary minus, implicit multiplication
// like "2pi" or "3(4+1)", constants pi/e and common functions.
.pragma library

var FUNCS = {
    sqrt: Math.sqrt, abs: Math.abs, sin: Math.sin, cos: Math.cos, tan: Math.tan,
    asin: Math.asin, acos: Math.acos, atan: Math.atan, ln: Math.log, log: Math.log10,
    exp: Math.exp, round: Math.round, floor: Math.floor, ceil: Math.ceil
};
var CONSTS = { pi: Math.PI, e: Math.E, "π": Math.PI };

function tokenize(s) {
    var out = [], i = 0;
    while (i < s.length) {
        var c = s[i];
        if (/\s/.test(c)) { i++; continue; }
        if (/[0-9.]/.test(c)) {
            var j = i;
            while (j < s.length && /[0-9._]/.test(s[j])) j++;
            if (j < s.length && /[eE]/.test(s[j]) && /[0-9+-]/.test(s[j + 1] || "")) {
                j++; if (/[+-]/.test(s[j])) j++;
                while (j < s.length && /[0-9]/.test(s[j])) j++;
            }
            var num = Number(s.substring(i, j).replace(/_/g, ""));
            if (isNaN(num)) return null;
            out.push({ t: "num", v: num }); i = j; continue;
        }
        if (/[a-zπ]/i.test(c)) {
            var k = i;
            while (k < s.length && /[a-zπ0-9]/i.test(s[k])) k++;
            out.push({ t: "id", v: s.substring(i, k).toLowerCase() }); i = k; continue;
        }
        if ("+-*/%^()×÷−".indexOf(c) >= 0) {
            out.push({ t: "op", v: c === "×" ? "*" : c === "÷" ? "/" : c === "−" ? "-" : c }); i++; continue;
        }
        return null;
    }
    return out;
}

function Parser(tokens) { this.t = tokens; this.p = 0; }
Parser.prototype.peek = function () { return this.t[this.p]; };
Parser.prototype.next = function () { return this.t[this.p++]; };
Parser.prototype.isOp = function (v) { var k = this.peek(); return k && k.t === "op" && k.v === v; };

Parser.prototype.expr = function () {
    var v = this.term();
    while (this.isOp("+") || this.isOp("-")) {
        var op = this.next().v, r = this.term();
        v = op === "+" ? v + r : v - r;
    }
    return v;
};
Parser.prototype.term = function () {
    var v = this.unary();
    for (;;) {
        if (this.isOp("*") || this.isOp("/") || this.isOp("%")) {
            var op = this.next().v, r = this.unary();
            if (op === "*") v *= r; else if (op === "/") v /= r; else v %= r;
        } else {
            // implicit multiplication: number/paren followed by id or "("
            var k = this.peek();
            if (k && (k.t === "id" || k.t === "num" || (k.t === "op" && k.v === "("))) v *= this.unary();
            else break;
        }
    }
    return v;
};
Parser.prototype.unary = function () {
    if (this.isOp("-")) { this.next(); return -this.unary(); }
    if (this.isOp("+")) { this.next(); return this.unary(); }
    return this.power();
};
Parser.prototype.power = function () {
    var base = this.primary();
    if (this.isOp("^")) { this.next(); return Math.pow(base, this.unary()); }
    return base;
};
Parser.prototype.primary = function () {
    var k = this.next();
    if (!k) throw "end";
    if (k.t === "num") return k.v;
    if (k.t === "op" && k.v === "(") {
        var v = this.expr();
        if (!this.isOp(")")) throw "paren";
        this.next();
        return v;
    }
    if (k.t === "id") {
        if (CONSTS.hasOwnProperty(k.v)) return CONSTS[k.v];
        if (FUNCS.hasOwnProperty(k.v)) {
            var arg;
            if (this.isOp("(")) { this.next(); arg = this.expr(); if (!this.isOp(")")) throw "paren"; this.next(); }
            else arg = this.unary();
            return FUNCS[k.v](arg);
        }
    }
    throw "token";
};

// Returns a finite number, or null when `text` is not a calculation.
function evaluate(text) {
    var s = String(text).trim().replace(/=$/, "");
    if (!s.length) return null;
    // Require at least one operator or function so plain numbers/words
    // don't produce a "calculation" result.
    if (!/[+\-*/%^×÷−(]/.test(s) && !/^(sqrt|sin|cos|tan|ln|log|exp|abs)/i.test(s)) return null;
    var toks = tokenize(s);
    if (!toks || !toks.length) return null;
    try {
        var p = new Parser(toks);
        var v = p.expr();
        if (p.p !== toks.length) return null;
        return isFinite(v) ? v : null;
    } catch (e) {
        return null;
    }
}

function format(v) {
    if (v === null) return "";
    if (Math.abs(v) >= 1e15 || (Math.abs(v) < 1e-9 && v !== 0)) return v.toExponential(6);
    var r = Math.round(v * 1e10) / 1e10;
    return r.toLocaleString(undefined, { maximumFractionDigits: 10 });
}
