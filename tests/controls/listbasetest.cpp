///////////////////////////////////////////////////////////////////////////////
// Name:        tests/controls/listbasetest.cpp
// Purpose:     Common wxListCtrl and wxListView tests
// Author:      Steven Lamerton
// Created:     2010-07-20
// Copyright:   (c) 2008,2025 Vadim Zeitlin <vadim@wxwidgets.org>,
//              (c) 2010 Steven Lamerton
///////////////////////////////////////////////////////////////////////////////

#include "testprec.h"

#if wxUSE_LISTCTRL


#ifndef WX_PRECOMP
    #include "wx/app.h"
#endif // WX_PRECOMP

#include "wx/listctrl.h"
#include "listbasetest.h"
#include "testableframe.h"
#include "asserthelper.h"
#include "wx/uiaction.h"
#include "wx/imaglist.h"
#include "wx/artprov.h"
#include "wx/stopwatch.h"

void ListBaseTestCase::ColumnsOrder()
{
#ifdef wxHAS_LISTCTRL_COLUMN_ORDER
    wxListCtrl* const list = GetList();

    int n;
    wxListItem li;
    li.SetMask(wxLIST_MASK_TEXT);

    // first set up some columns
    static const int NUM_COLS = 3;

    list->InsertColumn(0, "Column 0");
    list->InsertColumn(1, "Column 1");
    list->InsertColumn(2, "Column 2");

    // and a couple of test items too
    list->InsertItem(0, "Item 0");
    list->SetItem(0, 1, "first in first");

    list->InsertItem(1, "Item 1");
    list->SetItem(1, 2, "second in second");


    // check that the order is natural in the beginning
    const wxArrayInt orderOrig = list->GetColumnsOrder();
    for ( n = 0; n < NUM_COLS; n++ )
        CHECK( orderOrig[n]  == n );

    // then rearrange them: using { 2, 0, 1 } order means that column 2 is
    // shown first, then column 0 and finally column 1
    wxArrayInt order(3);
    order[0] = 2;
    order[1] = 0;
    order[2] = 1;
    list->SetColumnsOrder(order);

    // check that we get back the same order as we set
    const wxArrayInt orderNew = list->GetColumnsOrder();
    for ( n = 0; n < NUM_COLS; n++ )
        CHECK( orderNew[n]  == order[n] );

    // and the order -> index mappings for individual columns
    for ( n = 0; n < NUM_COLS; n++ )
        CHECK( list->GetColumnIndexFromOrder(n)  == order[n] );

    // and also the reverse mapping
    CHECK( list->GetColumnOrder(0)  == 1 );
    CHECK( list->GetColumnOrder(1)  == 2 );
    CHECK( list->GetColumnOrder(2)  == 0 );


    // finally check that accessors still use indices, not order
    CHECK( list->GetColumn(0, li) );
    CHECK( li.GetText()  == "Column 0" );

    li.SetId(0);
    li.SetColumn(1);
    CHECK( list->GetItem(li) );
    CHECK( li.GetText()  == "first in first" );

    li.SetId(1);
    li.SetColumn(2);
    CHECK( list->GetItem(li) );
    CHECK( li.GetText()  == "second in second" );
#endif // wxHAS_LISTCTRL_COLUMN_ORDER
}



