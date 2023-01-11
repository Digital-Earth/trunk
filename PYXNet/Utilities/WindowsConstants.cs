/******************************************************************************
WindowsConstants.cs

begin      : November 14, 2008
copyright  : (c) 2008 by the PYXIS innovation inc.
web        : www.pyxisinnovation.com
******************************************************************************/

using System;
using System.Collections.Generic;
using System.Text;
using System.Runtime.InteropServices;

namespace Windows.Core
{
    #region SendMessageTimeoutFlags
    /// <summary>
    /// Window Style Flags
    /// </summary>
    [Flags]
    public enum SendMessageTimeoutFlags : uint
    {
        SMTO_NORMAL = 0x0000,
        SMTO_BLOCK = 0x0001,
        SMTO_ABORTIFHUNG = 0x0002,
        SMTO_NOTIMEOUTIFNOTHUNG = 0x0008
    } 
    #endregion

    #region MemAllocationType
    /// <summary>
    /// Window Style Flags
    /// </summary>
    [Flags]
    public enum MemAllocationType : uint
    {
        COMMIT = 0x1000,
        RELEASE = 0x8000
    } 
    #endregion

    #region WindowsMessages
    /// <summary>
    /// Windows Messages Flags
    /// </summary>
    [Flags]
    public enum WindowsMessages : int
    {
        WM_ACTIVATE = 0x6,
        WM_ACTIVATEAPP = 0x1C,
        WM_ASKCBFORMATNAME = 0x30C,
        WM_CANCELJOURNAL = 0x4B,
        WM_CANCELMODE = 0x1F,
        WM_CAPTURECHANGED = 0x1F,
        WM_CAPTURECHANGED_R = 0x215,
        WM_CHANGECBCHAIN = 0x30D,
        WM_CHAR = 0x102,
        WM_CHARTOITEM = 0x2F,
        WM_CHILDACTIVATE = 0x22,
        WM_CHOOSEFONT_GETLOGFONT = 0x401,
        WM_CHOOSEFONT_SETFLAGS = (0x400 + 102),
        WM_CHOOSEFONT_SETLOGFONT = (0x400 + 101),
        WM_CLEAR = 0x303,
        WM_CLOSE = 0x10,
        WM_COMMAND = 0x111,
        WM_COMPACTING = 0x41,
        WM_COMPAREITEM = 0x39,
        WM_CONTEXTMENU = 0x7B,
        WM_CONVERTREQUESTEX = 0x108,
        WM_COPY = 0x301,
        WM_COPYDATA = 0x4A,
        WM_CREATE = 0x1,
        WM_CTLCOLORBTN = 0x135,
        WM_CTLCOLORDLG = 0x136,
        WM_CTLCOLOREDIT = 0x133,
        WM_CTLCOLORLISTBOX = 0x134,
        WM_CTLCOLORMSGBOX = 0x132,
        WM_CTLCOLORSCROLLBAR = 0x137,
        WM_CTLCOLORSTATIC = 0x138,
        WM_CUT = 0x300,
        WM_DDE_ACK = (0x3E0 + 4),
        WM_DDE_ADVISE = (0x3E0 + 2),
        WM_DDE_DATA = (0x3E0 + 5),
        WM_DDE_EXECUTE = (0x3E0 + 8),
        WM_DDE_FIRST = 0x3E0,
        WM_DDE_INITIATE = 0x3E0,
        WM_DDE_LAST = (0x3E0 + 8),
        WM_DDE_POKE = (0x3E0 + 7),
        WM_DDE_REQUEST = (0x3E0 + 6),
        WM_DDE_TERMINATE = (0x3E0 + 1),
        WM_DDE_UNADVISE = (0x3E0 + 3),
        WM_DEADCHAR = 0x103,
        WM_DELETEITEM = 0x2D,
        WM_DESTROY = 0x2,
        WM_DESTROYCLIPBOARD = 0x307,
        WM_DEVICECHANGE = 0x219,
        WM_DEVMODECHANGE = 0x1B,
        WM_DRAWCLIPBOARD = 0x308,
        WM_DRAWITEM = 0x2B,
        WM_DROPFILES = 0x233,
        WM_ENABLE = 0xA,
        WM_ENDSESSION = 0x16,
        WM_ENTERIDLE = 0x121,
        WM_ENTERSIZEMOVE = 0x231,
        WM_ENTERMENULOOP = 0x211,
        WM_ERASEBKGND = 0x14,
        WM_EXITMENULOOP = 0x212,
        WM_EXITSIZEMOVE = 0x232,
        WM_FONTCHANGE = 0x1D,
        WM_GETDLGCODE = 0x87,
        WM_GETFONT = 0x31,
        WM_GETHOTKEY = 0x33,
        WM_GETMINMAXINFO = 0x24,
        WM_GETTEXT = 0xD,
        WM_GETTEXTLENGTH = 0xE,
        WM_HELP = 0x53,
        WM_HOTKEY = 0x312,
        WM_HSCROLL = 0x114,
        WM_HSCROLLCLIPBOARD = 0x30E,
        WM_ICONERASEBKGND = 0x27,
        WM_IME_CHAR = 0x286,
        WM_IME_COMPOSITION = 0x10F,
        WM_IME_COMPOSITIONFULL = 0x284,
        WM_IME_CONTROL = 0x283,
        WM_IME_ENDCOMPOSITION = 0x10E,
        WM_IME_KEYDOWN = 0x290,
        WM_IME_KEYLAST = 0x10F,
        WM_IME_KEYUP = 0x291,
        WM_IME_NOTIFY = 0x282,
        WM_IME_SELECT = 0x285,
        WM_IME_SETCONTEXT = 0x281,
        WM_IME_STARTCOMPOSITION = 0x10D,
        WM_INITDIALOG = 0x110,
        WM_INITMENU = 0x116,
        WM_INITMENUPOPUP = 0x117,
        WM_INPUTLANGCHANGEREQUEST = 0x50,
        WM_INPUTLANGCHANGE = 0x51,
        WM_KEYDOWN = 0x100,
        WM_KEYUP = 0x101,
        WM_KILLFOCUS = 0x8,
        WM_LBUTTONDBLCLK = 0x203,
        WM_LBUTTONDOWN = 0x201,
        WM_LBUTTONUP = 0x202,
        WM_MBUTTONDBLCLK = 0x209,
        WM_MBUTTONDOWN = 0x207,
        WM_MBUTTONUP = 0x208,
        WM_MDIACTIVATE = 0x222,
        WM_MDICASCADE = 0x227,
        WM_MDICREATE = 0x220,
        WM_MDIDESTROY = 0x221,
        WM_MDIGETACTIVE = 0x229,
        WM_MDIICONARRANGE = 0x228,
        WM_MDIMAXIMIZE = 0x225,
        WM_MDINEXT = 0x224,
        WM_MDIREFRESHMENU = 0x234,
        WM_MDIRESTORE = 0x223,
        WM_MDISETMENU = 0x230,
        WM_MDITILE = 0x226,
        WM_MEASUREITEM = 0x2C,
        WM_MENUCHAR = 0x120,
        WM_MENUSELECT = 0x11F,
        WM_MENURBUTTONUP = 0x122,
        WM_MENUDRAG = 0x123,
        WM_MENUGETOBJECT = 0x124,
        WM_MENUCOMMAND = 0x126,
        WM_MOUSEACTIVATE = 0x21,
        WM_MOUSEHOVER = 0x2A1,
        WM_MOUSELEAVE = 0x2A3,
        WM_MOUSEMOVE = 0x200,
        WM_MOUSEWHEEL = 0x20A,
        WM_MOVE = 0x3,
        WM_MOVING = 0x216,
        WM_NCACTIVATE = 0x86,
        WM_NCCALCSIZE = 0x83,
        WM_NCCREATE = 0x81,
        WM_NCDESTROY = 0x82,

