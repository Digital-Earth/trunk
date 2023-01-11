#pragma once
#include <windows.h>
#include <richedit.h>
#include <textserv.h>
#include <Richole.h>
#include <vcclr.h>
namespace Infralution {
    namespace RichText {


#ifndef LY_PER_INCH
#define LY_PER_INCH   1440
#define HOST_BORDER 0
#endif

        using namespace System;
        using namespace System::Drawing;
        using namespace System::Runtime::InteropServices;

        typedef struct tagCOOKIE
        {
            LPSTR text;
            DWORD size;
            DWORD count;
        } COOKIE, *PCOOKIE;

        const IID IID_ITextServices2 = { // 8d33f740-cf58-11ce-a89d-00aa006cadc5
            0x8d33f740,
            0xcf58,
            0x11ce,
            {0xa8, 0x9d, 0x00, 0xaa, 0x00, 0x6c, 0xad, 0xc5}
        };



        class CTextHost : public ITextHost, public IRichEditOleCallback
        {
        public:

        // Lifecycle

            CTextHost()
            {
                m_spTextServices = NULL;
                m_spTextDocument = NULL;

                SetRectEmpty(&m_rcClient);
                SetRectEmpty(&m_rcViewInset);

                InitCharFormat();
                InitParaFormat();

                m_dwPropertyBits = TXTBIT_RICHTEXT | TXTBIT_MULTILINE | TXTBIT_WORDWRAP | TXTBIT_USECURRENTBKG;
                CreateTextServicesObject();
            }

            ~CTextHost()
            {
                free(m_pCF);
                if (m_spTextServices != NULL)
                    m_spTextServices->Release();
                if (m_spTextDocument != NULL)
                    m_spTextDocument->Release();
            }

       // Public Interface

            void SetText(System::String^ text)
            {
                HRESULT hr;
                long    len;
                LRESULT lResult = 0;
                EDITSTREAM editStream;

                pin_ptr<const wchar_t> pText = PtrToStringChars(text);
                len = text->Length;

                // find the require WideCharBuffer length
                //
                long wcLen = WideCharToMultiByte(CP_ACP, 0, pText, len, NULL, 0, NULL, NULL);

                // allocate memory for the string
                //
                m_editCookie.text = (LPSTR) malloc(wcLen + 1);
                WideCharToMultiByte(CP_ACP, 0, pText, len, m_editCookie.text, wcLen, NULL, NULL);
                m_editCookie.size = wcLen;
                m_editCookie.count = 0;

                editStream.dwCookie = (DWORD_PTR) &m_editCookie;
                editStream.dwError = 0;
                editStream.pfnCallback = EditStreamInCallback;
                hr = m_spTextServices->TxSendMessage(EM_STREAMIN, (WPARAM)(SF_RTF | SF_UNICODE), (LPARAM)&editStream, &lResult);
                if (hr != S_OK) throw gcnew System::ApplicationException("TxSendMessage Failed: " + hr.ToString());

                // free the memory allocated for the string
                //
                free(m_editCookie.text);
            }

            HRESULT Draw(HDC hdcDraw, RECT *prc)
            {
                m_spTextServices->TxDraw(
                    DVASPECT_CONTENT,        // Draw Aspect
                    0,                       // Lindex
                    NULL,                    // Info for drawing optimization
                    NULL,                    // target device information
                    hdcDraw,                 // Draw device HDC
                    NULL,                    // Target device HDC
                    (RECTL *) prc,           // Bounding client rectangle
                    NULL,                    // Clipping rectangle for metafiles
                    (RECT *) NULL,           // Update rectangle
                    NULL,                    // Call back function
                    NULL,                    // Call back parameter
                    TXTVIEW_INACTIVE);       // What view of the object could be TXTVIEW_ACTIVE
                return S_OK;
            }

