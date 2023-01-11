/**
 * @author Eberhard Graether / http://egraether.com/
 * @author Mark Lundin 	/ http://mark-lundin.com
 */

module.exports = function(THREE){

var _this, STATE;
var EPS = 0.000001;
var SF = 63600.0; // scale factor

// events
var changeEvent = { type: 'change' };
var startEvent = { type: 'start'};
var endEvent = { type: 'end'};

THREE.TrackballControls = function ( object, domElement ) {

	_this = this;
	STATE = { NONE: -1, ROTATE: 0, ZOOM: 1, PAN: 2, TOUCH_ROTATE: 3, TOUCH_ZOOM_PAN: 4 };
	this.states = STATE;

	this.object = object;
	this.domElement = ( domElement !== undefined ) ? domElement : document;

	// API

	this.enabled = true;

	this.screen = { left: 0, top: 0, width: 0, height: 0 };

	this.rotateSpeed = 0.1;
	this.zoomSpeed = 0.4;
	this.panSpeed = 0.1;

	this.noRotate = false;
	this.noZoom = false;
	this.noPan = false;
	this.noRoll = false;

	this.staticMoving = false;
	this.dynamicDampingFactor = 0.12;  // 0.2

	this.minDistance = 100.3 * SF;
	this.maxDistance = 3000.0 * SF;

	this.keys = [ 65 /*A*/, 83 /*S*/, 68 /*D*/ ];

	// internals

	this.target = new THREE.Vector3();

	this.lastPosition = new THREE.Vector3();

	this.state = STATE.NONE;
	this.prevState = STATE.NONE;

	this.rotateStart = new THREE.Vector3();
	this.rotateEnd = new THREE.Vector3();

	this.zoomStart = new THREE.Vector2();
	this.zoomEnd = new THREE.Vector2();

	this.touchZoomDistanceStart = 0;
	this.touchZoomDistanceEnd = 0;

	this.panStart = new THREE.Vector2();
	this.panEnd = new THREE.Vector2();
	
	this.eye = new THREE.Vector3();

	this.target0 = this.target.clone();
	this.position0 = this.object.position.clone();
	this.up0 = this.object.up.clone();

	


	// methods

	

	this.handleEvent = function ( event ) {

		if ( typeof this[ event.type ] == 'function' ) {

			this[ event.type ]( event );

		}

	};

	var getMouseOnScreen = ( function () {

		var vector = new THREE.Vector2();

		return function ( pageX, pageY ) {

			vector.set(
				( pageX - _this.screen.left ) / _this.screen.width,
				( pageY - _this.screen.top ) / _this.screen.height
			);

			return vector;

		};

	}() );

	var getMouseProjectionOnBall = ( function () {

		var vector = new THREE.Vector3();
		var objectUp = new THREE.Vector3();
		var mouseOnBall = new THREE.Vector3();

		return function ( pageX, pageY ) {

			mouseOnBall.set(
				( pageX - _this.screen.width * 0.5 - _this.screen.left ) / (_this.screen.width*.5),
				( _this.screen.height * 0.5 + _this.screen.top - pageY ) / (_this.screen.height*.5),
				0.0
			);

			var dampen = (_this.eye.length() - (_this.minDistance + 0.1))/(_this.maxDistance - _this.minDistance);

			var length = mouseOnBall.length();

			if ( _this.noRoll ) {

				// console.log("MP ", length, dampen);
				mouseOnBall.z = Math.sqrt( 1.0 - length*length );

				// if ( length < Math.SQRT1_2 ) {

				// 	mouseOnBall.z = Math.sqrt( 1.0 - length*length );

				// } else {

				// 	mouseOnBall.z = .5 / length;
					
				// }

			} else if ( length > 1.0 ) {

				mouseOnBall.normalize();

			} else {

				mouseOnBall.z = Math.sqrt( 1.0 - length * length );

			}

			_this.eye.copy( _this.object.position ).sub( _this.target );

			vector.copy( _this.object.up ).setLength( mouseOnBall.y )
			vector.add( objectUp.copy( _this.object.up ).cross( _this.eye ).setLength( mouseOnBall.x ) );
			vector.add( _this.eye.setLength( mouseOnBall.z ) );

			return vector;

		};

	}() );



	// listeners

	function keydown( event ) {

		if ( _this.enabled === false ) return;

		window.removeEventListener( 'keydown', keydown );

		_this.prevState = _this.state;

		if ( _this.state !== STATE.NONE ) {

			return;

		} else if ( event.keyCode === _this.keys[ STATE.ROTATE ] && !_this.noRotate ) {

			_this.state = STATE.ROTATE;

		} else if ( event.keyCode === _this.keys[ STATE.ZOOM ] && !_this.noZoom ) {

			_this.state = STATE.ZOOM;

		} else if ( event.keyCode === _this.keys[ STATE.PAN ] && !_this.noPan ) {

			_this.state = STATE.PAN;

		}

	}

	function keyup( event ) {

		if ( _this.enabled === false ) return;

		_this.state = _this.prevState;

		window.addEventListener( 'keydown', keydown, false );

	}

	function mousedown( event ) {

		if ( _this.enabled === false ) return;

		event.preventDefault();
		event.stopPropagation();

		if ( _this.state === STATE.NONE ) {

			_this.state = event.button;

		}

		if ( _this.state === STATE.ROTATE && !_this.noRotate ) {

			_this.rotateStart.copy( getMouseProjectionOnBall( event.pageX, event.pageY ) );
			_this.rotateEnd.copy( _this.rotateStart );

		} else if ( _this.state === STATE.ZOOM && !_this.noZoom ) {

			_this.zoomStart.copy( getMouseOnScreen( event.pageX, event.pageY ) );
			_this.zoomEnd.copy(_this.zoomStart);

		} else if ( _this.state === STATE.PAN && !_this.noPan ) {

			_this.panStart.copy( getMouseOnScreen( event.pageX, event.pageY ) );
			_this.panEnd.copy(_this.panStart)

		}

		document.addEventListener( 'mousemove', mousemove, false );
		document.addEventListener( 'mouseup', mouseup, false );

		_this.dispatchEvent( startEvent );

	}

	function mousemove( event ) {

		if ( _this.enabled === false ) return;

		event.preventDefault();
		event.stopPropagation();

		if ( _this.state === STATE.ROTATE && !_this.noRotate ) {

			_this.rotateEnd.copy( getMouseProjectionOnBall( event.pageX, event.pageY ) );

		} else if ( _this.state === STATE.ZOOM && !_this.noZoom ) {

			_this.zoomEnd.copy( getMouseOnScreen( event.pageX, event.pageY ) );

		} else if ( _this.state === STATE.PAN && !_this.noPan ) {

			_this.panEnd.copy( getMouseOnScreen( event.pageX, event.pageY ) );

		}

	}

	function mouseup( event ) {

		if ( _this.enabled === false ) return;

		event.preventDefault();
		event.stopPropagation();

		_this.state = STATE.NONE;

		document.removeEventListener( 'mousemove', mousemove );
		document.removeEventListener( 'mouseup', mouseup );
		_this.dispatchEvent( endEvent );

	}

	function mousewheel( event ) {

		if ( _this.enabled === false ) return;

		event.preventDefault();
		event.stopPropagation();

		var delta = 0;

		if ( event.wheelDelta ) { // WebKit / Opera / Explorer 9

			delta = event.wheelDelta / 40;

		} else if ( event.detail ) { // Firefox

			delta = - event.detail / 3;

		}

		_this.zoomStart.y += delta * 0.01;
		_this.dispatchEvent( startEvent );
		_this.dispatchEvent( endEvent );

	}

	function touchstart( event ) {

		if ( _this.enabled === false ) return;

		switch ( event.touches.length ) {

			case 1:
				_this.state = STATE.TOUCH_ROTATE;
				_this.rotateStart.copy( getMouseProjectionOnBall( event.touches[ 0 ].pageX, event.touches[ 0 ].pageY ) );
				_this.rotateEnd.copy( _this.rotateStart );
				break;

			case 2:
				_this.state = STATE.TOUCH_ZOOM_PAN;
				var dx = event.touches[ 0 ].pageX - event.touches[ 1 ].pageX;
				var dy = event.touches[ 0 ].pageY - event.touches[ 1 ].pageY;
				_this.touchZoomDistanceEnd = _this.touchZoomDistanceStart = Math.sqrt( dx * dx + dy * dy );

				var x = ( event.touches[ 0 ].pageX + event.touches[ 1 ].pageX ) / 2;
				var y = ( event.touches[ 0 ].pageY + event.touches[ 1 ].pageY ) / 2;
				_this.panStart.copy( getMouseOnScreen( x, y ) );
				_this.panEnd.copy( _this.panStart );
				break;

			default:
				_this.state = STATE.NONE;

		}
		_this.dispatchEvent( startEvent );


	}

	function touchmove( event ) {

		if ( _this.enabled === false ) return;

		event.preventDefault();
		event.stopPropagation();

		switch ( event.touches.length ) {

			case 1:
				_this.rotateEnd.copy( getMouseProjectionOnBall( event.touches[ 0 ].pageX, event.touches[ 0 ].pageY ) );
				break;

			case 2:
				var dx = event.touches[ 0 ].pageX - event.touches[ 1 ].pageX;
				var dy = event.touches[ 0 ].pageY - event.touches[ 1 ].pageY;
				_this.touchZoomDistanceEnd = Math.sqrt( dx * dx + dy * dy );

				var x = ( event.touches[ 0 ].pageX + event.touches[ 1 ].pageX ) / 2;
				var y = ( event.touches[ 0 ].pageY + event.touches[ 1 ].pageY ) / 2;
				_this.panEnd.copy( getMouseOnScreen( x, y ) );
				break;

			default:
				_this.state = STATE.NONE;

		}

	}

	function touchend( event ) {

		if ( _this.enabled === false ) return;

		switch ( event.touches.length ) {

			case 1:
				_this.rotateEnd.copy( getMouseProjectionOnBall( event.touches[ 0 ].pageX, event.touches[ 0 ].pageY ) );
				_this.rotateStart.copy( _this.rotateEnd );
				break;

			case 2:
				_this.touchZoomDistanceStart = _this.touchZoomDistanceEnd = 0;

				var x = ( event.touches[ 0 ].pageX + event.touches[ 1 ].pageX ) / 2;
				var y = ( event.touches[ 0 ].pageY + event.touches[ 1 ].pageY ) / 2;
				_this.panEnd.copy( getMouseOnScreen( x, y ) );
				_this.panStart.copy( _this.panEnd );
				break;

		}

		_this.state = STATE.NONE;
		_this.dispatchEvent( endEvent );

	}

	this.domElement.addEventListener( 'contextmenu', function ( event ) { event.preventDefault(); }, false );

	this.domElement.addEventListener( 'mousedown', mousedown, false );

	this.domElement.addEventListener( 'mousewheel', mousewheel, false );
	this.domElement.addEventListener( 'DOMMouseScroll', mousewheel, false ); // firefox

	this.domElement.addEventListener( 'touchstart', touchstart, false );
	this.domElement.addEventListener( 'touchend', touchend, false );
	this.domElement.addEventListener( 'touchmove', touchmove, false );

	window.addEventListener( 'keydown', keydown, false );
	window.addEventListener( 'keyup', keyup, false );

	this.handleResize();

	// force an update at start
	this.update();

};

THREE.TrackballControls.prototype = Object.create( THREE.EventDispatcher.prototype );
THREE.TrackballControls.prototype.constructor = THREE.TrackballControls;


/**
 *  In order for methods to be overwritten we add them to prototype chain
 */

THREE.TrackballControls.prototype.zoomCamera = function () {

	if ( _this.state === STATE.TOUCH_ZOOM_PAN ) {

		var factor = _this.touchZoomDistanceStart / _this.touchZoomDistanceEnd;
		_this.touchZoomDistanceStart = _this.touchZoomDistanceEnd;
		_this.eye.multiplyScalar( factor );

	} else {

		var dampen = (_this.eye.length() - (_this.minDistance + 0.1))/(_this.maxDistance - _this.minDistance);
		var factor = 1.0 + ( _this.zoomEnd.y - _this.zoomStart.y ) * _this.zoomSpeed * dampen;

		if ( factor !== 1.0 && factor > 0.0 ) {

			_this.eye.multiplyScalar( factor );

			if ( _this.staticMoving ) {

				_this.zoomStart.copy( _this.zoomEnd );

			} else {

				_this.zoomStart.y += ( _this.zoomEnd.y - _this.zoomStart.y ) * this.dynamicDampingFactor;

			}

		}

	}

};

THREE.TrackballControls.prototype.handleResize = function () {

	if ( this.domElement === document ) {

		this.screen.left = 0;
		this.screen.top = 0;
		this.screen.width = window.innerWidth;
		this.screen.height = window.innerHeight;

	} else {

		var box = this.domElement.getBoundingClientRect();
		// adjustments come from similar code in the jquery offset() function
		var d = this.domElement.ownerDocument.documentElement;
		this.screen.left = box.left + window.pageXOffset - d.clientLeft;
		this.screen.top = box.top + window.pageYOffset - d.clientTop;
		this.screen.width = box.width;
		this.screen.height = box.height;

	}

};

THREE.TrackballControls.prototype.rotateCamera = (function(){

	var axis = new THREE.Vector3(),
		quaternion = new THREE.Quaternion();


	return function () {

		var angle = Math.acos( _this.rotateStart.dot( _this.rotateEnd ) / _this.rotateStart.length() / _this.rotateEnd.length() );

		if ( angle ) {

			axis.crossVectors( _this.rotateStart, _this.rotateEnd ).normalize();

			var dampen = Math.min((_this.eye.length() - (_this.minDistance - 0.01))/(_this.maxDistance - _this.minDistance), 1.0);
			// console.log("DAMPEN MM ", dampen);
			angle *= _this.rotateSpeed * dampen;

			quaternion.setFromAxisAngle( axis, -angle );

			_this.eye.applyQuaternion( quaternion );
			_this.object.up.applyQuaternion( quaternion );

			_this.rotateEnd.applyQuaternion( quaternion );

			// force static for rotation
			if ( true || _this.staticMoving ) {

				_this.rotateStart.copy( _this.rotateEnd );

			} else {

				quaternion.setFromAxisAngle( axis, angle * ( _this.dynamicDampingFactor - 1.0 ) );
				_this.rotateStart.applyQuaternion( quaternion );

			}

		}
	}

}());


THREE.TrackballControls.prototype.panCamera = (function(){

	var mouseChange = new THREE.Vector2(),
		objectUp = new THREE.Vector3(),
		pan = new THREE.Vector3();

	return function () {

		mouseChange.copy( _this.panEnd ).sub( _this.panStart );

		if ( mouseChange.lengthSq() ) {

			var dampen = Math.min((_this.eye.length() - (_this.minDistance - 0.01))/(_this.maxDistance - _this.minDistance), 1.0);
			mouseChange.multiplyScalar( _this.eye.length() * _this.panSpeed * dampen );

			pan.copy( _this.eye ).cross( _this.object.up ).setLength( mouseChange.x );
			pan.add( objectUp.copy( _this.object.up ).setLength( mouseChange.y ) );

			_this.object.position.add( pan );
			_this.target.add( pan );

			if ( _this.staticMoving ) {

				_this.panStart.copy( _this.panEnd );

			} else {

				_this.panStart.add( mouseChange.subVectors( _this.panEnd, _this.panStart ).multiplyScalar( _this.dynamicDampingFactor ) );

			}

		}
	}

}());

THREE.TrackballControls.prototype.checkDistances = function () {

	if ( !_this.noZoom || !_this.noPan ) {

		if ( _this.eye.lengthSq() > _this.maxDistance * _this.maxDistance ) {

			_this.object.position.addVectors( _this.target, _this.eye.setLength( _this.maxDistance ) );

		}

		if ( _this.eye.lengthSq() < _this.minDistance * _this.minDistance ) {

			_this.object.position.addVectors( _this.target, _this.eye.setLength( _this.minDistance ) );

		}

	}

};

THREE.TrackballControls.prototype.update = function () {

	_this.eye.subVectors( _this.object.position, _this.target );

	if ( !_this.noRotate ) {

		_this.rotateCamera();

	}

	if ( !_this.noZoom ) {

		_this.zoomCamera();

	}

	if ( !_this.noPan ) {

		_this.panCamera();

	}

	_this.object.position.addVectors( _this.target, _this.eye );

	_this.checkDistances();

	_this.object.lookAt( _this.target );

	if ( _this.lastPosition.distanceToSquared( _this.object.position ) > EPS ) {

		_this.dispatchEvent( changeEvent );

		_this.lastPosition.copy( _this.object.position );

	}

};

THREE.TrackballControls.prototype.reset = function () {

	_this.state = STATE.NONE;
	_this.prevState = STATE.NONE;

	_this.target.copy( _this.target0 );
	_this.object.position.copy( _this.position0 );
	_this.object.up.copy( _this.up0 );

	_this.eye.subVectors( _this.object.position, _this.target );

	_this.object.lookAt( _this.target );

	_this.dispatchEvent( changeEvent );

	_this.lastPosition.copy( _this.object.position );

};


return THREE.TrackballControls;
}