        WM_NCHITTEST = 0x84,
        WM_NCLBUTTONDBLCLK = 0xA3,
        WM_NCLBUTTONDOWN = 0xA1,
        WM_NCLBUTTONUP = 0xA2,
        WM_NCMBUTTONDBLCLK = 0xA9,
        WM_NCMBUTTONDOWN = 0xA7,
        WM_NCMBUTTONUP = 0xA8,
        WM_NCMOUSEMOVE = 0xA0,
        WM_NCPAINT = 0x85,
        WM_NCRBUTTONDBLCLK = 0xA6,
        WM_NCRBUTTONDOWN = 0xA4,
        WM_NCRBUTTONUP = 0xA5,
        WM_NEXTDLGCTL = 0x28,
        WM_NEXTMENU = 0x213,
        WM_NULL = 0x0,
        WM_PAINT = 0xF,
        WM_PAINTCLIPBOARD = 0x309,
        WM_PAINTICON = 0x26,
        WM_PALETTECHANGED = 0x311,
        WM_PALETTEISCHANGING = 0x310,
        WM_PARENTNOTIFY = 0x210,
        WM_PASTE = 0x302,
        WM_PENWINFIRST = 0x380,
        WM_PENWINLAST = 0x38F,
        WM_POWER = 0x48,
        WM_POWERBROADCAST = 0x218,
        WM_PRINT = 0x317,
        WM_PRINTCLIENT = 0x318,
        WM_PSD_ENVSTAMPRECT = (0x400 + 5),
        WM_PSD_FULLPAGERECT = (0x400 + 1),
        WM_PSD_GREEKTEXTRECT = (0x400 + 4),
        WM_PSD_MARGINRECT = (0x400 + 3),
        WM_PSD_MINMARGINRECT = (0x400 + 2),
        WM_PSD_PAGESETUPDLG = (0x400),
        WM_PSD_YAFULLPAGERECT = (0x400 + 6),
        WM_QUERYDRAGICON = 0x37,
        WM_QUERYENDSESSION = 0x11,
        WM_QUERYNEWPALETTE = 0x30F,
        WM_QUERYOPEN = 0x13,
        WM_QUEUESYNC = 0x23,
        WM_QUIT = 0x12,
        WM_RBUTTONDBLCLK = 0x206,
        WM_RBUTTONDOWN = 0x204,
        WM_RBUTTONUP = 0x205,
        WM_RENDERALLFORMATS = 0x306,
        WM_RENDERFORMAT = 0x305,
        WM_SETCURSOR = 0x20,
        WM_SETFOCUS = 0x7,
        WM_SETFONT = 0x30,
        WM_SETHOTKEY = 0x32,
        WM_SETREDRAW = 0xB,
        WM_SETTEXT = 0xC,
        WM_SETTINGCHANGE = 0x1A,
        WM_SHOWWINDOW = 0x18,
        WM_SIZE = 0x5,
        WM_SIZING = 0x214,
        WM_SIZECLIPBOARD = 0x30B,
        WM_SPOOLERSTATUS = 0x2A,
        WM_SYSCHAR = 0x106,
        WM_SYSCOLORCHANGE = 0x15,
        WM_SYSCOMMAND = 0x112,
        WM_SYSDEADCHAR = 0x107,
        WM_SYSKEYDOWN = 0x104,
        WM_SYSKEYUP = 0x105,
        WM_TIMECHANGE = 0x1E,
        WM_TIMER = 0x113,
        WM_UNDO = 0x304,
        WM_USER = 0x400,
        WM_VKEYTOITEM = 0x2E,
        WM_VSCROLL = 0x115,
        WM_VSCROLLCLIPBOARD = 0x30A,
        WM_WINDOWPOSCHANGED = 0x47,
        WM_WINDOWPOSCHANGING = 0x46,
        WM_WININICHANGE = 0x1A,
        WM_APPCOMMAND = 0x319
    } 
    #endregion