void ListBaseTestCase::ItemRect()
{
    wxListCtrl* const list = GetList();

    // set up for the test
    list->InsertColumn(0, "Column 0", wxLIST_FORMAT_LEFT, 60);
    list->InsertColumn(1, "Column 1", wxLIST_FORMAT_LEFT, 50);
    list->InsertColumn(2, "Column 2", wxLIST_FORMAT_LEFT, 40);

    list->InsertItem(0, "Item 0");
    list->SetItem(0, 1, "first column");
    list->SetItem(0, 1, "second column");

    // do test
    wxRect r;
    WX_ASSERT_FAILS_WITH_ASSERT( list->GetItemRect(1, r) );
    CHECK( list->GetItemRect(0, r) );
    CHECK( r.GetWidth()  == 150 );

    CHECK( list->GetSubItemRect(0, 0, r) );
    CHECK( r.GetWidth()  == 60 );

    CHECK( list->GetSubItemRect(0, 1, r) );
    CHECK( r.GetWidth()  == 50 );

    CHECK( list->GetSubItemRect(0, 2, r) );
    CHECK( r.GetWidth()  == 40 );

    WX_ASSERT_FAILS_WITH_ASSERT( list->GetSubItemRect(0, 3, r) );


    // As we have a header, the top item shouldn't be at (0, 0), but somewhere
    // below the header.
    //
    // Notice that we consider that the header can't be less than 10 pixels
    // because we don't know its exact height.
    CHECK( list->GetItemRect(0, r) );
    CHECK( r.y >= 10 );

    // However if we remove the header now, the item should be at (0, 0).
    list->SetWindowStyle(wxLC_REPORT | wxLC_NO_HEADER);
    CHECK( list->GetItemRect(0, r) );
    CHECK( r.y  == 0 );
}

void ListBaseTestCase::ItemText()
{
    wxListCtrl* const list = GetList();

    list->InsertColumn(0, "First");
    list->InsertColumn(1, "Second");

    list->InsertItem(0, "0,0");
    CHECK( list->GetItemText(0) == "0,0" );
    CHECK( list->GetItemText(0, 1)  == "" );

    list->SetItem(0, 1, "0,1");
    CHECK( list->GetItemText(0, 1) == "0,1" );
}

void ListBaseTestCase::ChangeMode()
{
    wxListCtrl* const list = GetList();

    list->InsertColumn(0, "Header");
    list->InsertItem(0, "First");
    list->InsertItem(1, "Second");
    CHECK( list->GetItemCount()  == 2 );

    // check that switching the mode preserves the items
    list->SetWindowStyle(wxLC_ICON);
    CHECK( list->GetItemCount()  == 2 );
    CHECK( list->GetItemText(0)  == "First" );

    // and so does switching back
    list->SetWindowStyle(wxLC_REPORT);
    CHECK( list->GetItemCount()  == 2 );
    CHECK( list->GetItemText(0)  == "First" );
}

