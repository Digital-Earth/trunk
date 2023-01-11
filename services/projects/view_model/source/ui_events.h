#pragma once
#ifndef VIEW_MODEL__UI_EVENTS_H
#define VIEW_MODEL__UI_EVENTS_H
/******************************************************************************
ui_events.h

begin		: 2010-01-01
copyright	: (C) 2010 by the PYXIS innovation inc.
web			: www.pyxisinnovation.com
******************************************************************************/

#include "pyxis\utility\notifier.h"

class IUIContorllerContext;

/*! UIEvent

the UIEvent is a base class that save the context data about a UI Event. the context data is:
1. The UIControllerContext (like Form in .Net) - the context in which this event is living. then the handler can get access to the ControllerContext.
2. The Alt/Shift/Ctrl keys state while the event was triggered.
3. event Consumed flag. once an event is marked as consumed, it would not be propegated to the next IUIController in list

*/
//! UIEvent - base class for repesnting an UI event.
class VIEW_MODEL_API UIEvent : public NotifierEvent
{
public:
	
	UIEvent(const bool & altKey,const bool & shiftKey,const bool & ctrlKey) : m_eventConsumed(false), m_altKey(altKey), m_shiftKey(shiftKey), m_ctrlKey(ctrlKey)//, m_controllerContext(NULL)
	{
	}

	static PYXPointer<UIEvent> create(const bool & altKey,const bool & shiftKey,const bool & ctrlKey) 
	{ 
		return PYXNEW(UIEvent,altKey,shiftKey,ctrlKey); 
	}
	
	virtual ~UIEvent()
	{
	}

protected:
	//IUIContorllerContext * m_controllerContext;

	bool m_eventConsumed;

	bool m_altKey;
	bool m_ctrlKey;
	bool m_shiftKey;

public:
	//! check if event is consumed	
	const bool & isConsumed() const { return m_eventConsumed; }

	//! set event to be consumed
	void setConsumed() { m_eventConsumed = true; }

	const bool & isAltKeyPressed() const { return m_altKey; }
	const bool & isCtrlKeyPressed() const { return m_ctrlKey; }
	const bool & isShiftKeyPressed() const { return m_shiftKey; }

	/*
	//! return the Context. Note, the context must be set before calling this function
	IUIContorllerContext & getContext() { return *m_controllerContext; };

	//! check if the Context as been set
	bool hasContext() const { return m_controllerContext != NULL; };

	//! set the context for this event
	void setContext(IUIContorllerContext * context) { m_controllerContext = context; };
	*/
};

/*! class for KeyEvent: KeyDown,KeyUp and KeyPressed events

the KeyEvent has two members for KeyChar and KeyCode because .Net can give us KeyChar or KeyCode.
use the UIKeyCode enum to compare with keyCode values.
	
*/
//! class for KeyEvent: KeyDown,KeyUp and KeyPressed events
class VIEW_MODEL_API UIKeyEvent : public UIEvent
{
public:	
	UIKeyEvent(const char & keyChar,const int & keyCode,const bool & altKey,const bool & shiftKey,const bool & ctrlKey) 
		: UIEvent(altKey,shiftKey,ctrlKey), m_keyChar(keyChar), m_keyCode(keyCode)
	{
	}

	static PYXPointer<UIKeyEvent> create(const char & keyChar,const int & keyCode,const bool & altKey,const bool & shiftKey,const bool & ctrlKey) 
	{ 
		return PYXNEW(UIKeyEvent,keyChar,keyCode,altKey,shiftKey,ctrlKey); 
	}
	
	virtual ~UIKeyEvent()
	{
	}

protected:
	char m_keyChar;
	int  m_keyCode;

public:
	const char & getKeyChar() const { return m_keyChar; };
	const int &  getKeyCode() const { return m_keyCode; };

	bool isKeyChar(const char & chr) { return m_keyChar == chr; };	
};


