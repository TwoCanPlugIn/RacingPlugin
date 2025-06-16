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

	wxStaticBoxSizer* sizerGeneral;
	sizerGeneral = new wxStaticBoxSizer( new wxStaticBox( this, wxID_ANY, _("General Options") ), wxVERTICAL );

	wxGridSizer* gridSpinControls;
	gridSpinControls = new wxGridSizer( 2, 2, 0, 0 );

	labelTimer = new wxStaticText( sizerGeneral->GetStaticBox(), wxID_ANY, _("Countdown Timer (minutes)"), wxDefaultPosition, wxDefaultSize, 0 );
	labelTimer->Wrap( -1 );
	gridSpinControls->Add( labelTimer, 0, wxALL, 5 );

	spinCountdown = new wxSpinCtrl( sizerGeneral->GetStaticBox(), wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS, 0, 10, 0 );
	gridSpinControls->Add( spinCountdown, 0, wxALL, 5 );

	labelTacking = new wxStaticText( sizerGeneral->GetStaticBox(), wxID_ANY, _("Tacking Angle (degrees)"), wxDefaultPosition, wxDefaultSize, 0 );
	labelTacking->Wrap( -1 );
	gridSpinControls->Add( labelTacking, 0, wxALL, 5 );

	spinTackingAngle = new wxSpinCtrl( sizerGeneral->GetStaticBox(), wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS, 0, 90, 0 );
	gridSpinControls->Add( spinTackingAngle, 0, wxALL, 5 );


	sizerGeneral->Add( gridSpinControls, 1, wxEXPAND, 5 );

	checkWindAngle = new wxCheckBox( sizerGeneral->GetStaticBox(), wxID_ANY, _("Show Wind Direction"), wxDefaultPosition, wxDefaultSize, 0 );
	sizerGeneral->Add( checkWindAngle, 0, wxALL, 5 );

	checkStartLine = new wxCheckBox( sizerGeneral->GetStaticBox(), wxID_ANY, _("Show Startline"), wxDefaultPosition, wxDefaultSize, 0 );
	sizerGeneral->Add( checkStartLine, 0, wxALL, 5 );

	checkLayline = new wxCheckBox( sizerGeneral->GetStaticBox(), wxID_ANY, _("Show Laylines"), wxDefaultPosition, wxDefaultSize, 0 );
	sizerGeneral->Add( checkLayline, 0, wxALL, 5 );

	checkMultiCanvas = new wxCheckBox( sizerGeneral->GetStaticBox(), wxID_ANY, _("Multi Canvas"), wxDefaultPosition, wxDefaultSize, 0 );
	sizerGeneral->Add( checkMultiCanvas, 0, wxALL, 5 );


	sizerDialog->Add( sizerGeneral, 1, wxEXPAND, 5 );

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
	spinCountdown->Connect( wxEVT_COMMAND_SPINCTRL_UPDATED, wxSpinEventHandler( RacingSettingsBase::OnCountdownTimerUpdated ), NULL, this );
	spinTackingAngle->Connect( wxEVT_COMMAND_SPINCTRL_UPDATED, wxSpinEventHandler( RacingSettingsBase::OnTackingAngleUpdated ), NULL, this );
	checkWindAngle->Connect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( RacingSettingsBase::OnWindAngleChanged ), NULL, this );
	checkStartLine->Connect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( RacingSettingsBase::OnStartlineChanged ), NULL, this );
	checkLayline->Connect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( RacingSettingsBase::OnLaylineChanged ), NULL, this );
	checkMultiCanvas->Connect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( RacingSettingsBase::OnCanvasChanged ), NULL, this );
	checkNMEA2000->Connect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( RacingSettingsBase::OnNMEA2000Changed ), NULL, this );
	checkNMEA0183->Connect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( RacingSettingsBase::OnNMEA0183Changed ), NULL, this );
	buttonOK->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( RacingSettingsBase::OnOK ), NULL, this );
	buttonApply->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( RacingSettingsBase::OnApply ), NULL, this );
	buttonCancel->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( RacingSettingsBase::OnCancel ), NULL, this );
}

RacingSettingsBase::~RacingSettingsBase()
{
}