void ListBaseTestCase::MultiSelect()
{
#if wxUSE_UIACTIONSIMULATOR
    if ( !EnableUITests() )
        return;

#if defined(__WXGTK__) && !defined(__WXGTK3__)
    // FIXME: This test fails on GitHub CI under wxGTK2 although works fine on
    //        development machine, no idea why though!
    if ( IsAutomaticTest() )
        return;
#endif // wxGTK2

    wxListCtrl* const list = GetList();

    EventCounter focused(list, wxEVT_LIST_ITEM_FOCUSED);
    EventCounter selected(list, wxEVT_LIST_ITEM_SELECTED);
    EventCounter deselected(list, wxEVT_LIST_ITEM_DESELECTED);

    list->InsertColumn(0, "Header");

    for ( int i = 0; i < 10; ++i )
        list->InsertItem(i, wxString::Format("Item %d", i));

    wxUIActionSimulator sim;

    wxRect pos;
    list->GetItemRect(2, pos); // Choose the third item as anchor

    // We move in slightly so we are not on the edge
    wxPoint point = list->ClientToScreen(pos.GetPosition()) + wxPoint(10, 10);

    sim.MouseMove(point);
    wxYield();

    sim.MouseClick(); // select the anchor
    wxYield();

    list->GetItemRect(5, pos);
    point = list->ClientToScreen(pos.GetPosition()) + wxPoint(10, 10);

    sim.MouseMove(point);
    wxYield();

    sim.KeyDown(WXK_SHIFT);
    sim.MouseClick();
    sim.KeyUp(WXK_SHIFT);
    wxYield();

    // when the first item was selected the focus changes to it, but not
    // on subsequent clicks
    CHECK( list->GetSelectedItemCount() == 4 ); // item 2 to 5 (inclusive) are selected
    CHECK( focused.GetCount() == 2 ); // count the focus which was on the anchor
    CHECK( selected.GetCount() == 4 );
    CHECK( deselected.GetCount() == 0 );

    focused.Clear();
    selected.Clear();
    deselected.Clear();

    sim.Char(WXK_END, wxMOD_SHIFT); // extend the selection to the last item
    wxYield();

    CHECK( list->GetSelectedItemCount() == 8 ); // item 2 to 9 (inclusive) are selected
    CHECK( focused.GetCount() == 1 ); // focus is on the last item
    CHECK( selected.GetCount() == 4); // only newly selected items got the event
    CHECK( deselected.GetCount() == 0 );

    focused.Clear();
    selected.Clear();
    deselected.Clear();

    sim.Char(WXK_HOME, wxMOD_SHIFT); // select from anchor to the first item
    wxYield();

    CHECK( list->GetSelectedItemCount() == 3 ); // item 0 to 2 (inclusive) are selected
    CHECK( focused.GetCount() == 1 ); // focus is on item 0
    CHECK( selected.GetCount() == 2 ); // events are only generated for item 0 and 1
    CHECK( deselected.GetCount() == 7 ); // item 2 (exclusive) to 9 are deselected

    focused.Clear();
    selected.Clear();
    deselected.Clear();

    list->EnsureVisible(0);
    wxYield();

    list->GetItemRect(2, pos);
    point = list->ClientToScreen(pos.GetPosition()) + wxPoint(10, 10);

    sim.MouseMove(point);
    wxYield();

    sim.MouseClick();
    wxYield();

    CHECK( list->GetSelectedItemCount() == 1 ); // anchor is the only selected item
    CHECK( focused.GetCount() == 1 ); // because the focus changed from item 0 to anchor
    CHECK( selected.GetCount() == 0 ); // anchor is already in selection state
    CHECK( deselected.GetCount() == 2 ); // items 0 and 1 are deselected

    focused.Clear();
    selected.Clear();
    deselected.Clear();

    list->GetItemRect(3, pos);
    point = list->ClientToScreen(pos.GetPosition()) + wxPoint(10, 10);

    // select and deselect item 3 while leaving item 2 selected
    for ( int i = 0; i < 2; ++i )
    {
        sim.MouseMove(point + wxPoint(i*10, 0));
        wxYield();

        sim.KeyDown(WXK_CONTROL);
        sim.MouseClick();
        sim.KeyUp(WXK_CONTROL);
        wxYield();
    }

    // select only item 3
    sim.MouseMove(point);
    wxYield();

    sim.MouseClick();
    wxYield();

    CHECK( list->GetSelectedItemCount() == 1 ); // item 3 is the only selected item
    CHECK( focused.GetCount() == 1 ); // because the focus changed from anchor to item 3
    CHECK( selected.GetCount() == 2 ); // item 3 was selected twice
    CHECK( deselected.GetCount() == 2 ); // anchor and item 3 were each deselected once
#endif // wxUSE_UIACTIONSIMULATOR
}