    #region ExtendedWindowStyleFlags
    /// <summary>
    /// Extended Windows Style Flags
    /// </summary>
    [Flags]
    public enum ExtendedWindowStyleFlags : int
    {
        WS_EX_DLGMODALFRAME = 0x00000001,
        WS_EX_NOPARENTNOTIFY = 0x00000004,
        WS_EX_TOPMOST = 0x00000008,
        WS_EX_ACCEPTFILES = 0x00000010,
        WS_EX_TRANSPARENT = 0x00000020,

        WS_EX_MDICHILD = 0x00000040,
        WS_EX_TOOLWINDOW = 0x00000080,
        WS_EX_WINDOWEDGE = 0x00000100,
        WS_EX_CLIENTEDGE = 0x00000200,
        WS_EX_CONTEXTHELP = 0x00000400,

        WS_EX_RIGHT = 0x00001000,
        WS_EX_LEFT = 0x00000000,
        WS_EX_RTLREADING = 0x00002000,
        WS_EX_LTRREADING = 0x00000000,
        WS_EX_LEFTSCROLLBAR = 0x00004000,
        WS_EX_RIGHTSCROLLBAR = 0x00000000,

        WS_EX_CONTROLPARENT = 0x00010000,
        WS_EX_STATICEDGE = 0x00020000,
        WS_EX_APPWINDOW = 0x00040000,

