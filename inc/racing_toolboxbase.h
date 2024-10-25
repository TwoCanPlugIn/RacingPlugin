///////////////////////////////////////////////////////////////////////////
// C++ code generated with wxFormBuilder (version 4.2.1-0-g80c4cb6)
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
#include <wx/spinctrl.h>
#include <wx/checkbox.h>
#include <wx/sizer.h>
#include <wx/panel.h>

///////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////
/// Class RacingToolboxBase
///////////////////////////////////////////////////////////////////////////////
class RacingToolboxBase : public wxPanel
{
	private:

	protected:
		wxStaticText* labelCountdownTimer;
		wxSpinCtrl* spinCountdownTimer;
		wxStaticText* labelTackingAngle;
		wxSpinCtrl* spinTackingAngle;
		wxCheckBox* chkWindAngle;
		wxCheckBox* chkStartLine;
		wxCheckBox* chkLayLines;
		wxCheckBox* chkMultiCanvas;

		// Virtual event handlers, override them in your derived class
		virtual void OnCountdownTimerChanged( wxSpinEvent& event ) { event.Skip(); }
		virtual void OnTackingAngleChanged( wxSpinEvent& event ) { event.Skip(); }
		virtual void OnWindAngleChanged( wxCommandEvent& event ) { event.Skip(); }
		virtual void OnStartLineChanged( wxCommandEvent& event ) { event.Skip(); }
		virtual void OnLayLinesChanged( wxCommandEvent& event ) { event.Skip(); }
		virtual void OnCanvasChanged(wxCommandEvent& event) { event.Skip(); }

	public:

		RacingToolboxBase( wxWindow* parent, wxWindowID id = wxID_ANY, const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxSize( 300,300 ), long style = wxTAB_TRAVERSAL, const wxString& name = wxEmptyString );

		~RacingToolboxBase();

};