            long GetNaturalWidth(HDC hdcDraw)
            {
                long lWidth;
                long height = 1;
                SIZEL szExtent;
                szExtent.cy = height;
                szExtent.cx = 10000;
                lWidth = 10000;
                m_spTextServices->TxGetNaturalSize(DVASPECT_CONTENT, 
                    hdcDraw, 
                    NULL,
                    NULL,
                    TXTNS_FITTOCONTENT,
                    &szExtent,
                    &lWidth,
                    &height);
                return lWidth;
            }

            long GetNaturalHeight(HDC hdcDraw, long width)
            {
                long lHeight;
                SIZEL szExtent;
                szExtent.cx = width;
                szExtent.cy = 1;
                lHeight = 1;
                m_spTextServices->TxGetNaturalSize(DVASPECT_CONTENT, 
                    hdcDraw, 
                    NULL,
                    NULL,
                    TXTNS_FITTOCONTENT,
                    &szExtent,
                    &width,
                    &lHeight);
                return lHeight;
            }

            void SetWordWrap(bool value)
            {
                if (value != WordWrap())
                {
                    if (value)
                    {
                        m_dwPropertyBits |= TXTBIT_WORDWRAP;
                    }
                    else
                    {
                        m_dwPropertyBits &= ~TXTBIT_WORDWRAP;
                    }
                    m_spTextServices->OnTxPropertyBitsChange(TXTBIT_WORDWRAP, m_dwPropertyBits);
                }
            }

            bool WordWrap()
            {
                return ((m_dwPropertyBits & TXTBIT_WORDWRAP) != 0);
            }

            void SetMultiLine(bool value)
            {
                if (value != MultiLine())
                {
                    if (value)
                    {
                        m_dwPropertyBits |= TXTBIT_MULTILINE;
                    }
                    else
                    {
                         m_dwPropertyBits &= ~TXTBIT_MULTILINE;
                    }
                    m_spTextServices->OnTxPropertyBitsChange(TXTBIT_MULTILINE, m_dwPropertyBits);
                }
            }

            bool MultiLine()
            {
                return ((m_dwPropertyBits & TXTBIT_MULTILINE) != 0);
            }

            System::Drawing::Font^ DefaultFont()
            {
                return gcnew Drawing::Font("Arial", 10.0);
            }

            void SetFont(System::Drawing::Font^ font)
            {
                m_pCF->yHeight = (long) (20.0 * font->SizeInPoints);
                m_pCF->dwEffects = CFM_EFFECTS | CFE_AUTOBACKCOLOR;
                m_pCF->dwEffects &= ~(CFE_PROTECTED | CFE_LINK); // | CFE_AUTOCOLOR);
                if(!font->Bold)
                    m_pCF->dwEffects &= ~CFE_BOLD;
                if(!font->Italic)
                    m_pCF->dwEffects &= ~CFE_ITALIC;
                if(!font->Underline)
                    m_pCF->dwEffects &= ~CFE_UNDERLINE;
                if(!font->Strikeout)
                    m_pCF->dwEffects &= ~CFE_STRIKEOUT;

                HFONT hFont = (HFONT) font->ToHfont().ToPointer();
                LOGFONT lf;

                // Get LOGFONT for passed hfont
                if (!GetObject(hFont, sizeof(LOGFONT), &lf)) throw gcnew System::NotSupportedException("GetObject(LOGFONT) failed");
 
                m_pCF->bCharSet = lf.lfCharSet;
                m_pCF->bPitchAndFamily = lf.lfPitchAndFamily;
                wcscpy_s(m_pCF->szFaceName, LF_FACESIZE, lf.lfFaceName);

                if (m_spTextServices)
                {
                    m_spTextServices->OnTxPropertyBitsChange(TXTBIT_CHARFORMATCHANGE, m_dwPropertyBits);
                }
                ::DeleteObject(hFont);
            }