        WS_EX_LAYERED = 0x00080000,

        WS_EX_NOINHERITLAYOUT = 0x00100000, // Disable inheritence of mirroring by children
        WS_EX_LAYOUTRTL = 0x00400000, // Right to left mirroring

        WS_EX_COMPOSITED = 0x02000000,
        WS_EX_NOACTIVATE = 0x08000000
    } 
    #endregion

    #region OLECMDF
    /// <summary>
    /// OLECMDF Flags
    /// </summary>
    [Flags]
    public enum OLECMDF
    {
        // Fields
        OLECMDF_DEFHIDEONCTXTMENU = 0x20,
        OLECMDF_ENABLED = 2,
        OLECMDF_INVISIBLE = 0x10,
        OLECMDF_LATCHED = 4,
        OLECMDF_NINCHED = 8,
        OLECMDF_SUPPORTED = 1
    } 
    #endregion

    #region OLECMDID
    /// <summary>
    /// OLECMDID Flags
    /// </summary>
    [Flags]
    public enum OLECMDID
    {
        // Fields
        OLECMDID_PAGESETUP = 8,
        OLECMDID_PRINT = 6,
        OLECMDID_PRINTPREVIEW = 7,
        OLECMDID_PROPERTIES = 10,
        OLECMDID_SAVEAS = 4,
        OLECMDID_SHOWSCRIPTERROR = 40
    } 
    #endregion

    #region OLECMDEXECOPT
    /// <summary>
    /// OLECMDEXECOPT Flags
    /// </summary>
    [Flags]
    public enum OLECMDEXECOPT
    {
        // Fields
        OLECMDEXECOPT_DODEFAULT = 0,
        OLECMDEXECOPT_DONTPROMPTUSER = 2,
        OLECMDEXECOPT_PROMPTUSER = 1,
        OLECMDEXECOPT_SHOWHELP = 3
    } 
    #endregion

    #region WindowStyles
    /// <summary>
    /// Window Styles Flags
    /// </summary>
    [Flags]
    public enum WindowStyles : int
    {
        WS_OVERLAPPED = 0x00000000,
        WS_POPUP = unchecked((int)0x80000000),
        WS_CHILD = 0x40000000,
        WS_MINIMIZE = 0x20000000,
        WS_VISIBLE = 0x10000000,
        WS_DISABLED = 0x08000000,
        WS_CLIPSIBLINGS = 0x04000000,
        WS_CLIPCHILDREN = 0x02000000,
        WS_MAXIMIZE = 0x01000000,
        WS_CAPTION = 0x00C00000,
        WS_BORDER = 0x00800000,
        WS_DLGFRAME = 0x00400000,
        WS_VSCROLL = 0x00200000,
        WS_HSCROLL = 0x00100000,
        WS_SYSMENU = 0x00080000,
        WS_THICKFRAME = 0x00040000,
        WS_GROUP = 0x00020000,
        WS_TABSTOP = 0x00010000,
        WS_MINIMIZEBOX = 0x00020000,
        WS_MAXIMIZEBOX = 0x00010000,
        WS_TILED = 0x00000000,
        WS_ICONIC = 0x20000000,
        WS_SIZEBOX = 0x00040000,
        WS_POPUPWINDOW = unchecked((int)0x80880000),
        WS_OVERLAPPEDWINDOW = WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_THICKFRAME | WS_MINIMIZEBOX | WS_MAXIMIZEBOX,
        WS_TILEDWINDOW = WS_OVERLAPPEDWINDOW,
        WS_CHILDWINDOW = WS_CHILD,
    } 
    #endregion