/* Copyed from .Net Keys enum */
// Summary:
//     Specifies key codes and modifiers.    
enum UIKeyCode
{
    // Summary:
    //     The bitmask to extract modifiers from a key value.
    knKeys_Modifiers = -65536,
    //
    // Summary:
    //     No key pressed.
    knKeys_None = 0,
    //
    // Summary:
    //     The left mouse button.
    knKeys_LButton = 1,
    //
    // Summary:
    //     The right mouse button.
    knKeys_RButton = 2,
    //
    // Summary:
    //     The CANCEL key.
    knKeys_Cancel = 3,
    //
    // Summary:
    //     The middle mouse button (three-button mouse).
    knKeys_MButton = 4,
    //
    // Summary:
    //     The first x mouse button (five-button mouse).
    knKeys_XButton1 = 5,
    //
    // Summary:
    //     The second x mouse button (five-button mouse).
    knKeys_XButton2 = 6,
    //
    // Summary:
    //     The BACKSPACE key.
    knKeys_Back = 8,
    //
    // Summary:
    //     The TAB key.
    knKeys_Tab = 9,
    //
    // Summary:
    //     The LINEFEED key.
    knKeys_LineFeed = 10,
    //
    // Summary:
    //     The CLEAR key.
    knKeys_Clear = 12,
    //
    // Summary:
    //     The ENTER key.
    knKeys_Enter = 13,
    //
    // Summary:
    //     The RETURN key.
    knKeys_Return = 13,
    //
    // Summary:
    //     The SHIFT key.
    knKeys_ShiftKey = 16,
    //
    // Summary:
    //     The CTRL key.
    knKeys_ControlKey = 17,
    //
    // Summary:
    //     The ALT key.
    knKeys_Menu = 18,
    //
    // Summary:
    //     The PAUSE key.
    knKeys_Pause = 19,
    //
    // Summary:
    //     The CAPS LOCK key.
    knKeys_CapsLock = 20,
    //
    // Summary:
    //     The CAPS LOCK key.
    knKeys_Capital = 20,
    //
    // Summary:
    //     The IME Kana mode key.
    knKeys_KanaMode = 21,
    //
    // Summary:
    //     The IME Hanguel mode key. (maintained for compatibility; use HangulMode)
    knKeys_HanguelMode = 21,
    //
    // Summary:
    //     The IME Hangul mode key.
    knKeys_HangulMode = 21,
    //
    // Summary:
    //     The IME Junja mode key.
    knKeys_JunjaMode = 23,
    //
    // Summary:
    //     The IME final mode key.
    knKeys_FinalMode = 24,
    //
    // Summary:
    //     The IME Kanji mode key.
    knKeys_KanjiMode = 25,
    //
    // Summary:
    //     The IME Hanja mode key.
    knKeys_HanjaMode = 25,
    //
    // Summary:
    //     The ESC key.
    knKeys_Escape = 27,
    //
    // Summary:
    //     The IME convert key.
    knKeys_IMEConvert = 28,
    //
    // Summary:
    //     The IME nonconvert key.
    knKeys_IMENonconvert = 29,
    //
    // Summary:
    //     The IME accept key. Obsolete, use System.Windows.Forms.Keys.IMEAccept instead.
    knKeys_IMEAceept = 30,
    //
    // Summary:
    //     The IME accept key, replaces System.Windows.Forms.Keys.IMEAceept.
    knKeys_IMEAccept = 30,
    //
    // Summary:
    //     The IME mode change key.
    knKeys_IMEModeChange = 31,
    //
    // Summary:
    //     The SPACEBAR key.
    knKeys_Space = 32,
    //
    // Summary:
    //     The PAGE UP key.
    knKeys_Prior = 33,
    //
    // Summary:
    //     The PAGE UP key.
    knKeys_PageUp = 33,
    //
    // Summary:
    //     The PAGE DOWN key.
    knKeys_Next = 34,
    //
    // Summary:
    //     The PAGE DOWN key.
    knKeys_PageDown = 34,
    //
    // Summary:
    //     The END key.
    knKeys_End = 35,
    //
    // Summary:
    //     The HOME key.
    knKeys_Home = 36,
    //
    // Summary:
    //     The LEFT ARROW key.
    knKeys_Left = 37,
    //
    // Summary:
    //     The UP ARROW key.
    knKeys_Up = 38,
    //
    // Summary:
    //     The RIGHT ARROW key.
    knKeys_Right = 39,
    //
    // Summary:
    //     The DOWN ARROW key.
    knKeys_Down = 40,
    //
    // Summary:
    //     The SELECT key.
    knKeys_Select = 41,
    //
    // Summary:
    //     The PRINT key.
    knKeys_Print = 42,
    //
    // Summary:
    //     The EXECUTE key.
    knKeys_Execute = 43,
    //
    // Summary:
    //     The PRINT SCREEN key.
    knKeys_PrintScreen = 44,
    //
    // Summary:
    //     The PRINT SCREEN key.
    knKeys_Snapshot = 44,
    //
    // Summary:
    //     The INS key.
    knKeys_Insert = 45,
    //
    // Summary:
    //     The DEL key.
    knKeys_Delete = 46,
    //
    // Summary:
    //     The HELP key.
    knKeys_Help = 47,
    //
    // Summary:
    //     The 0 key.
    knKeys_D0 = 48,
    //
    // Summary:
    //     The 1 key.
    knKeys_D1 = 49,
    //
    // Summary:
    //     The 2 key.
    knKeys_D2 = 50,
    //
    // Summary:
    //     The 3 key.
    knKeys_D3 = 51,
    //
    // Summary:
    //     The 4 key.
    knKeys_D4 = 52,
    //
    // Summary:
    //     The 5 key.
    knKeys_D5 = 53,
    //
    // Summary:
    //     The 6 key.
    knKeys_D6 = 54,
    //
    // Summary:
    //     The 7 key.
    knKeys_D7 = 55,
    //
    // Summary:
    //     The 8 key.
    knKeys_D8 = 56,
    //
    // Summary:
    //     The 9 key.
    knKeys_D9 = 57,
    //
    // Summary:
    //     The A key.
    knKeys_A = 65,
    //
    // Summary:
    //     The B key.
    knKeys_B = 66,
    //
    // Summary:
    //     The C key.
    knKeys_C = 67,
    //
    // Summary:
    //     The D key.
    knKeys_D = 68,
    //
    // Summary:
    //     The E key.
    knKeys_E = 69,
    //
    // Summary:
    //     The F key.
    knKeys_F = 70,
    //
    // Summary:
    //     The G key.
    knKeys_G = 71,
    //
    // Summary:
    //     The H key.
    knKeys_H = 72,
    //
    // Summary:
    //     The I key.
    knKeys_I = 73,
    //
    // Summary:
    //     The J key.
    knKeys_J = 74,
    //
    // Summary:
    //     The K key.
    knKeys_K = 75,
    //
    // Summary:
    //     The L key.
    knKeys_L = 76,
    //
    // Summary:
    //     The M key.
    knKeys_M = 77,
    //
    // Summary:
    //     The N key.
    knKeys_N = 78,
    //
    // Summary:
    //     The O key.
    knKeys_O = 79,
    //
    // Summary:
    //     The P key.
    knKeys_P = 80,
    //
    // Summary:
    //     The Q key.
    knKeys_Q = 81,
    //
    // Summary:
    //     The R key.
    knKeys_R = 82,
    //
    // Summary:
    //     The S key.
    knKeys_S = 83,
    //
    // Summary:
    //     The T key.
    knKeys_T = 84,
    //
    // Summary:
    //     The U key.
    knKeys_U = 85,
    //
    // Summary:
    //     The V key.
    knKeys_V = 86,
    //
    // Summary:
    //     The W key.
    knKeys_W = 87,
    //
    // Summary:
    //     The X key.
    knKeys_X = 88,
    //
    // Summary:
    //     The Y key.
    knKeys_Y = 89,
    //
    // Summary:
    //     The Z key.
    knKeys_Z = 90,
    //
    // Summary:
    //     The left Windows logo key (Microsoft Natural Keyboard).
    knKeys_LWin = 91,
    //
    // Summary:
    //     The right Windows logo key (Microsoft Natural Keyboard).
    knKeys_RWin = 92,
    //
    // Summary:
    //     The application key (Microsoft Natural Keyboard).
    knKeys_Apps = 93,
    //
    // Summary:
    //     The computer sleep key.
    knKeys_Sleep = 95,
    //
    // Summary:
    //     The 0 key on the numeric keypad.
    knKeys_NumPad0 = 96,
    //
    // Summary:
    //     The 1 key on the numeric keypad.
    knKeys_NumPad1 = 97,
    //
    // Summary:
    //     The 2 key on the numeric keypad.
    knKeys_NumPad2 = 98,
    //
    // Summary:
    //     The 3 key on the numeric keypad.
    knKeys_NumPad3 = 99,
    //
    // Summary:
    //     The 4 key on the numeric keypad.
    knKeys_NumPad4 = 100,
    //
    // Summary:
    //     The 5 key on the numeric keypad.
    knKeys_NumPad5 = 101,
    //
    // Summary:
    //     The 6 key on the numeric keypad.
    knKeys_NumPad6 = 102,
    //
    // Summary:
    //     The 7 key on the numeric keypad.
    knKeys_NumPad7 = 103,
    //
    // Summary:
    //     The 8 key on the numeric keypad.
    knKeys_NumPad8 = 104,
    //
    // Summary:
    //     The 9 key on the numeric keypad.
    knKeys_NumPad9 = 105,
    //
    // Summary:
    //     The multiply key.
    knKeys_Multiply = 106,
    //
    // Summary:
    //     The add key.
    knKeys_Add = 107,
    //
    // Summary:
    //     The separator key.
    knKeys_Separator = 108,
    //
    // Summary:
    //     The subtract key.
    knKeys_Subtract = 109,
    //
    // Summary:
    //     The decimal key.
    knKeys_Decimal = 110,
    //
    // Summary:
    //     The divide key.
    knKeys_Divide = 111,
    //
    // Summary:
    //     The F1 key.
    knKeys_F1 = 112,
    //
    // Summary:
    //     The F2 key.
    knKeys_F2 = 113,
    //
    // Summary:
    //     The F3 key.
    knKeys_F3 = 114,
    //
    // Summary:
    //     The F4 key.
    knKeys_F4 = 115,
    //
    // Summary:
    //     The F5 key.
    knKeys_F5 = 116,
    //
    // Summary:
    //     The F6 key.
    knKeys_F6 = 117,
    //
    // Summary:
    //     The F7 key.
    knKeys_F7 = 118,
    //
    // Summary:
    //     The F8 key.
    knKeys_F8 = 119,
    //
    // Summary:
    //     The F9 key.
    knKeys_F9 = 120,
    //
    // Summary:
    //     The F10 key.
    knKeys_F10 = 121,
    //
    // Summary:
    //     The F11 key.
    knKeys_F11 = 122,
    //
    // Summary:
    //     The F12 key.
    knKeys_F12 = 123,
    //
    // Summary:
    //     The F13 key.
    knKeys_F13 = 124,
    //
    // Summary:
    //     The F14 key.
    knKeys_F14 = 125,
    //
    // Summary:
    //     The F15 key.
    knKeys_F15 = 126,
    //
    // Summary:
    //     The F16 key.
    knKeys_F16 = 127,
    //
    // Summary:
    //     The F17 key.
    knKeys_F17 = 128,
    //
    // Summary:
    //     The F18 key.
    knKeys_F18 = 129,
    //
    // Summary:
    //     The F19 key.
    knKeys_F19 = 130,
    //
    // Summary:
    //     The F20 key.
    knKeys_F20 = 131,
    //
    // Summary:
    //     The F21 key.
    knKeys_F21 = 132,
    //
    // Summary:
    //     The F22 key.
    knKeys_F22 = 133,
    //
    // Summary:
    //     The F23 key.
    knKeys_F23 = 134,
    //
    // Summary:
    //     The F24 key.
    knKeys_F24 = 135,
    //
    // Summary:
    //     The NUM LOCK key.
    knKeys_NumLock = 144,
    //
    // Summary:
    //     The SCROLL LOCK key.
    knKeys_Scroll = 145,
    //
    // Summary:
    //     The left SHIFT key.
    knKeys_LShiftKey = 160,
    //
    // Summary:
    //     The right SHIFT key.
    knKeys_RShiftKey = 161,
    //
    // Summary:
    //     The left CTRL key.
    knKeys_LControlKey = 162,
    //
    // Summary:
    //     The right CTRL key.
    knKeys_RControlKey = 163,
    //
    // Summary:
    //     The left ALT key.
    knKeys_LMenu = 164,
    //
    // Summary:
    //     The right ALT key.
    knKeys_RMenu = 165,
    //
    // Summary:
    //     The browser back key (Windows 2000 or later).
    knKeys_BrowserBack = 166,
    //
    // Summary:
    //     The browser forward key (Windows 2000 or later).
    knKeys_BrowserForward = 167,
    //
    // Summary:
    //     The browser refresh key (Windows 2000 or later).
    knKeys_BrowserRefresh = 168,
    //
    // Summary:
    //     The browser stop key (Windows 2000 or later).
    knKeys_BrowserStop = 169,
    //
    // Summary:
    //     The browser search key (Windows 2000 or later).
    knKeys_BrowserSearch = 170,
    //
    // Summary:
    //     The browser favorites key (Windows 2000 or later).
    knKeys_BrowserFavorites = 171,
    //
    // Summary:
    //     The browser home key (Windows 2000 or later).
    knKeys_BrowserHome = 172,
    //
    // Summary:
    //     The volume mute key (Windows 2000 or later).
    knKeys_VolumeMute = 173,
    //
    // Summary:
    //     The volume down key (Windows 2000 or later).
    knKeys_VolumeDown = 174,
    //
    // Summary:
    //     The volume up key (Windows 2000 or later).
    knKeys_VolumeUp = 175,
    //
    // Summary:
    //     The media next track key (Windows 2000 or later).
    knKeys_MediaNextTrack = 176,
    //
    // Summary:
    //     The media previous track key (Windows 2000 or later).
    knKeys_MediaPreviousTrack = 177,
    //
    // Summary:
    //     The media Stop key (Windows 2000 or later).
    knKeys_MediaStop = 178,
    //
    // Summary:
    //     The media play pause key (Windows 2000 or later).
    knKeys_MediaPlayPause = 179,
    //
    // Summary:
    //     The launch mail key (Windows 2000 or later).
    knKeys_LaunchMail = 180,
    //
    // Summary:
    //     The select media key (Windows 2000 or later).
    knKeys_SelectMedia = 181,
    //
    // Summary:
    //     The start application one key (Windows 2000 or later).
    knKeys_LaunchApplication1 = 182,
    //
    // Summary:
    //     The start application two key (Windows 2000 or later).
    knKeys_LaunchApplication2 = 183,
    //
    // Summary:
    //     The OEM 1 key.
    knKeys_Oem1 = 186,
    //
    // Summary:
    //     The OEM Semicolon key on a US standard keyboard (Windows 2000 or later).
    knKeys_OemSemicolon = 186,
    //
    // Summary:
    //     The OEM plus key on any country/region keyboard (Windows 2000 or later).
    knKeys_Oemplus = 187,
    //
    // Summary:
    //     The OEM comma key on any country/region keyboard (Windows 2000 or later).
    knKeys_Oemcomma = 188,
    //
    // Summary:
    //     The OEM minus key on any country/region keyboard (Windows 2000 or later).
    knKeys_OemMinus = 189,
    //
    // Summary:
    //     The OEM period key on any country/region keyboard (Windows 2000 or later).
    knKeys_OemPeriod = 190,
    //
    // Summary:
    //     The OEM question mark key on a US standard keyboard (Windows 2000 or later).
    knKeys_OemQuestion = 191,
    //
    // Summary:
    //     The OEM 2 key.
    knKeys_Oem2 = 191,
    //
    // Summary:
    //     The OEM tilde key on a US standard keyboard (Windows 2000 or later).
    knKeys_Oemtilde = 192,
    //
    // Summary:
    //     The OEM 3 key.
    knKeys_Oem3 = 192,
    //
    // Summary:
    //     The OEM 4 key.
    knKeys_Oem4 = 219,
    //
    // Summary:
    //     The OEM open bracket key on a US standard keyboard (Windows 2000 or later).
    knKeys_OemOpenBrackets = 219,
    //
    // Summary:
    //     The OEM pipe key on a US standard keyboard (Windows 2000 or later).
    knKeys_OemPipe = 220,
    //
    // Summary:
    //     The OEM 5 key.
    knKeys_Oem5 = 220,
    //
    // Summary:
    //     The OEM 6 key.
    knKeys_Oem6 = 221,
    //
    // Summary:
    //     The OEM close bracket key on a US standard keyboard (Windows 2000 or later).
    knKeys_OemCloseBrackets = 221,
    //
    // Summary:
    //     The OEM 7 key.
    knKeys_Oem7 = 222,
    //
    // Summary:
    //     The OEM singled/double quote key on a US standard keyboard (Windows 2000
    //     or later).
    knKeys_OemQuotes = 222,
    //
    // Summary:
    //     The OEM 8 key.
    knKeys_Oem8 = 223,
    //
    // Summary:
    //     The OEM 102 key.
    knKeys_Oem102 = 226,
    //
    // Summary:
    //     The OEM angle bracket or backslash key on the RT 102 key keyboard (Windows
    //     2000 or later).
    knKeys_OemBackslash = 226,
    //
    // Summary:
    //     The PROCESS KEY key.
    knKeys_ProcessKey = 229,
    //
    // Summary:
    //     Used to pass Unicode characters as if they were keystrokes. The Packet key
    //     value is the low word of a 32-bit virtual-key value used for non-keyboard
    //     input methods.
    knKeys_Packet = 231,
    //
    // Summary:
    //     The ATTN key.
    knKeys_Attn = 246,
    //
    // Summary:
    //     The CRSEL key.
    knKeys_Crsel = 247,
    //
    // Summary:
    //     The EXSEL key.
    knKeys_Exsel = 248,
    //
    // Summary:
    //     The ERASE EOF key.
    knKeys_EraseEof = 249,
    //
    // Summary:
    //     The PLAY key.
    knKeys_Play = 250,
    //
    // Summary:
    //     The ZOOM key.
    knKeys_Zoom = 251,
    //
    // Summary:
    //     A constant reserved for future use.
    knKeys_NoName = 252,
    //
    // Summary:
    //     The PA1 key.
    knKeys_Pa1 = 253,
    //
    // Summary:
    //     The CLEAR key.
    knKeys_OemClear = 254,
    //
    // Summary:
    //     The bitmask to extract a key code from a key value.
    knKeys_KeyCode = 65535,
    //
    // Summary:
    //     The SHIFT modifier key.
    knKeys_Shift = 65536,
    //
    // Summary:
    //     The CTRL modifier key.
    knKeys_Control = 131072,
    //
    // Summary:
    //     The ALT modifier key.
    knKeys_Alt = 262144,
};