            void SetHorizontalAlignment(WORD value)
            {
                if (value != m_PF.wAlignment)
                {
                    m_PF.wAlignment = value;
                    if (m_spTextServices)
                    {
                        m_spTextServices->OnTxPropertyBitsChange(TXTBIT_PARAFORMATCHANGE, m_dwPropertyBits);
                    }
                }
            }

            void SetTextColor(COLORREF value)
            {
                if (value != m_pCF->crTextColor)
                {
                    m_pCF->crTextColor = value;
                    if (m_spTextServices)
                    {
                        m_spTextServices->OnTxPropertyBitsChange(TXTBIT_CHARFORMATCHANGE, m_dwPropertyBits);
                    }
                }
            }

        private:

      // Required COM methods

            HRESULT STDMETHODCALLTYPE QueryInterface( 
                /* [in] */ REFIID riid,
                /* [iid_is][out] */ void __RPC_FAR *__RPC_FAR *ppvObject)
            {
                *ppvObject = NULL;
                return S_FALSE;
            }

            ULONG STDMETHODCALLTYPE AddRef(void)
            {
                return 0;
            }

            ULONG STDMETHODCALLTYPE Release(void)
            {
                return 0;
            }


        // ITextHost interface

            HDC TxGetDC()
            {
                return NULL;
            }

            INT TxReleaseDC(HDC hdc)
            {
                return 1;
            }

            BOOL TxShowScrollBar(INT fnBar, BOOL fShow)
            {
                return FALSE;
            }

            BOOL TxEnableScrollBar(INT fuSBFlags, INT fuArrowflags)
            {
                return FALSE;
            }

            BOOL TxSetScrollRange(INT fnBar, LONG nMinPos, INT nMaxPos, BOOL fRedraw)
            {
                return FALSE;
            }

            BOOL TxSetScrollPos(INT fnBar, INT nPos, BOOL fRedraw)
            {
                return FALSE;
            }

            void TxInvalidateRect(LPCRECT prc, BOOL fMode)
            {
            }

            void TxViewChange(BOOL fUpdate)
            {
            }

            BOOL TxCreateCaret(HBITMAP hbmp, INT xWidth, INT yHeight)
            {
                return FALSE;
            }

            BOOL TxShowCaret(BOOL fShow)
            {
                return FALSE;
            }

            BOOL TxSetCaretPos(INT x, INT y)
            {
                return FALSE;
            }

            BOOL TxSetTimer(UINT idTimer, UINT uTimeout)
            {
                return FALSE;
            }

            void TxKillTimer(UINT idTimer)
            {
            }

            void TxScrollWindowEx(INT dx, INT dy, LPCRECT lprcScroll, LPCRECT lprcClip, HRGN hrgnUpdate, LPRECT lprcUpdate, UINT fuScroll)
            {
            }

            void TxSetCapture(BOOL fCapture)
            {
            }

            void TxSetFocus()
            {
            }

            void TxSetCursor(HCURSOR hcur, BOOL fText)
            {
            }

            BOOL TxScreenToClient(LPPOINT lppt)
            {
                return FALSE;
            }

            BOOL TxClientToScreen(LPPOINT lppt)
            {
                return FALSE;
            }

            HRESULT TxActivate(LONG * plOldState)
            {
                return S_OK;
            }

            HRESULT TxDeactivate(LONG lNewState)
            {
                return S_OK;
            }

            HRESULT TxGetClientRect(LPRECT prc)
            {
                *prc = m_rcClient;
                return S_OK;
            }

            HRESULT TxGetViewInset(LPRECT prc)
            {
                *prc = m_rcViewInset;
                return S_OK;
            }

            HRESULT TxGetCharFormat(const CHARFORMATW **ppCF)
            {
                *ppCF = m_pCF;
                return S_OK;
            }

            HRESULT TxGetParaFormat(const PARAFORMAT **ppPF)
            {
                *ppPF = &m_PF;
                return S_OK;
            }

