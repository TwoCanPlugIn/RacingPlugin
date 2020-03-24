///////////////////////////////////////////////////////////////////////////
// C++ code generated with wxFormBuilder (version Oct 26 2018)
// http://www.wxformbuilder.org/
//
// PLEASE DO *NOT* EDIT THIS FILE!
///////////////////////////////////////////////////////////////////////////

#pragma once

#include <wx/artprov.h>
#include <wx/xrc/xmlres.h>
#include <wx/string.h>
#include <wx/stattext.h>
#include <wx/gdicmn.h>
#include <wx/font.h>
#include <wx/colour.h>
#include <wx/settings.h>
#include <wx/bitmap.h>
#include <wx/image.h>
#include <wx/icon.h>
#include <wx/button.h>
#include <wx/sizer.h>
#include <wx/frame.h>

///////////////////////////////////////////////////////////////////////////


///////////////////////////////////////////////////////////////////////////////
/// Class racingWindow
///////////////////////////////////////////////////////////////////////////////
class RacingWindowBase : public wxFrame
{
	private:

	protected:
		wxStaticText* labelSpeed;
		wxStaticText* labelTimer;
		wxStaticText* labelTTG;
		wxStaticText* labelDistance;
		wxButton* buttonTimer;
		wxButton* buttonreset;
		wxButton* buttonPort;
		wxButton* buttonStarboard;
		wxButton* buttonCancel;

		// Virtual event handlers, overide them in your derived class
		virtual void OnClose( wxCloseEvent& event ) { event.Skip(); }
		virtual void OnStart( wxCommandEvent& event ) { event.Skip(); }
		virtual void OnReset( wxCommandEvent& event ) { event.Skip(); }
		virtual void OnPort( wxCommandEvent& event ) { event.Skip(); }
		virtual void OnStarboard( wxCommandEvent& event ) { event.Skip(); }
		virtual void OnCancel( wxCommandEvent& event ) { event.Skip(); }


	public:

		RacingWindowBase( wxWindow* parent, wxWindowID id = wxID_ANY, const wxString& title = wxT("Race Start Display"), const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxSize( 248,427 ), long style = wxDEFAULT_FRAME_STYLE| wxCAPTION | wxSTAY_ON_TOP | wxTAB_TRAVERSAL);

		~RacingWindowBase();

};

