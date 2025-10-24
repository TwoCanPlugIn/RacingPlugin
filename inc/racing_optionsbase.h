///////////////////////////////////////////////////////////////////////////
// C++ code generated with wxFormBuilder (version 4.2.1-0-g80c4cb6)
// http://www.wxformbuilder.org/
//
// PLEASE DO *NOT* EDIT THIS FILE!
///////////////////////////////////////////////////////////////////////////

#pragma once

#include <wx/artprov.h>
#include <wx/xrc/xmlres.h>
#include <wx/intl.h>
#include <wx/string.h>
#include <wx/srchctrl.h>
#include <wx/gdicmn.h>
#include <wx/font.h>
#include <wx/colour.h>
#include <wx/settings.h>
#include <wx/treectrl.h>
#include <wx/sizer.h>
#include <wx/scrolwin.h>
#include <wx/button.h>
#include <wx/dialog.h>

///////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////
/// Class RacingOptionsBase
///////////////////////////////////////////////////////////////////////////////
class RacingOptionsBase : public wxDialog
{
	private:

	protected:
		wxSearchCtrl* searchOptions;
		wxTreeCtrl* treeOptions;
		wxStdDialogButtonSizer* sizerButtons;
		wxButton* sizerButtonsOK;
		wxButton* sizerButtonsApply;
		wxButton* sizerButtonsCancel;

		// Virtual event handlers, override them in your derived class
		virtual void OnChar( wxKeyEvent& event ) { event.Skip(); }
		virtual void OnInit( wxInitDialogEvent& event ) { event.Skip(); }
		virtual void OnSearch( wxCommandEvent& event ) { event.Skip(); }
		virtual void OnTreeSelected( wxTreeEvent& event ) { event.Skip(); }
		virtual void OnTreeChanging( wxTreeEvent& event ) { event.Skip(); }
		virtual void OnApply( wxCommandEvent& event ) { event.Skip(); }
		virtual void OnCancel( wxCommandEvent& event ) { event.Skip(); }
		virtual void OnOk( wxCommandEvent& event ) { event.Skip(); }


	public:
		wxScrolledWindow* scrollWindow;
		wxFlexGridSizer* sizerFlex;

		RacingOptionsBase( wxWindow* parent, wxWindowID id = wxID_ANY, const wxString& title = _("Options"), const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxSize( 500,400 ), long style = wxCAPTION|wxCLOSE_BOX|wxDEFAULT_DIALOG_STYLE|wxMINIMIZE_BOX|wxSYSTEM_MENU );

		~RacingOptionsBase();

};