/*! 
UIMouseEvent - event data for MouseUp/MouseMove/MouseDown/MouseWheel events.

the data to muse event comes from the MouseEventData from C# side.
*/
//! UIMouseEvent - event data for MouseUp/MouseMove/MouseDown/MouseWheel events.
class VIEW_MODEL_API UIMouseEvent : public UIEvent 
{
public:
	UIMouseEvent(const double & x,const double & y,const int & delta,
				 const bool & leftButton,const bool & rightButton,const bool & middleButton,
				 const bool & altKey,const bool & shiftKey,const bool & ctrlKey) 
		: UIEvent(altKey,shiftKey,ctrlKey), m_mouseX(x), m_mouseY(y), m_wheelDelta(delta), m_leftButton(leftButton), m_rightButton(rightButton), m_middleButton(middleButton)
	{
	}

	static PYXPointer<UIMouseEvent> create(const double & x,const double & y,const int & delta,
				 const bool & leftButton,const bool & rightButton,const bool & middleButton,
				 const bool & altKey,const bool & shiftKey,const bool & ctrlKey) 
	{ 
		return PYXNEW(UIMouseEvent,x,y,delta,leftButton,rightButton,middleButton,altKey,shiftKey,ctrlKey); 
	}
	
	virtual ~UIMouseEvent()
	{
	}

protected:
	double m_mouseX;
	double m_mouseY;
	int m_wheelDelta;
	bool m_leftButton;
	bool m_rightButton;
	bool m_middleButton;

public:
	const double & getMouseX() const { return m_mouseX; };
	const double & getMouseY() const { return m_mouseY; };

	//! get the ammount of whell ticks. can haave both positive or negative values.
	const int & getWheelDelta() const { return m_wheelDelta; };
	
	const bool & isLeftButtonDown() const { return m_leftButton; };
	const bool & isRightButtonDown() const { return m_rightButton; };
	const bool & isMiddleButtonDown() const { return m_middleButton; };

//Helper functions
public:
	//! return the distance in pixels from a given point on the ViewPort
	double mouseDistanceFrom(const double & X,const double & Y) const;

	//! check if a mouse is inside a rect
	bool   isInsideRect(const double & Xmin,const double & Xmax,const double & Ymin,const double & Ymax) const;
};

#endif
