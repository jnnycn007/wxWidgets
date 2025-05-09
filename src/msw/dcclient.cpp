/////////////////////////////////////////////////////////////////////////////
// Name:        src/msw/dcclient.cpp
// Purpose:     wxClientDC class
// Author:      Julian Smart
// Created:     01/02/97
// Copyright:   (c) Julian Smart
// Licence:     wxWindows licence
/////////////////////////////////////////////////////////////////////////////

// ===========================================================================
// declarations
// ===========================================================================

// ----------------------------------------------------------------------------
// headers
// ----------------------------------------------------------------------------

// For compilers that support precompilation, includes "wx.h".
#include "wx/wxprec.h"


#include "wx/dcclient.h"
#include "wx/msw/dcclient.h"

#ifndef WX_PRECOMP
    #include "wx/string.h"
    #include "wx/log.h"
    #include "wx/window.h"
#endif

#include "wx/stack.h"

#include "wx/msw/private.h"
#include "wx/msw/private/paint.h"

#include <unordered_map>

// ----------------------------------------------------------------------------
// local data structures
// ----------------------------------------------------------------------------

// This is a base class for two concrete subclasses below and contains HDC
// cached for the duration of the WM_PAINT processing together with some
// bookkeeping information.
class wxPaintDCInfo
{
public:
    wxPaintDCInfo(HDC hdc)
        : m_hdc(hdc)
    {
    }

    // The derived class must perform some cleanup.
    virtual ~wxPaintDCInfo() = 0;

    WXHDC GetHDC() const { return (WXHDC)m_hdc; }

protected:
    const HDC m_hdc;

    wxDECLARE_NO_COPY_CLASS(wxPaintDCInfo);
};

namespace
{

// This subclass contains information for the HDCs we create ourselves, i.e.
// those for which we call BeginPaint() -- and hence need to call EndPaint().
class wxPaintDCInfoOur : public wxPaintDCInfo
{
public:
    wxPaintDCInfoOur(wxWindow* win)
        : wxPaintDCInfo(::BeginPaint(GetHwndOf(win), GetPaintStructPtr(m_ps))),
          m_hwnd(GetHwndOf(win))
    {
    }

    virtual ~wxPaintDCInfoOur()
    {
        ::EndPaint(m_hwnd, &m_ps);
    }

private:
    // This helper is only needed in order to call it from the ctor initializer
    // list.
    static PAINTSTRUCT* GetPaintStructPtr(PAINTSTRUCT& ps)
    {
        wxZeroMemory(ps);
        return &ps;
    }

    const HWND m_hwnd;
    PAINTSTRUCT m_ps;

    wxDECLARE_NO_COPY_CLASS(wxPaintDCInfoOur);
};

// The global map containing HDC to use for the given window. The entries in
// this map only exist during WM_PAINT processing and are destroyed when it is
// over.
//
// It is needed because in some circumstances it can happen that more than one
// wxPaintDC is created for the same window during its WM_PAINT handling (and
// as this can happen implicitly, e.g. by calling a function in some library,
// this can be quite difficult to find) but we need to reuse the same HDC for
// all of them because we can't call BeginPaint() more than once. So we cache
// the first HDC created for the window in this map and then reuse it later if
// needed. And, of course, remove it from the map when the painting is done.
using PaintDCInfos = std::unordered_map<wxWindow*, wxPaintDCInfo*>;

PaintDCInfos gs_PaintDCInfos;

} // anonymous namespace

// ===========================================================================
// implementation
// ===========================================================================

// ----------------------------------------------------------------------------
// wxMSWWindowDCImpl
// ----------------------------------------------------------------------------

wxIMPLEMENT_ABSTRACT_CLASS(wxWindowDCImpl, wxMSWDCImpl);

wxWindowDCImpl::wxWindowDCImpl( wxDC *owner ) :
   wxMSWDCImpl( owner )
{
}

wxWindowDCImpl::wxWindowDCImpl( wxDC *owner, wxWindow *window ) :
   wxMSWDCImpl( owner )
{
    wxCHECK_RET( window, wxT("invalid window in wxWindowDCImpl") );

    InitWindow(window);
    m_hDC = (WXHDC) ::GetWindowDC(GetHwndOf(m_window));

    // m_bOwnsDC was already set to false in the base class ctor, so the DC
    // will be released (and not deleted) in ~wxDC
    InitDC();
}

void wxWindowDCImpl::InitDC()
{
    // the background mode is only used for text background and is set in
    // DrawText() to OPAQUE as required, otherwise always TRANSPARENT,
    ::SetBkMode(GetHdc(), TRANSPARENT);

    // since we are a window dc we need to grab the palette from the window
#if wxUSE_PALETTE
    InitializePalette();
#endif
}

void wxWindowDCImpl::DoGetSize(int *width, int *height) const
{
    wxCHECK_RET( m_window, wxT("wxWindowDCImpl without a window?") );

    m_window->GetSize(width, height);
}

// ----------------------------------------------------------------------------
// wxClientDCImpl
// ----------------------------------------------------------------------------

wxIMPLEMENT_ABSTRACT_CLASS(wxClientDCImpl, wxWindowDCImpl);

wxClientDCImpl::wxClientDCImpl( wxDC *owner ) :
   wxWindowDCImpl( owner )
{
}