    #region MsgButtons
    /// <summary>
    /// Message Buttons Flags
    /// </summary>
    [Flags]
    public enum MsgButtons
    {
        MK_LBUTTON = 0x0001,
        MK_RBUTTON = 0x0002,
        MK_SHIFT = 0x0004,
        MK_CONTROL = 0x0008,
        MK_MBUTTON = 0x0010,
        MK_XBUTTON1 = 0x0020,
        MK_XBUTTON2 = 0x0040,
    } 
    #endregion

    #region WindowActiveFlags
    /// <summary>
    /// Window Active Flags
    /// </summary>
    [Flags]
    public enum WindowActiveFlags
    {
        WA_INACTIVE = 0,
        WA_ACTIVE = 1,
        WA_CLICKACTIVE = 2
    } 
    #endregion

    #region KeybdEventFlags
    /// <summary>
    /// Keyboard Event Flags
    /// </summary>
    [Flags]
    public enum KeybdEventFlags
    {
        None = 0,
        ExtendedKey = 0x0001,
        KeyUp = 0x0002
    } 
    #endregion

    #region VirtualKeys
    /// <summary>
    /// Virtual Keys Flags
    /// </summary>
    [Flags]
    public enum VirtualKeys
    {
        VK_LBUTTON = 0x01,
        VK_RBUTTON = 0x02,
        VK_CANCEL = 0x03,
        VK_MBUTTON = 0x04,
        VK_XBUTTON1 = 0x05,
        VK_XBUTTON2 = 0x06,
        VK_BACK = 0x08,
        VK_TAB = 0x09,
        VK_CLEAR = 0x0C,
        VK_RETURN = 0x0D,
        VK_SHIFT = 0x10,
        VK_CONTROL = 0x11,
        VK_MENU = 0x12,
        VK_PAUSE = 0x13,
        VK_CAPITAL = 0x14,
        VK_ESCAPE = 0x1B,
        VK_SPACE = 0x20,
        VK_PRIOR = 0x21,
        VK_NEXT = 0x22,
        VK_END = 0x23,
        VK_HOME = 0x24,
        VK_LEFT = 0x25,
        VK_UP = 0x26,
        VK_RIGHT = 0x27,
        VK_DOWN = 0x28,
        VK_SELECT = 0x29,
        VK_PRINT = 0x2A,
        VK_EXECUTE = 0x2B,
        VK_SNAPSHOT = 0x2C,
        VK_INSERT = 0x2D,
        VK_DELETE = 0x2E,
        VK_HELP = 0x2F,
        VK_0 = 0x30,
        VK_1 = 0x31,
        VK_2 = 0x32,
        VK_3 = 0x33,
        VK_4 = 0x34,
        VK_5 = 0x35,
        VK_6 = 0x36,
        VK_7 = 0x37,
        VK_8 = 0x38,
        VK_9 = 0x39,
        VK_A = 0x41,
        VK_B = 0x42,
        VK_C = 0x43,
        VK_D = 0x44,
        VK_E = 0x45,
        VK_F = 0x46,
        VK_G = 0x47,
        VK_H = 0x48,
        VK_I = 0x49,
        VK_J = 0x4A,
        VK_K = 0x4B,
        VK_L = 0x4C,
        VK_M = 0x4D,
        VK_N = 0x4E,
        VK_O = 0x4F,
        VK_P = 0x50,
        VK_Q = 0x51,
        VK_R = 0x52,
        VK_S = 0x53,
        VK_T = 0x54,
        VK_U = 0x55,
        VK_V = 0x56,
        VK_W = 0x57,
        VK_X = 0x58,
        VK_Y = 0x59,
        VK_Z = 0x5A,
        VK_LWIN = 0x5B,
        VK_RWIN = 0x5C,
        VK_APPS = 0x5D,
        VK_NUMPAD0 = 0x60,
        VK_NUMPAD1 = 0x61,
        VK_NUMPAD2 = 0x62,
        VK_NUMPAD3 = 0x63,
        VK_NUMPAD4 = 0x64,
        VK_NUMPAD5 = 0x65,
        VK_NUMPAD6 = 0x66,
        VK_NUMPAD7 = 0x67,
        VK_NUMPAD8 = 0x68,
        VK_NUMPAD9 = 0x69,
        VK_MULTIPLY = 0x6A,
        VK_ADD = 0x6B,
        VK_SEPARATOR = 0x6C,
        VK_SUBTRACT = 0x6D,
        VK_DECIMAL = 0x6E,
        VK_DIVIDE = 0x6F,
        VK_F1 = 0x70,
        VK_F2 = 0x71,
        VK_F3 = 0x72,
        VK_F4 = 0x73,
        VK_F5 = 0x74,
        VK_F6 = 0x75,
        VK_F7 = 0x76,
        VK_F8 = 0x77,
        VK_F9 = 0x78,
        VK_F10 = 0x79,
        VK_F11 = 0x7A,
        VK_F12 = 0x7B,
        VK_F13 = 0x7C,
        VK_F14 = 0x7D,
        VK_F15 = 0x7E,
        VK_F16 = 0x7F,
        VK_F17 = 0x80,
        VK_F18 = 0x81,
        VK_F19 = 0x82,
        VK_F20 = 0x83,
        VK_F21 = 0x84,
        VK_F22 = 0x85,
        VK_F23 = 0x86,
        VK_F24 = 0x87,
        VK_NUMLOCK = 0x90,
        VK_SCROLL = 0x91,
        VK_LSHIFT = 0xA0,
        VK_RSHIFT = 0xA1,
        VK_LCONTROL = 0xA2,
        VK_RCONTROL = 0xA3,
        VK_LMENU = 0xA4,
        VK_RMENU = 0xA5,
        VK_OEM_1 = 0xBA,
        VK_OEM_PLUS = 0xBB,
        VK_OEM_COMMA = 0xBC,
        VK_OEM_MINUS = 0xBD,
        VK_OEM_PERIOD = 0xBE,
        VK_OEM_2 = 0xBF,
        VK_OEM_3 = 0xC0,
        VK_OEM_4 = 0xDB,
        VK_OEM_5 = 0xDC,
        VK_OEM_6 = 0xDD,
        VK_OEM_7 = 0xDE,
        VK_OEM_8 = 0xDF,
        VK_OEM_AX = 0xE1,
        VK_OEM_102 = 0xE2,
        VK_ICO_HELP = 0xE3,
        VK_ICO_00 = 0xE4,
        VK_PROCESSKEY = 0xE5,
        VK_ATTN = 0xF6,
        VK_CRSEL = 0xF7,
        VK_EXSEL = 0xF8,
        VK_EREOF = 0xF9,
        VK_PLAY = 0xFA,
        VK_ZOOM = 0xFB,
        VK_NONAME = 0xFC,
        VK_PA1 = 0xFD,
        VK_OEM_CLEAR = 0xFE,
    } 
    #endregion