void ListBaseTestCase::ItemClick()
{
#if wxUSE_UIACTIONSIMULATOR
    if ( !EnableUITests() )
        return;

#ifdef __WXMSW__
    // FIXME: This test fails on MSW buildbot slaves although works fine on
    //        development machine, no idea why. It seems to be a problem with
    //        wxUIActionSimulator rather the wxListCtrl control itself however.
    if ( IsAutomaticTest() )
        return;
#endif // __WXMSW__

    wxListCtrl* const list = GetList();

    list->InsertColumn(0, "Column 0", wxLIST_FORMAT_LEFT, 60);
    list->InsertColumn(1, "Column 1", wxLIST_FORMAT_LEFT, 50);
    list->InsertColumn(2, "Column 2", wxLIST_FORMAT_LEFT, 40);

    list->InsertItem(0, "Item 0");
    list->SetItem(0, 1, "first column");
    list->SetItem(0, 2, "second column");

    EventCounter selected(list, wxEVT_LIST_ITEM_SELECTED);
    EventCounter focused(list, wxEVT_LIST_ITEM_FOCUSED);
    EventCounter activated(list, wxEVT_LIST_ITEM_ACTIVATED);
    EventCounter rclick(list, wxEVT_LIST_ITEM_RIGHT_CLICK);
    EventCounter deselected(list, wxEVT_LIST_ITEM_DESELECTED);

    wxUIActionSimulator sim;

    wxRect pos;
    list->GetItemRect(0, pos);

    //We move in slightly so we are not on the edge
    wxPoint point = list->ClientToScreen(pos.GetPosition()) + wxPoint(10, 10);

    sim.MouseMove(point);
    wxYield();

    sim.MouseClick();
    wxYield();

    sim.MouseDblClick();
    wxYield();

    sim.MouseClick(wxMOUSE_BTN_RIGHT);
    wxYield();

    // We want a point within the listctrl but below any items
    point = list->ClientToScreen(pos.GetPosition()) + wxPoint(10, 50);

    sim.MouseMove(point);
    wxYield();

    sim.MouseClick();
    wxYield();

    // when the first item was selected the focus changes to it, but not
    // on subsequent clicks
    CHECK( focused.GetCount() == 1 );
    CHECK( selected.GetCount() == 1 );
    CHECK( deselected.GetCount() == 1 );
    CHECK( activated.GetCount() == 1 );
    CHECK( rclick.GetCount() == 1 );
#endif // wxUSE_UIACTIONSIMULATOR
}

void ListBaseTestCase::KeyDown()
{
#if wxUSE_UIACTIONSIMULATOR
    if ( !EnableUITests() )
        return;

    wxListCtrl* const list = GetList();

    EventCounter keydown(list, wxEVT_LIST_KEY_DOWN);

    wxUIActionSimulator sim;

    list->SetFocus();
    wxYield();
    sim.Text("aAbB"); // 4 letters + 2 shift mods.
    wxYield();

    CHECK( keydown.GetCount() == 6 );
#endif
}

void ListBaseTestCase::DeleteItems()
{
#ifndef __WXOSX__
    wxListCtrl* const list = GetList();

    EventCounter deleteitem(list, wxEVT_LIST_DELETE_ITEM);
    EventCounter deleteall(list, wxEVT_LIST_DELETE_ALL_ITEMS);


    list->InsertColumn(0, "Column 0", wxLIST_FORMAT_LEFT, 60);
    list->InsertColumn(1, "Column 1", wxLIST_FORMAT_LEFT, 50);
    list->InsertColumn(2, "Column 2", wxLIST_FORMAT_LEFT, 40);

    list->InsertItem(0, "Item 0");
    list->InsertItem(1, "Item 1");
    list->InsertItem(2, "Item 1");

    list->DeleteItem(0);
    list->DeleteItem(0);
    list->DeleteAllItems();

    //Add some new items to tests ClearAll with
    list->InsertColumn(0, "Column 0");
    list->InsertItem(0, "Item 0");
    list->InsertItem(1, "Item 1");

    //Check that ClearAll actually sends a DELETE_ALL_ITEMS event
    list->ClearAll();

    //ClearAll and DeleteAllItems shouldn't send an event if there was nothing
    //to clear
    list->ClearAll();
    list->DeleteAllItems();

    CHECK( deleteitem.GetCount() == 2 );
    CHECK( deleteall.GetCount() == 2 );
#endif
}

