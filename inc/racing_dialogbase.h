///////////////////////////////////////////////////////////////////////////
// C++ code generated with wxFormBuilder (version Nov  9 2019)
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
#include <wx/sizer.h>
#include <wx/button.h>
#include <wx/bitmap.h>
#include <wx/image.h>
#include <wx/icon.h>
#include <wx/dialog.h>

///////////////////////////////////////////////////////////////////////////


///////////////////////////////////////////////////////////////////////////////
/// Class racing_dialogbase
///////////////////////////////////////////////////////////////////////////////
class RacingDialogBase : public wxDialog
{
	private:

	protected:
		wxStaticText* labelSpeed;
		wxStaticText* labelTimer;
		wxStaticText* labelTTG;
		wxStaticText* labelDistance;
		wxButton* buttonStart;
		wxButton* buttonReset;
		wxButton* buttonPort;
		wxButton* buttonStbd;
		wxStdDialogButtonSizer* sizerDialogButtons;
		wxButton* sizerDialogButtonsCancel;

		// Virtual event handlers, overide them in your derived class
		virtual void OnInit( wxInitDialogEvent& event ) { event.Skip(); }
		virtual void OnStart( wxCommandEvent& event ) { event.Skip(); }
		virtual void OnReset( wxCommandEvent& event ) { event.Skip(); }
		virtual void OnPort( wxCommandEvent& event ) { event.Skip(); }
		virtual void OnStbd( wxCommandEvent& event ) { event.Skip(); }
		virtual void OnCancel( wxCommandEvent& event ) { event.Skip(); }
		virtual void OnClose( wxCloseEvent& event ) { event.Skip(); }


	public:

		RacingDialogBase( wxWindow* parent, wxWindowID id = wxID_ANY, const wxString& title = wxT("Race Timer"), const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxSize( 408,425 ), long style = wxDEFAULT_DIALOG_STYLE );
		~RacingDialogBase();

};