    #region TtyKeys
    /// <summary>
    /// Misc Keys Flags
    /// </summary>
    [Flags]
    public enum TtyKeys
    {
        XK_BackSpace = 0xff08,  /* Back space, back char */
        XK_Tab = 0xff09,
        XK_Linefeed = 0xff0a,  /* Linefeed, LF */
        XK_Clear = 0xff0b,
        XK_Return = 0xff0d,  /* Return, enter */
        XK_Pause = 0xff13,  /* Pause, hold */
        XK_Scroll_Lock = 0xff14,
        XK_Sys_Req = 0xff15,
        XK_Escape = 0xff1b,
        XK_Delete = 0xffff  /* Delete, rubout */
    } 
    #endregion

    #region MiscKeys
    /// <summary>
    /// Misc Keys Flags
    /// </summary>
    [Flags]
    public enum MiscKeys
    {
        XK_ISO_Lock = 0xfe01,
        XK_ISO_Last_Group_Lock = 0xfe0f,
        XK_Select = 0xff60,
        XK_Print = 0xff61,
        XK_Execute = 0xff62,
        XK_Insert = 0xff63,
        XK_Undo = 0xff65,
        XK_Redo = 0xff66,
        XK_Menu = 0xff67,
        XK_Find = 0xff68,
        XK_Cancel = 0xff69,
        XK_Help = 0xff6a,
        XK_Break = 0xff6b,
        XK_Mode_switch = 0xff7e,
        XK_script_switch = 0xff7e,
        XK_Num_Lock = 0xff7f
    } 
    #endregion