wxClientDCImpl::wxClientDCImpl( wxDC *owner, wxWindow *window ) :
   wxWindowDCImpl( owner )
{
    wxCHECK_RET( window, wxT("invalid window in wxClientDCImpl") );

    InitWindow(window);
    m_hDC = (WXHDC)::GetDC(GetHwndOf(window));

    // m_bOwnsDC was already set to false in the base class ctor, so the DC
    // will be released (and not deleted) in ~wxDC

    InitDC();
}

void wxClientDCImpl::InitDC()
{
    wxWindowDCImpl::InitDC();

    // Account for the origin of the client area which is non-zero only for
    // TLWs with (left or top) toolbar: we shouldn't draw over the toolbar.
    wxPoint ptOrigin = m_window->GetClientAreaOrigin();
    if ( ptOrigin.x || ptOrigin.y )
    {
        // no need to shift DC origin if shift is null
        SetDeviceOrigin(ptOrigin.x, ptOrigin.y);
    }

    // in wxUniv build we must manually do some DC adjustments usually
    // performed by Windows for us
#if defined(__WXUNIVERSAL__)
    // clip the DC to avoid overwriting the non client area
    wxSize size = m_window->GetClientSize();
    DoSetClippingRegion(0, 0, size.x, size.y);
#endif // __WXUNIVERSAL__
}

wxClientDCImpl::~wxClientDCImpl()
{
}

void wxClientDCImpl::DoGetSize(int *width, int *height) const
{
    wxCHECK_RET( m_window, wxT("wxClientDCImpl without a window?") );

    m_window->GetClientSize(width, height);
}

// ----------------------------------------------------------------------------
// wxPaintDCImpl
// ----------------------------------------------------------------------------

wxIMPLEMENT_ABSTRACT_CLASS(wxPaintDCImpl, wxClientDCImpl);

wxPaintDCImpl::wxPaintDCImpl( wxDC *owner ) :
   wxClientDCImpl( owner )
{
}

wxPaintDCImpl::wxPaintDCImpl( wxDC *owner, wxWindow *window ) :
   wxClientDCImpl( owner )
{
    wxCHECK_RET( window, wxT("null canvas in wxPaintDCImpl ctor") );

    using namespace wxMSWImpl;
    wxCHECK_RET( !paintStack.empty(),
                 "wxPaintDC can't be created outside wxEVT_PAINT handler" );
    wxCHECK_RET( paintStack.top().window == window,
                 "wxPaintDC must be associated with the window being repainted" );

    paintStack.top().createdPaintDC = true;

    InitWindow(window);

    // do we have a DC for this window in the cache?
    m_hDC = FindDCInCache(m_window);
    if ( !m_hDC )
    {
        // not in cache, create a new one
        wxPaintDCInfoOur* const info = new wxPaintDCInfoOur(m_window);
        gs_PaintDCInfos[m_window] = info;
        m_hDC = info->GetHDC();
    }

    // Note: at this point m_hDC can be null under MicroWindows, when dragging.
    if (!GetHDC())
        return;

    // (re)set the DC parameters.
    InitDC();

    // the HDC can have a clipping box (which we didn't set), make sure our
    // DoGetClippingRect() checks for it
    m_clipping = true;
}

wxPaintDCImpl::~wxPaintDCImpl()
{
    if ( m_hDC )
    {
        SelectOldObjects(m_hDC);
        m_hDC = 0;
    }
}


/* static */
wxPaintDCInfo *wxPaintDCImpl::FindInCache(wxWindow *win)
{
    PaintDCInfos::const_iterator it = gs_PaintDCInfos.find( win );

    return it != gs_PaintDCInfos.end() ? it->second : nullptr;
}

/* static */
WXHDC wxPaintDCImpl::FindDCInCache(wxWindow* win)
{
    wxPaintDCInfo* const info = FindInCache(win);

    return info ? info->GetHDC() : 0;
}

/* static */
void wxPaintDCImpl::EndPaint(wxWindow *win)
{
    wxPaintDCInfo *info = FindInCache(win);
    if ( info )
    {
        gs_PaintDCInfos.erase(win);
        delete info;
    }
}

wxPaintDCInfo::~wxPaintDCInfo()
{
}

/*
 * wxPaintDCEx
 */

class wxPaintDCExImpl: public wxPaintDCImpl
{
public:
    wxPaintDCExImpl( wxDC *owner, wxWindow *window, WXHDC dc );
    ~wxPaintDCExImpl();
};


wxIMPLEMENT_ABSTRACT_CLASS(wxPaintDCEx, wxPaintDC);

wxPaintDCEx::wxPaintDCEx(wxWindow *window, WXHDC dc)
           : wxPaintDC(new wxPaintDCExImpl(this, window, dc))
{
}

wxPaintDCExImpl::wxPaintDCExImpl(wxDC *owner, wxWindow *window, WXHDC dc)
               : wxPaintDCImpl( owner )
{
    wxCHECK_RET( dc, wxT("wxPaintDCEx requires an existing device context") );

    InitWindow(window);
    m_hDC = dc;
}

wxPaintDCExImpl::~wxPaintDCExImpl()
{
    // prevent the base class dtor from ReleaseDC()ing it again
    m_hDC = 0;
}