void ListBaseTestCase::InsertItem()
{
    wxListCtrl* const list = GetList();

    EventCounter insert(list, wxEVT_LIST_INSERT_ITEM);

    list->InsertColumn(0, "Column 0", wxLIST_FORMAT_LEFT, 60);

    wxListItem item;
    item.SetId(0);
    item.SetText("some text");

    list->InsertItem(item);
    list->InsertItem(1, "more text");

    CHECK( insert.GetCount() == 2 );
}

void ListBaseTestCase::Find()
{
    wxListCtrl* const list = GetList();

    // set up for the test
    list->InsertColumn(0, "Column 0");
    list->InsertColumn(1, "Column 1");

    list->InsertItem(0, "Item 0");
    list->SetItem(0, 1, "first column");

    list->InsertItem(1, "Item 1");
    list->SetItem(1, 1, "first column");

    list->InsertItem(2, "Item 40");
    list->SetItem(2, 1, "first column");

    list->InsertItem(3, "ITEM 01");
    list->SetItem(3, 1, "first column");

    CHECK( list->FindItem(-1, "Item 1") == 1 );
    CHECK( list->FindItem(-1, "Item 4", true) == 2 );
    CHECK( list->FindItem(1, "Item 40") == 2 );
    CHECK( list->FindItem(2, "Item 0", true) == 3 );
}

void ListBaseTestCase::Visible()
{
    wxListCtrl* const list = GetList();

    list->InsertColumn(0, "Column 0");
    list->InsertItem(0, wxString::Format("string 0"));

    int count = list->GetCountPerPage();

    for( int i = 1; i < count + 10; i++ )
    {
        list->InsertItem(i, wxString::Format("string %d", i));
    }

    CHECK( list->GetItemCount() == count + 10 );
    CHECK( list->GetTopItem() == 0 );
    CHECK(list->IsVisible(0));
    CHECK(!list->IsVisible(count + 1));

    CHECK(list->EnsureVisible(count + 9));
    CHECK(list->IsVisible(count + 9));
    CHECK(!list->IsVisible(9));

    CHECK(list->GetTopItem() != 0);
}

void ListBaseTestCase::ItemFormatting()
{
    wxListCtrl* const list = GetList();

    list->InsertColumn(0, "Column 0");

    list->InsertItem(0, "Item 0");
    list->InsertItem(1, "Item 1");
    list->InsertItem(2, "Item 2");

    list->SetTextColour(*wxYELLOW);
    list->SetBackgroundColour(*wxGREEN);
    list->SetItemTextColour(0, *wxRED);
    list->SetItemBackgroundColour(1, *wxBLUE);

    CHECK( list->GetBackgroundColour() == *wxGREEN );
    CHECK( list->GetItemBackgroundColour(1) == *wxBLUE );

    CHECK( list->GetTextColour() == *wxYELLOW );
    CHECK( list->GetItemTextColour(0) == *wxRED );
}

void ListBaseTestCase::EditLabel()
{
#if wxUSE_UIACTIONSIMULATOR
    if ( !EnableUITests() )
        return;

    wxListCtrl* const list = GetList();

    list->SetWindowStyleFlag(wxLC_REPORT | wxLC_EDIT_LABELS);

    list->InsertColumn(0, "Column 0");

    list->InsertItem(0, "Item 0");
    list->InsertItem(1, "Item 1");

    EventCounter beginedit(list, wxEVT_LIST_BEGIN_LABEL_EDIT);
    EventCounter endedit(list, wxEVT_LIST_END_LABEL_EDIT);

    wxUIActionSimulator sim;

    list->EditLabel(0);
    wxYield();

    sim.Text("sometext");
    wxYield();

    sim.Char(WXK_RETURN);

    wxYield();

    CHECK( beginedit.GetCount() == 1 );
    CHECK( endedit.GetCount() == 1 );
#endif
}