    #region KeypadKeys
    /// <summary>
    /// Keypad Keys Flags
    /// </summary>
    [Flags]
    public enum KeypadKeys
    {
        XK_KP_Space = 0xff80,
        XK_KP_Tab = 0xff89,
        XK_KP_Enter = 0xff8d,  /* Enter */
        XK_KP_F1 = 0xff91,  /* PF1, KP_A, ... */
        XK_KP_F2 = 0xff92,
        XK_KP_F3 = 0xff93,
        XK_KP_F4 = 0xff94,
        XK_KP_Home = 0xff95,
        XK_KP_Left = 0xff96,
        XK_KP_Up = 0xff97,
        XK_KP_Right = 0xff98,
        XK_KP_Down = 0xff99,
        XK_KP_Prior = 0xff9a,
        XK_KP_Page_Up = 0xff9a,
        XK_KP_Next = 0xff9b,
        XK_KP_Page_Down = 0xff9b,
        XK_KP_End = 0xff9c,
        XK_KP_Begin = 0xff9d,
        XK_KP_Insert = 0xff9e,
        XK_KP_Delete = 0xff9f,
        XK_KP_Equal = 0xffbd,  /* Equals */
        XK_KP_Multiply = 0xffaa,
        XK_KP_Add = 0xffab,
        XK_KP_Separator = 0xffac,  /* Separator, often comma */
        XK_KP_Subtract = 0xffad,
        XK_KP_Decimal = 0xffae,
        XK_KP_Divide = 0xffaf,

        XK_KP_0 = 0xffb0,
        XK_KP_1 = 0xffb1,
        XK_KP_2 = 0xffb2,
        XK_KP_3 = 0xffb3,
        XK_KP_4 = 0xffb4,
        XK_KP_5 = 0xffb5,
        XK_KP_6 = 0xffb6,
        XK_KP_7 = 0xffb7,
        XK_KP_8 = 0xffb8,
        XK_KP_9 = 0xffb9
    } 
    #endregion

    #region DeadKeys
    /// <summary>
    /// Dead Keys Flags
    /// </summary>
    [Flags]
    public enum DeadKeys
    {
        XK_dead_grave = 0xfe50,
        XK_dead_acute = 0xfe51,
        XK_dead_circumflex = 0xfe52,
        XK_dead_tilde = 0xfe53,
        XK_dead_macron = 0xfe54,
        XK_dead_breve = 0xfe55,
        XK_dead_abovedot = 0xfe56,
        XK_dead_diaeresis = 0xfe57,
        XK_dead_abovering = 0xfe58,
        XK_dead_doubleacute = 0xfe59,
        XK_dead_caron = 0xfe5a,
        XK_dead_cedilla = 0xfe5b,
        XK_dead_ogonek = 0xfe5c,
        XK_dead_iota = 0xfe5d,
        XK_dead_voiced_sound = 0xfe5e,
        XK_dead_semivoiced_sound = 0xfe5f,
        XK_dead_belowdot = 0xfe60,
        XK_dead_hook = 0xfe61,
        XK_dead_horn = 0xfe62
    } 
    #endregion

    #region PeekMessageFlags
    /// <summary>
    /// Peek Message Flags
    /// </summary>
    [Flags]
    public enum PeekMessageFlags
    {
        PM_NOREMOVE = 0x00000000,
        PM_REMOVE = 0x00000001,
        PM_NOYIELD = 0x00000002
    } 
    #endregion

    #region StdCursor
    /// <summary>
    /// Standard Cursor Flags
    /// </summary>
    [Flags]
    public enum StdCursor
    {
        Default = 0,
        AppStarting = 1,
        Arrow = 2,
        Cross = 3,
        Hand = 4,
        Help = 5,
        HSplit = 6,
        IBeam = 7,
        No = 8,
        NoMove2D = 9,
        NoMoveHoriz = 10,
        NoMoveVert = 11,
        PanEast = 12,
        PanNE = 13,
        PanNorth = 14,
        PanNW = 15,
        PanSE = 16,
        PanSouth = 17,
        PanSW = 18,
        PanWest = 19,
        SizeAll = 20,
        SizeNESW = 21,
        SizeNS = 22,
        SizeNWSE = 23,
        SizeWE = 24,
        UpArrow = 25,
        VSplit = 26,
        WaitCursor = 27
    } 
    #endregion