            COLORREF TxGetSysColor(int nIndex)
            {
                if (nIndex == COLOR_WINDOWTEXT)
                    return m_pCF->crTextColor;  
                else
                    return GetSysColor(nIndex);
            }

            HRESULT TxGetBackStyle(TXTBACKSTYLE *pstyle)
            {
                *pstyle = TXTBACK_TRANSPARENT;
                return S_OK;
            }

            HRESULT TxGetMaxLength(DWORD *plength)
            {
                *plength = INFINITE;
                return S_OK;
            }

            HRESULT TxGetScrollBars(DWORD *pdwScrollBar)
            {
                *pdwScrollBar = 0;
                return S_OK;
            }

            HRESULT TxGetPasswordChar(TCHAR *pch)
            {
                return S_FALSE;
            }

            HRESULT TxGetAcceleratorPos(LONG *pcp)       
            {
                *pcp = -1;
                return S_OK;
            }

            HRESULT TxGetExtent(LPSIZEL lpExtent)
            {
                return E_NOTIMPL;
            }

            HRESULT OnTxCharFormatChange(const CHARFORMATW * pcf)
            {
                memcpy(m_pCF, pcf, pcf->cbSize);
                return S_OK;
            }

            HRESULT OnTxParaFormatChange(const PARAFORMAT * ppf)
            {
                memcpy(&m_PF, ppf, ppf->cbSize);
                return S_OK;
            }

            HRESULT TxGetPropertyBits(DWORD dwMask, DWORD *pdwBits)
            {
                *pdwBits = m_dwPropertyBits;
                return S_OK;
            }

            HRESULT TxNotify(DWORD iNotify, void *pv)
            {
                return S_OK;
            }

            HIMC TxImmGetContext()
            {
                return NULL;
            }

            void TxImmReleaseContext(HIMC himc)
            {
            }

            HRESULT TxGetSelectionBarWidth(LONG *lSelBarWidth)        
            {
                *lSelBarWidth = 100;
                return S_OK;
            }

            // IRichEditOleCallback methods - required to support inplace images

            STDMETHOD(ContextSensitiveHelp) (BOOL fEnterMode)                                                                                        {   return E_NOTIMPL;   }
            STDMETHOD(DeleteObject)         (LPOLEOBJECT lpoleobj)                                                                                      {   return E_NOTIMPL;   }
            STDMETHOD(GetClipboardData)     (CHARRANGE FAR *lpchrg, DWORD reco, LPDATAOBJECT FAR *lplpdataobj)                                          {   return E_NOTIMPL;   }
            STDMETHOD(GetContextMenu)       (WORD seltype, LPOLEOBJECT lpoleobj, CHARRANGE FAR *lpchrg, HMENU FAR *lphmenu)                             {   return E_NOTIMPL;   }
            STDMETHOD(GetDragDropEffect)    (BOOL fDrag, DWORD grfKeyState, LPDWORD pdwEffect)                                                          {   return E_NOTIMPL;   }
            STDMETHOD(GetInPlaceContext)    (LPOLEINPLACEFRAME FAR *lplpFrame, LPOLEINPLACEUIWINDOW FAR *lplpDoc, LPOLEINPLACEFRAMEINFO lpFrameInfo)    {   return E_NOTIMPL;   }
            STDMETHOD(QueryAcceptData)      (LPDATAOBJECT lpdataobj, CLIPFORMAT FAR *lpcfFormat, DWORD reco, BOOL fReally, HGLOBAL hMetaPict)           {   return E_NOTIMPL;   }
            STDMETHOD(QueryInsertObject)    (LPCLSID lpclsid, LPSTORAGE lpstg, LONG cp)                                                                 {   return S_OK;        }
            STDMETHOD(ShowContainerUI)      (BOOL fShow)                                                                                                {   return E_NOTIMPL;   }