void ListBaseTestCase::ImageList()
{
    wxListCtrl* const list = GetList();

    wxSize size(32, 32);

    wxImageList* imglist = new wxImageList(size.x, size.y);
    imglist->Add(wxArtProvider::GetIcon(wxART_INFORMATION, wxART_OTHER, size));
    imglist->Add(wxArtProvider::GetIcon(wxART_QUESTION, wxART_OTHER, size));
    imglist->Add(wxArtProvider::GetIcon(wxART_WARNING, wxART_OTHER, size));

    list->AssignImageList(imglist, wxIMAGE_LIST_NORMAL);

    CHECK( list->GetImageList(wxIMAGE_LIST_NORMAL) == imglist );
}

void ListBaseTestCase::HitTest()
{
#ifdef __WXMSW__ // ..until proven to work with other platforms
    wxListCtrl* const list = GetList();
    list->SetWindowStyle(wxLC_REPORT);

    // set small image list
    wxSize size(16, 16);
    wxImageList* m_imglistSmall = new wxImageList(size.x, size.y);
    m_imglistSmall->Add(wxArtProvider::GetIcon(wxART_INFORMATION, wxART_LIST, size));
    list->AssignImageList(m_imglistSmall, wxIMAGE_LIST_SMALL);

    // insert 2 columns
    list->InsertColumn(0, "Column 0");
    list->InsertColumn(1, "Column 1");

    // and a couple of test items too
    list->InsertItem(0, "Item 0", 0);
    list->SetItem(0, 1, "0, 1");

    list->InsertItem(1, "Item 1", 0);

    // enable checkboxes to test state icon
    list->EnableCheckBoxes();

    // get coordinates
    wxRect rectSubItem0, rectIcon;
    list->GetSubItemRect(0, 0, rectSubItem0); // column 0
    list->GetItemRect(0, rectIcon, wxLIST_RECT_ICON); // icon
    int y = rectSubItem0.GetTop() + (rectSubItem0.GetBottom() -
            rectSubItem0.GetTop()) / 2;
    int flags = 0;

    // state icon (checkbox)
    int xCheckBox = rectSubItem0.GetLeft() + (rectIcon.GetLeft() -
                    rectSubItem0.GetLeft()) / 2;
    list->HitTest(wxPoint(xCheckBox, y), flags);
    CHECK( flags == wxLIST_HITTEST_ONITEMSTATEICON );

    // icon
    int xIcon = rectIcon.GetLeft() + (rectIcon.GetRight() - rectIcon.GetLeft()) / 2;
    list->HitTest(wxPoint(xIcon, y), flags);
    CHECK( flags == wxLIST_HITTEST_ONITEMICON );

    // label, beyond column 0
    wxRect rectItem;
    list->GetItemRect(0, rectItem); // entire item
    int xHit = rectSubItem0.GetRight() + (rectItem.GetRight() - rectSubItem0.GetRight()) / 2;
    list->HitTest(wxPoint(xHit, y), flags);
    CHECK( flags == wxLIST_HITTEST_ONITEMLABEL );
#endif // __WXMSW__
}

namespace
{
    //From the sample but fixed so it actually inverts
    int wxCALLBACK
    MyCompareFunction(wxIntPtr item1, wxIntPtr item2, wxIntPtr WXUNUSED(sortData))
    {
        // inverse the order
        if (item1 < item2)
            return 1;
        if (item1 > item2)
            return -1;

        return 0;
    }

}

void ListBaseTestCase::Sort()
{
    wxListCtrl* const list = GetList();

    list->InsertColumn(0, "Column 0");

    list->InsertItem(0, "Item 0");
    list->SetItemData(0, 0);
    list->InsertItem(1, "Item 1");
    list->SetItemData(1, 1);

    list->SortItems(MyCompareFunction, 0);

    CHECK( list->GetItemText(0) == "Item 1" );
    CHECK( list->GetItemText(1) == "Item 0" );
}

#endif //wxUSE_LISTCTRL