    #region HitTest
    /// <summary>
    /// Hit Test Flags
    /// </summary>
    [Flags]
    public enum HitTest
    {
        HTERROR = -2,
        HTTRANSPARENT = -1,
        HTNOWHERE = 0,
        HTCLIENT = 1,
        HTCAPTION = 2,
        HTSYSMENU = 3,
        HTGROWBOX = 4,
        HTSIZE = HTGROWBOX,
        HTMENU = 5,
        HTHSCROLL = 6,
        HTVSCROLL = 7,
        HTMINBUTTON = 8,
        HTMAXBUTTON = 9,
        HTLEFT = 10,
        HTRIGHT = 11,
        HTTOP = 12,
        HTTOPLEFT = 13,
        HTTOPRIGHT = 14,
        HTBOTTOM = 15,
        HTBOTTOMLEFT = 16,
        HTBOTTOMRIGHT = 17,
        HTBORDER = 18,
        HTREDUCE = HTMINBUTTON,
        HTZOOM = HTMAXBUTTON,
        HTSIZEFIRST = HTLEFT,
        HTSIZELAST = HTBOTTOMRIGHT,
        HTOBJECT = 19,
        HTCLOSE = 20,
        HTHELP = 21
    } 
    #endregion

    #region TitleStyle
    /// <summary>
    /// Title Style Flags
    /// </summary>
    [Flags]
    public enum TitleStyle
    {
        None = 0,
        Normal = 1,
        Tool = 2
    } 
    #endregion

    #region ClipboardFormats
    /// <summary>
    /// Clipboard Formats Flags
    /// </summary>
    [Flags]
    public enum ClipboardFormats : ushort
    {
        CF_TEXT = 1,
        CF_BITMAP = 2,
        CF_METAFILEPICT = 3,
        CF_SYLK = 4,
        CF_DIF = 5,
        CF_TIFF = 6,
        CF_OEMTEXT = 7,
        CF_DIB = 8,
        CF_PALETTE = 9,
        CF_PENDATA = 10,
        CF_RIFF = 11,
        CF_WAVE = 12,
        CF_UNICODETEXT = 13,
        CF_ENHMETAFILE = 14,
        CF_HDROP = 15,
        CF_LOCALE = 16,
        CF_DIBV5 = 17
    } 
    #endregion

    #region ScrollBarCommands
    /// <summary>
    /// ScrollBar Commands Flags
    /// </summary>
    [Flags]
    public enum ScrollBarCommands
    {
        SB_LINEUP = 0,
        SB_LINELEFT = 0,
        SB_LINEDOWN = 1,
        SB_LINERIGHT = 1,
        SB_PAGEUP = 2,
        SB_PAGELEFT = 2,
        SB_PAGEDOWN = 3,
        SB_PAGERIGHT = 3,
        SB_THUMBPOSITION = 4,
        SB_THUMBTRACK = 5,
        SB_TOP = 6,
        SB_LEFT = 6,
        SB_BOTTOM = 7,
        SB_RIGHT = 7,
        SB_ENDSCROLL = 8
    } 
    #endregion

    #region ProcessRights
    /// <summary>
    /// Process Rights Flags
    /// http://msdn2.microsoft.com/en-us/library/ms684880.aspx
    /// </summary>
    [Flags]
    public enum ProcessRights : uint
    {
        PROCESS_ALL_ACCESS = 0x1F0FFF,
        PROCESS_CREATE_PROCESS = 0x0080,
        PROCESS_CREATE_THREAD = 0x0002,
        PROCESS_DUP_HANDLE = 0x0040,
        PROCESS_QUERY_INFORMATION = 0x0400,
        PROCESS_QUERY_LIMITED_INFORMATION = 0x1000,
        PROCESS_SET_INFORMATION = 0x0200,
        PROCESS_SET_QUOTA = 0x0100,
        PROCESS_SUSPEND_RESUME = 0x0800,
        PROCESS_TERMINATE = 0x0001,
        PROCESS_VM_OPERATION = 0x0008,
        PROCESS_VM_READ = 0x0010,
        PROCESS_VM_WRITE = 0x0020,
        SYNCHRONIZE = 0x00100000
    } 
    #endregion
}
