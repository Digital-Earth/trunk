
(function(){
'use strict';
var svgNS = "http://www.w3.org/2000/svg";
/*!
 Transformation Matrix v2.0
 (c) Epistemex 2014-2015
 www.epistemex.com
 By Ken Fyrstenberg
 Contributions by leeoniya.
 License: MIT, header required.
 */

/**
 * 2D transformation matrix object initialized with identity matrix.
 *
 * The matrix can synchronize a canvas context by supplying the context
 * as an argument, or later apply current absolute transform to an
 * existing context.
 *
 * All values are handled as floating point values.
 *
 * @param {CanvasRenderingContext2D} [context] - Optional context to sync with Matrix
 * @prop {number} a - scale x
 * @prop {number} b - shear y
 * @prop {number} c - shear x
 * @prop {number} d - scale y
 * @prop {number} e - translate x
 * @prop {number} f - translate y
 * @prop {CanvasRenderingContext2D|null} [context=null] - set or get current canvas context
 * @constructor
 */

function Matrix(context) {

    var me = this;
    me._t = me.transform;

    me.a = me.d = 1;
    me.b = me.c = me.e = me.f = 0;

    me.props = [1,0,0,1,0,0];

    me.cssParts = ['matrix(','',')'];

    me.a1 = me.b1 = me.c1 = me.d1 = me.e1 = me.f1 = 0;

    me.context = context;

    me.cos = me.sin = 0;


    // reset canvas transformations (if any) to enable 100% sync.
    if (context) context.setTransform(1, 0, 0, 1, 0, 0);
}
Matrix.prototype = {

    /**
     * Concatenates transforms of this matrix onto the given child matrix and
     * returns a new matrix. This instance is used on left side.
     *
     * @param {Matrix} cm - child matrix to apply concatenation to
     * @returns {Matrix}
     */
    concat: function(cm) {
        return this.clone()._t(cm.a, cm.b, cm.c, cm.d, cm.e, cm.f);
    },

    /**
     * Flips the horizontal values.
     */
    flipX: function() {
        return this._t(-1, 0, 0, 1, 0, 0);
    },

    /**
     * Flips the vertical values.
     */
    flipY: function() {
        return this._t(1, 0, 0, -1, 0, 0);
    },

    /**
     * Reflects incoming (velocity) vector on the normal which will be the
     * current transformed x axis. Call when a trigger condition is met.
     *
     * NOTE: BETA, simple implementation
     *
     * @param {number} x - vector end point for x (start = 0)
     * @param {number} y - vector end point for y (start = 0)
     * @returns {{x: number, y: number}}
     */
    reflectVector: function(x, y) {

        var v = this.applyToPoint(0, 1),
            d = 2 * (v.x * x + v.y * y);

        x -= d * v.x;
        y -= d * v.y;

        return {x:x, y:y};
    },

    /**
     * Short-hand to reset current matrix to an identity matrix.
     */
    reset: function() {
        return this.setTransform(1, 0, 0, 1, 0, 0);
    },

    /**
     * Rotates current matrix accumulative by angle.
     * @param {number} angle - angle in radians
     */
    rotate: function(angle) {
        if(angle === 0){
            return this;
        }
        this.cos = Math.cos(angle);
        this.sin = Math.sin(angle);
        return this._t(this.cos, this.sin, -this.sin, this.cos, 0, 0);
    },

    /**
     * Converts a vector given as x and y to angle, and
     * rotates (accumulative).
     * @param x
     * @param y
     * @returns {*}
     */
    rotateFromVector: function(x, y) {
        return this.rotate(Math.atan2(y, x));
    },

    /**
     * Helper method to make a rotation based on an angle in degrees.
     * @param {number} angle - angle in degrees
     */
    rotateDeg: function(angle) {
        return this.rotate(angle * Math.PI / 180);
    },

    /**
     * Scales current matrix uniformly and accumulative.
     * @param {number} f - scale factor for both x and y (1 does nothing)
     */
    scaleU: function(f) {
        return this._t(f, 0, 0, f, 0, 0);
    },

    /**
     * Scales current matrix accumulative.
     * @param {number} sx - scale factor x (1 does nothing)
     * @param {number} sy - scale factor y (1 does nothing)
     */
    scale: function(sx, sy) {
        if(sx == 1 && sy == 1){
            return this;
        }
        return this._t(sx, 0, 0, sy, 0, 0);
    },

    /**
     * Scales current matrix on x axis accumulative.
     * @param {number} sx - scale factor x (1 does nothing)
     */
    scaleX: function(sx) {
        return this._t(sx, 0, 0, 1, 0, 0);
    },

    /**
     * Scales current matrix on y axis accumulative.
     * @param {number} sy - scale factor y (1 does nothing)
     */
    scaleY: function(sy) {
        return this._t(1, 0, 0, sy, 0, 0);
    },

    /**
     * Apply shear to the current matrix accumulative.
     * @param {number} sx - amount of shear for x
     * @param {number} sy - amount of shear for y
     */
    shear: function(sx, sy) {
        return this._t(1, sy, sx, 1, 0, 0);
    },

    /**
     * Apply shear for x to the current matrix accumulative.
     * @param {number} sx - amount of shear for x
     */
    shearX: function(sx) {
        return this._t(1, 0, sx, 1, 0, 0);
    },

    /**
     * Apply shear for y to the current matrix accumulative.
     * @param {number} sy - amount of shear for y
     */
    shearY: function(sy) {
        return this._t(1, sy, 0, 1, 0, 0);
    },

    /**
     * Apply skew to the current matrix accumulative.
     * @param {number} ax - angle of skew for x
     * @param {number} ay - angle of skew for y
     */
    skew: function(ax, ay) {
        return this.shear(Math.tan(ax), Math.tan(ay));
    },

    /**
     * Apply skew for x to the current matrix accumulative.
     * @param {number} ax - angle of skew for x
     */
    skewX: function(ax) {
        return this.shearX(Math.tan(ax));
    },

    /**
     * Apply skew for y to the current matrix accumulative.
     * @param {number} ay - angle of skew for y
     */
    skewY: function(ay) {
        return this.shearY(Math.tan(ay));
    },

    /**
     * Set current matrix to new absolute matrix.
     * @param {number} a - scale x
     * @param {number} b - shear y
     * @param {number} c - shear x
     * @param {number} d - scale y
     * @param {number} e - translate x
     * @param {number} f - translate y
     */
    setTransform: function(a, b, c, d, e, f) {
        this.props[0] = a;
        this.props[1] = b;
        this.props[2] = c;
        this.props[3] = d;
        this.props[4] = e;
        this.props[5] = f;
        return this._x();
    },

    /**
     * Translate current matrix accumulative.
     * @param {number} tx - translation for x
     * @param {number} ty - translation for y
     */
    translate: function(tx, ty) {
        return this._t(1, 0, 0, 1, tx, ty);
    },

    /**
     * Translate current matrix on x axis accumulative.
     * @param {number} tx - translation for x
     */
    translateX: function(tx) {
        return this._t(1, 0, 0, 1, tx, 0);
    },

    /**
     * Translate current matrix on y axis accumulative.
     * @param {number} ty - translation for y
     */
    translateY: function(ty) {
        return this._t(1, 0, 0, 1, 0, ty);
    },

    /**
     * Multiplies current matrix with new matrix values.
     * @param {number} a2 - scale x
     * @param {number} b2 - shear y
     * @param {number} c2 - shear x
     * @param {number} d2 - scale y
     * @param {number} e2 - translate x
     * @param {number} f2 - translate y
     */
    transform: function(a2, b2, c2, d2, e2, f2) {

        this.a1 = this.props[0];
        this.b1 = this.props[1];
        this.c1 = this.props[2];
        this.d1 = this.props[3];
        this.e1 = this.props[4];
        this.f1 = this.props[5];

        /* matrix order (canvas compatible):
         * ace
         * bdf
         * 001
         */
        this.props[0] = this.a1 * a2 + this.c1 * b2;
        this.props[1] = this.b1 * a2 + this.d1 * b2;
        this.props[2] = this.a1 * c2 + this.c1 * d2;
        this.props[3] = this.b1 * c2 + this.d1 * d2;
        this.props[4] = this.a1 * e2 + this.c1 * f2 + this.e1;
        this.props[5] = this.b1 * e2 + this.d1 * f2 + this.f1;

        return this._x();
    },

    /**
     * Divide this matrix on input matrix which must be invertible.
     * @param {Matrix} m - matrix to divide on (divisor)
     * @returns {Matrix}
     */
    divide: function(m) {

        if (!m.isInvertible())
            throw "Input matrix is not invertible";

        var im = m.inverse();

        return this._t(im.a, im.b, im.c, im.d, im.e, im.f);
    },

    /**
     * Divide current matrix on scalar value != 0.
     * @param {number} d - divisor (can not be 0)
     * @returns {Matrix}
     */
    divideScalar: function(d) {

        var me = this;
        me.a /= d;
        me.b /= d;
        me.c /= d;
        me.d /= d;
        me.e /= d;
        me.f /= d;

        return me._x();
    },

    /**
     * Get an inverse matrix of current matrix. The method returns a new
     * matrix with values you need to use to get to an identity matrix.
     * Context from parent matrix is not applied to the returned matrix.
     * @returns {Matrix}
     */
    inverse: function() {

        if (this.isIdentity()) {
            return new Matrix();
        }
        else if (!this.isInvertible()) {
            throw "Matrix is not invertible.";
        }
        else {
            var me = this,
                a = me.a,
                b = me.b,
                c = me.c,
                d = me.d,
                e = me.e,
                f = me.f,

                m = new Matrix(),
                dt = a * d - b * c;	// determinant(), skip DRY here...

            m.a = d / dt;
            m.b = -b / dt;
            m.c = -c / dt;
            m.d = a / dt;
            m.e = (c * f - d * e) / dt;
            m.f = -(a * f - b * e) / dt;

            return m;
        }
    },

    /**
     * Interpolate this matrix with another and produce a new matrix.
     * t is a value in the range [0.0, 1.0] where 0 is this instance and
     * 1 is equal to the second matrix. The t value is not constrained.
     *
     * Context from parent matrix is not applied to the returned matrix.
     *
     * Note: this interpolation is naive. For animation use the
     * intrpolateAnim() method instead.
     *
     * @param {Matrix} m2 - the matrix to interpolate with.
     * @param {number} t - interpolation [0.0, 1.0]
     * @param {CanvasRenderingContext2D} [context] - optional context to affect
     * @returns {Matrix} - new instance with the interpolated result
     */
    interpolate: function(m2, t, context) {

        var me = this,
            m = context ? new Matrix(context) : new Matrix();

        m.a = me.a + (m2.a - me.a) * t;
        m.b = me.b + (m2.b - me.b) * t;
        m.c = me.c + (m2.c - me.c) * t;
        m.d = me.d + (m2.d - me.d) * t;
        m.e = me.e + (m2.e - me.e) * t;
        m.f = me.f + (m2.f - me.f) * t;

        return m._x();
    },

    /**
     * Interpolate this matrix with another and produce a new matrix.
     * t is a value in the range [0.0, 1.0] where 0 is this instance and
     * 1 is equal to the second matrix. The t value is not constrained.
     *
     * Context from parent matrix is not applied to the returned matrix.
     *
     * Note: this interpolation method uses decomposition which makes
     * it suitable for animations (in particular where rotation takes
     * places).
     *
     * @param {Matrix} m2 - the matrix to interpolate with.
     * @param {number} t - interpolation [0.0, 1.0]
     * @param {CanvasRenderingContext2D} [context] - optional context to affect
     * @returns {Matrix} - new instance with the interpolated result
     */
    interpolateAnim: function(m2, t, context) {

        var me = this,
            m = context ? new Matrix(context) : new Matrix(),
            d1 = me.decompose(),
            d2 = m2.decompose(),
            rotation = d1.rotation + (d2.rotation - d1.rotation) * t,
            translateX = d1.translate.x + (d2.translate.x - d1.translate.x) * t,
            translateY = d1.translate.y + (d2.translate.y - d1.translate.y) * t,
            scaleX = d1.scale.x + (d2.scale.x - d1.scale.x) * t,
            scaleY = d1.scale.y + (d2.scale.y - d1.scale.y) * t
            ;

        m.translate(translateX, translateY);
        m.rotate(rotation);
        m.scale(scaleX, scaleY);

        return m._x();
    },

    /**
     * Decompose the current matrix into simple transforms using either
     * QR (default) or LU decomposition. Code adapted from
     * http://www.maths-informatique-jeux.com/blog/frederic/?post/2013/12/01/Decomposition-of-2D-transform-matrices
     *
     * The result must be applied in the following order to reproduce the current matrix:
     *
     *     QR: translate -> rotate -> scale -> skewX
     *     LU: translate -> skewY  -> scale -> skewX
     *
     * @param {boolean} [useLU=false] - set to true to use LU rather than QR algorithm
     * @returns {*} - an object containing current decomposed values (rotate, skew, scale, translate)
     */
    decompose: function(useLU) {

        var a = this.props[0],
            b = this.props[1],
            c = this.props[2],
            d = this.props[3],
            acos = Math.acos,
            atan = Math.atan,
            sqrt = Math.sqrt,
            pi = Math.PI,

            translate = {x: this.props[4], y: this.props[5]},
            rotation  = 0,
            scale     = {x: 1, y: 1},
            skew      = {x: 0, y: 0},

            determ = a * d - b * c;	// determinant(), skip DRY here...

        if (useLU) {
            if (a) {
                skew = {x:atan(c/a), y:atan(b/a)};
                scale = {x:a, y:determ/a};
            }
            else if (b) {
                rotation = pi * 0.5;
                scale = {x:b, y:determ/b};
                skew.x = atan(d/b);
            }
            else { // a = b = 0
                scale = {x:c, y:d};
                skew.x = pi * 0.25;
            }
        }
        else {
            // Apply the QR-like decomposition.
            if (a || b) {
                var r = sqrt(a*a + b*b);
                rotation = b > 0 ? acos(a/r) : -acos(a/r);
                scale = {x:r, y:determ/r};
                skew.x = atan((a*c + b*d) / (r*r));
            }
            else if (c || d) {
                var s = sqrt(c*c + d*d);
                rotation = pi * 0.5 - (d > 0 ? acos(-c/s) : -acos(c/s));
                scale = {x:determ/s, y:s};
                skew.y = atan((a*c + b*d) / (s*s));
            }
            else { // a = b = c = d = 0
                scale = {x:0, y:0};		// = invalid matrix
            }
        }

        return {
            scale    : scale,
            translate: translate,
            rotation : rotation,
            skew     : skew
        };
    },

    /**
     * Returns the determinant of the current matrix.
     * @returns {number}
     */
    determinant : function() {
        return this.a * this.d - this.b * this.c;
    },

    /**
     * Apply current matrix to x and y point.
     * Returns a point object.
     *
     * @param {number} x - value for x
     * @param {number} y - value for y
     * @returns {{x: number, y: number}} A new transformed point object
     */
    applyToPoint: function(x, y) {

        var me = this;

        return {
            x: x * this.props[0] + y * this.props[2] + this.props[4],
            y: x * this.props[1] + y * this.props[3] + this.props[5]
        };
        /*return {
         x: x * me.a + y * me.c + me.e,
         y: x * me.b + y * me.d + me.f
         };*/
    },
    applyToPointStringified: function(x, y) {
        return (x * this.props[0] + y * this.props[2] + this.props[4])+','+(x * this.props[1] + y * this.props[3] + this.props[5]);
    },

    /**
     * Apply current matrix to array with point objects or point pairs.
     * Returns a new array with points in the same format as the input array.
     *
     * A point object is an object literal:
     *
     * {x: x, y: y}
     *
     * so an array would contain either:
     *
     * [{x: x1, y: y1}, {x: x2, y: y2}, ... {x: xn, y: yn}]
     *
     * or
     * [x1, y1, x2, y2, ... xn, yn]
     *
     * @param {Array} points - array with point objects or pairs
     * @returns {Array} A new array with transformed points
     */
    applyToArray: function(points) {

        var i = 0, p, l,
            mxPoints = [];

        if (typeof points[0] === 'number') {

            l = points.length;

            while(i < l) {
                p = this.applyToPoint(points[i++], points[i++]);
                mxPoints.push(p.x, p.y);
            }
        }
        else {
            l = points.length;
            for(i = 0; i<l; i++) {
                mxPoints.push(this.applyToPoint(points[i].x, points[i].y));
            }
        }

        return mxPoints;
    },

    /**
     * Apply current matrix to a typed array with point pairs. Although
     * the input array may be an ordinary array, this method is intended
     * for more performant use where typed arrays are used. The returned
     * array is regardless always returned as a Float32Array.
     *
     * @param {*} points - (typed) array with point pairs
     * @param {boolean} [use64=false] - use Float64Array instead of Float32Array
     * @returns {*} A new typed array with transformed points
     */
    applyToTypedArray: function(points, use64) {

        var i = 0, p,
            l = points.length,
            mxPoints = use64 ? new Float64Array(l) : new Float32Array(l);

        while(i < l) {
            p = this.applyToPoint(points[i], points[i+1]);
            mxPoints[i++] = p.x;
            mxPoints[i++] = p.y;
        }

        return mxPoints;
    },

    /**
     * Apply to any canvas 2D context object. This does not affect the
     * context that optionally was referenced in constructor unless it is
     * the same context.
     * @param {CanvasRenderingContext2D} context
     */
    applyToContext: function(context) {
        var me = this;
        context.setTransform(me.a, me.b, me.c, me.d, me.e, me.f);
        return me;
    },

    /**
     * Returns true if matrix is an identity matrix (no transforms applied).
     * @returns {boolean} True if identity (not transformed)
     */
    isIdentity: function() {
        var me = this;
        return (me._q(me.a, 1) &&
            me._q(me.b, 0) &&
            me._q(me.c, 0) &&
            me._q(me.d, 1) &&
            me._q(me.e, 0) &&
            me._q(me.f, 0));
    },

    /**
     * Returns true if matrix is invertible
     * @returns {boolean}
     */
    isInvertible: function() {
        return !this._q(this.determinant(), 0);
    },

    /**
     * Test if matrix is valid.
     */
    isValid : function() {
        return !this._q(this.a * this.d, 0);
    },

    /**
     * Clones current instance and returning a new matrix.
     * @param {boolean} [noContext=false] don't clone context reference if true
     * @returns {Matrix}
     */
    clone : function(noContext) {
        var me = this,
            m = new Matrix();
        m.a = me.a;
        m.b = me.b;
        m.c = me.c;
        m.d = me.d;
        m.e = me.e;
        m.f = me.f;
        if (!noContext) m.context = me.context;

        return m;
    },

    /**
     * Compares current matrix with another matrix. Returns true if equal
     * (within epsilon tolerance).
     * @param {Matrix} m - matrix to compare this matrix with
     * @returns {boolean}
     */
    isEqual: function(m) {

        var me = this,
            q = me._q;

        return (q(me.a, m.a) &&
            q(me.b, m.b) &&
            q(me.c, m.c) &&
            q(me.d, m.d) &&
            q(me.e, m.e) &&
            q(me.f, m.f));
    },

    /**
     * Returns an array with current matrix values.
     * @returns {Array}
     */
    toArray: function() {
        return [this.props[0],this.props[1],this.props[2],this.props[3],this.props[4],this.props[5]];
    },

    /**
     * Generates a string that can be used with CSS `transform:`.
     * @returns {string}
     */
    toCSS: function() {
        this.cssParts[1] = this.props.join(',');
        return this.cssParts.join('');
        //return "matrix(" + this.a + ',' + this.b + ',' + this.c + ',' + this.d + ',' + this.e + ',' + this.f + ")";
    },

    /**
     * Returns a JSON compatible string of current matrix.
     * @returns {string}
     */
    toJSON: function() {
        var me = this;
        return '{"a":' + me.a + ',"b":' + me.b + ',"c":' + me.c + ',"d":' + me.d + ',"e":' + me.e + ',"f":' + me.f + '}';
    },

    /**
     * Returns a string with current matrix as comma-separated list.
     * @returns {string}
     */
    toString: function() {
        return "" + this.toArray();
    },

    /**
     * Compares floating point values with some tolerance (epsilon)
     * @param {number} f1 - float 1
     * @param {number} f2 - float 2
     * @returns {boolean}
     * @private
     */
    _q: function(f1, f2) {
        return Math.abs(f1 - f2) < 1e-14;
    },

    /**
     * Apply current absolute matrix to context if defined, to sync it.
     * @private
     */
    _x: function() {
        if (this.context)
            this.context.setTransform(this.a, this.b, this.c, this.d, this.e, this.f);
        return this;
    }
};
/**
 * Copyright 2014 Google Inc. All rights reserved.
 *
 * Use of this source code is governed by a BSD-style
 * license that can be found in the LICENSE file.
 *
 * @fileoverview Description of this file.
 *
 * A polyfill for HTML Canvas features, including
 * Path2D support.
 */
if (CanvasRenderingContext2D.prototype.ellipse === undefined) {
    CanvasRenderingContext2D.prototype.ellipse = function(x, y, radiusX, radiusY, rotation, startAngle, endAngle, antiClockwise) {
        this.save();
        this.translate(x, y);
        this.rotate(rotation);
        this.scale(radiusX, radiusY);
        this.arc(0, 0, 1, startAngle, endAngle, antiClockwise);
        this.restore();
    };
}
var BM_Path2D;
function BM_CanvasRenderingContext2D(renderer){
    this.renderer = renderer;
}
(function() {

    var canvasPrototype = CanvasRenderingContext2D.prototype;

    var bm_canvasPrototype = BM_CanvasRenderingContext2D.prototype;

    function Path_(arg) {
        this.ops_ = [];
        if (arg === undefined) {
            return;
        }
        if (typeof arg == 'string') {
            try {
                this.ops_ = parser.parse(arg);
            } catch(e) {
                // Treat an invalid SVG path as an empty path.
            }
        } else if (arg.hasOwnProperty('ops_')) {
            this.ops_ = arg.ops_.slice(0);
        } else {
            throw 'Error: ' + typeof arg + 'is not a valid argument to Path';
        }
    }

    // TODO(jcgregorio) test for arcTo and implement via something.


    // Path methods that map simply to the CanvasRenderingContext2D.
    var simple_mapping = [
        'closePath',
        'moveTo',
        'lineTo',
        'quadraticCurveTo',
        'bezierCurveTo',
        'rect',
        'arc',
        'arcTo',
        'ellipse'
    ];

    function createFunction(name) {
        return function() {
            var i, len = arguments.length;
            var args = [];
            for(i=0;i<len;i+=1){
                args.push(arguments[i]);
            }
            this.ops_.push({type: name, args: args});
        };
    }

    // Add simple_mapping methods to Path2D.
    for (var i=0; i<simple_mapping.length; i++) {
        var name = simple_mapping[i];
        Path_.prototype[name] = createFunction(name);
    }

    Path_.prototype.addPath = function(path, tr) {
        var hasTx = false;
        if (tr && (tr[0] != 1 || tr[1] !== 0 || tr[2] !== 0 || tr[3] != 1 || tr[4] !== 0 || tr[5] !== 0)) {

            hasTx = true;
            this.ops_.push({type: 'save', args: []});
            this.ops_.push({type: 'transform', args: [tr[0], tr[1], tr[2], tr[3], tr[4], tr[5]]});
        }
        this.ops_ = this.ops_.concat(path.ops_);
        if (hasTx) {
            this.ops_.push({type: 'restore', args: []});
        }
    };

    var original_fill = canvasPrototype.fill;
    var original_stroke = canvasPrototype.stroke;
    var original_clip = canvasPrototype.clip;

    // Replace methods on CanvasRenderingContext2D with ones that understand Path2D.
    bm_canvasPrototype.fill = function(arg) {
        if (arg instanceof Path_) {
            this.renderer.canvasContext.beginPath();
            for (var i = 0, len = arg.ops_.length; i < len; i++) {
                var op = arg.ops_[i];
                if(op.type == 'transform'){
                    this.renderer.ctxTransform(op.args);
                }else if(op.type == 'save'){
                    this.renderer.save();
                }else if(op.type == 'restore'){
                    this.renderer.restore();
                }else{
                    this.renderer.canvasContext[op.type].apply(this.renderer.canvasContext, op.args);
                }
            }
            len = arguments.length;
            var args = [];
            for(i=1;i<len;i+=1){
                args.push(arguments[i]);
            }
            original_fill.apply(this.renderer.canvasContext, args);
        } else {
            original_fill.apply(this.renderer.canvasContext, arguments);
        }
    };

    bm_canvasPrototype.stroke = function(arg) {
        if (arg instanceof Path_) {
            this.renderer.canvasContext.beginPath();
            for (var i = 0, len = arg.ops_.length; i < len; i++) {
                var op = arg.ops_[i];
                this.renderer.canvasContext[op.type].apply(this.renderer.canvasContext, op.args);
            }
            original_stroke.call(this.renderer.canvasContext);
        } else {
            original_stroke.call(this.renderer.canvasContext);
        }
    };

    bm_canvasPrototype.clip = function(arg) {
        if (arg instanceof Path_) {
            this.renderer.canvasContext.beginPath();
            for (var i = 0, len = arg.ops_.length; i < len; i++) {
                var op = arg.ops_[i];
                this.renderer.canvasContext[op.type].apply(this.renderer.canvasContext, op.args);
            }
            len = arguments.length;
            var args = [];
            for(i=1;i<len;i+=1){
                args.push(arguments[i]);
            }
            original_clip.apply(this.renderer.canvasContext, args);
        } else {
            original_clip.apply(this.renderer.canvasContext, arguments);
        }
    };

    // Set up externs.
    BM_Path2D = Path_;
})();
function matrixManagerFunction(){

    var mat = new Matrix();

    var returnMatrix3D = function(rX, rY, rZ, scaleX, scaleY, scaleZ, tX, tY, tZ) {

        var rotationXMatrix, rotationYMatrix, rotationZMatrix, s, scaleMatrix, transformationMatrix, translationMatrix;
        rotationXMatrix = $M([
            [1, 0, 0, 0],
            [0, Math.cos(rX), Math.sin(-rX), 0],
            [0, Math.sin(rX), Math.cos(rX), 0],
            [0, 0, 0, 1]]);

        rotationYMatrix = $M([
            [Math.cos(rY), 0, Math.sin(rY), 0],
            [0, 1, 0, 0],
            [Math.sin(-rY), 0, Math.cos(rY), 0],
            [0, 0, 0, 1]]);

        rotationZMatrix = $M([
            [Math.cos(rZ), Math.sin(-rZ), 0, 0],
            [Math.sin(rZ), Math.cos(rZ), 0, 0],
            [0, 0, 1, 0],
            [0, 0, 0, 1]]);


        scaleMatrix = $M([[scaleX, 0, 0, 0], [0, scaleY, 0, 0], [0, 0, scaleZ, 0], [0, 0, 0, 1]]);

        transformationMatrix = rotationXMatrix.x(rotationYMatrix).x(rotationZMatrix).x(scaleMatrix);
        transformationMatrix = transformationMatrix.transpose();
        translationMatrix = $M([[1, 0, 0, 0], [0, 1, 0, 0], [0, 0, 1, 0], [tX, tY, tZ, 1]]);
        transformationMatrix = transformationMatrix.x(translationMatrix); // Apply transformation matrix AFTER transposing rotation and scale

        s = "matrix3d(";
        s += transformationMatrix.e(1, 1).toFixed(5) + "," + transformationMatrix.e(1, 2).toFixed(5) + "," + transformationMatrix.e(1, 3).toFixed(5) + "," + transformationMatrix.e(1, 4).toFixed(5) + ",";
        s += transformationMatrix.e(2, 1).toFixed(5) + "," + transformationMatrix.e(2, 2).toFixed(5) + "," + transformationMatrix.e(2, 3).toFixed(5) + "," + transformationMatrix.e(2, 4).toFixed(5) + ",";
        s += transformationMatrix.e(3, 1).toFixed(5) + "," + transformationMatrix.e(3, 2).toFixed(5) + "," + transformationMatrix.e(3, 3).toFixed(5) + "," + transformationMatrix.e(3, 4).toFixed(5) + ",";
        s += transformationMatrix.e(4, 1).toFixed(5) + "," + transformationMatrix.e(4, 2).toFixed(5) + "," + transformationMatrix.e(4, 3).toFixed(5) + "," + transformationMatrix.e(4, 4).toFixed(5);
        s += ")";

        return s;
    };

    /*var returnMatrix2D = function(rX, scaleX, scaleY, tX, tY){
        var rotationMatrix,  s, scaleMatrix, transformationMatrix, translationMatrix;
        //cos(X), sin(X), -sin(X), cos(X)
        rotationMatrix = $M([
            [Math.cos(-rX), Math.sin(-rX), 0],
            [Math.sin(rX), Math.cos(-rX), 0],
            [0, 0, 1]
        ]);
        scaleMatrix = $M([[scaleX, 0, 0], [0, scaleY, 0], [0, 0, 1]]);
        transformationMatrix = rotationMatrix.x(scaleMatrix);
        transformationMatrix = transformationMatrix.transpose();
        translationMatrix = $M([[1, 0, 0], [0, 1, 0], [tX, tY, 1]]);
        transformationMatrix = transformationMatrix.x(translationMatrix);

        s = "matrix(";
        s += transformationMatrix.e(1, 1).toFixed(5) + "," + transformationMatrix.e(1, 2).toFixed(5) + ",";
        s += transformationMatrix.e(2, 1).toFixed(5) + "," + transformationMatrix.e(2, 2).toFixed(5) + ",";
        s += transformationMatrix.e(3, 1).toFixed(5) + "," + transformationMatrix.e(3, 2).toFixed(5);
        s += ")";

        return s;
    };*/

    var returnMatrix2D = function(rX, scaleX, scaleY, tX, tY){
        return mat.reset().translate(tX,tY).rotate(rX).scale(scaleX,scaleY).toCSS();
    };

    /*var returnMatrix2DArray = function(rX, scaleX, scaleY, tX, tY){
        var rotationMatrix,  scaleMatrix, transformationMatrix, translationMatrix;
        //cos(X), sin(X), -sin(X), cos(X)
        rotationMatrix = $M([
            [Math.cos(-rX), Math.sin(-rX), 0],
            [Math.sin(rX), Math.cos(-rX), 0],
            [0, 0, 1]
        ]);
        scaleMatrix = $M([[scaleX, 0, 0], [0, scaleY, 0], [0, 0, 1]]);
        transformationMatrix = rotationMatrix.x(scaleMatrix);
        transformationMatrix = transformationMatrix.transpose();
        translationMatrix = $M([[1, 0, 0], [0, 1, 0], [tX, tY, 1]]);
        transformationMatrix = transformationMatrix.x(translationMatrix);

        return [transformationMatrix.e(1, 1).toFixed(5),transformationMatrix.e(1, 2).toFixed(5)
            ,transformationMatrix.e(2, 1).toFixed(5),transformationMatrix.e(2, 2).toFixed(5)
            ,transformationMatrix.e(3, 1).toFixed(5),transformationMatrix.e(3, 2).toFixed(5)];
    };*/

    var returnMatrix2DArray = function(rX, scaleX, scaleY, tX, tY){
        return mat.reset().translate(tX,tY).rotate(rX).scale(scaleX,scaleY).toArray();
    };

    var get2DMatrix = function(animData){
        return returnMatrix2D(animData.r,animData.s[0],animData.s[1],animData.p[0],animData.p[1]);
    };

    var getMatrix = function(animData, isThreeD){
        if(!isThreeD){
            return returnMatrix2D(animData.tr.r[2],animData.tr.s[0],animData.tr.s[1],animData.tr.p[0],animData.tr.p[1]);
        }
        return returnMatrix3D(-animData.tr.r[0],animData.tr.r[1],animData.tr.r[2],animData.tr.s[0],animData.tr.s[1],animData.tr.s[2],animData.tr.p[0],animData.tr.p[1],animData.tr.p[2]);
    };

    var getMatrix2 = function(animData, isThreeD){
        if(!isThreeD){
            return returnMatrix2D(animData.r[2],animData.s[0],animData.s[1],animData.p[0],animData.p[1]);
        }
        return returnMatrix3D(-animData.r[0],animData.r[1],animData.r[2],animData.s[0],animData.s[1],animData.s[2],animData.p[0],animData.p[1],animData.p[2]);
    };

    var getMatrixArray = function(animData, isThreeD){
        if(!isThreeD){
            return returnMatrix2DArray(animData.r[2],animData.s[0],animData.s[1],animData.p[0],animData.p[1]);
        }
        return null;
    };

    return {
        get2DMatrix : get2DMatrix,
        getMatrix : getMatrix,
        getMatrix2 : getMatrix2,
        getMatrixArray : getMatrixArray,
        getMatrixArrayFromParams : returnMatrix2DArray,
        getMatrix2FromParams : returnMatrix2D
    };

}
var MatrixManager = matrixManagerFunction;
(function () {
    var lastTime = 0;
    var vendors = ['ms', 'moz', 'webkit', 'o'];
    for(var x = 0; x < vendors.length && !window.requestAnimationFrame; ++x) {
        window.requestAnimationFrame = window[vendors[x] + 'RequestAnimationFrame'];
        window.cancelAnimationFrame = window[vendors[x] + 'CancelAnimationFrame'] || window[vendors[x] + 'CancelRequestAnimationFrame'];
    }
    if(!window.requestAnimationFrame)
        window.requestAnimationFrame = function (callback, element) {
            var currTime = new Date().getTime();
            var timeToCall = Math.max(0, 16 - (currTime - lastTime));
            var id = window.setTimeout(function () {
                    callback(currTime + timeToCall);
                },
                timeToCall);
            lastTime = currTime + timeToCall;
            return id;
        };
    if(!window.cancelAnimationFrame)
        window.cancelAnimationFrame = function (id) {
            clearTimeout(id);
        };
}());
var subframeEnabled = false;
var cachedColors = {};

function styleDiv(element){
    element.style.position = 'absolute';
    element.style.top = 0;
    element.style.left = 0;
    element.style.display = 'block';
    element.style.verticalAlign = 'top';
    element.style.backfaceVisibility  = element.style.webkitBackfaceVisibility = 'hidden';
    //element.style.transformStyle = element.style.webkitTransformStyle = "preserve-3d";
    styleUnselectableDiv(element);
}

function styleUnselectableDiv(element){
    element.style.userSelect = 'none';
    element.style.MozUserSelect = 'none';
    element.style.webkitUserSelect = 'none';
    element.style.oUserSelect = 'none';

}

function randomString(length, chars){
    if(chars === undefined){
        chars = 'abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ1234567890';
    }
    var i;
    var result = '';
    for (i = length; i > 0; --i) result += chars[Math.round(Math.random() * (chars.length - 1))];
    return result;
}

function componentToHex(c) {
    var hex = c.toString(16);
    return hex.length == 1 ? '0' + hex : hex;
}

var rgbToHex = (function(){
    var colorMap = [];
    var i;
    var hex;
    for(i=0;i<256;i+=1){
        hex = i.toString(16);
        colorMap[i] = hex.length == 1 ? '0' + hex : hex;
    }

    return function(r, g, b) {
        if(r<0){
            r = 0;
        }
        if(g<0){
            g = 0;
        }
        if(b<0){
            b = 0;
        }
        return '#' + colorMap[r] + colorMap[g] + colorMap[b];
    };
}());

function fillToRgba(hex,alpha){
    if(!cachedColors[hex]){
        var result = /^#?([a-f\d]{2})([a-f\d]{2})([a-f\d]{2})$/i.exec(hex);
        cachedColors[hex] = parseInt(result[1], 16)+','+parseInt(result[2], 16)+','+parseInt(result[3], 16);
    }
    return 'rgba('+cachedColors[hex]+','+alpha+')';
}

var fillColorToString = (function(){

    var colorMap = [];
    return function(colorArr,alpha){
        if(!colorMap[colorArr[0]]){
            colorMap[colorArr[0]] = [];
        }
        if(!colorMap[colorArr[0]][colorArr[1]]){
            colorMap[colorArr[0]][colorArr[1]] = [];
        }
        if(!colorMap[colorArr[0]][colorArr[1]][colorArr[2]]){
            if(alpha !== undefined){
                colorArr[3] = alpha;
            }
            colorMap[colorArr[0]][colorArr[1]][colorArr[2]] = 'rgba('+colorArr.join(',')+')';
        }
        return colorMap[colorArr[0]][colorArr[1]][colorArr[2]];
    };
}());
/*(function(window){
    if (CanvasRenderingContext2D.prototype.ellipse == undefined) {
        CanvasRenderingContext2D.prototype.ellipse = function(x, y, radiusX, radiusY, rotation, startAngle, endAngle, antiClockwise) {
            this.save();
            this.translate(x, y);
            this.rotate(rotation);
            this.scale(radiusX, radiusY);
            this.arc(0, 0, 1, startAngle, endAngle, antiClockwise);
            this.restore();
        }
    }
    if(!supportsPath2D){
        function Path2D_(){
            this.commandArrays = [];
        }
        Path2D_.prototype.moveTo = function(x,y){
            this.commandArrays.push({type:'move',coord:[x,y]})
        };
        Path2D_.prototype.lineTo = function(x,y){
            this.commandArrays.push({type:'line',coord:[x,y]})
        };

        Path2D_.prototype.bezierCurveTo = function(cx,cy,cx2,cy2,x,y){
            this.commandArrays.push({type:'bezierCurve',coord:[cx,cy,cx2,cy2,x,y]})
        };

        Path2D_.prototype.drawToContext = function(ctx){
            var command,i, len = this.commandArrays.length;
            ctx.beginPath();
            for(i=0;i<len;i+=1){
                command = this.commandArrays[i];
                switch(command.type){
                    case "move":
                        ctx.moveTo(command.coord[0],command.coord[1]);
                        break;
                    case "line":
                        ctx.lineTo(command.coord[0],command.coord[1]);
                        break;
                    case "bezierCurve":
                        ctx.bezierCurveTo(command.coord[0],command.coord[1],command.coord[2],command.coord[3],command.coord[4],command.coord[5]);
                        break;
                }
            }
        };
        window.Path2D = Path2D_;
    }
}(window))*/
function createElement(parent,child,params){
    if(child){
        child.prototype = Object.create(parent.prototype);
        child.prototype.constructor = child;
        child.prototype.parent = parent.prototype;
    }else{
        var instance = Object.create(parent.prototype,params);
        var getType = {};
        if(instance && getType.toString.call(instance.init) === '[object Function]'){
            instance.init();
        }
        return instance;
    }
}

function defineDescriptor(o, propertyName, value, params){
    var descriptor = {
        writable : false,
        configurable : false,
        enumerable : false,
        value : value
    };
    if(params){
        for( var s in params){
            descriptor[s] = params[s];
        }
    }
    var getType = {};
    if(o && getType.toString.call(o) === '[object Function]'){
        o = o.prototype;
    }
    Object.defineProperty(o, propertyName, descriptor);
}

function defineAccessor(o, propertyName, params){
    var value;
    var accessor = {
        enumerable : false,
        configurable : false,
        get: function(){return value;},
        set: function(val){value = val;}
    };
    if(params){
        for( var s in params){
            accessor[s] = params[s];
        }
    }
    var getType = {};
    if(o && getType.toString.call(o) === '[object Function]'){
        o = o.prototype;
    }
    Object.defineProperty(o, propertyName, accessor);
}

function bezFunction(){

    var easingFunctions = [];
    var math = Math;

    function pointOnLine2D(x1,y1, x2,y2, x3,y3){
        return math.abs(((x2 - x1) * (y3 - y1)) - ((x3 - x1) * (y2 - y1))) < 0.00001;
    }

    function getEasingCurveByIndex(index){
        return easingFunctions[index].fnc;
    }

    function getEasingCurve(aa,bb,cc,dd,encodedFuncName) {
        if(!encodedFuncName){
            encodedFuncName = ('bez_' + aa+'_'+bb+'_'+cc+'_'+dd).replace(/\./g, 'p');
        }
        if(easingFunctions[encodedFuncName]){
            return easingFunctions[encodedFuncName];
        }
        var A0, B0, C0;
        var A1, B1, C1;
        easingFunctions[encodedFuncName] = function(x, t, b, c, d) {
            var tt = t/d;
            x = tt;
            var i = 0, z;
            while (++i < 14) {
                C0 = 3 * aa;
                B0 = 3 * (cc - aa) - C0;
                A0 = 1 - C0 - B0;
                z = (x * (C0 + x * (B0 + x * A0))) - tt;
                if (math.abs(z) < 1e-3) break;
                x -= z / (C0 + x * (2 * B0 + 3 * A0 * x));
            }
            C1 = 3 * bb;
            B1 = 3 * (dd - bb) - C1;
            A1 = 1 - C1 - B1;
            var polyB = x * (C1 + x * (B1 + x * A1));
            return c * polyB + b;
        };
        return easingFunctions[encodedFuncName];
    }
    var getBezierLength = (function(){
        var storedBezierCurves = {};

        return function(pt1,pt2,pt3,pt4){
            var bezierName = (pt1.join('_')+'_'+pt2.join('_')+'_'+pt3.join('_')+'_'+pt4.join('_')).replace(/\./g, 'p');
            if(storedBezierCurves[bezierName]){
                return storedBezierCurves[bezierName];
            }
            var curveSegments = 100;
            var k;
            var i, len;
            var ptCoord,perc,addedLength = 0;
            var ptDistance;
            var point = [],lastPoint = [];
            var lengthData = {
                addedLength: 0,
                segments: []
            };
            if((pt1[0] != pt2[0] || pt1[1] != pt2[1]) && pointOnLine2D(pt1[0],pt1[1],pt2[0],pt2[1],pt3[0],pt3[1]) && pointOnLine2D(pt1[0],pt1[1],pt2[0],pt2[1],pt4[0],pt4[1])){
                curveSegments = 2;
            }
            len = pt3.length;
            for(k=0;k<curveSegments;k+=1){
                perc = k/(curveSegments-1);
                ptDistance = 0;
                for(i=0;i<len;i+=1){
                    ptCoord = math.pow(1-perc,3)*pt1[i]+3*math.pow(1-perc,2)*perc*pt3[i]+3*(1-perc)*math.pow(perc,2)*pt4[i]+math.pow(perc,3)*pt2[i];
                    point[i] = ptCoord;
                    if(lastPoint[i] !== null){
                        ptDistance += math.pow(point[i] - lastPoint[i],2);
                    }
                    lastPoint[i] = point[i];
                }
                if(ptDistance){
                    ptDistance = math.sqrt(ptDistance);
                    addedLength += ptDistance;
                }
                lengthData.segments.push({l:addedLength,p:perc});
            }
            lengthData.addedLength = addedLength;
            storedBezierCurves[bezierName] = lengthData;
            return lengthData;
        };
    }());

    function BezierData(length){
        this.segmentLength = 0;
        this.points = new Array(length);
    }

    function PointData(partial,point){
        this.partialLength = partial;
        this.point = point;
    }

    function buildBezierData(keyData){
        var pt1 = keyData.s;
        var pt2 = keyData.e;
        var pt3 = keyData.to;
        var pt4 = keyData.ti;
        var curveSegments = 100;
        var k;
        var i, len;
        var ptCoord,perc,addedLength = 0;
        var ptDistance;
        var point,lastPoint = null;
        if((pt1[0] != pt2[0] || pt1[1] != pt2[1]) && pointOnLine2D(pt1[0],pt1[1],pt2[0],pt2[1],pt1[0]+pt3[0],pt1[1]+pt3[1]) && pointOnLine2D(pt1[0],pt1[1],pt2[0],pt2[1],pt2[0]+pt4[0],pt2[1]+pt4[1])){
            curveSegments = 2;
        }
        var bezierData = new BezierData(curveSegments);
        len = pt3.length;
        for(k=0;k<curveSegments;k+=1){
            point = [];
            perc = k/(curveSegments-1);
            ptDistance = 0;
            for(i=0;i<len;i+=1){
                ptCoord = math.pow(1-perc,3)*pt1[i]+3*math.pow(1-perc,2)*perc*(pt1[i] + pt3[i])+3*(1-perc)*math.pow(perc,2)*(pt2[i] + pt4[i])+math.pow(perc,3)*pt2[i];
                point.push(ptCoord);
                if(lastPoint !== null){
                    ptDistance += math.pow(point[i] - lastPoint[i],2);
                }
            }
            ptDistance = math.sqrt(ptDistance);
            addedLength += ptDistance;
            bezierData.points[k] = new PointData(ptDistance,point);
            lastPoint = point;
        }
        bezierData.segmentLength = addedLength;
        keyData.bezierData = bezierData;
    }

    function getDistancePerc(perc,bezierData){
        var segments = bezierData.segments;
        var len = segments.length;
        var initPos = math.floor((len-1)*perc);
        var lengthPos = perc*bezierData.addedLength;
        var lPerc = 0;
        if(lengthPos == segments[initPos].l){
            return segments[initPos].p;
        }else{
            var dir = segments[initPos].l > lengthPos ? -1 : 1;
            var flag = true;
            while(flag){
                if(segments[initPos].l <= lengthPos && segments[initPos+1].l > lengthPos){
                    lPerc = (lengthPos - segments[initPos].l)/(segments[initPos+1].l-segments[initPos].l);
                    flag = false;
                }else{
                    initPos += dir;
                }
                if(initPos < 0 || initPos >= len - 1){
                    flag = false;
                }
            }
            return segments[initPos].p + (segments[initPos+1].p - segments[initPos].p)*lPerc;
        }
    }

    function SegmentPoints(){
        this.pt1 = new Array(2);
        this.pt2 = new Array(2);
        this.pt3 = new Array(2);
        this.pt4 = new Array(2);
    }

    function getNewSegment(pt1,pt2,pt3,pt4,startPerc,endPerc, bezierData){
        var pts = new SegmentPoints();
        startPerc = startPerc < 0 ? 0 : startPerc;
        var t0 = getDistancePerc(startPerc,bezierData);
        endPerc = endPerc > 1 ? 1 : endPerc;
        var t1 = getDistancePerc(endPerc,bezierData);
        var i, len = pt1.length;
        var u0 = 1 - t0;
        var u1 = 1 - t1;
        for(i=0;i<len;i+=1){
            pts.pt1[i] =  u0*u0*u0* pt1[i] + (t0*u0*u0 + u0*t0*u0 + u0*u0*t0) * pt3[i] + (t0*t0*u0 + u0*t0*t0 + t0*u0*t0)* pt4[i] + t0*t0*t0* pt2[i];
            pts.pt3[i] = u0*u0*u1*pt1[i] + (t0*u0*u1 + u0*t0*u1 + u0*u0*t1)* pt3[i] + (t0*t0*u1 + u0*t0*t1 + t0*u0*t1)* pt4[i] + t0*t0*t1* pt2[i];
            pts.pt4[i] = u0*u1*u1* pt1[i] + (t0*u1*u1 + u0*t1*u1 + u0*u1*t1)* pt3[i] + (t0*t1*u1 + u0*t1*t1 + t0*u1*t1)* pt4[i] + t0*t1*t1* pt2[i];
            pts.pt2[i] = u1*u1*u1* pt1[i] + (t1*u1*u1 + u1*t1*u1 + u1*u1*t1)* pt3[i] + (t1*t1*u1 + u1*t1*t1 + t1*u1*t1)*pt4[i] + t1*t1*t1* pt2[i];
        }
        return pts;
    }

    return {
        getEasingCurve : getEasingCurve,
        getEasingCurveByIndex : getEasingCurveByIndex,
        getBezierLength : getBezierLength,
        getNewSegment : getNewSegment,
        buildBezierData : buildBezierData
    };
}

var bez = bezFunction();
function dataFunctionManager(){
    var frameRate = 0;
    var matrixInstance =  new MatrixManager();
    var degToRads = Math.PI/180;

    function convertNumericValue(data,multiplier){
        var i, len = data.length;
        var j, jLen;
        for(i=0;i<len;i+=1){
            if(data[i].t !== undefined){
                if(!!(data[i].s instanceof Array)){
                    jLen = data[i].s.length;
                    for(j=0;j<jLen;j+=1){
                        data[i].s[j] *= multiplier;
                        if(data[i].e !== undefined){
                            data[i].e[j] *= multiplier;
                        }
                    }
                }else if(data[i].s){
                    data[i].s *= multiplier;
                    if(data[i].e){
                        data[i].e *= multiplier;
                    }
                }
            }else{
                data[i] = data[i]*multiplier;
            }
        }
    }

    function completeLayers(layers, mainLayers){
        if(!mainLayers){
            mainLayers = layers;
        }
        var layerFrames, offsetFrame, layerData;
        var animArray, lastFrame;
        var i, len = layers.length;
        var j, jLen, k, kLen;
        for(i=0;i<len;i+=1){
            layerData = layers[i];
            layerFrames = layerData.outPoint - layerData.startTime;
            offsetFrame = layerData.startTime;
            //layerData.layerName = convertLayerNameToID(layerData.layerName);
            /*if(layerData.parent){
                layerData.parent = convertLayerNameToID(layerData.parent);
            }*/
            if(layerData.tt){
                layers[i-1].td = layerData.tt;
            }
            layerData.renderedFrame = {};
            layerData.renderedData = {};
            animArray = [];
            lastFrame = -1;
            if(layerData.tm){
                layerData.trmp = layerData.tm;
                var timeValues = new Array(layerFrames);
                for(j=0 ; j<layerFrames; j+=1){
                    timeValues.push(Math.floor(getInterpolatedValue(layerData.tm,j,offsetFrame)*frameRate));
                }
                layerData.tm = timeValues;
            }
            if(layerData.ks.o instanceof Array){
                convertNumericValue(layerData.ks.o,1/100);
            }else{
                layerData.ks.o /= 100;
            }
            if(layerData.ks.s instanceof Array){
                convertNumericValue(layerData.ks.s,1/100);
            }else{
                layerData.ks.s /= 100;
            }
            if(layerData.ks.r instanceof Array){
                convertNumericValue(layerData.ks.r,degToRads);
            }else{
                layerData.ks.r *= degToRads;
            }
            if(layerData.hasMask){
                var maskProps = layerData.masksProperties;
                jLen = maskProps.length;
                for(j=0;j<jLen;j+=1){
                    if(maskProps[j].pt.i){
                        convertPathsToAbsoluteValues(maskProps[j].pt);
                    }else{
                        kLen = maskProps[j].pt.length;
                        for(k=0;k<kLen;k+=1){
                            if(maskProps[j].pt[k].s){
                                convertPathsToAbsoluteValues(maskProps[j].pt[k].s[0]);
                            }
                            if(maskProps[j].pt[k].e){
                                convertPathsToAbsoluteValues(maskProps[j].pt[k].e[0]);
                            }
                        }
                    }
                }
            }
            if(layerData.ty=='PreCompLayer'){
                if(layerData.refId && !layerData.layers){
                    layerData.layers = findCompLayers(layerData.refId,mainLayers);
                }else{
                    completeLayers(layerData.layers,mainLayers);
                }
            }else if(layerData.ty == 'ShapeLayer'){
                completeShapes(layerData.shapes);
            }
        }
    }

    function findCompLayers(id,layers,mainLayers){
        if(!mainLayers){
            mainLayers = layers;
        }
        var i, len = layers.length;
        for(i=0;i<len;i+=1){
            if(layers[i].compId == id){
                if(!layers[i].layers){
                    layers[i].layers = findCompLayers(layers[i].refId,mainLayers);
                }
                return layers[i].layers;
            }
            if(layers[i].ty == 'PreCompLayer'){
                var elem = findCompLayers(id,layers[i].layers,mainLayers);
                if(elem){
                    return elem;
                }
            }
        }
        return null;
    }

    function completeShapes(arr,trimmedFlag){
        var i, len = arr.length;
        var j, jLen;
        var transformData;
        var isTrimmed = trimmedFlag ? trimmedFlag : false;
        for(i=len-1;i>=0;i-=1){
            arr[i].renderedData = [];
            if(arr[i].ty == 'tm'){
                isTrimmed = true;
            }
            if(arr[i].ty == 'fl' || arr[i].ty == 'st'){
                if(arr[i].o instanceof Array){
                    convertNumericValue(arr[i].o,1/100);
                }else{
                    arr[i].o *= 1/100;
                }
            }else if(arr[i].ty == 'sh'){
                arr[i].trimmed = isTrimmed;
                if(arr[i].ks.i){
                    convertPathsToAbsoluteValues(arr[i].ks);
                }else{
                    jLen = arr[i].ks.length;
                    for(j=0;j<jLen;j+=1){
                        if(arr[i].ks[j].s){
                            convertPathsToAbsoluteValues(arr[i].ks[j].s[0]);
                            convertPathsToAbsoluteValues(arr[i].ks[j].e[0]);
                        }
                    }
                }
            }else if(arr[i].ty == 'gr'){
                completeShapes(arr[i].it,isTrimmed);
            }else if(arr[i].ty == 'tr'){
                transformData = arr[i];
                transformData.renderedData = [];
                if(transformData.o instanceof Array){
                    convertNumericValue(transformData.o,1/100);
                }else{
                    transformData.o /= 100;
                }
                if(transformData.s instanceof Array){
                    convertNumericValue(transformData.s,1/100);
                }else{
                    transformData.s /= 100;
                }
                if(transformData.r instanceof Array){
                    convertNumericValue(transformData.r,degToRads);
                }else{
                    transformData.r *= degToRads;
                }
            }else if(arr[i].ty == 'rc' || arr[i].ty == 'el'){
                arr[i].trimmed = isTrimmed;
                arr[i].trimmed = true;
            }
        }
    }

    function convertPathsToAbsoluteValues(path){
        var i, len = path.i.length;
        for(i=0;i<len;i+=1){
            path.i[i][0] += path.v[i][0];
            path.i[i][1] += path.v[i][1];
            path.o[i][0] += path.v[i][0];
            path.o[i][1] += path.v[i][1];
        }
    }

    function completeData(animationData){
        animationData.__renderedFrames = new Array(Math.floor(animationData.animation.totalFrames));
        animationData.__renderFinished = false;
        frameRate = animationData.animation.frameRate;
        completeLayers(animationData.animation.layers);
    }

    function convertLayerNameToID(string){
        string = string.replace(/ /g,"_");
        string = string.replace(/-/g,"_");
        string = string.replace(/\./g,"_");
        string = string.replace(/\//g,"_");
        return string;
    }

    function getInterpolatedValue(keyframes, frameNum, offsetTime,paramArr,arrPos,arrLen){
        var keyData, nextKeyData,propertyArray,bezierData;
        var i;
        var len,paramCnt = 0;
        if(!(keyframes.length)){
            if(paramArr){
                while(arrLen>0){
                    paramArr[arrPos+paramCnt] = keyframes;
                    paramCnt += 1;
                    arrLen -= 1;
                }
            }
            return keyframes;
        }else if(keyframes[0].t === undefined){
            if(paramArr){
                while(arrLen>0){
                    paramArr[arrPos+paramCnt] = keyframes[paramCnt];
                    paramCnt += 1;
                    arrLen -= 1;
                }
            }
            return keyframes;
        }
        i = 0;
        len = keyframes.length- 1;
        var dir=1;
        var flag = true;

        while(flag){
            keyData = keyframes[i];
            nextKeyData = keyframes[i+1];
            if(i == len-1 && frameNum >= nextKeyData.t - offsetTime){
                break;
            }
            if((nextKeyData.t - offsetTime) > frameNum && dir == 1){
                break;
            }else if((nextKeyData.t - offsetTime) < frameNum && dir == -1){
                i += 1;
                keyData = keyframes[i];
                nextKeyData = keyframes[i+1];
                break;
            }
            if(i < len - 1 && dir == 1 || i > 0 && dir == -1){
                i += dir;
            }else{
                flag = false;
            }
        }

        if(keyData.to && !keyData.bezierData){
            bez.buildBezierData(keyData);
        }
        var k, kLen;
        var perc,jLen, j = 0;
        var fnc;
        if(!paramArr){
            propertyArray = [];
        }
        if(keyData.to){
            bezierData = keyData.bezierData;
            if(frameNum >= nextKeyData.t-offsetTime || frameNum < keyData.t-offsetTime){
                var ind = frameNum >= nextKeyData.t-offsetTime ? bezierData.points.length - 1 : 0;
                if(paramArr){
                    while(arrLen>0){
                        paramArr[arrPos+paramCnt] = bezierData.points[ind].point[paramCnt];
                        paramCnt += 1;
                        arrLen -= 1;
                    }
                }
                return bezierData.points[ind].point;
            }
            if(keyData.__fnct){
                fnc = keyData.__fnct;
            }else{
                fnc = bez.getEasingCurve(keyData.o.x,keyData.o.y,keyData.i.x,keyData.i.y,keyData.n);
                keyData.__fnct = fnc;
            }
            perc = fnc('',(frameNum)-(keyData.t-offsetTime),0,1,(nextKeyData.t-offsetTime)-(keyData.t-offsetTime));
            var distanceInLine = bezierData.segmentLength*perc;

            var segmentPerc;
            var addedLength = 0;
            dir = 1;
            flag = true;
            jLen = bezierData.points.length;
            while(flag){
                addedLength +=bezierData.points[j].partialLength*dir;
                if(distanceInLine === 0 || perc === 0 || j == bezierData.points.length - 1){
                    if(paramArr){
                        while(arrLen>0){
                            paramArr[arrPos+paramCnt] = bezierData.points[j].point[paramCnt];
                            paramCnt += 1;
                            arrLen -= 1;
                        }
                    }else{
                        propertyArray = bezierData.points[j].point;
                    }
                    break;
                }else if(distanceInLine > addedLength && distanceInLine < addedLength + bezierData.points[j+1].partialLength){
                    segmentPerc = (distanceInLine-addedLength)/(bezierData.points[j+1].partialLength);
                    if(paramArr){
                        while(arrLen>0){
                            paramArr[arrPos+paramCnt] = bezierData.points[j].point[paramCnt] + (bezierData.points[j+1].point[paramCnt] - bezierData.points[j].point[paramCnt])*segmentPerc;
                            paramCnt += 1;
                            arrLen -= 1;
                        }
                    }else{
                        kLen = bezierData.points[j].point.length;
                        for(k=0;k<kLen;k+=1){
                            propertyArray.push(bezierData.points[j].point[k] + (bezierData.points[j+1].point[k] - bezierData.points[j].point[k])*segmentPerc);
                        }
                    }
                    break;
                }
                if(j < jLen - 1 && dir == 1 || j > 0 && dir == -1){
                    j += dir;
                }else{
                    flag = false;
                }
            }
        }else{
            var outX,outY,inX,inY, isArray = false, keyValue;
            len = keyData.s.length;
            for(i=0;i<len;i+=1){
                if(keyData.h !== 1){
                    if(keyData.o.x instanceof Array){
                        isArray = true;
                        if(!keyData.__fnct){
                            keyData.__fnct = [];
                        }
                        if(!keyData.__fnct[i]){
                            outX = keyData.o.x[i] ? keyData.o.x[i] : keyData.o.x[0];
                            outY = keyData.o.y[i] ? keyData.o.y[i] : keyData.o.y[0];
                            inX = keyData.i.x[i] ? keyData.i.x[i] : keyData.i.x[0];
                            inY = keyData.i.y[i] ? keyData.i.y[i] : keyData.i.y[0];
                        }
                    }else{
                        isArray = false;
                        if(!keyData.__fnct) {
                            outX = keyData.o.x;
                            outY = keyData.o.y;
                            inX = keyData.i.x;
                            inY = keyData.i.y;
                        }
                    }
                    if(isArray){
                        if(keyData.__fnct[i]){
                            fnc = keyData.__fnct[i];
                        }else{
                            fnc = bez.getEasingCurve(outX,outY,inX,inY);
                            keyData.__fnct[i] = fnc;
                        }
                    }else{
                        if(keyData.__fnct){
                            fnc = keyData.__fnct;
                        }else{
                            fnc = bez.getEasingCurve(outX,outY,inX,inY);
                            keyData.__fnct = fnc;
                        }
                    }
                    if(frameNum >= nextKeyData.t-offsetTime){
                        perc = 1;
                    }else if(frameNum < keyData.t-offsetTime){
                        perc = 0;
                    }else{
                        perc = fnc('',(frameNum)-(keyData.t-offsetTime),0,1,(nextKeyData.t-offsetTime)-(keyData.t-offsetTime));
                    }
                }
                keyValue = keyData.h === 1 ? keyData.s[i] : keyData.s[i]+(keyData.e[i]-keyData.s[i])*perc;
                if(paramArr){
                    if(arrLen > 0) {
                        paramArr[arrPos + paramCnt] = keyValue;
                        paramCnt += 1;
                        arrLen -= 1;
                    }
                }else{
                    propertyArray.push(keyValue);
                }
            }
        }
        return propertyArray;
    }

    function getSegmentsLength(keyframes,closed){
        if(keyframes.__lengths){
            return;
        }
        keyframes.__lengths = [];
        keyframes.__totalLength = 0;
        var pathV = keyframes.v;
        var pathO = keyframes.o;
        var pathI = keyframes.i;
        var i, len = pathV.length;
        for(i=0;i<len-1;i+=1){
            keyframes.__lengths.push(bez.getBezierLength(pathV[i],pathV[i+1],pathO[i],pathI[i+1]));
            keyframes.__totalLength += keyframes.__lengths[i].addedLength;
        }
        if(closed){
            keyframes.__lengths.push(bez.getBezierLength(pathV[i],pathV[0],pathO[i],pathI[0]));
            keyframes.__totalLength += keyframes.__lengths[i].addedLength;
        }
    }

    function interpolateShape(shapeData, frameNum, offsetTime, renderType, isMask, trimData){
        var isTrimmed = trimData && trimData.length > 0;
        var pathData = {};
        pathData.closed = isMask ? shapeData.cl : shapeData.closed;
        var keyframes = isMask ? shapeData.pt : shapeData.ks;
        if(keyframes.v){
            if(!isTrimmed){
                pathData.pathNodes = keyframes;
            }else{
                pathData.pathNodes = trimPath(keyframes,pathData.closed, trimData, false);
            }
            return pathData;
        }else{
            var j,jLen, k, kLen;
            var coordsIData,coordsOData,coordsVData;
            if(frameNum < keyframes[0].t-offsetTime || frameNum > keyframes[keyframes.length - 1].t-offsetTime){
                var ob, pos, stored, key;
                if(frameNum < keyframes[0].t-offsetTime){
                    key = '__minValue';
                    pos = 0;
                    stored = keyframes.__minValue;
                    ob = keyframes[pos].s;
                }else{
                    key = '__maxValue';
                    pos = keyframes.length - 2;
                    stored = keyframes.__maxValue;
                    ob = keyframes[pos].e;
                }
                if(!stored){
                    jLen = keyframes[pos].s[0].i.length;
                    shapeData = {
                        i: new Array(jLen),
                        o: new Array(jLen),
                        v: new Array(jLen)
                    };
                    for(j=0;j<jLen;j+=1){
                        kLen = keyframes[pos].s[0].i[j].length;
                        coordsIData = new Array(kLen);
                        coordsOData = new Array(kLen);
                        coordsVData = new Array(kLen);
                        for(k=0;k<kLen;k+=1){
                            coordsIData[k] = ob[0].i[j][k];
                            coordsOData[k] = ob[0].o[j][k];
                            coordsVData[k] = ob[0].v[j][k];
                        }
                        shapeData.i[j] = coordsIData;
                        shapeData.o[j] = coordsOData;
                        shapeData.v[j] = coordsVData;
                    }
                    keyframes[key] = shapeData;
                    stored = shapeData;
                }
                pathData.pathNodes = isTrimmed ? trimPath(stored,pathData.closed, trimData, false) : stored;
                return pathData;
            }else{
                var i = 0;
                var len = keyframes.length- 1;
                var dir = 1;
                var flag = true;
                var keyData,nextKeyData;

                while(flag){
                    keyData = keyframes[i];
                    nextKeyData = keyframes[i+1];
                    if((nextKeyData.t - offsetTime) > frameNum && dir == 1){
                        break;
                    }
                    if(i < len - 1 && dir == 1 || i > 0 && dir == -1){
                        i += dir;
                    }else{
                        flag = false;
                    }
                }

                var perc;
                if(keyData.h !== 1){
                    var fnc;
                    if(keyData.__fnct){
                        fnc = keyData.__fnct;
                    }else{
                        fnc = bez.getEasingCurve(keyData.o.x,keyData.o.y,keyData.i.x,keyData.i.y);
                        keyData.__fnct = fnc;
                    }
                    if(frameNum >= nextKeyData.t-offsetTime){
                        perc = 1;
                    }else if(frameNum < keyData.t-offsetTime){
                        perc = 0;
                    }else{
                        perc = fnc('',(frameNum)-(keyData.t-offsetTime),0,1,(nextKeyData.t-offsetTime)-(keyData.t-offsetTime));
                    }
                }
                if(keyData.h === 1 && keyData.__hValue){
                    shapeData = keyData.__hValue;
                }else{
                    jLen = keyData.s[0].i.length;
                    shapeData = {
                        i: new Array(jLen),
                        o: new Array(jLen),
                        v: new Array(jLen)
                    };
                    for(j=0;j<jLen;j+=1){
                        kLen = keyData.s[0].i[j].length;
                        coordsIData = new Array(kLen);
                        coordsOData = new Array(kLen);
                        coordsVData = new Array(kLen);
                        for(k=0;k<kLen;k+=1){
                            if(keyData.h === 1){
                                coordsIData[k] = keyData.s[0].i[j][k];
                                coordsOData[k] = keyData.s[0].o[j][k];
                                coordsVData[k] = keyData.s[0].v[j][k];
                            }else{
                                coordsIData[k] = keyData.s[0].i[j][k]+(keyData.e[0].i[j][k]-keyData.s[0].i[j][k])*perc;
                                coordsOData[k] = keyData.s[0].o[j][k]+(keyData.e[0].o[j][k]-keyData.s[0].o[j][k])*perc;
                                coordsVData[k] = keyData.s[0].v[j][k]+(keyData.e[0].v[j][k]-keyData.s[0].v[j][k])*perc;
                            }
                        }
                        shapeData.i[j] = coordsIData;
                        shapeData.o[j] = coordsOData;
                        shapeData.v[j] = coordsVData;
                        if(keyData.h === 1){
                            keyData.__hValue = shapeData;
                        }
                    }
                }
                pathData.pathNodes = isTrimmed ? trimPath(shapeData,pathData.closed, trimData, false) : shapeData;
                return pathData;
            }
        }
    }

    var trimPath = (function(){

        var pathStarted = false;
        var pathString = '';
        var nextI,nextV,nextO, stops;
        var nextLengths;
        var nextTotalLength;
        var segmentCount;
        function addSegment(pt1,pt2,pt3,pt4, lengthData){
            nextO[segmentCount] = pt2;
            nextI[segmentCount+1] = pt3;
            nextV[segmentCount+1] = pt4;
            if(!pathStarted){
                pathString += " M"+pt1.join(',');
                pathStarted = true;
                stops[segmentCount] = pt1;
            }else{
                nextV[segmentCount] = pt1;
            }
            pathString += " C"+pt2.join(',') + " "+pt3.join(',') + " "+pt4.join(',');
            //nextLengths[segmentCount] = lengthData;
            segmentCount+=1;
            //nextTotalLength += lengthData.addedLength;
        }

        return function trimPath_(paths,closed, trimData, stringifyFlag){
            getSegmentsLength(paths,closed);
            var j, jLen = trimData.length;
            var finalPaths = paths;
            nextV = nextI = nextO = stops = null;
            var pathV,pathO,pathI;
            var k, kLen;
            for(j=jLen-1;j>=0;j-=1){
                var segments = [];
                var o = (trimData[j].o%360)/360;
                if(o === 0 && trimData[j].s === 0 && trimData[j].e == 100){
                    continue;
                }
                pathString = '';
                pathStarted = false;
                nextI = [];
                nextO = [];
                nextV = [];
                stops = [];
                nextLengths = [];
                nextTotalLength = 0;
                if(o < 0){
                    o += 1;
                }
                var s = trimData[j].s/100 + o;
                var e = (trimData[j].e/100) + o;
                if(s == e){
                    if(stringifyFlag){
                        return '';
                    }else{
                        return {};
                    }
                }
                if(s>e){
                    var _s = s;
                    s = e;
                    e = _s;
                }
                if(e <= 1){
                    segments.push({s:finalPaths.__totalLength*s,e:finalPaths.__totalLength*e});
                }else if(s >= 1){
                    segments.push({s:finalPaths.__totalLength*(s-1),e:finalPaths.__totalLength*(e-1)});
                }else{
                    segments.push({s:finalPaths.__totalLength*s,e:finalPaths.__totalLength});
                    segments.push({s:0,e:finalPaths.__totalLength*(e-1)});
                }

                pathV=[];
                pathO=[];
                pathI=[];

                var lengths;
                pathV = finalPaths.v;
                pathO = finalPaths.o;
                pathI = finalPaths.i;
                lengths = finalPaths.__lengths;
                kLen = pathV.length;
                var addedLength = 0, segmentLength = 0;
                var i, len = segments.length;
                var segment;
                segmentCount = 0;
                for(i=0;i<len;i+=1){
                    addedLength = 0;
                    for(k=1;k<kLen;k++){
                        segmentLength = lengths[k-1].addedLength;
                        if(addedLength + segmentLength < segments[i].s){
                            addedLength += segmentLength;
                            continue;
                        }else if(addedLength > segments[i].e){
                            break;
                        }
                        if(segments[i].s <= addedLength && segments[i].e >= addedLength + segmentLength){
                            addSegment(pathV[k-1],pathO[k-1],pathI[k],pathV[k],lengths[k-1]);
                        }else{
                            segment = bez.getNewSegment(pathV[k-1],pathV[k],pathO[k-1],pathI[k], (segments[i].s - addedLength)/segmentLength,(segments[i].e - addedLength)/segmentLength, lengths[k-1]);
                            addSegment(segment.pt1,segment.pt3,segment.pt4,segment.pt2/*,bez.getBezierLength(segment.pt1,segment.pt4,segment.pt2,segment.pt3)*/);
                        }
                        addedLength += segmentLength;
                    }
                    if(closed !== false){
                        if(addedLength <= segments[i].e){
                            segmentLength = lengths[k-1].addedLength;
                            if(segments[i].s <= addedLength && segments[i].e >= addedLength + segmentLength){
                                addSegment(pathV[k-1],pathO[k-1],pathI[0],pathV[0],lengths[k-1]);
                            }else{
                                segment = bez.getNewSegment(pathV[k-1],pathV[0],pathO[k-1],pathI[0], (segments[i].s - addedLength)/segmentLength,(segments[i].e - addedLength)/segmentLength, lengths[k-1]);
                                addSegment(segment.pt1,segment.pt3,segment.pt4,segment.pt2/*,bez.getBezierLength(segment.pt1,segment.pt4,segment.pt2,segment.pt3)*/);
                            }
                        }
                    }else{
                        pathStarted = false;
                    }
                }
                closed = false;
            }
            if(!nextV){
                pathV = finalPaths.v;
                pathO = finalPaths.o;
                pathI = finalPaths.i;
                stops = [];
            }else{
                pathV = nextV;
                pathO = nextO;
                pathI = nextI;
            }
            kLen = pathV.length;
            if(stringifyFlag){
                pathString = '';
                for(k=1;k<kLen;k++){
                    if(stops[k-1]){
                        pathString += "M"+stops[k-1].join(',');
                    }else if(k == 1){
                        pathString += "M"+pathV[0].join(',');
                    }
                    pathString += " C"+pathO[k-1].join(',') + " "+pathI[k].join(',') + " "+pathV[k].join(',');
                }
                if(closed !== false){
                    pathString += " C"+pathO[k-1].join(',') + " "+pathI[0].join(',') + " "+pathV[0].join(',');
                }
                return pathString;
            }else{
                return {
                    i: pathI,
                    o: pathO,
                    v: pathV,
                    s: stops,
                    c: closed
                };
            }
        };
    }());

    var mtParams = [0,1,1,0,0];
    function iterateLayers(layers, frameNum,renderType){
        var dataOb;
        var maskProps;
        var timeRemapped;

        var offsettedFrameNum, i, len, renderedData;
        var j, jLen = layers.length, item;
        for(j=0;j<jLen;j+=1){
            item = layers[j];
            offsettedFrameNum = frameNum - item.startTime;
            dataOb = {};
            dataOb.a = getInterpolatedValue(item.ks.a,offsettedFrameNum, item.startTime);
            dataOb.o = getInterpolatedValue(item.ks.o,offsettedFrameNum, item.startTime);
            if(item.ks.p.s){
                getInterpolatedValue(item.ks.p.x,offsettedFrameNum, item.startTime,mtParams,3,1);
                getInterpolatedValue(item.ks.p.y,offsettedFrameNum, item.startTime,mtParams,4,1);
            }else{
                getInterpolatedValue(item.ks.p,offsettedFrameNum, item.startTime,mtParams,3,2);
            }
            getInterpolatedValue(item.ks.r,offsettedFrameNum, item.startTime,mtParams,0,1);
            getInterpolatedValue(item.ks.s,offsettedFrameNum, item.startTime,mtParams,1,2);
            renderedData = {};
            renderedData.an = {
                tr: dataOb
            };
            renderedData.an.matrixArray = matrixInstance.getMatrixArrayFromParams(mtParams[0],mtParams[1],mtParams[2],mtParams[3],mtParams[4]);
            item.renderedData[offsettedFrameNum] = renderedData;
            if(item.hasMask){
                maskProps = item.masksProperties;
                len = maskProps.length;
                for(i=0;i<len;i+=1){
                    if(!maskProps[i].paths){
                        maskProps[i].paths = [];
                        maskProps[i].opacity = [];
                    }

                    maskProps[i].paths[offsettedFrameNum] = interpolateShape(maskProps[i],offsettedFrameNum, item.startTime,renderType,true);
                    maskProps[i].opacity[offsettedFrameNum] = getInterpolatedValue(maskProps[i].o,offsettedFrameNum, item.startTime);
                    maskProps[i].opacity[offsettedFrameNum] = maskProps[i].opacity[offsettedFrameNum] instanceof Array ? maskProps[i].opacity[offsettedFrameNum][0]/100 : maskProps[i].opacity[offsettedFrameNum]/100;
                }
            }
            if((frameNum < item.inPoint || frameNum > item.outPoint)){
               continue;
            }
            if(item.ty == 'PreCompLayer'){
                timeRemapped = item.tm ? item.tm[offsettedFrameNum] < 0 ? 0 : offsettedFrameNum >= item.tm.length ? item.tm[item.tm.length - 1] :  item.tm[offsettedFrameNum] : offsettedFrameNum;
                if(timeRemapped === undefined){
                    timeRemapped = getInterpolatedValue(item.trmp,offsettedFrameNum, 0)[0]*frameRate;
                    item.tm[offsettedFrameNum] = timeRemapped;
                }
                iterateLayers(item.layers,timeRemapped,renderType);
            }else if(item.ty == 'ShapeLayer'){
                iterateShape(item.shapes,offsettedFrameNum,item.startTime,renderType);
            }
        }
    }

    function convertRectToPath(pos,size,round, d){
        round = Math.min(size[0],size[1],round/2);
        var nextV = new Array(8);
        var nextI = new Array(8);
        var nextO = new Array(8);
        var cPoint = round/2;
        //round *= 1;

        if(d === 2) {

            nextV[0] = [pos[0]+size[0]/2,pos[1]-size[1]/2+round];
            nextO[0] = nextV[0];
            nextI[0] = [pos[0]+size[0]/2,pos[1]-size[1]/2+cPoint];

            nextV[1] = [pos[0]+size[0]/2,pos[1]+size[1]/2-round];
            nextO[1] = [pos[0]+size[0]/2,pos[1]+size[1]/2-cPoint];
            nextI[1] = nextV[1];

            nextV[2] = [pos[0]+size[0]/2-round,pos[1]+size[1]/2];
            nextO[2] = nextV[2];
            nextI[2] = [pos[0]+size[0]/2-cPoint,pos[1]+size[1]/2];

            nextV[3] = [pos[0]-size[0]/2+round,pos[1]+size[1]/2];
            nextO[3] = [pos[0]-size[0]/2+cPoint,pos[1]+size[1]/2];
            nextI[3] = nextV[3];

            nextV[4] = [pos[0]-size[0]/2,pos[1]+size[1]/2-round];
            nextO[4] = nextV[4];
            nextI[4] = [pos[0]-size[0]/2,pos[1]+size[1]/2-cPoint];

            nextV[5] = [pos[0]-size[0]/2,pos[1]-size[1]/2+round];
            nextO[5] = [pos[0]-size[0]/2,pos[1]-size[1]/2+cPoint];
            nextI[5] = nextV[5];

            nextV[6] = [pos[0]-size[0]/2+round,pos[1]-size[1]/2];
            nextO[6] = nextV[6];
            nextI[6] = [pos[0]-size[0]/2+cPoint,pos[1]-size[1]/2];

            nextV[7] = [pos[0]+size[0]/2-round,pos[1]-size[1]/2];
            nextO[7] = [pos[0]+size[0]/2-cPoint,pos[1]-size[1]/2];
            nextI[7] = nextV[7];
        }else{
            nextV[0] = [pos[0]+size[0]/2,pos[1]-size[1]/2+round];
            nextO[0] = [pos[0]+size[0]/2,pos[1]-size[1]/2+cPoint];
            nextI[0] = nextV[0];

            nextV[1] = [pos[0]+size[0]/2-round,pos[1]-size[1]/2];
            nextO[1] = nextV[1];
            nextI[1] = [pos[0]+size[0]/2-cPoint,pos[1]-size[1]/2];

            nextV[2] = [pos[0]-size[0]/2+round,pos[1]-size[1]/2];
            nextO[2] = [pos[0]-size[0]/2+cPoint,pos[1]-size[1]/2];
            nextI[2] = nextV[2];

            nextV[3] = [pos[0]-size[0]/2,pos[1]-size[1]/2+round];
            nextO[3] = nextV[3];
            nextI[3] = [pos[0]-size[0]/2,pos[1]-size[1]/2+cPoint];

            nextV[4] = [pos[0]-size[0]/2,pos[1]+size[1]/2-round];
            nextO[4] = [pos[0]-size[0]/2,pos[1]+size[1]/2-cPoint];
            nextI[4] = nextV[4];

            nextV[5] = [pos[0]-size[0]/2+round,pos[1]+size[1]/2];
            nextO[5] = nextV[5];
            nextI[5] = [pos[0]-size[0]/2+cPoint,pos[1]+size[1]/2];

            nextV[6] = [pos[0]+size[0]/2-round,pos[1]+size[1]/2];
            nextO[6] = [pos[0]+size[0]/2-cPoint,pos[1]+size[1]/2];
            nextI[6] = nextV[6];

            nextV[7] = [pos[0]+size[0]/2,pos[1]+size[1]/2-round];
            nextO[7] = nextV[7];
            nextI[7] = [pos[0]+size[0]/2,pos[1]+size[1]/2-cPoint];

        }


        return {v:nextV,o:nextO,i:nextI,c:true};
    }

    function iterateShape(arr,offsettedFrameNum,startTime,renderType,addedTrim){
        var i, len = arr.length;
        var shapeItem;
        var fillColor, fillOpacity;
        var elmPos,elmSize,elmRound;
        var strokeColor,strokeOpacity,strokeWidth;
        if(!addedTrim){
            addedTrim = [];
        }
        var trimS,trimE,trimO;
        var j, jLen;
        for(i=len-1;i>=0;i-=1){
            shapeItem = arr[i];
            if(shapeItem.ty == 'sh'){
                shapeItem.renderedData[offsettedFrameNum] = {
                    path: interpolateShape(shapeItem,offsettedFrameNum, startTime,renderType,false,addedTrim)
                };
            }else if(shapeItem.ty == 'fl'){
                fillColor = getInterpolatedValue(shapeItem.c,offsettedFrameNum, startTime);
                fillOpacity = getInterpolatedValue(shapeItem.o,offsettedFrameNum, startTime);
                shapeItem.renderedData[offsettedFrameNum] = {
                    opacity:  fillOpacity instanceof Array ? fillOpacity[0] : fillOpacity
                };
                if(renderType == 'canvas'){
                    roundColor(fillColor);
                    shapeItem.renderedData[offsettedFrameNum].color = fillColor;
                }else{
                    shapeItem.renderedData[offsettedFrameNum].color = rgbToHex(Math.round(fillColor[0]),Math.round(fillColor[1]),Math.round(fillColor[2]));
                }
            }else if(shapeItem.ty == 'rc'){
                elmPos = getInterpolatedValue(shapeItem.p,offsettedFrameNum, startTime);
                elmSize = getInterpolatedValue(shapeItem.s,offsettedFrameNum, startTime);
                elmRound = getInterpolatedValue(shapeItem.r,offsettedFrameNum, startTime);
                if(!shapeItem.trimmed){
                    shapeItem.renderedData[offsettedFrameNum] = {
                        position : elmPos,
                        size : elmSize,
                        roundness : elmRound
                    };
                }else{
                    shapeItem.renderedData[offsettedFrameNum] = {
                        path: {
                            closed: true
                        }
                    };
                    shapeItem.renderedData[offsettedFrameNum].path.pathNodes = trimPath(convertRectToPath(elmPos,elmSize,elmRound,shapeItem.d),true, addedTrim, false);
                }
            }else if(shapeItem.ty == 'el'){
                elmPos = getInterpolatedValue(shapeItem.p,offsettedFrameNum, startTime);
                elmSize = getInterpolatedValue(shapeItem.s,offsettedFrameNum, startTime);
                shapeItem.renderedData[offsettedFrameNum] = {
                    p : elmPos,
                    size : elmSize
                };
                if(renderType == 'svg'){

                    var pathNodes = {
                        v: new Array(4),
                        i:new Array(4),
                        o:new Array(4)
                    };
                    if(shapeItem.d !== 2 && shapeItem.d !== 3){
                        pathNodes.v[0] = [elmPos[0],elmPos[1]-elmSize[1]/2];
                        pathNodes.i[0] = [elmPos[0] - (elmSize[0]/2)*0.55,elmPos[1] - elmSize[1]/2];
                        pathNodes.o[0] = [elmPos[0] + (elmSize[0]/2)*0.55,elmPos[1] - elmSize[1]/2];
                        pathNodes.v[1] = [elmPos[0] + elmSize[0]/2,elmPos[1]];
                        pathNodes.i[1] = [elmPos[0] + (elmSize[0]/2),elmPos[1] - (elmSize[1]/2)*0.55];
                        pathNodes.o[1] = [elmPos[0] + (elmSize[0]/2),elmPos[1] + (elmSize[1]/2)*0.55];
                        pathNodes.v[2] = [elmPos[0],elmPos[1]+elmSize[1]/2];
                        pathNodes.i[2] = [elmPos[0] + (elmSize[0]/2)*0.55,elmPos[1] + (elmSize[1]/2)];
                        pathNodes.o[2] = [elmPos[0] - (elmSize[0]/2)*0.55,elmPos[1] + (elmSize[1]/2)];
                        pathNodes.v[3] = [elmPos[0] - elmSize[0]/2,elmPos[1]];
                        pathNodes.i[3] = [elmPos[0] - (elmSize[0]/2),elmPos[1] + (elmSize[1]/2)*0.55];
                        pathNodes.o[3] = [elmPos[0] - (elmSize[0]/2),elmPos[1] - (elmSize[1]/2)*0.55];
                    }else{
                        pathNodes.v[0] = [elmPos[0],elmPos[1]-elmSize[1]/2];
                        pathNodes.o[0] = [elmPos[0] - (elmSize[0]/2)*0.55,elmPos[1] - elmSize[1]/2];
                        pathNodes.i[0] = [elmPos[0] + (elmSize[0]/2)*0.55,elmPos[1] - elmSize[1]/2];
                        pathNodes.v[1] = [elmPos[0] - elmSize[0]/2,elmPos[1]];
                        pathNodes.o[1] = [elmPos[0] - (elmSize[0]/2),elmPos[1] + (elmSize[1]/2)*0.55];
                        pathNodes.i[1] = [elmPos[0] - (elmSize[0]/2),elmPos[1] - (elmSize[1]/2)*0.55];
                        pathNodes.v[2] = [elmPos[0],elmPos[1]+elmSize[1]/2];
                        pathNodes.o[2] = [elmPos[0] + (elmSize[0]/2)*0.55,elmPos[1] + (elmSize[1]/2)];
                        pathNodes.i[2] = [elmPos[0] - (elmSize[0]/2)*0.55,elmPos[1] + (elmSize[1]/2)];
                        pathNodes.v[3] = [elmPos[0] + elmSize[0]/2,elmPos[1]];
                        pathNodes.o[3] = [elmPos[0] + (elmSize[0]/2),elmPos[1] - (elmSize[1]/2)*0.55];
                        pathNodes.i[3] = [elmPos[0] + (elmSize[0]/2),elmPos[1] + (elmSize[1]/2)*0.55];
                    }

                    if(!shapeItem.trimmed){
                        shapeItem.renderedData[offsettedFrameNum].path = {pathNodes:pathNodes};
                        shapeItem.closed = true;
                    }else{
                        shapeItem.renderedData[offsettedFrameNum] = {
                            path: {
                                closed: true
                            }
                        };
                        shapeItem.renderedData[offsettedFrameNum].path.pathNodes = trimPath(pathNodes,true, addedTrim, false);
                        shapeItem.closed = true;
                    }
                }
            }else if(shapeItem.ty == 'st'){
                strokeColor = getInterpolatedValue(shapeItem.c,offsettedFrameNum, startTime);
                strokeOpacity = getInterpolatedValue(shapeItem.o,offsettedFrameNum, startTime);
                strokeWidth = getInterpolatedValue(shapeItem.w,offsettedFrameNum, startTime);
                shapeItem.renderedData[offsettedFrameNum] = {
                    opacity : strokeOpacity instanceof Array ? strokeOpacity[0] : strokeOpacity,
                    width : strokeWidth instanceof Array ? strokeWidth[0] : strokeWidth
                };
                if(shapeItem.d){
                    var dashes = [];
                    jLen = shapeItem.d.length;
                    var val;
                    for(j=0;j<jLen;j+=1){
                        val = getInterpolatedValue(shapeItem.d[j].v,offsettedFrameNum, startTime);
                        dashes.push({
                            v : val instanceof Array ? val[0] : val,
                            n : shapeItem.d[j].n
                        });
                    }
                    shapeItem.renderedData[offsettedFrameNum].dashes = dashes;
                }
                if(renderType == 'canvas'){
                    roundColor(strokeColor);
                    shapeItem.renderedData[offsettedFrameNum].color = strokeColor;
                }else{
                    shapeItem.renderedData[offsettedFrameNum].color = rgbToHex(Math.round(strokeColor[0]),Math.round(strokeColor[1]),Math.round(strokeColor[2]));
                }
            }else if(shapeItem.ty == 'tr'){
                shapeItem.renderedData[offsettedFrameNum] = {
                    a : getInterpolatedValue(shapeItem.a,offsettedFrameNum, startTime),
                    o : getInterpolatedValue(shapeItem.o,offsettedFrameNum, startTime)
                };
                getInterpolatedValue(shapeItem.s,offsettedFrameNum, startTime,mtParams,1,2);
                getInterpolatedValue(shapeItem.r,offsettedFrameNum, startTime,mtParams,0,1);
                getInterpolatedValue(shapeItem.p,offsettedFrameNum, startTime,mtParams,3,2);
                shapeItem.renderedData[offsettedFrameNum].mtArr = matrixInstance.getMatrixArrayFromParams(mtParams[0],mtParams[1],mtParams[2],mtParams[3],mtParams[4]);
            }else if(shapeItem.ty == 'tm'){
                trimS = getInterpolatedValue(shapeItem.s,offsettedFrameNum, startTime);
                trimE = getInterpolatedValue(shapeItem.e,offsettedFrameNum, startTime);
                trimO = getInterpolatedValue(shapeItem.o,offsettedFrameNum, startTime);
                var trimData = {
                    s: trimS,
                    e: trimE,
                    o: trimO
                };
                addedTrim.push(trimData);
                shapeItem.renderedData[offsettedFrameNum] = trimData;
                /*var currentStrimS = addedTrim.s;
                var currentStrimE = addedTrim.e;
                addedTrim.o += trimData.o;
                addedTrim.s = currentStrimS + (currentStrimE - currentStrimS)*(trimData.s/100);
                addedTrim.e = currentStrimE - (currentStrimE - currentStrimS)*(trimData.e/100);*/
            }else if(shapeItem.ty == 'gr'){
                iterateShape(shapeItem.it,offsettedFrameNum,startTime,renderType,addedTrim);
            }
        }
    }

    function roundColor(arr){
        var i, len = arr.length;
        for(i=0;i<len ;i+=1){
            arr[i] = Math.round(arr[i]);
        }
    }

    function prerenderFrames(animationData,num){
        var totalFrames = 1;
        while(totalFrames > 0){
            num += 1;
            if(num >= Math.floor(animationData.animation.totalFrames)){
                animationData.__renderFinished = true;
                break;
            }
            if(!animationData.__renderedFrames[num]){
                renderFrame(animationData,num);
                totalFrames -= 1;
            }
        }
    }

    function renderFrame(animationData,num){
        if(animationData.__renderedFrames[num]==2){
            if(!animationData.__renderFinished){
                prerenderFrames(animationData,num);
            }
            return;
        }
        frameRate = animationData.animation.frameRate;
        animationData.__renderedFrames[num] = 2;
        iterateLayers(animationData.animation.layers, num, animationData._animType);
    }

    function populateLayers(layers, num, rendered){
        var i, len = layers.length, j, jLen;
        var offsettedFrameNum, timeRemapped;
        var shapes;
        for(i=0;i<len;i+=1){
            if(rendered[i] === ''){
                continue;
            }
            offsettedFrameNum = num - layers[i].startTime;
            layers[i].renderedData[offsettedFrameNum] = rendered[i];
            if(layers[i].ty == 'PreCompLayer'){
                timeRemapped = layers[i].tm ? layers[i].tm[offsettedFrameNum] < 0 ? 0 : offsettedFrameNum >= layers[i].tm.length ? layers[i].tm[layers[i].tm.length - 1] : layers[i].tm[offsettedFrameNum] : offsettedFrameNum;
                populateLayers(layers[i].layers,timeRemapped,rendered.renderedArray);
            }else if(layers[i].ty == 'ShapeLayer'){
                shapes = layers[i].shapes;
                jLen = shapes.length;
                for(j=0;j<jLen;j+=1){
                    shapes[j].renderedData[offsettedFrameNum] = rendered[i].shapes[j];
                }
            }
        }
    }

    var moduleOb = {};
    moduleOb.completeData = completeData;
    moduleOb.renderFrame = renderFrame;

    return moduleOb;
}

var dataManager = dataFunctionManager();
function SVGRenderer(animationItem){
    this.animationItem = animationItem;
    this.layers = null;
    this.lastFrame = -1;
    this.globalData = {
        frameNum: -1
    };
    this.elements = [];
    this.destroyed = false;
}

SVGRenderer.prototype.buildItems = function(layers,parentContainer,elements){
    var count = 0, i, len = layers.length;
    if(!elements){
        elements = this.elements;
    }
    if(!parentContainer){
        parentContainer = this.animationItem.container;
    }
    for (i = len - 1; i >= 0; i--) {
        if (layers[i].ty == 'StillLayer') {
            count++;
            elements[i] = this.createImage(layers[i],parentContainer);
        } else if (layers[i].ty == 'PreCompLayer') {
            elements[i] = this.createComp(layers[i],parentContainer);
            var elems = [];
            this.buildItems(layers[i].layers,elements[i].getDomElement(),elems);
            elements[i].setElements(elems);
        } else if (layers[i].ty == 'SolidLayer') {
            elements[i] = this.createSolid(layers[i],parentContainer);
        } else if (layers[i].ty == 'ShapeLayer') {
            elements[i] = this.createShape(layers[i],parentContainer);
        } else if (layers[i].ty == 'TextLayer') {
            elements[i] = this.createText(layers[i],parentContainer);
        }else{
            elements[i] = this.createBase(layers[i],parentContainer);
            //console.log('NO TYPE: ',layers[i]);
        }
        if(layers[i].td){
            elements[i+1].setMatte(elements[i].layerId);
        }
        //NullLayer
    }
};

SVGRenderer.prototype.createBase = function (data,parentContainer) {
    return new BaseElement(data, parentContainer,this.globalData);
};

SVGRenderer.prototype.createShape = function (data,parentContainer) {
    return new IShapeElement(data, parentContainer,this.globalData);
};

SVGRenderer.prototype.createText = function (data,parentContainer) {
    return new ITextElement(data, parentContainer,this.globalData);
};

SVGRenderer.prototype.createImage = function (data,parentContainer) {
    return new IImageElement(data, parentContainer,this.globalData);
};

SVGRenderer.prototype.createComp = function (data,parentContainer) {
    return new ICompElement(data, parentContainer,this.globalData);

};

SVGRenderer.prototype.createSolid = function (data,parentContainer) {
    return new ISolidElement(data, parentContainer,this.globalData);
};

SVGRenderer.prototype.configAnimation = function(animData){
    this.animationItem.container = document.createElementNS(svgNS,'svg');
    this.animationItem.container.setAttribute('xmlns','http://www.w3.org/2000/svg');
    this.animationItem.container.setAttribute('width',animData.animation.compWidth);
    this.animationItem.container.setAttribute('height',animData.animation.compHeight);
    this.animationItem.container.setAttribute('viewBox','0 0 '+animData.animation.compWidth+' '+animData.animation.compHeight);
    this.animationItem.container.setAttribute('preserveAspectRatio','xMidYMid meet');
    this.animationItem.container.style.width = '100%';
    this.animationItem.container.style.height = '100%';
    this.animationItem.container.style.transform = 'translate3d(0,0,0)';
    this.animationItem.container.style.transformOrigin = this.animationItem.container.style.mozTransformOrigin = this.animationItem.container.style.webkitTransformOrigin = this.animationItem.container.style['-webkit-transform'] = "0px 0px 0px";
    this.animationItem.wrapper.appendChild(this.animationItem.container);
    //Mask animation
    var defs = document.createElementNS(svgNS, 'defs');
    this.globalData.defs = defs;
    this.animationItem.container.appendChild(defs);
    this.globalData.getAssetData = this.animationItem.getAssetData.bind(this.animationItem);
    this.globalData.getPath = this.animationItem.getPath.bind(this.animationItem);
    this.globalData.elementLoaded = this.animationItem.elementLoaded.bind(this.animationItem);
    this.globalData.compSize = {
        w: animData.animation.compWidth,
        h: animData.animation.compHeight
    };
    var maskElement = document.createElementNS(svgNS, 'clipPath');
    var rect = document.createElementNS(svgNS,'rect');
    rect.setAttribute('width',animData.animation.compWidth);
    rect.setAttribute('height',animData.animation.compHeight);
    rect.setAttribute('x',0);
    rect.setAttribute('y',0);
    var maskId = 'animationMask_'+randomString(10);
    maskElement.setAttribute('id', maskId);
    maskElement.appendChild(rect);
    var maskedElement = document.createElementNS(svgNS,'g');
    maskedElement.setAttribute("clip-path", "url(#"+maskId+")");
    this.animationItem.container.appendChild(maskedElement);
    defs.appendChild(maskElement);
    this.animationItem.container = maskedElement;
    this.layers = animData.animation.layers;
};

SVGRenderer.prototype.buildStage = function (container, layers,elements) {
    var i, len = layers.length, layerData;
    if(!elements){
        elements = this.elements;
    }
    for (i = len - 1; i >= 0; i--) {
        layerData = layers[i];
        if (layerData.parent !== undefined) {
            this.buildItemParenting(layerData,elements[i],layers,layerData.parent,elements);
        }

        if (layerData.ty == 'PreCompLayer') {
            this.buildStage(elements[i].getComposingElement(), layerData.layers, elements[i].getElements());
        }
    }
};
SVGRenderer.prototype.buildItemParenting = function (layerData,element,layers,parentName,elements) {
    if(!layerData.parents){
        layerData.parents = [];
    }
    var i=0, len = layers.length;
    while(i<len){
        if(layers[i].ind == parentName){
            element.getHierarchy().push(elements[i]);
            if(layers[i].parent !== undefined){
                this.buildItemParenting(layerData,element,layers,layers[i].parent,elements);
            }
        }
        i += 1;
    }
};

SVGRenderer.prototype.destroy = function () {
    this.animationItem.wrapper.innerHTML = '';
    this.animationItem.container = null;
    this.globalData.defs = null;
    var i, len = this.layers.length;
    for (i = 0; i < len; i++) {
        this.elements[i].destroy();
    }
    this.elements.length = 0;
    this.destroyed = true;
};

SVGRenderer.prototype.updateContainerSize = function () {
};

SVGRenderer.prototype.renderFrame = function(num){
    if(this.lastFrame == num || this.destroyed){
        return;
    }
    this.lastFrame = num;
    this.globalData.frameNum = num;
    var i, len = this.layers.length;
    for (i = 0; i < len; i++) {
        this.elements[i].prepareFrame(num - this.layers[i].startTime);
    }
    for (i = 0; i < len; i++) {
        this.elements[i].renderFrame(num - this.layers[i].startTime);
    }
};
function CanvasRenderer(animationItem, config){
    this.animationItem = animationItem;
    this.renderConfig = config ? config : {
        clearCanvas: true,
        context: null,
        scaleMode: 'fit'
    };
    this.lastFrame = -1;
    this.globalData = {
        frameNum: -1
    };
    this.contextData = {
        saved : new Array(15),
        savedOp: new Array(15),
        cArrPos : 0,
        cTr : new Matrix(),
        cO : 1
    };
    var i, len = 15;
    for(i=0;i<len;i+=1){
        this.contextData.saved[i] = new Array(6);
    }
    this.elements = [];
}

CanvasRenderer.prototype.buildItems = function(layers,elements){
    if(!elements){
        elements = this.elements;
    }
    var count = 0, i, len = layers.length;
    for (i = 0; i < len; i++) {
        if (layers[i].ty == 'StillLayer') {
            count++;
            elements.push(this.createImage(layers[i]));
        } else if (layers[i].ty == 'PreCompLayer') {
            elements.push(this.createComp(layers[i]));
            var elems = [];
            this.buildItems(layers[i].layers,elems);
            elements[elements.length - 1].setElements(elems);
        } else if (layers[i].ty == 'SolidLayer') {
            elements.push(this.createSolid(layers[i]));
        } else if (layers[i].ty == 'ShapeLayer') {
            elements.push(this.createShape(layers[i]));
        } else if (layers[i].ty == 'TextLayer') {
            elements.push(this.createText(layers[i]));
        }else{
            elements.push(this.createBase(layers[i]));
            //console.log('NO TYPE: ',layers[i]);
        }
    }
};

CanvasRenderer.prototype.createBase = function (data) {
    return new CVBaseElement(data, this.globalData);
};

CanvasRenderer.prototype.createShape = function (data) {
    return new CVShapeElement(data, this.globalData);
};

CanvasRenderer.prototype.createText = function (data) {
    return new CVTextElement(data, this.globalData);
};

CanvasRenderer.prototype.createImage = function (data) {
    return new CVImageElement(data, this.globalData);
};

CanvasRenderer.prototype.createComp = function (data) {
    return new CVCompElement(data, this.globalData);
};

CanvasRenderer.prototype.createSolid = function (data) {
    return new CVSolidElement(data, this.globalData);
};

CanvasRenderer.prototype.ctxTransform = function(props){
    if(!this.renderConfig.clearCanvas){
        this.canvasContext.transform(props[0],props[1],props[2],props[3],props[4],props[5]);
        return;
    }
    this.contextData.cTr.transform(props[0],props[1],props[2],props[3],props[4],props[5]);
    var trProps = this.contextData.cTr.props;
    this.canvasContext.setTransform(trProps[0],trProps[1],trProps[2],trProps[3],trProps[4],trProps[5]);
    ///this.canvasContext.transform(props[0],props[1],props[2],props[3],props[4],props[5]);
};

CanvasRenderer.prototype.ctxOpacity = function(op){
    if(!this.renderConfig.clearCanvas){
        this.canvasContext.globalAlpha *= op < 0 ? 0 : op;
        return;
    }
    this.contextData.cO *= op < 0 ? 0 : op;
     this.canvasContext.globalAlpha = this.contextData.cO;
    ///this.canvasContext.globalAlpha = this.canvasContext.globalAlpha * op;
};

CanvasRenderer.prototype.reset = function(){
    if(!this.renderConfig.clearCanvas){
        this.canvasContext.restore();
        return;
    }
    this.contextData.cArrPos = 0;
    this.contextData.cTr.reset();
    this.contextData.cO = 1;
};

CanvasRenderer.prototype.save = function(actionFlag){
    if(!this.renderConfig.clearCanvas){
        this.canvasContext.save();
        return;
    }
    if(actionFlag){
        this.canvasContext.save();
    }
    var props = this.contextData.cTr.props;
    if(this.contextData.saved[this.contextData.cArrPos] === null || this.contextData.saved[this.contextData.cArrPos] === undefined){
        this.contextData.saved[this.contextData.cArrPos] = new Array(6);
    }
    var i, len = 6,arr = this.contextData.saved[this.contextData.cArrPos];
    for(i=0;i<len;i+=1){
        arr[i] = props[i];
    }
    this.contextData.savedOp[this.contextData.cArrPos] = this.contextData.cO;
    this.contextData.cArrPos += 1;
};

CanvasRenderer.prototype.restore = function(actionFlag){
    if(!this.renderConfig.clearCanvas){
        this.canvasContext.restore();
        return;
    }
    if(actionFlag){
        this.canvasContext.restore();
    }
    this.contextData.cArrPos -= 1;
    var popped = this.contextData.saved[this.contextData.cArrPos];
    var i, len = 6,arr = this.contextData.cTr.props;
    for(i=0;i<len;i+=1){
        arr[i] = popped[i];
    }
    this.canvasContext.setTransform(popped[0],popped[1],popped[2],popped[3],popped[4],popped[5]);
    popped = this.contextData.savedOp[this.contextData.cArrPos];
    this.contextData.cO = popped;
    this.canvasContext.globalAlpha = popped;
};

CanvasRenderer.prototype.configAnimation = function(animData){
    if(this.animationItem.wrapper){
        this.animationItem.container = document.createElement('canvas');
        this.animationItem.container.style.width = '100%';
        this.animationItem.container.style.height = '100%';
        this.animationItem.container.style.transformOrigin = this.animationItem.container.style.mozTransformOrigin = this.animationItem.container.style.webkitTransformOrigin = this.animationItem.container.style['-webkit-transform'] = "0px 0px 0px";
        this.animationItem.wrapper.appendChild(this.animationItem.container);
        this.canvasContext = this.animationItem.container.getContext('2d');
    }else{
        this.canvasContext = this.renderConfig.context;
    }
    this.globalData.canvasContext = this.canvasContext;
    this.globalData.bmCtx = new BM_CanvasRenderingContext2D(this);
    this.globalData.renderer = this;
    this.globalData.totalFrames = Math.floor(animData.animation.totalFrames);
    this.globalData.compWidth = animData.animation.compWidth;
    this.globalData.compHeight = animData.animation.compHeight;
    this.layers = animData.animation.layers;
    this.transformCanvas = {};
    this.transformCanvas.w = animData.animation.compWidth;
    this.transformCanvas.h = animData.animation.compHeight;
    this.updateContainerSize();
};

CanvasRenderer.prototype.updateContainerSize = function () {
    var elementWidth,elementHeight;
    if(this.animationItem.wrapper && this.animationItem.container){
        elementWidth = this.animationItem.wrapper.offsetWidth;
        elementHeight = this.animationItem.wrapper.offsetHeight;
        this.animationItem.container.setAttribute('width',elementWidth);
        this.animationItem.container.setAttribute('height',elementHeight);
    }else{
        elementWidth = this.canvasContext.canvas.width;
        elementHeight = this.canvasContext.canvas.height;
    }
    if(this.renderConfig.scaleMode == 'fit'){
        var elementRel = elementWidth/elementHeight;
        var animationRel = this.transformCanvas.w/this.transformCanvas.h;
        if(animationRel>elementRel){
            this.transformCanvas.sx = elementWidth/this.transformCanvas.w;
            this.transformCanvas.sy = elementWidth/this.transformCanvas.w;
            this.transformCanvas.tx = 0;
            this.transformCanvas.ty = (elementHeight-this.transformCanvas.h*(elementWidth/this.transformCanvas.w))/2;
        }else{
            this.transformCanvas.sx = elementHeight/this.transformCanvas.h;
            this.transformCanvas.sy = elementHeight/this.transformCanvas.h;
            this.transformCanvas.tx = (elementWidth-this.transformCanvas.w*(elementHeight/this.transformCanvas.h))/2;
            this.transformCanvas.ty = 0;
        }
    }else{
        this.transformCanvas.sx = 1;
        this.transformCanvas.sy = 1;
        this.transformCanvas.tx = 0;
        this.transformCanvas.ty = 0;
    }
    this.transformCanvas.props = [this.transformCanvas.sx,0,0,this.transformCanvas.sy,this.transformCanvas.tx,this.transformCanvas.ty];
    this.clipper = new BM_Path2D();
    this.clipper.rect(0,0,this.transformCanvas.w,this.transformCanvas.h);
};

CanvasRenderer.prototype.buildStage = function (container, layers, elements) {
    if(!elements){
        elements = this.elements;
    }
    var i, len = layers.length, layerData;
    for (i = len - 1; i >= 0; i--) {
        layerData = layers[i];
        if (layerData.parent !== undefined) {
            this.buildItemHierarchy(layerData,elements[i], layers, layerData.parent,elements);
        }
        if (layerData.ty == 'PreCompLayer') {
            this.buildStage(null, layerData.layers, elements[i].getElements());
        }
    }
};

CanvasRenderer.prototype.buildItemHierarchy = function (data,element, layers, parentName,elements) {
    var i=0, len = layers.length;
    while(i<len){
        if(layers[i].ind === parentName){
            element.getHierarchy().push(elements[i]);
            if (layers[i].parent !== undefined) {
                this.buildItemHierarchy(data,element, layers, layers[i].parent,elements);
            }
        }
        i += 1;
    }
};

CanvasRenderer.prototype.prepareFrame = function(num){
    if(this.destroyed) {
        return;
    }
    var i, len = this.elements.length;
    for (i = 0; i < len; i++) {
        this.elements[i].prepareFrame(num - this.layers[i].startTime);
    }
};

CanvasRenderer.prototype.draw = function(){
    var i, len = this.layers.length;
    for (i = len - 1; i >= 0; i-=1) {
        this.elements[i].draw();
    }
};

CanvasRenderer.prototype.destroy = function () {
    if(this.renderConfig.clearCanvas) {
        this.animationItem.wrapper.innerHTML = '';
    }
    var i, len = this.layers.length;
    for (i = len - 1; i >= 0; i-=1) {
        this.elements[i].destroy();
    }
    this.elements.length = 0;
    this.globalData.bmCtx = null;
    this.globalData.canvasContext = null;
    this.animationItem.container = null;
    this.destroyed = true;
};

CanvasRenderer.prototype.renderFrame = function(num){
    if((this.lastFrame == num && this.renderConfig.clearCanvas === true) || this.destroyed){
        return;
    }
    this.lastFrame = num;
    this.globalData.frameNum = num - this.animationItem.firstFrame;
    if(this.renderConfig.clearCanvas === true){
        this.reset();
        this.canvasContext.canvas.width = this.canvasContext.canvas.width;
    }else{
        this.save();
    }
    this.ctxTransform(this.transformCanvas.props);
    this.globalData.bmCtx.clip(this.clipper);
    this.prepareFrame(num);
    this.draw();
    if(this.renderConfig.clearCanvas !== true){
        this.restore();
    }
};
function MaskElement(data,element,globalData) {
    this.data = data;
    this.storedData = [];
    this.element = element;
    this.globalData = globalData;
    this.paths = [];
    this.registeredEffects = [];
    this.masksProperties = [];
    this.maskElement = null;
}

MaskElement.prototype.init = function () {
    this.masksProperties = this.data.masksProperties;
    var maskedElement = this.element.maskedElement;
    var defs = this.globalData.defs;
    var i, len = this.masksProperties.length;


    var path, properties = this.data.masksProperties;
    var count = 0;
    var currentMasks = [];
    var j, jLen;
    var layerId = randomString(10);
    var rect;
    this.maskElement = document.createElementNS(svgNS, 'mask');
    for (i = 0; i < len; i++) {

        //console.log('properties[i].mode: ',properties[i].mode);
        if((properties[i].mode == 's' || properties[i].mode == 'i') && count == 0){
            rect = document.createElementNS(svgNS, 'rect');
            rect.setAttribute('fill', '#ffffff');
            rect.setAttribute('x', '0');
            rect.setAttribute('y', '0');
            rect.setAttribute('width', '100%');
            rect.setAttribute('height', '100%');
            currentMasks.push(rect);
        }

        if((properties[i].mode == 'f' && count > 0) || properties[i].mode == 'n') {
            continue;
        }
        count += 1;
        path = document.createElementNS(svgNS, 'path');
        if (properties[i].cl) {
            if(properties[i].mode == 's'){
                path.setAttribute('fill', '#000000');
            }else{
                path.setAttribute('fill', '#ffffff');
            }
        } else {
            path.setAttribute('fill', 'none');
            if(properties[i].mode == 's'){
                path.setAttribute('fill', '#000000');
            }else{
                path.setAttribute('fill', '#ffffff');
            }
            path.setAttribute('stroke-width', '1');
            path.setAttribute('stroke-miterlimit', '10');
        }
        path.setAttribute('clip-rule','nonzero');
        this.storedData[i] = {
         elem: path,
            lastPath: ''
        };
        if(properties[i].mode == 'i'){
            jLen = currentMasks.length;
            var g = document.createElementNS(svgNS,'g');
            for(j=0;j<jLen;j+=1){
                g.appendChild(currentMasks[j]);
            }
            var mask = document.createElementNS(svgNS,'mask');
            mask.setAttribute('mask-type','alpha');
            mask.setAttribute('id',layerId+'_'+count);
            mask.appendChild(path);
            defs.appendChild(mask);
            g.setAttribute('mask','url(#'+layerId+'_'+count+')');

            currentMasks.length = 0;
            currentMasks.push(g);
        }else{
            currentMasks.push(path);
        }
        if(properties[i].inv && !this.solidPath){
            this.solidPath = this.createLayerSolidPath();
        }
    }

    len = currentMasks.length;
    for(i=0;i<len;i+=1){
        this.maskElement.appendChild(currentMasks[i]);
    }

    this.maskElement.setAttribute('id', layerId);
    if(count > 0){
        maskedElement.setAttribute("mask", "url(#" + layerId + ")");
    }

    defs.appendChild(this.maskElement);
};

MaskElement.prototype.renderFrame = function (num) {
    var i, len = this.data.masksProperties.length;
    var count = 0;
    for (i = 0; i < len; i++) {
        if((this.data.masksProperties[i].mode == 'f' && count > 0)  || this.data.masksProperties[i].mode == 'n'){
            continue;
        }
        count += 1;
        this.drawPath(this.data.masksProperties[i],this.data.masksProperties[i].paths[num].pathNodes,this.storedData[i]);
    }
};

MaskElement.prototype.processMaskFromEffects = function (num, masks) {
    var i, len = this.registeredEffects.length;
    for (i = 0; i < len; i++) {
        this.registeredEffects[i].renderMask(num, masks);
    }
};

MaskElement.prototype.registerEffect = function (effect) {
    this.registeredEffects.push(effect);
};

MaskElement.prototype.getMaskelement = function () {
    return this.maskElement;
};

MaskElement.prototype.createLayerSolidPath = function(){
    var path = 'M0,0 ';
    path += ' h' + this.globalData.compSize.w ;
    path += ' v' + this.globalData.compSize.h ;
    path += ' h-' + this.globalData.compSize.w ;
    path += ' v-' + this.globalData.compSize.h + ' ';
    return path;
};

MaskElement.prototype.drawPath = function(pathData,pathNodes,storedData){
    var pathString = '';
    var i, len;
    if(!pathNodes.__renderedString){
        len = pathNodes.v.length;
            for(i=1;i<len;i+=1){
                if(i==1){
                    pathString += " M"+pathNodes.v[0][0]+','+pathNodes.v[0][1];
                }
                pathString += " C"+pathNodes.o[i-1][0]+','+pathNodes.o[i-1][1] + " "+pathNodes.i[i][0]+','+pathNodes.i[i][1] + " "+pathNodes.v[i][0]+','+pathNodes.v[i][1];
            }
            if(pathData.cl){
                pathString += " C"+pathNodes.o[i-1][0]+','+pathNodes.o[i-1][1] + " "+pathNodes.i[0][0]+','+pathNodes.i[0][1] + " "+pathNodes.v[0][0]+','+pathNodes.v[0][1];
            }
        pathNodes.__renderedString = pathString;
    }else{
        pathString = pathNodes.__renderedString;
    }



    if(storedData.lastPath !== pathString){
        if(pathData.inv){
            storedData.elem.setAttribute('d',this.solidPath + pathString);
        }else{
            storedData.elem.setAttribute('d',pathString);
        }
        storedData.lastPath = pathString;
    }
};

MaskElement.prototype.destroy = function(){
    this.element = null;
    this.globalData = null;
    this.maskElement = null;
    this.data = null;
    this.paths = null;
    this.registeredEffects = null;
    this.masksProperties = null;
};
var BaseElement = function (data,parentContainer,globalData){
    this.globalData = globalData;
    this.data = data;
    this.ownMatrix = new Matrix();
    this.finalTransform = {
        mat: new Matrix(),
        op: 1
    };
    this.matteElement = null;
    this.renderedFrames = [];
    this.lastData = {};
    this.parentContainer = parentContainer;
    this.layerId = randomString(10);
    this.hidden = false;
    this.init();
};

BaseElement.prototype.init = function(){
    this.createElements();
    if(this.data.hasMask){
        this.addMasks(this.data);
    }
    if(this.data.eff){
        //this.createEffectsManager(this.data);
    }
};

BaseElement.prototype.createElements = function(){
    if(this.data.td){
        if(this.data.td == 3){
            this.layerElement = document.createElementNS(svgNS,'mask');
            this.layerElement.setAttribute('id',this.layerId);
            this.layerElement.setAttribute('mask-type','luminance');
            this.globalData.defs.appendChild(this.layerElement);
        }else if(this.data.td == 2){
            var maskGroup = document.createElementNS(svgNS,'mask');
            maskGroup.setAttribute('id',this.layerId);
            maskGroup.setAttribute('mask-type','alpha');
            var maskGrouper = document.createElementNS(svgNS,'g');
            maskGroup.appendChild(maskGrouper);
            this.layerElement = document.createElementNS(svgNS,'g');
            var fil = document.createElementNS(svgNS,'filter');
            var filId = randomString(10);
            fil.setAttribute('id',filId);
            fil.setAttribute('filterUnits','objectBoundingBox');
            fil.setAttribute('x','0%');
            fil.setAttribute('y','0%');
            fil.setAttribute('width','100%');
            fil.setAttribute('height','100%');
            var feCTr = document.createElementNS(svgNS,'feComponentTransfer');
            feCTr.setAttribute('in','SourceGraphic');
            fil.appendChild(feCTr);
            var feFunc = document.createElementNS(svgNS,'feFuncA');
            feFunc.setAttribute('type','table');
            feFunc.setAttribute('tableValues','1.0 0.0');
            feCTr.appendChild(feFunc);
            this.globalData.defs.appendChild(fil);
            var alphaRect = document.createElementNS(svgNS,'rect');
            alphaRect.setAttribute('width','100%');
            alphaRect.setAttribute('height','100%');
            alphaRect.setAttribute('x','0');
            alphaRect.setAttribute('y','0');
            alphaRect.setAttribute('fill','#ffffff');
            alphaRect.setAttribute('opacity','0');
            maskGrouper.setAttribute('filter','url(#'+filId+')');
            maskGrouper.appendChild(alphaRect);
            maskGrouper.appendChild(this.layerElement);
            this.globalData.defs.appendChild(maskGroup);
        }else{
            this.layerElement = document.createElementNS(svgNS,'g');
            var masker = document.createElementNS(svgNS,'mask');
            masker.setAttribute('id',this.layerId);
            masker.setAttribute('mask-type','alpha');
            masker.appendChild(this.layerElement);
            this.globalData.defs.appendChild(masker);
        }
        if(this.data.hasMask){
            this.maskedElement = this.layerElement;
        }
    }else if(this.data.hasMask){
        this.layerElement = document.createElementNS(svgNS,'g');
        if(this.data.tt){
            this.matteElement = document.createElementNS(svgNS,'g');
            this.matteElement.appendChild(this.layerElement);
            this.parentContainer.appendChild(this.matteElement);
        }else{
            this.parentContainer.appendChild(this.layerElement);
        }
        this.maskedElement = this.layerElement;
    }else if(this.data.tt){
        this.matteElement = document.createElementNS(svgNS,'g');
        this.matteElement.setAttribute('id',this.layerId);
        this.parentContainer.appendChild(this.matteElement);
        this.layerElement = this.matteElement;
    }else{
        this.layerElement = this.parentContainer;
    }
};

BaseElement.prototype.prepareFrame = function(num){
    this.currentAnimData = this.data.renderedData[num].an;
    var mat = this.currentAnimData.matrixArray;
    this.ownMatrix.reset().transform(mat[0],mat[1],mat[2],mat[3],mat[4],mat[5]).translate(-this.currentAnimData.tr.a[0],-this.currentAnimData.tr.a[1]);
};

BaseElement.prototype.renderFrame = function(num,parentTransform){
    if(this.data.ty == 'NullLayer'){
        return;
    }
    if(this.data.inPoint - this.data.startTime <= num && this.data.outPoint - this.data.startTime > num)
    {
        if(this.isVisible !== true){
            this.isVisible = true;
        }
        this.finalTransform.opacity = 1;
    }else{
        if(this.isVisible !== false){
            this.isVisible = false;
        }
        this.finalTransform.opacity = 0;
    }

    if(this.data.eff){
       // this.effectsManager.renderFrame(num,this.currentAnimData.mk);
    }

    if(num === this.data.renderedFrame.num){
        return this.isVisible;
    }

    if(this.data.hasMask){
        this.maskManager.renderFrame(num);
    }
    this.finalTransform.opacity *= this.currentAnimData.tr.o;

    var mat;
    var finalMat = this.finalTransform.mat;

    if(parentTransform){
        mat = parentTransform.mat.props;
        finalMat.reset().transform(mat[0],mat[1],mat[2],mat[3],mat[4],mat[5]);
        this.finalTransform.opacity *= parentTransform.opacity;
    }

    if(this.hierarchy){
        var i, len = this.hierarchy.length;
        if(!parentTransform){
            finalMat.reset();
        }
        for(i=len-1;i>=0;i-=1){
            mat = this.hierarchy[i].ownMatrix.props;
            finalMat.transform(mat[0],mat[1],mat[2],mat[3],mat[4],mat[5]);
        }
        mat = this.ownMatrix.props;
        finalMat.transform(mat[0],mat[1],mat[2],mat[3],mat[4],mat[5]);
    }else{
        if(this.isVisible){
            if(!parentTransform){
                this.finalTransform.mat.props[0] = this.ownMatrix.props[0];
                this.finalTransform.mat.props[1] = this.ownMatrix.props[1];
                this.finalTransform.mat.props[2] = this.ownMatrix.props[2];
                this.finalTransform.mat.props[3] = this.ownMatrix.props[3];
                this.finalTransform.mat.props[4] = this.ownMatrix.props[4];
                this.finalTransform.mat.props[5] = this.ownMatrix.props[5];
            }else{
                mat = this.ownMatrix.props;
                finalMat.transform(mat[0],mat[1],mat[2],mat[3],mat[4],mat[5]);
            }
        }
    }
    if(this.data.hasMask){
        if(!this.renderedFrames[this.globalData.frameNum]){
            this.renderedFrames[this.globalData.frameNum] = {
                tr:'matrix('+finalMat.props.join(',')+')',
                o:this.finalTransform.opacity
            };
        }
        var renderedFrameData = this.renderedFrames[this.globalData.frameNum];
        if(this.lastData.tr != renderedFrameData.tr){
            this.lastData.tr = renderedFrameData.tr;
            this.layerElement.setAttribute('transform',renderedFrameData.tr);
        }
        if(this.lastData.o !== renderedFrameData.o){
            this.lastData.o = renderedFrameData.o;
            this.layerElement.setAttribute('opacity',renderedFrameData.o);
        }
    }

    return this.isVisible;
};

BaseElement.prototype.destroy = function(){
    this.layerElement = null;
    this.parentContainer = null;
    if(this.matteElement) {
        this.matteElement = null;
    }
    if(this.maskManager) {
        this.maskManager.destroy();
    }
};

BaseElement.prototype.getDomElement = function(){
    return this.layerElement;
};
BaseElement.prototype.getMaskManager = function(){
    return this.maskManager;
};
BaseElement.prototype.addMasks = function(data){
    //this.maskManager = createElement(MaskElement,null,params);
    this.maskManager = new MaskElement(data,this,this.globalData);
    this.maskManager.init();
};
BaseElement.prototype.createEffectsManager = function(data){
    var params = {
        'effects':{value:data.eff},
        'element':{value:this}
    };
    this.effectsManager = createElement(EffectsManager,null,params);
};
BaseElement.prototype.getType = function(){
    return this.type;
};

BaseElement.prototype.getLayerSize = function(){
    if(this.data.ty == 'TextLayer'){
        return {w:this.data.textData.width,h:this.data.textData.height};
    }else{
        return {w:this.data.width,h:this.data.height};
    }
};

BaseElement.prototype.getHierarchy = function(){
    if(!this.hierarchy){
        this.hierarchy = [];
    }
    return this.hierarchy;
};

BaseElement.prototype.setMatte = function(id){
    if(!this.matteElement){
        return;
    }
    this.matteElement.setAttribute("mask", "url(#" + id + ")");
};

BaseElement.prototype.hide = function(){

};

function ICompElement(data,parentContainer,globalData){
    this.parent.constructor.call(this,data,parentContainer,globalData);
    this.layers = data.layers;
}
createElement(BaseElement, ICompElement);

ICompElement.prototype.getComposingElement = function(){
    return this.layerElement;
};

ICompElement.prototype.hide = function(){
    if(!this.hidden){
        var i,len = this.elements.length;
        for( i = 0; i < len; i+=1 ){
            this.elements[i].hide();
        }
        this.hidden = true;
    }
};

ICompElement.prototype.renderFrame = function(num,parentMatrix){
    var renderParent = this.parent.renderFrame.call(this,num,parentMatrix);
    if(renderParent===false){
        this.hide();
        return;
    }

    this.hidden = false;
    var i,len = this.layers.length;
    var timeRemapped = this.data.tm ? this.data.tm[num] < 0 ? 0 : num >= this.data.tm.length ? this.data.tm[this.data.tm.length - 1] : this.data.tm[num] : num;
    for( i = 0; i < len; i+=1 ){
        this.elements[i].prepareFrame(timeRemapped - this.layers[i].startTime);
    }
    for( i = 0; i < len; i+=1 ){
        if(this.data.hasMask){
            this.elements[i].renderFrame(timeRemapped - this.layers[i].startTime);
        }else{
            this.elements[i].renderFrame(timeRemapped - this.layers[i].startTime,this.finalTransform);
        }
    }
};

ICompElement.prototype.setElements = function(elems){
    this.elements = elems;
};

ICompElement.prototype.getElements = function(){
    return this.elements;
};

ICompElement.prototype.destroy = function(){
    this.parent.destroy.call();
    var i,len = this.layers.length;
    for( i = 0; i < len; i+=1 ){
        this.elements[i].destroy();
    }
};
function IImageElement(data,parentContainer,globalData){
    this.assetData = globalData.getAssetData(data.id);
    this.path = globalData.getPath();
    this.parent.constructor.call(this,data,parentContainer,globalData);
}
createElement(BaseElement, IImageElement);

IImageElement.prototype.createElements = function(){

    var self = this;

    var imageLoaded = function(){
        self.image.setAttributeNS('http://www.w3.org/1999/xlink','href',self.path+self.assetData.p);
        self.maskedElement = self.image;
    };

    var img = new Image();
    img.addEventListener('load', imageLoaded, false);
    img.addEventListener('error', imageLoaded, false);

    img.src = this.path+this.assetData.p;

    this.parent.createElements.call(this);

    this.image = document.createElementNS(svgNS,'image');
    this.image.setAttribute('width',this.assetData.w+"px");
    this.image.setAttribute('height',this.assetData.h+"px");
    this.layerElement.appendChild(this.image);

};

IImageElement.prototype.hide = function(){
    if(!this.hidden){
        this.image.setAttribute('opacity','0');
        this.hidden = true;
    }
};

IImageElement.prototype.renderFrame = function(num,parentMatrix){
    var renderParent = this.parent.renderFrame.call(this,num,parentMatrix);
    if(renderParent===false){
        this.hide();
        return;
    }
    if(this.hidden){
        this.lastData.o = -1;
        this.hidden = false;
        this.image.setAttribute('opacity', '1');
    }
    if(!this.data.hasMask){
        if(!this.renderedFrames[this.globalData.frameNum]){
            this.renderedFrames[this.globalData.frameNum] = {
                tr: 'matrix('+this.finalTransform.mat.props.join(',')+')',
                o: this.finalTransform.opacity
            };
        }
        var renderedFrameData = this.renderedFrames[this.globalData.frameNum];
        if(this.lastData.tr != renderedFrameData.tr){
            this.lastData.tr = renderedFrameData.tr;
            this.image.setAttribute('transform',renderedFrameData.tr);
        }
        if(this.lastData.o !== renderedFrameData.o){
            this.lastData.o = renderedFrameData.o;
            this.image.setAttribute('opacity',renderedFrameData.o);
        }
    }
};

IImageElement.prototype.destroy = function(){
    this.parent.destroy.call();
    this.image =  null;
};
function IShapeElement(data,parentContainer,globalData){
    this.shapes = [];
    this.parent.constructor.call(this,data,parentContainer,globalData);
}
createElement(BaseElement, IShapeElement);

IShapeElement.prototype.createElements = function(){
    //TODO check if I can use symbol so i can set its viewBox
    this.parent.createElements.call(this);
    this.mainShape = new ShapeItemElement(this.data.shapes,this.layerElement,this.globalData);
};

IShapeElement.prototype.renderFrame = function(num,parentMatrix){
    var renderParent = this.parent.renderFrame.call(this,num,parentMatrix);
    if(renderParent===false){
        this.hide();
        return;
    }

    this.renderShapes(num);
};

IShapeElement.prototype.hide = function(){
    if(!this.hidden){
        this.mainShape.hideShape();
        this.hidden = true;
    }
};

IShapeElement.prototype.renderShapes = function(num){
    this.hidden = false;
    if(this.data.hasMask){
        this.mainShape.renderShape(num,{opacity:1,mat:new Matrix()});
    }else{
        this.mainShape.renderShape(num,this.finalTransform);
    }
};

IShapeElement.prototype.destroy = function(){
    this.parent.destroy.call();
    this.mainShape.destroy();
};
function ShapeItemElement(data,parentElement,globalData){
    this.lcEnum = {
        '1': 'butt',
        '2': 'round',
        '3': 'butt'
    };
    this.ljEnum = {
        '1': 'miter',
        '2': 'round',
        '3': 'bevel'
    };
    this.stylesList = [];
    this.viewData = [];
    this.shape = parentElement;
    this.data = data;
    this.globalData = globalData;
    this.searchShapes(this.data,this.viewData);
    styleUnselectableDiv(this.shape);
}

ShapeItemElement.prototype.searchShapes = function(arr,data){
    var i, len = arr.length - 1;
    var j, jLen;
    var ownArrays = [];
    for(i=len;i>=0;i-=1){
        if(arr[i].ty == 'fl' || arr[i].ty == 'st'){
            data[i] = {
                renderedFrames : [],
                lastData : {
                    c: '',
                    o:-1,
                    w: ''
                }
            };
            var pathElement = document.createElementNS(svgNS, "path");
            if(arr[i].ty == 'st') {
                pathElement.setAttribute('stroke-linecap', this.lcEnum[arr[i].lc] || 'round');
                pathElement.setAttribute('stroke-linejoin',this.ljEnum[arr[i].lj] || 'round');
                if(arr[i].lj == 1) {
                    pathElement.setAttribute('stroke-miterlimit',arr[i].ml);
                }
            }
            this.shape.appendChild(pathElement);
            this.stylesList.push({
                pathElement: pathElement,
                type: arr[i].ty,
                d: '',
                ld: 'a'
            });
            data[i].style = this.stylesList[this.stylesList.length - 1];
            ownArrays.push(data[i].style);
        }else if(arr[i].ty == 'gr'){
            data[i] = {
                it: []
            };
            this.searchShapes(arr[i].it,data[i].it);
        }else if(arr[i].ty == 'tr'){
            data[i] = {
                transform : {
                    mat: new Matrix(),
                    opacity: 1
                }
            };
        }else if(arr[i].ty == 'sh' || arr[i].ty == 'rc' || arr[i].ty == 'el'){
            data[i] = {
                elements : [],
                renderedFrames : [],
                styles : [],
                lastData : {
                    d: '',
                    o:'',
                    tr:''
                }
            };
            jLen = this.stylesList.length;
            for(j=0;j<jLen;j+=1){
                if(!this.stylesList[j].closed){
                    data[i].styles.push(this.stylesList[j]);
                    if(this.stylesList[j].type == 'st'){
                        this.stylesList[j].pathElement.setAttribute('fill-opacity',0);
                    }
                }
            }
        }
    }
    len = ownArrays.length;
    for(i=0;i<len;i+=1){
        ownArrays[i].closed = true;
    }
};

ShapeItemElement.prototype.getElement = function(){
    return this.shape;
};

ShapeItemElement.prototype.hideShape = function(){
    var i, len = this.stylesList.length;
    for(i=len-1;i>=0;i-=1){
        this.stylesList[i].pathElement.setAttribute('d','M 0,0');
        this.stylesList[i].ld = 'M 0,0';
    }
};

ShapeItemElement.prototype.renderShape = function(num,parentTransform,items,data){
    var i, len;
    if(!items){
        items = this.data;
        len = this.stylesList.length;
        for(i=0;i<len;i+=1){
            this.stylesList[i].d = '';
        }
    }
    if(!data){
        data = this.viewData;
    }
    this.frameNum = num;
    ///
    ///
    len = items.length - 1;
    var groupTransform,groupMatrix;
    groupTransform = parentTransform;
    for(i=len;i>=0;i-=1){
        if(items[i].ty == 'tr'){
            var mtArr = items[i].renderedData[num].mtArr;
            groupTransform = data[i].transform;
            groupMatrix = groupTransform.mat;
            groupMatrix.reset();
            if(parentTransform){
                var props = parentTransform.mat.props;
                groupTransform.opacity = parentTransform.opacity;
                groupTransform.opacity *= items[i].renderedData[num].o;
                groupMatrix.transform(props[0],props[1],props[2],props[3],props[4],props[5]);
            }else{
                groupTransform.opacity = items[i].renderedData[num].o;
            }
            groupMatrix.transform(mtArr[0],mtArr[1],mtArr[2],mtArr[3],mtArr[4],mtArr[5]).translate(-items[i].renderedData[num].a[0],-items[i].renderedData[num].a[1]);
        }else if(items[i].ty == 'sh'){
            this.renderPath(items[i],data[i],num,groupTransform);
        }else if(items[i].ty == 'el'){
            this.renderPath(items[i],data[i],num,groupTransform);
            //this.renderEllipse(items[i],data[i],num,groupTransform);
        }else if(items[i].ty == 'rc'){
            if(items[i].trimmed){
                this.renderPath(items[i],data[i],num,groupTransform);
            }else{
                this.renderRect(items[i],data[i],num,groupTransform);
            }
        }else if(items[i].ty == 'fl'){
            this.renderFill(items[i],data[i],num,groupTransform);
        }else if(items[i].ty == 'st'){
            this.renderStroke(items[i],data[i],num,groupTransform);
        }else if(items[i].ty == 'gr'){
            this.renderShape(num,groupTransform,items[i].it,data[i].it);
        }else if(items[i].ty == 'tm'){
            //
        }
    }
    len = this.stylesList.length;
    for(i=0;i<len;i+=1){
        if(this.stylesList[i].d == '' && this.stylesList[i].ld !== ''){
            this.stylesList[i].pathElement.setAttribute('d','M 0,0');
            this.stylesList[i].ld = this.stylesList[i].d;
        }else if(this.stylesList[i].ld !== this.stylesList[i].d){
            this.stylesList[i].pathElement.setAttribute('d',this.stylesList[i].d);
            this.stylesList[i].ld = this.stylesList[i].d;
        }
    }

};

ShapeItemElement.prototype.renderPath = function(pathData,viewData,num,transform){
    var len,i;
    if(!viewData.renderedFrames[this.globalData.frameNum]){

        var pathNodes = pathData.renderedData[num].path.pathNodes;
        if(!pathNodes.v){
            return;
        }
        len = pathNodes.v.length;
        var stops = pathNodes.s ? pathNodes.s : [];
        var pathStringTransformed = '';
        for(i=1;i<len;i+=1){
            if(stops[i-1]){
                //pathStringTransformed += " M"+transform.mat.applyToPointStringified(stops[i-1][0],stops[i-1][1]);
                pathStringTransformed += " M"+stops[i-1][0]+','+stops[i-1][1];
            }else if(i==1){
                //pathStringTransformed += " M"+transform.mat.applyToPointStringified(pathNodes.v[0][0],pathNodes.v[0][1]);
                pathStringTransformed += " M"+pathNodes.v[0][0]+','+pathNodes.v[0][1];
            }
            //pathStringTransformed += " C"+transform.mat.applyToPointStringified(pathNodes.o[i-1][0],pathNodes.o[i-1][1]) + " "+transform.mat.applyToPointStringified(pathNodes.i[i][0],pathNodes.i[i][1]) + " "+transform.mat.applyToPointStringified(pathNodes.v[i][0],pathNodes.v[i][1]);
            pathStringTransformed += " C"+pathNodes.o[i-1][0]+','+pathNodes.o[i-1][1] + " "+pathNodes.i[i][0]+','+pathNodes.i[i][1] + " "+pathNodes.v[i][0]+','+pathNodes.v[i][1];
        }
        if(len == 1){
            if(stops[0]){
                //pathStringTransformed += " M"+transform.mat.applyToPointStringified(stops[i-1][0],stops[0][1]);
                pathStringTransformed += " M"+stops[0][0]+','+stops[0][1];
            }else{
                //pathStringTransformed += " M"+transform.mat.applyToPointStringified(pathNodes.v[0][0],pathNodes.v[0][1]);
                pathStringTransformed += " M"+pathNodes.v[0][0]+','+pathNodes.v[0][1];
            }
        }
        if(pathData.closed && !(pathData.trimmed && !pathNodes.c)){
            //pathStringTransformed += " C"+transform.mat.applyToPointStringified(pathNodes.o[i-1][0],pathNodes.o[i-1][1]) + " "+transform.mat.applyToPointStringified(pathNodes.i[0][0],pathNodes.i[0][1]) + " "+transform.mat.applyToPointStringified(pathNodes.v[0][0],pathNodes.v[0][1]);
            pathStringTransformed += " C"+pathNodes.o[i-1][0]+','+pathNodes.o[i-1][1] + " "+pathNodes.i[0][0]+','+pathNodes.i[0][1] + " "+pathNodes.v[0][0]+','+pathNodes.v[0][1];
        }

        viewData.renderedFrames[this.globalData.frameNum] = {
            dTr: pathStringTransformed
        };
    }
    var renderedFrameData = viewData.renderedFrames[this.globalData.frameNum];

    len = viewData.styles.length;
    for(i=0;i<len;i+=1){
        viewData.styles[i].d += renderedFrameData.dTr;
    }
};

ShapeItemElement.prototype.renderFill = function(styleData,viewData,num,groupTransform){
    var fillData = styleData.renderedData[num];
    var styleElem = viewData.style;
    if(!viewData.renderedFrames[this.globalData.frameNum]){
        viewData.renderedFrames[this.globalData.frameNum] = {
            c: fillData.color,
            o: fillData.opacity*groupTransform.opacity,
            t: 'matrix('+groupTransform.mat.props.join(',')+')'
        };
    }

    var renderedFrameData = viewData.renderedFrames[this.globalData.frameNum];
    if(viewData.lastData.c != renderedFrameData.c){
        styleElem.pathElement.setAttribute('fill',renderedFrameData.c);
        viewData.lastData.c = renderedFrameData.c;
    }
    if(viewData.lastData.o != renderedFrameData.o){
        styleElem.pathElement.setAttribute('fill-opacity',renderedFrameData.o);
        viewData.lastData.o = renderedFrameData.o;
    }
    if(viewData.lastData.t != renderedFrameData.t){
        styleElem.pathElement.setAttribute('transform',renderedFrameData.t);
        viewData.lastData.t = renderedFrameData.t;
    }
};

ShapeItemElement.prototype.renderStroke = function(styleData,viewData,num,groupTransform){
    var fillData = styleData.renderedData[num];
    var styleElem = viewData.style;
    if(!viewData.renderedFrames[this.globalData.frameNum]){
        viewData.renderedFrames[this.globalData.frameNum] = {
            c: fillData.color,
            o: fillData.opacity*groupTransform.opacity,
            w: fillData.width,
            t: 'matrix('+groupTransform.mat.props.join(',')+')'
        };
        if(fillData.dashes){
            viewData.renderedFrames[this.globalData.frameNum].d = fillData.dashes;
        }
    }

    var renderedFrameData = viewData.renderedFrames[this.globalData.frameNum];
    var c = renderedFrameData.c;
    var o = renderedFrameData.o;
    var w = renderedFrameData.w;
    var d = renderedFrameData.d;
    var t = renderedFrameData.t;
    var dasharray,dashoffset;
    if(d){
        var j, jLen = d.length;
        dasharray = '';
        dashoffset = '';
        for(j=0;j<jLen;j+=1){
            if(d[j].n != 'o'){
                dasharray += ' ' + d[j].v;
            }else{
                dashoffset += d[j].v;
            }
        }
        if(viewData.lastData.da != dasharray){
            styleElem.pathElement.setAttribute('stroke-dasharray',dasharray);
            viewData.lastData.da = dasharray;
        }
        if(viewData.lastData.do != dashoffset){
            styleElem.pathElement.setAttribute('stroke-dashoffset',dashoffset);
            viewData.lastData.do = dashoffset;
        }
    }
    if(viewData.lastData.c != c){
        styleElem.pathElement.setAttribute('stroke',c);
        viewData.lastData.c = c;
    }
    if(viewData.lastData.o != o){
        styleElem.pathElement.setAttribute('stroke-opacity',o);
        viewData.lastData.o = o;
    }
    if(viewData.lastData.w !== w){
        styleElem.pathElement.setAttribute('stroke-width',w);
        viewData.lastData.w = w;
    }
    if(viewData.lastData.t !== t){
        styleElem.pathElement.setAttribute('transform',t);
        viewData.lastData.t = t;
    }
};

ShapeItemElement.prototype.destroy = function(items, data){
    this.shape = null;
    this.data = null;
    this.viewData = null;
    /*if(!items){
        items = this.data;
    }
    if(!data){
        data = this.viewData;
    }
    var i, len = items.length;
    var groupTransform,groupMatrix;
    groupTransform = parentTransform;
    for(i = 0; i < len; i += 1){
        if(items[i].ty == 'tr'){
        }else if(items[i].ty == 'sh'){
            this.renderPath(items[i],data[i],num,groupTransform);
        }else if(items[i].ty == 'el'){
            this.renderPath(items[i],data[i],num,groupTransform);
            //this.renderEllipse(items[i],data[i],num,groupTransform);
        }else if(items[i].ty == 'rc'){
            if(items[i].trimmed){
                this.renderPath(items[i],data[i],num,groupTransform);
            }else{
                this.renderRect(items[i],data[i],num,groupTransform);
            }
        }else if(items[i].ty == 'fl'){
            this.renderFill(items[i],data[i],num,groupTransform);
        }else if(items[i].ty == 'st'){
            this.renderStroke(items[i],data[i],num,groupTransform);
        }else if(items[i].ty == 'gr'){
            this.renderShape(num,groupTransform,items[i].it,data[i].it);
        }else if(items[i].ty == 'tm'){
            //
        }
    }*/
};
function ISolidElement(data,parentContainer,globalData){
    this.parent.constructor.call(this,data,parentContainer,globalData);
}
createElement(BaseElement, ISolidElement);

ISolidElement.prototype.createElements = function(){
    this.parent.createElements.call(this);

    var rect = document.createElementNS(svgNS,'rect');
    rect.setAttribute('width',this.data.width);
    rect.setAttribute('height',this.data.height);
    /*rect.setAttribute('width',1);
    rect.setAttribute('height',1);*/
    rect.setAttribute('fill',this.data.color);
    this.layerElement.appendChild(rect);
    this.rectElement = rect;
};

ISolidElement.prototype.hide = function(){
    if(!this.hidden){
        this.rectElement.setAttribute('opacity','0');
        this.hidden = true;
    }
};

ISolidElement.prototype.renderFrame = function(num,parentMatrix){
    var renderParent = this.parent.renderFrame.call(this,num,parentMatrix);
    if(renderParent===false){
        this.hide();
        return;
    }
    if(this.hidden){
        this.lastData.o = -1;
        this.hidden = false;
        if(this.data.hasMask) {
            this.rectElement.setAttribute('opacity', '1');
        }
    }
    if(!this.data.hasMask){
        if(!this.renderedFrames[this.globalData.frameNum]){
            this.renderedFrames[this.globalData.frameNum] = {
                tr: 'matrix('+this.finalTransform.mat.props.join(',')+')',
                o: this.finalTransform.opacity
            };
        }
        var renderedFrameData = this.renderedFrames[this.globalData.frameNum];
        if(this.lastData.tr != renderedFrameData.tr){
            this.lastData.tr = renderedFrameData.tr;
            this.rectElement.setAttribute('transform',renderedFrameData.tr);
        }
        if(this.lastData.o !== renderedFrameData.o){
            this.lastData.o = renderedFrameData.o;
            this.rectElement.setAttribute('opacity',renderedFrameData.o);
        }
    }
};

ICompElement.prototype.destroy = function(){
    this.parent.destroy.call();
    this.rectElement = null;
};
function ITextElement(data, animationItem,parentContainer,globalData){
    this.parent.constructor.call(this,data, animationItem,parentContainer,globalData);
}
createElement(BaseElement, ITextElement);

ITextElement.prototype.createElements = function(){

    this.parent.createElements.call(this);
    /*this.svgElem = document.createElementNS (svgNS, "g");

    var textElement = document.createElementNS(svgNS,'text');
    textElement.textContent = this.data.textData.text;
    textElement.setAttribute('fill', this.data.textData.fillColor);
    textElement.setAttribute('x', '0');
    textElement.setAttribute('y',this.data.textData.height - (this.data.textData.fontSize-this.data.textData.height)/2);
    this.svgElem.setAttribute('width',this.data.textData.width);
    this.svgElem.setAttribute('height',this.data.textData.height);
    this.svgElem.style.transform=this.svgElem.style.webkitTransform='translate(' + this.data.textData.xOffset+"px," + this.data.textData.yOffset+"px)";
    textElement.setAttribute('font-size', this.data.textData.fontSize);
    textElement.setAttribute('font-family', "Arial, sans-serif");
    this.svgElem.appendChild(textElement);

    this.parent.createElements.call(this);

    this.anchorElement.appendChild(this.svgElem);
    this.maskedElement = textElement;*/
};

function CVBaseElement(data,globalData){
    this.data = data;
    this.globalData = globalData;
    this.canvasContext = globalData.canvasContext;
    this.currentAnimData = null;
    this.renderFrame = false;
    this.ownMatrix = new Matrix();
    this.finalTransform = {
        mat: new Matrix(),
        opacity: 1
    };
    this.init();
}

CVBaseElement.prototype.init = function(){
    this.createElements();
    if(this.data.hasMask){
        this.addMasks(this.data);
    }
    if(this.data.eff){
        //this.createEffectsManager(this.data);
    }
};

CVBaseElement.prototype.createElements = function(){

};

CVBaseElement.prototype.prepareFrame = function(num){
    this.currentAnimData = this.data.renderedData[num].an;
    var mat = this.currentAnimData.matrixArray;
    this.ownMatrix.reset().transform(mat[0],mat[1],mat[2],mat[3],mat[4],mat[5]).translate(-this.currentAnimData.tr.a[0],-this.currentAnimData.tr.a[1]);
    if(this.data.inPoint - this.data.startTime <= num && this.data.outPoint - this.data.startTime >= num)
    {
        this.renderFrame = true;
        this.finalTransform.opacity = 1;
    }else{
        this.renderFrame = false;
        this.finalTransform.opacity = 0;
        return false;
    }

    if(this.data.hasMask){
        this.maskManager.prepareFrame(num);
    }
};

CVBaseElement.prototype.draw = function(parentTransform){
    if(this.data.ty == 'NullLayer'){
        return;
    }
    if(!this.renderFrame){
        return false;
    }
    var ctx = this.canvasContext;
    ////

    var mat, finalMat = this.finalTransform.mat;

    this.finalTransform.opacity *= this.currentAnimData.tr.o;

    if(parentTransform){
        mat = parentTransform.mat.props;
        finalMat.reset().transform(mat[0],mat[1],mat[2],mat[3],mat[4],mat[5]);
        this.finalTransform.opacity *= parentTransform.opacity;
    }

    if(this.hierarchy){
        var i, len = this.hierarchy.length;
        if(!parentTransform){
            finalMat.reset();
        }
        for(i=len-1;i>=0;i-=1){
            mat = this.hierarchy[i].ownMatrix.props;
            finalMat.transform(mat[0],mat[1],mat[2],mat[3],mat[4],mat[5]);
        }
        mat = this.ownMatrix.props;
        finalMat.transform(mat[0],mat[1],mat[2],mat[3],mat[4],mat[5]);
    }else{
        if(this.renderFrame){
            if(!parentTransform){
                this.finalTransform.mat = this.ownMatrix;
            }else{
                mat = this.ownMatrix.props;
                finalMat.transform(mat[0],mat[1],mat[2],mat[3],mat[4],mat[5]);
            }
        }
        }

    ////
    if(this.data.hasMask){
        this.globalData.renderer.save(true);
        this.maskManager.draw(this.finalTransform);
    }

};

CVBaseElement.prototype.getCurrentAnimData = function(){
    return this.currentAnimData;
};
CVBaseElement.prototype.addMasks = function(data){
    var params = {
        'data':{value:data},
        'element':{value:this},
        'globalData':{value:this.globalData}
    };
    this.maskManager = createElement(CVMaskElement,null,params);
};
CVBaseElement.prototype.createEffectsManager = function(data){
    var params = {
        'effects':{value:data.eff},
        'element':{value:this}
    };
    this.effectsManager = createElement(EffectsManager,null,params);
};
CVBaseElement.prototype.getType = function(){
    return this.type;
};

CVBaseElement.prototype.getHierarchy = function(){
    if(!this.hierarchy){
        this.hierarchy = [];
    }
    return this.hierarchy;
};

CVBaseElement.prototype.getLayerSize = function(){
    if(this.data.ty == 'TextLayer'){
        return {w:this.data.textData.width,h:this.data.textData.height};
    }else{
        return {w:this.data.width,h:this.data.height};
    }
};


CVBaseElement.prototype.destroy = function(){
    this.canvasContext = null;
    this.data = null;
    this.globalData = null;
    if(this.maskManager) {
        this.maskManager.destroy();
    }
};


function CVCompElement(data,globalData){
    this.parent.constructor.call(this,data,globalData);
    this.layers = data.layers;
}
createElement(CVBaseElement, CVCompElement);

CVCompElement.prototype.prepareFrame = function(num){
    var renderParent = this.parent.prepareFrame.call(this,num);
    if(renderParent===false){
        return;
    }
    var i,len = this.elements.length;
    var timeRemapped = this.data.tm ? this.data.tm[num] < 0 ? 0 : num >= this.data.tm.length ? this.data.tm[this.data.tm.length - 1] : this.data.tm[num] : num;
    for( i = 0; i < len; i+=1 ){
        this.elements[i].prepareFrame(timeRemapped - this.layers[i].startTime);
    }
};

CVCompElement.prototype.draw = function(parentMatrix){
    if(this.parent.draw.call(this,parentMatrix)===false){
        return;
    }
    var i,len = this.layers.length;
    for( i = len - 1; i >= 0; i -= 1 ){
        this.elements[i].draw(this.finalTransform);
    }
    if(this.data.hasMask){
        this.globalData.renderer.restore(true);
    }
};

CVCompElement.prototype.setElements = function(elems){
    this.elements = elems;
};

CVCompElement.prototype.getElements = function(){
    return this.elements;
};

CVCompElement.prototype.destroy = function(){
    var i,len = this.layers.length;
    for( i = len - 1; i >= 0; i -= 1 ){
        this.elements[i].destroy();
    }
    this.layers = null;
    this.elements = null;
    this.parent.destroy.call();
};
function CVImageElement(data,globalData){
    this.animationItem = globalData.renderer.animationItem;
    this.assetData = this.animationItem.getAssetData(data.id);
    this.path = this.animationItem.getPath();
    this.parent.constructor.call(this,data,globalData);
    this.animationItem.pendingElements += 1;
}
createElement(CVBaseElement, CVImageElement);

CVImageElement.prototype.createElements = function(){
    var self = this;

    var imageLoaded = function(){
        self.animationItem.elementLoaded();
    };
    var imageFailed = function(){
        //console.log('imageFailed');
        self.failed = true;
        self.animationItem.elementLoaded();
    };

    this.img = new Image();
    this.img.addEventListener('load', imageLoaded, false);
    this.img.addEventListener('error', imageFailed, false);
    this.img.src = this.path+this.assetData.p;

    this.parent.createElements.call(this);

};

CVImageElement.prototype.draw = function(parentMatrix){
    if(this.failed){
        return;
    }
    if(this.parent.draw.call(this,parentMatrix)===false){
        return;
    }
    var ctx = this.canvasContext;
    this.globalData.renderer.save();
    var finalMat = this.finalTransform.mat.props;
    this.globalData.renderer.ctxTransform(finalMat);
    this.globalData.renderer.ctxOpacity(this.finalTransform.opacity);
    ctx.drawImage(this.img,0,0);
    this.globalData.renderer.restore(this.data.hasMask);
};

CVImageElement.prototype.destroy = function(){
    this.img = null;
    this.animationItem = null;
    this.parent.destroy.call();
};
function CVShapeElement(data,globalData){
    this.shapes = [];
    this.parent.constructor.call(this,data,globalData);
}
createElement(CVBaseElement, CVShapeElement);

CVShapeElement.prototype.createElements = function(){

    this.parent.createElements.call(this);
    this.mainShape = new CVShapeItemElement(this.data.shapes,true,this.globalData);
};

CVShapeElement.prototype.prepareFrame = function(num){
    var renderParent = this.parent.prepareFrame.call(this,num);
    if(renderParent===false){
        return;
    }
    this.mainShape.prepareFrame(num);
};

CVShapeElement.prototype.draw = function(parentMatrix){
    if(this.parent.draw.call(this, parentMatrix)===false){
        return;
    }
    this.drawShapes(this.finalTransform);
    if(this.data.hasMask){
        this.globalData.renderer.restore(true);
    }
};

CVShapeElement.prototype.drawShapes = function(parentTransform){
    this.mainShape.renderShape(parentTransform);
};

CVShapeElement.prototype.destroy = function(){
    this.mainShape.destroy();
    this.parent.destroy.call();
};
function CVShapeItemElement(data,mainFlag,globalData){
    this.data = data;
    this.globalData = globalData;
    this.canvasContext = globalData.canvasContext;
    this.frameNum = -1;
    this.dataLength = this.data.length;
    this.mainFlag = mainFlag;
    this.stylesList = [];
    this.ownStylesList = [];
    this.stylesPool = [];
    this.currentStylePoolPos = 0;
    this.transform = {
        opacity: 1,
        mat: new Matrix()
    };
    this.mat = new Matrix();
    var i,len=this.dataLength-1;
    this.renderedPaths = new Array(this.globalData.totalFrames);
    var styleData;
    for(i=len;i>=0;i-=1){
        if(this.data[i].ty == 'gr'){
            this.data[i].item = new CVShapeItemElement(this.data[i].it,false,this.globalData);
        }else if(this.data[i].ty == 'st' || this.data[i].ty == 'fl'){
            styleData = {
                type:'fill',
                /*path: new BM_Path2D(),*/
                styleOpacity: 0,
                opacity: 0,
                value:'rgba(0,0,0,0)',
                closed: false
            };
            if(this.data[i].ty == 'fl'){
                styleData.type = 'fill';
            }else{
                styleData.type = 'stroke';
                styleData.width = 0;
            }
            this.stylesPool.push(styleData);
        }
    }
}

CVShapeItemElement.prototype.drawPaths = function(cacheFlag){
    var stylesList,cache;
    if(cacheFlag){
        cache = [];
        stylesList = this.stylesList;
    }else{
        stylesList = this.renderedPaths[this.globalData.frameNum];
    }
    var i, len = stylesList.length;
    var ctx = this.canvasContext;
    this.globalData.renderer.save();
    ctx.lineCap = 'round';
    ctx.lineJoin = 'round';
    for(i=0;i<len;i+=1){
        if(stylesList[i].type == 'stroke'){
            if(stylesList[i].opacity != 1){
                this.globalData.renderer.save();
                this.globalData.renderer.ctxOpacity(stylesList[i].opacity);
                ///ctx.globalAlpha *= stylesList[i].opacity;
            }
            ctx.strokeStyle = stylesList[i].value;
            ctx.lineWidth = stylesList[i].width;
            if(stylesList[i].dasharray){
                ctx.setLineDash(stylesList[i].dasharray);
                ctx.lineDashOffset = stylesList[i].dashoffset;
            }else{
                ctx.setLineDash([]);
                ctx.lineDashOffset = 0;
            }
            this.globalData.bmCtx.stroke(stylesList[i].path);
            if(stylesList[i].opacity != 1){
                this.globalData.renderer.restore();
            }
            if(cacheFlag){
                cache.push({
                    type: stylesList[i].type,
                    opacity: stylesList[i].opacity,
                    value: stylesList[i].value,
                    width: stylesList[i].width,
                    path: stylesList[i].path
                });
                if(stylesList[i].dasharray){
                    cache[cache.length-1].dasharray = stylesList[i].dasharray;
                    cache[cache.length-1].dashoffset = stylesList[i].dashoffset;
                }
            }
        }else if(stylesList[i].type == 'fill'){
            if(stylesList[i].opacity != 1){
                this.globalData.renderer.save();
                this.globalData.renderer.ctxOpacity(stylesList[i].opacity);
                ///ctx.globalAlpha *= stylesList[i].opacity;
            }
            ctx.fillStyle = stylesList[i].value;
            this.globalData.bmCtx.fill(stylesList[i].path);
            if(stylesList[i].opacity != 1){
                this.globalData.renderer.restore();
            }
            if(cacheFlag){
                cache.push({
                    type: stylesList[i].type,
                    opacity: stylesList[i].opacity,
                    value: stylesList[i].value,
                    path: stylesList[i].path
                });
            }
        }
    }
    this.globalData.renderer.restore();
    if(cacheFlag){
        this.renderedPaths[this.globalData.frameNum] = cache;
    }
};

CVShapeItemElement.prototype.prepareFrame = function(num){
    this.frameNum = num;
    var i,len=this.dataLength-1;
    for(i=len;i>=0;i-=1){
        if(this.data[i].ty == 'gr'){
            this.data[i].item.prepareFrame(num);
        }
    }
};

CVShapeItemElement.prototype.renderShape = function(parentTransform,parentStylesList){
    if(this.renderedPaths[this.globalData.frameNum]){
        this.drawPaths(false);
        return;
    }
    this.transform.opacity = 1;
    var i, len;
    this.ownStylesList.length = 0;
    this.currentStylePoolPos = 0;
    if(!parentStylesList){
        this.stylesList.length = 0;
    }else{
        this.stylesList = parentStylesList;
    }
    if(parentTransform){
        this.transform.mat.props[0] = parentTransform.mat.props[0];
        this.transform.mat.props[1] = parentTransform.mat.props[1];
        this.transform.mat.props[2] = parentTransform.mat.props[2];
        this.transform.mat.props[3] = parentTransform.mat.props[3];
        this.transform.mat.props[4] = parentTransform.mat.props[4];
        this.transform.mat.props[5] = parentTransform.mat.props[5];
        this.transform.opacity *= parentTransform.opacity;
    }else{
        this.transform.mat.props[0] = this.transform.mat.props[3] = 1;
        this.transform.mat.props[1] = this.transform.mat.props[2] = this.transform.mat.props[4] = this.transform.mat.props[5] = 0;
    }
    len = this.dataLength - 1;
    for(i=len;i>=0;i-=1){
        if(this.data[i].ty == 'gr'){
            this.data[i].item.renderShape(this.transform,this.stylesList);
        }else if(this.data[i].ty == 'tr'){
            this.renderTransform(this.data[i]);
        }else if(this.data[i].ty == 'sh'){
            this.renderPath(this.data[i]);
        }else if(this.data[i].ty == 'el'){
            this.renderEllipse(this.data[i]);
        }else if(this.data[i].ty == 'rc'){
            if(this.data[i].trimmed){
                this.renderPath(this.data[i]);
            }else{
                this.renderRect(this.data[i]);
            }
        }else if(this.data[i].ty == 'fl'){
            this.renderFill(this.data[i]);
        }else if(this.data[i].ty == 'st'){
            this.renderStroke(this.data[i]);
        }else{
            //console.log(this.data[i].ty);
        }
    }
    if(this.mainFlag){
        this.drawPaths(true);
    }else{
        len = this.ownStylesList.length;
        for(i=0;i<len;i+=1){
            this.ownStylesList[i].closed = true;
        }
    }
};

CVShapeItemElement.prototype.renderTransform = function(animData){
    var tr = animData.renderedData[this.frameNum];
    var matrixValue = tr.mtArr;
    this.transform.mat.transform(matrixValue[0],matrixValue[1],matrixValue[2],matrixValue[3],matrixValue[4],matrixValue[5]).translate(-tr.a[0],-tr.a[1]);
    this.transform.opacity *= tr.o;
};

CVShapeItemElement.prototype.renderPath = function(data){
    if(data.trimmed){
        var ctx = this.canvasContext;
        ctx.lineCap = 'round';
        ctx.lineJoin = 'round';
    }
    var path = data.renderedData[this.frameNum].path;
    var path2d = new BM_Path2D();
    var pathNodes = path.pathNodes;
    if(pathNodes instanceof Array){
        pathNodes = pathNodes[0];
    }
    if(!pathNodes.v){
        return;
    }
    var i,len = pathNodes.v.length;
    var stops = pathNodes.s ? pathNodes.s : [];
    for(i=1;i<len;i+=1){
        if(stops[i-1]){
            path2d.moveTo(stops[i-1][0],stops[i-1][1]);
        }else if(i==1){
            path2d.moveTo(pathNodes.v[0][0],pathNodes.v[0][1]);
        }
        path2d.bezierCurveTo(pathNodes.o[i-1][0],pathNodes.o[i-1][1],pathNodes.i[i][0],pathNodes.i[i][1],pathNodes.v[i][0],pathNodes.v[i][1]);
    }
    if(len == 1){
        if(stops[0]){
            path2d.moveTo(stops[0][0],stops[0][1]);
        }else{
            path2d.moveTo(pathNodes.v[0][0],pathNodes.v[0][1]);
        }
    }
    if(data.closed && !(data.trimmed && !pathNodes.c)){
        path2d.bezierCurveTo(pathNodes.o[i-1][0],pathNodes.o[i-1][1],pathNodes.i[0][0],pathNodes.i[0][1],pathNodes.v[0][0],pathNodes.v[0][1]);
    }
    this.addPathToStyles(path2d);
};

CVShapeItemElement.prototype.renderEllipse = function(animData){
    var path2d = new BM_Path2D();
    var ell = animData.renderedData[this.frameNum];
    path2d.moveTo(ell.p[0]+ell.size[0]/2,ell.p[1]);
    path2d.ellipse(ell.p[0], ell.p[1], ell.size[0]/2, ell.size[1]/2, 0, 0, Math.PI*2, false);
    this.addPathToStyles(path2d);
};

CVShapeItemElement.prototype.renderRect = function(animData){
    var path2d = new BM_Path2D();
    var rect = animData.renderedData[this.frameNum];
    var roundness = rect.roundness;
    if(roundness === 0){
        path2d.rect(rect.position[0] - rect.size[0]/2,rect.position[1] - rect.size[1]/2,rect.size[0],rect.size[1]);
    }else{
        var x = rect.position[0] - rect.size[0]/2;
        var y = rect.position[1] - rect.size[1]/2;
        var w = rect.size[0];
        var h = rect.size[1];
        if(roundness instanceof Array){
            roundness = roundness[0];
        }
        if(roundness > w/2){
            roundness = w/2;
        }
        if(roundness > h/2){
            roundness = h/2;
        }
        path2d.moveTo(x + roundness, y);
        path2d.lineTo(x + w - roundness, y);
        path2d.quadraticCurveTo(x+w, y, x+w, y+roundness);
        path2d.lineTo(x+w, y+h-roundness);
        path2d.quadraticCurveTo(x+w, y+h, x+w-roundness, y+h);
        path2d.lineTo(x+roundness, y+h);
        path2d.quadraticCurveTo(x, y+h, x, y+h-roundness);
        path2d.lineTo(x, y+roundness);
        path2d.quadraticCurveTo(x, y, x+roundness, y);
    }
    this.addPathToStyles(path2d);
};

CVShapeItemElement.prototype.addPathToStyles = function(path2d){
    var i, len = this.stylesList.length;
    var canFill = true, strokeWidth = 0;
    for(i=len-1;i>=0;i-=1){
        if(!this.stylesList[i].closed){
            if(this.stylesList[i].type == 'stroke'){
                if(this.stylesList[i].width > strokeWidth){
                    this.stylesList[i].path.addPath(path2d, this.transform.mat.props);
                }
                if(this.stylesList[i].styleOpacity == 1 && this.stylesList[i].opacity == 1){
                    strokeWidth = this.stylesList[i].width;
                }
            }else if(canFill && this.stylesList[i].type == 'fill'){
                this.stylesList[i].path.addPath(path2d, this.transform.mat.props);
                if(this.stylesList[i].styleOpacity == 1 && this.stylesList[i].opacity == 1){
                    canFill = false;
                }
            }
        }
    }
};

CVShapeItemElement.prototype.renderFill = function(animData){
    var fill = animData.renderedData[this.frameNum];
    if(animData.fillEnabled!==false){
        this.stylesPool[this.currentStylePoolPos].path = new BM_Path2D();
        this.stylesPool[this.currentStylePoolPos].closed = false;
        this.stylesPool[this.currentStylePoolPos].styleOpacity = fill.opacity < 1 ? fill.opacity : 1;
        this.stylesPool[this.currentStylePoolPos].opacity = this.transform.opacity;
        this.stylesPool[this.currentStylePoolPos].value = fill.opacity < 1 ? fillColorToString(fill.color, fill.opacity) : fillColorToString(fill.color);
        this.stylesList.push(this.stylesPool[this.currentStylePoolPos]);
        this.ownStylesList.push(this.stylesList[this.stylesList.length -1]);
        this.currentStylePoolPos += 1;
        return;
    }
    this.stylesList.push(this.stylesPool[this.currentStylePoolPos]);
    this.ownStylesList.push(this.stylesList[this.stylesList.length -1]);
    this.currentStylePoolPos += 1;
};

CVShapeItemElement.prototype.renderStroke = function(animData){
    var stroke = animData.renderedData[this.frameNum];
    if(this.data.strokeEnabled!==false){
        this.stylesPool[this.currentStylePoolPos].path = new BM_Path2D();
        this.stylesPool[this.currentStylePoolPos].closed = false;
        this.stylesPool[this.currentStylePoolPos].styleOpacity = stroke.opacity < 1 ? stroke.opacity : 1;
        this.stylesPool[this.currentStylePoolPos].width = stroke.width;
        this.stylesPool[this.currentStylePoolPos].opacity = this.transform.opacity;
        this.stylesPool[this.currentStylePoolPos].value = stroke.opacity < 1 ? fillColorToString(stroke.color, stroke.opacity) : fillColorToString(stroke.color);

        if(stroke.dashes){
            var d = stroke.dashes;
            var j, jLen = d.length;
            var dasharray = [];
            var dashoffset = '';
            for(j=0;j<jLen;j+=1){
                if(d[j].n != 'o'){
                    dasharray.push(d[j].v);
                }else{
                    dashoffset = d[j].v;
                }
            }
            this.stylesPool[this.currentStylePoolPos].dasharray = dasharray;
            this.stylesPool[this.currentStylePoolPos].dashoffset = dashoffset;
        }
        this.stylesList.push(this.stylesPool[this.currentStylePoolPos]);
        this.ownStylesList.push(this.stylesList[this.stylesList.length -1]);
        this.currentStylePoolPos += 1;
        return;
    }
    this.stylesList.push(this.stylesPool[this.currentStylePoolPos]);
    this.ownStylesList.push(this.stylesList[this.stylesList.length -1]);
    this.currentStylePoolPos += 1;
};

CVShapeItemElement.prototype.destroy = function(){
    this.data = null;
    this.globalData = null;
    this.canvasContext = null;
};
function CVSolidElement(data,globalData){
    this.parent.constructor.call(this,data,globalData);
}
createElement(CVBaseElement, CVSolidElement);

CVSolidElement.prototype.draw = function(parentMatrix){
    if(this.parent.draw.call(this, parentMatrix)===false){
        return;
    }
    var ctx = this.canvasContext;
    this.globalData.renderer.save();
    var finalMat = this.finalTransform.mat.props;
    this.globalData.renderer.ctxTransform(finalMat);
    this.globalData.renderer.ctxOpacity(this.finalTransform.opacity);

    ctx.fillStyle=this.data.color;
    ctx.fillRect(0,0,this.data.width,this.data.height);
    this.globalData.renderer.restore(this.data.hasMask);
};
function CVTextElement(data, animationItem){
    this.parent.constructor.call(this,data, animationItem);
}
createElement(CVBaseElement, CVTextElement);

CVTextElement.prototype.createElements = function(){

    this.parent.createElements.call(this);
    /*this.svgElem = document.createElementNS (svgNS, "g");

    var textElement = document.createElementNS(svgNS,'text');
    textElement.textContent = this.data.textData.text;
    textElement.setAttribute('fill', this.data.textData.fillColor);
    textElement.setAttribute('x', '0');
    textElement.setAttribute('y',this.data.textData.height - (this.data.textData.fontSize-this.data.textData.height)/2);
    this.svgElem.setAttribute('width',this.data.textData.width);
    this.svgElem.setAttribute('height',this.data.textData.height);
    this.svgElem.style.transform=this.svgElem.style.webkitTransform='translate(' + this.data.textData.xOffset+"px," + this.data.textData.yOffset+"px)";
    textElement.setAttribute('font-size', this.data.textData.fontSize);
    textElement.setAttribute('font-family', "Arial, sans-serif");
    this.svgElem.appendChild(textElement);

    this.parent.createElements.call(this);

    this.anchorElement.appendChild(this.svgElem);
    this.maskedElement = textElement;*/
};

function CVMaskElement(){}

CVMaskElement.prototype.init = function () {
    this.registeredEffects = [];
    this.masksProperties = this.data.masksProperties;
    this.totalMasks = this.masksProperties.length;
    this.ctx = this.element.canvasContext;
    this.layerSize = this.element.getLayerSize();
    this.renderedFrames = new Array(this.globalData.totalFrames+1);
};

CVMaskElement.prototype.prepareFrame = function (num) {
    this.frameNum = num;
};

CVMaskElement.prototype.draw = function (transform) {
    var path;
    if(this.renderedFrames[this.globalData.frameNum]){
        path = this.renderedFrames[this.globalData.frameNum];
    }else{
        var tmpPath = new BM_Path2D();
        var i, len = this.data.masksProperties.length;
        path = new BM_Path2D();
        for (i = 0; i < len; i++) {
            if (this.masksProperties[i].inv) {
                this.createInvertedMask(tmpPath, this.data.masksProperties[i].paths[this.frameNum].pathNodes);
            }
            this.drawShape(tmpPath, this.data.masksProperties[i].paths[this.frameNum].pathNodes);
        }
        path.addPath(tmpPath,transform.mat.props);
        this.renderedFrames[this.globalData.frameNum] = path;
    }
    this.globalData.bmCtx.clip(path);
};

CVMaskElement.prototype.drawShape = function (path, data) {
    var j, jLen = data.v.length;
    path.moveTo(data.v[0][0], data.v[0][1]);
    for (j = 1; j < jLen; j++) {
        path.bezierCurveTo(data.o[j - 1][0], data.o[j - 1][1], data.i[j][0], data.i[j][1], data.v[j][0], data.v[j][1]);
    }
    path.bezierCurveTo(data.o[j - 1][0], data.o[j - 1][1], data.i[0][0], data.i[0][1], data.v[0][0], data.v[0][1]);
};

CVMaskElement.prototype.createInvertedMask = function(path){
    path.moveTo(0, 0);
    path.lineTo(this.globalData.compWidth, 0);
    path.lineTo(this.globalData.compWidth, this.globalData.compHeight);
    path.lineTo(0, this.globalData.compHeight);
    path.lineTo(0, 0);
};

CVMaskElement.prototype.destroy = function(){
    this.ctx = null;
};
var animationManager = (function(){
    var moduleOb = {};
    var registeredAnimations = window.registeredAnimations = [];
    var initTime = 0;
    var isPaused = true;
    var len = 0;

    function registerAnimation(element){
        if(!element){
            return null;
        }
        var i=0;
        while(i<len){
            if(registeredAnimations[i].elem == element && registeredAnimations[i].elem !== null ){
                return registeredAnimations[i].animation;
            }
            i+=1;
        }
        var animItem = new AnimationItem();
        animItem.setData(element);
        registeredAnimations.push({elem: element,animation:animItem});
        len += 1;
        return animItem;
    }

    function loadAnimation(params, name){
        var animItem = new AnimationItem();

        if (name) {
            animItem.name = name;
        }
        animItem.setParams(params);
        registeredAnimations.push({elem: null,animation:animItem});
        len += 1;
        return animItem;
    }


    function setSpeed(val,animation){
        var i;
        for(i=0;i<len;i+=1){
            registeredAnimations[i].animation.setSpeed(val, animation);
        }
    }

    function setDirection(val, animation){
        var i;
        for(i=0;i<len;i+=1){
            registeredAnimations[i].animation.setDirection(val, animation);
        }
    }

    function play(animation){
        var i;
        for(i=0;i<len;i+=1){
            registeredAnimations[i].animation.play(animation);
        }
    }

    function moveFrame (value, animation) {
        isPaused = false;
        initTime = Date.now();
        var i;
        for(i=0;i<len;i+=1){
            registeredAnimations[i].animation.moveFrame(value,animation);
        }
    }

    function resume() {
        var nowTime = Date.now();
        var elapsedTime = nowTime - initTime;
        var i;
        for(i=0;i<len;i+=1){
            if(registeredAnimations[i].animation.renderer.destroyed) {
                registeredAnimations.splice(i,1);
                i -= 1;
                len -= 1;
            }else{
                registeredAnimations[i].animation.advanceTime(elapsedTime);
            }
        }
        initTime = nowTime;
        //setTimeout(resume,10);
        requestAnimationFrame(resume);
    }

    function pause(animation) {
        var i;
        for(i=0;i<len;i+=1){
            registeredAnimations[i].animation.pause(animation);
        }
    }

    function goToAndStop(value,isFrame,animation) {
        var i;
        for(i=0;i<len;i+=1){
            registeredAnimations[i].animation.goToAndStop(value,isFrame,animation);
        }
    }

    function stop(animation) {
        var i;
        for(i=0;i<len;i+=1){
            registeredAnimations[i].animation.stop(animation);
        }
    }

    function togglePause(animation) {
        var i;
        for(i=0;i<len;i+=1){
            registeredAnimations[i].animation.togglePause(animation);
        }
    }

    function destroy(animation) {
        var i;
        for(i=0;i<len;i+=1){
            registeredAnimations[i].animation.destroy(animation);
        }
    }

    function searchAnimations(){
        var animElements = document.getElementsByClassName('bodymovin');
        Array.prototype.forEach.call(animElements,registerAnimation);
    }

    function resize(){
        var i;
        for(i=0;i<len;i+=1){
            registeredAnimations[i].animation.resize();
        }
    }

    function start(){
        initTime = Date.now();
        requestAnimationFrame(resume);
    }
    //start();

    setTimeout(start,0);

    moduleOb.registerAnimation = registerAnimation;
    moduleOb.loadAnimation = loadAnimation;
    moduleOb.setSpeed = setSpeed;
    moduleOb.setDirection = setDirection;
    moduleOb.play = play;
    moduleOb.moveFrame = moveFrame;
    moduleOb.pause = pause;
    moduleOb.stop = stop;
    moduleOb.togglePause = togglePause;
    moduleOb.searchAnimations = searchAnimations;
    moduleOb.resize = resize;
    moduleOb.start = start;
    moduleOb.goToAndStop = goToAndStop;
    moduleOb.destroy = destroy;
    return moduleOb;
}());
var AnimationItem = function () {
    this.name = '';
    this.path = '';
    this.isLoaded = false;
    this.currentFrame = 0;
    this.currentRawFrame = 0;
    this.totalFrames = 0;
    this.frameRate = 0;
    this.frameMult = 0;
    this.playSpeed = 1;
    this.playDirection = 1;
    this.pendingElements = 0;
    this.playCount = 0;
    this.prerenderFramesFlag = true;
    this.repeat = 'indefinite';
    this.animationData = {};
    this.layers = [];
    this.assets = [];
    this.isPaused = true;
    this.isScrolling = false;
    this.autoplay = false;
    this.loop = true;
    this.renderer = null;
    this.animationID = randomString(10);
    this.renderedFrameCount = 0;
    this.scaleMode = 'fit';
    this.math = Math;
    this.removed = false;
};

AnimationItem.prototype.setParams = function(params) {
    var self = this;
    if(params.context){
        this.context = params.context;
    }
    if(params.wrapper){
        this.wrapper = params.wrapper;
    }
    var animType = params.animType ? params.animType : 'canvas';
    switch(animType){
        case 'canvas':
            this.renderer = new CanvasRenderer(this, params.renderer);
            break;
        case 'svg':
            this.renderer = new SVGRenderer(this, params.renderer);
    }
    this.animType = animType;

    if(params.loop === '' || params.loop === null){
    }else if(params.loop === false){
        this.loop = false;
    }else if(params.loop === true){
        this.loop = true;
    }else{
        this.loop = parseInt(params.loop);
    }
    this.autoplay = 'autoplay' in params ? params.autoplay : true;
    this.name = this.name !== '' ? this.name : params.name ? params.name : '';
    this.prerenderFramesFlag = 'prerender' in params ? params.prerender : true;
    if(params.animationData){
        self.configAnimation(params.animationData);
    }else if(params.path){
        if(params.path.substr(-4) != 'json'){
            if (params.path.substr(-1, 1) != '/') {
                params.path += '/';
            }
            params.path += 'data.json';
        }

        var xhr = new XMLHttpRequest();
        this.path = params.path.substr(0,params.path.lastIndexOf('/')+1);
        xhr.open('GET', params.path, true);
        xhr.send();
        xhr.onreadystatechange = function () {
            if (xhr.readyState == 4) {
                if(xhr.status == 200){
                    self.configAnimation(JSON.parse(xhr.responseText));
                }else{
                    try{
                        var response = JSON.parse(xhr.responseText);
                        self.configAnimation(response);
                    }catch(err){
                    }
                }
            }
        };
    }
};

AnimationItem.prototype.setData = function (wrapper) {
    var params = {
        wrapper: wrapper
    };
    var wrapperAttributes = wrapper.attributes;

    params.path = wrapperAttributes.getNamedItem('data-animation-path') ? wrapperAttributes.getNamedItem('data-animation-path').value : wrapperAttributes.getNamedItem('data-bm-path') ? wrapperAttributes.getNamedItem('data-bm-path').value :  wrapperAttributes.getNamedItem('bm-path') ? wrapperAttributes.getNamedItem('bm-path').value : '';
    params.animType = wrapperAttributes.getNamedItem('data-anim-type') ? wrapperAttributes.getNamedItem('data-anim-type').value : wrapperAttributes.getNamedItem('data-bm-type') ? wrapperAttributes.getNamedItem('data-bm-type').value : wrapperAttributes.getNamedItem('bm-type') ? wrapperAttributes.getNamedItem('bm-type').value :  'canvas';

    var loop = wrapperAttributes.getNamedItem('data-anim-loop') ? wrapperAttributes.getNamedItem('data-anim-loop').value :  wrapperAttributes.getNamedItem('data-bm-loop') ? wrapperAttributes.getNamedItem('data-bm-loop').value :  wrapperAttributes.getNamedItem('bm-loop') ? wrapperAttributes.getNamedItem('bm-loop').value : '';
    if(loop === ''){
    }else if(loop === 'false'){
        params.loop = false;
    }else if(loop === 'true'){
        params.loop = true;
    }else{
        params.loop = parseInt(loop);
    }
    params.name = wrapperAttributes.getNamedItem('data-name') ? wrapperAttributes.getNamedItem('data-name').value :  wrapperAttributes.getNamedItem('data-bm-name') ? wrapperAttributes.getNamedItem('data-bm-name').value : wrapperAttributes.getNamedItem('bm-name') ? wrapperAttributes.getNamedItem('bm-name').value :  '';
    var prerender = wrapperAttributes.getNamedItem('data-anim-prerender') ? wrapperAttributes.getNamedItem('data-anim-prerender').value :  wrapperAttributes.getNamedItem('data-bm-prerender') ? wrapperAttributes.getNamedItem('data-bm-prerender').value :  wrapperAttributes.getNamedItem('bm-prerender') ? wrapperAttributes.getNamedItem('bm-prerender').value : '';

    if(prerender === 'false'){
        params.prerender = false;
    }
    this.setParams(params);
};

AnimationItem.prototype.configAnimation = function (animData) {
    this.renderer.configAnimation(animData);

    this.animationData = animData;
    this.animationData._id = this.animationID;
    this.animationData._animType = this.animType;
    this.layers = this.animationData.animation.layers;
    this.assets = this.animationData.assets;
    this.totalFrames = Math.floor(this.animationData.animation.totalFrames);
    this.frameRate = this.animationData.animation.frameRate;
    this.firstFrame = Math.round(this.animationData.animation.ff*this.frameRate);
    /*this.firstFrame = 0;
    this.totalFrames = 1;*/
    this.frameMult = this.animationData.animation.frameRate / 1000;
    dataManager.completeData(this.animationData);
    this.renderer.buildItems(this.animationData.animation.layers);
    this.updaFrameModifier();
    this.checkLoaded();
};

AnimationItem.prototype.elementLoaded = function () {
    this.pendingElements--;
    this.checkLoaded();
};

AnimationItem.prototype.checkLoaded = function () {
    if (this.pendingElements === 0) {
        this.renderer.buildStage(this.container, this.layers);
        if(this.prerenderFramesFlag){
            this.prerenderFrames(0);
            dataManager.renderFrame(this.animationData,this.currentFrame + this.firstFrame);
            this.renderer.renderFrame(this.currentFrame + this.firstFrame);
        }else{
            this.isLoaded = true;
            this.gotoFrame();
            if(this.autoplay){
                this.play();
            }
        }
    }
};

AnimationItem.prototype.prerenderFrames = function(count){
    if(!count){
        count = 0;
    }
    if(this.renderedFrameCount === Math.floor(this.totalFrames)){
        //TODO Need polyfill for ios 5.1
        this.isLoaded = true;
        this.gotoFrame();
        if(this.autoplay){
            this.play();
        }
    }else{
        dataManager.renderFrame(this.animationData,this.renderedFrameCount + this.firstFrame);
        this.renderedFrameCount+=1;
        if(count > 10){
            setTimeout(this.prerenderFrames.bind(this),0);
        }else{
            count += 1;
            this.prerenderFrames(count);
        }
    }
};

AnimationItem.prototype.resize = function () {
    this.renderer.updateContainerSize();
};

AnimationItem.prototype.gotoFrame = function () {
    if(subframeEnabled){
        this.currentFrame = this.math.round(this.currentRawFrame*100)/100;
    }else{
        this.currentFrame = this.math.floor(this.currentRawFrame);
    }
    this.renderFrame();
};

AnimationItem.prototype.renderFrame = function () {
    if(this.isLoaded === false){
        return;
    }
    dataManager.renderFrame(this.animationData,this.currentFrame + this.firstFrame);
    this.renderer.renderFrame(this.currentFrame + this.firstFrame);
};

AnimationItem.prototype.play = function (name) {
    if(name && this.name != name){
        return;
    }
    if(this.isPaused === true){
        this.isPaused = false;
    }
};

AnimationItem.prototype.pause = function (name) {
    if(name && this.name != name){
        return;
    }
    if(this.isPaused === false){
        this.isPaused = true;
    }
};

AnimationItem.prototype.togglePause = function (name) {
    if(name && this.name != name){
        return;
    }
    if(this.isPaused === true){
        this.isPaused = false;
        this.play();
    }else{
        this.isPaused = true;
        this.pause();
    }
};

AnimationItem.prototype.stop = function (name) {
    if(name && this.name != name){
        return;
    }
    this.isPaused = true;
    this.currentFrame = this.currentRawFrame = 0;
    this.playCount = 0;
    this.gotoFrame();
};

AnimationItem.prototype.goToAndStop = function (value, isFrame, name) {
    if(name && this.name != name){
        return;
    }
    if(isFrame){
        this.setCurrentRawFrameValue(value);
    }else{
        this.setCurrentRawFrameValue(value * this.frameModifier);
    }
    this.isPaused = true;
};

AnimationItem.prototype.advanceTime = function (value) {
    if (this.isPaused === true || this.isScrolling === true || this.isLoaded === false) {
        return;
    }
    this.setCurrentRawFrameValue(this.currentRawFrame + value * this.frameModifier);
};

AnimationItem.prototype.updateAnimation = function (perc) {
    this.setCurrentRawFrameValue(this.totalFrames * perc);
};

AnimationItem.prototype.moveFrame = function (value, name) {
    if(name && this.name != name){
        return;
    }
    this.setCurrentRawFrameValue(this.currentRawFrame+value);
};

AnimationItem.prototype.remove = function (name) {
    if(name && this.name != name){
        return;
    }
    this.renderer.destroy();
};

AnimationItem.prototype.destroy = function (name) {
    if((name && this.name != name) || (this.renderer && this.renderer.destroyed)){
        return;
    }
    this.renderer.destroy();
};

AnimationItem.prototype.setCurrentRawFrameValue = function(value){
    this.currentRawFrame = value;
    if (this.currentRawFrame >= this.totalFrames) {
        if(this.loop === false){
            this.currentRawFrame = this.totalFrames - 1;
            this.gotoFrame();
            this.pause();
            return;
        }else{
            this.playCount += 1;
            if(this.loop !== true){
                if(this.playCount == this.loop){
                    this.currentRawFrame = this.totalFrames - 1;
                    this.gotoFrame();
                    this.pause();
                    return;
                }
            }
        }
    } else if (this.currentRawFrame < 0) {
        this.playCount -= 1;
        if(this.playCount < 0){
            this.playCount = 0;
        }
        if(this.loop === false){
            this.currentRawFrame = 0;
            this.gotoFrame();
            this.pause();
            return;
        }else{
            this.currentRawFrame = this.totalFrames + this.currentRawFrame;
            this.gotoFrame();
            return;
        }
    }

    this.currentRawFrame = this.currentRawFrame % this.totalFrames;
    this.gotoFrame();
};

AnimationItem.prototype.setSpeed = function (val) {
    this.playSpeed = val;
    this.updaFrameModifier();
};

AnimationItem.prototype.setDirection = function (val) {
    this.playDirection = val < 0 ? -1 : 1;
    this.updaFrameModifier();
};

AnimationItem.prototype.updaFrameModifier = function () {
    this.frameModifier = this.frameMult * this.playSpeed * this.playDirection;
};

AnimationItem.prototype.getPath = function () {
    return this.path;
};

AnimationItem.prototype.getAssetData = function (id) {
    var i = 0, len = this.assets.length;
    while (i < len) {
        if(id == this.assets[i].id){
            return this.assets[i];
        }
        i += 1;
    }
    return this.assets;
};

AnimationItem.prototype.getAssets = function () {
    return this.assets;
};
(function (window){

    var bodymovinjs = {};

    function play(animation){
        animationManager.play(animation);
    }
    function pause(animation){
        animationManager.pause(animation);
    }
    function togglePause(animation){
        animationManager.togglePause(animation);
    }
    function setSpeed(value,animation){
        animationManager.setSpeed(value, animation);
    }
    function setDirection(value,animation){
        animationManager.setDirection(value, animation);
    }
    function stop(animation){
        animationManager.stop(animation);
    }
    function moveFrame(value){
        animationManager.moveFrame(value);
    }
    function searchAnimations(){
        animationManager.searchAnimations();
    }
    function registerAnimation(elem){
        return animationManager.registerAnimation(elem);
    }
    function resize(){
        animationManager.resize();
    }
    function start(){
        animationManager.start();
    }
    function goToAndStop(val,isFrame, animation){
        animationManager.goToAndStop(val,isFrame, animation);
    }
    function setSubframeRendering(flag){
        subframeEnabled = flag;
    }
    function loadAnimation(params, name){
        return animationManager.loadAnimation(params, name);
    }
    function destroy(animation){
        return animationManager.destroy(animation);
    }

    bodymovinjs.play = play;
    bodymovinjs.pause = pause;
    bodymovinjs.togglePause = togglePause;
    bodymovinjs.setSpeed = setSpeed;
    bodymovinjs.setDirection = setDirection;
    bodymovinjs.stop = stop;
    bodymovinjs.moveFrame = moveFrame;
    bodymovinjs.searchAnimations = searchAnimations;
    bodymovinjs.registerAnimation = registerAnimation;
    bodymovinjs.loadAnimation = loadAnimation;
    bodymovinjs.setSubframeRendering = setSubframeRendering;
    bodymovinjs.resize = resize;
    bodymovinjs.start = start;
    bodymovinjs.goToAndStop = goToAndStop;
    bodymovinjs.destroy = destroy;

    function checkReady(){
        if (document.readyState === "complete") {
            clearInterval(readyStateCheckInterval);
                searchAnimations();
        }
    }

    bodymovinjs.checkReady = checkReady;

    window.bodymovin = bodymovinjs;

    var readyStateCheckInterval = setInterval(checkReady, 100);

}(window));
}());