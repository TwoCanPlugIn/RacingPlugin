///////////////////////////////////////////////////////////////////////////
// C++ code generated with wxFormBuilder (version 4.2.1-0-g80c4cb6)
// http://www.wxformbuilder.org/
//
// PLEASE DO *NOT* EDIT THIS FILE!
///////////////////////////////////////////////////////////////////////////

#include "racing_settingsbase.h"

///////////////////////////////////////////////////////////////////////////

RacingSettingsBase::RacingSettingsBase( wxWindow* parent, wxWindowID id, const wxString& title, const wxPoint& pos, const wxSize& size, long style ) : wxDialog( parent, id, title, pos, size, style )
{
	this->SetSizeHints( wxDefaultSize, wxDefaultSize );

	wxBoxSizer* sizerDialog;
	sizerDialog = new wxBoxSizer( wxVERTICAL );

	wxStaticBoxSizer* sizerSettings;
	sizerSettings = new wxStaticBoxSizer( new wxStaticBox( this, wxID_ANY, _("True Wind") ), wxVERTICAL );

	checkNMEA2000 = new wxCheckBox( sizerSettings->GetStaticBox(), wxID_ANY, _("Generate NMEA 2000 True Wind message"), wxDefaultPosition, wxDefaultSize, 0 );
	sizerSettings->Add( checkNMEA2000, 0, wxALL, 5 );

	checkNMEA0183 = new wxCheckBox( sizerSettings->GetStaticBox(), wxID_ANY, _("Generate NMEA 0183 True Wind sentence"), wxDefaultPosition, wxDefaultSize, 0 );
	sizerSettings->Add( checkNMEA0183, 0, wxALL, 5 );


	sizerDialog->Add( sizerSettings, 1, wxEXPAND, 5 );

	wxBoxSizer* sizerButtons;
	sizerButtons = new wxBoxSizer( wxHORIZONTAL );

	buttonOK = new wxButton( this, wxID_ANY, _("OK"), wxDefaultPosition, wxDefaultSize, 0 );
	sizerButtons->Add( buttonOK, 0, wxALL, 5 );

	buttonApply = new wxButton( this, wxID_ANY, _("Apply"), wxDefaultPosition, wxDefaultSize, 0 );
	sizerButtons->Add( buttonApply, 0, wxALL, 5 );

	buttonCancel = new wxButton( this, wxID_ANY, _("Cancel"), wxDefaultPosition, wxDefaultSize, 0 );
	sizerButtons->Add( buttonCancel, 0, wxALL, 5 );


	sizerDialog->Add( sizerButtons, 0, wxEXPAND, 5 );


	this->SetSizer( sizerDialog );
	this->Layout();
	sizerDialog->Fit( this );

	this->Centre( wxBOTH );

	// Connect Events
	this->Connect( wxEVT_INIT_DIALOG, wxInitDialogEventHandler( RacingSettingsBase::OnInit ) );
	buttonOK->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( RacingSettingsBase::OnOK ), NULL, this );
	buttonApply->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( RacingSettingsBase::OnApply ), NULL, this );
	buttonCancel->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( RacingSettingsBase::OnCancel ), NULL, this );
}

RacingSettingsBase::~RacingSettingsBase()
{
}