            STDMETHODIMP GetNewStorage(LPSTORAGE FAR *lplpstg)
            {
                // Initialize a Storage Object from a DocFile in memory
                LPLOCKBYTES lpLockBytes = NULL;
                SCODE       sc  = ::CreateILockBytesOnHGlobal(NULL, TRUE, &lpLockBytes);
                if (sc != S_OK) return sc;
                sc = ::StgCreateDocfileOnILockBytes(lpLockBytes, STGM_SHARE_EXCLUSIVE|STGM_CREATE|STGM_READWRITE, 0, lplpstg);
                if (sc != S_OK) lpLockBytes->Release();
                return sc;
            }

        private:

        // Local Methods

            static DWORD CALLBACK EditStreamInCallback(DWORD_PTR dwCookie, LPBYTE pbBuff, LONG cb, LONG *pcb)
            {
                PCOOKIE pCookie = (PCOOKIE) dwCookie;

                if(pCookie->size - pCookie->count < (DWORD) cb)
                    *pcb = pCookie->size - pCookie->count;
                else
                    *pcb = cb;

                CopyMemory(pbBuff, pCookie->text + pCookie->count, *pcb);
                pCookie->count += *pcb;

                return 0;    //    callback succeeded - no errors
            }

            void InitParaFormat()
            {
                memset(&m_PF, 0, sizeof(PARAFORMAT2));
                m_PF.cbSize = sizeof(PARAFORMAT2);
                m_PF.dwMask = PFM_ALL;
                m_PF.wAlignment = PFA_LEFT;
                m_PF.cTabCount = 1;
                m_PF.rgxTabs[0] = lDefaultTab;
            }

            void InitCharFormat()
            {
                m_pCF = (CHARFORMAT2W*) malloc(sizeof(CHARFORMAT2W));
                memset(m_pCF, 0, sizeof(CHARFORMAT2W));
                m_pCF->cbSize = sizeof(CHARFORMAT2W);
                m_pCF->yOffset = 0;
                m_pCF->dwEffects = CFM_EFFECTS | CFE_AUTOBACKCOLOR;
                m_pCF->dwEffects &= ~(CFE_PROTECTED | CFE_LINK | CFE_AUTOCOLOR);
                m_pCF->dwMask = CFM_ALL | CFM_SIZE | CFM_BACKCOLOR | CFM_STYLE | CFM_COLOR;

                SetFont(DefaultFont());
                SetTextColor(GetSysColor(COLOR_WINDOWTEXT));
            }

            void CreateTextServicesObject()
            {
                HRESULT hr;
                IUnknown *spUnk;
                hr = CreateTextServices(NULL, static_cast<ITextHost*>(this), &spUnk);
                if (hr != S_OK) throw gcnew System::NotSupportedException("CreateTextServices failed");
                try
                {
                    hr = spUnk->QueryInterface(IID_ITextServices2, (void**)&m_spTextServices);
                    if (hr != S_OK) throw gcnew System::NotSupportedException("ITextServices not supported");

                    hr = spUnk->QueryInterface(IID_ITextDocument, (void**)&m_spTextDocument);
                    if (hr != S_OK) throw gcnew System::NotSupportedException("ITextDocument not supported");
           
                    IRichEditOleCallback* callback = this;
                    m_spTextServices->TxSendMessage(EM_SETOLECALLBACK , 0 , LPARAM(callback), &hr); 
                    if (FAILED(hr)) throw gcnew System::NotSupportedException("EM_SETOLECALLBACK failed");
                }
                finally
                {
                    spUnk->Release();
                }
            }

            // Variables
            RECT             m_rcClient;          // Client Rect
            RECT             m_rcViewInset;       // view rect inset
            SIZEL            m_sizelExtent;       // Extent array

            CHARFORMAT2W*    m_pCF;
            PARAFORMAT2      m_PF;
            DWORD            m_dwPropertyBits;    // Property bits
            COOKIE           m_editCookie;

            ITextServices*   m_spTextServices;
            ITextDocument*   m_spTextDocument;

        };

    } 
} // namespace