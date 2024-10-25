///////////////////////////////////////////////////////////////////////////
// C++ code generated with wxFormBuilder (version 4.2.1-0-g80c4cb6)
// http://www.wxformbuilder.org/
//
// PLEASE DO *NOT* EDIT THIS FILE!
///////////////////////////////////////////////////////////////////////////

#include "racing_toolboxbase.h"

///////////////////////////////////////////////////////////////////////////

RacingToolboxBase::RacingToolboxBase( wxWindow* parent, wxWindowID id, const wxPoint& pos, const wxSize& size, long style, const wxString& name ) : wxPanel( parent, id, pos, size, style, name )
{
	wxBoxSizer* bSizer1;
	bSizer1 = new wxBoxSizer( wxVERTICAL );

	labelCountdownTimer = new wxStaticText( this, wxID_ANY, wxT("Countdown Time (minutes)"), wxDefaultPosition, wxDefaultSize, 0 );
	labelCountdownTimer->Wrap( -1 );
	bSizer1->Add( labelCountdownTimer, 0, wxALL, 5 );

	spinCountdownTimer = new wxSpinCtrl( this, wxID_ANY, wxT("5"), wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS, 1, 20, 0 );
	bSizer1->Add( spinCountdownTimer, 0, wxALL, 5 );

	labelTackingAngle = new wxStaticText( this, wxID_ANY, wxT("Tacking Angle (degrees)"), wxDefaultPosition, wxDefaultSize, 0 );
	labelTackingAngle->Wrap( -1 );
	bSizer1->Add( labelTackingAngle, 0, wxALL, 5 );

	spinTackingAngle = new wxSpinCtrl( this, wxID_ANY, wxT("90"), wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS, 60, 180, 0 );
	bSizer1->Add( spinTackingAngle, 0, wxALL, 5 );

	chkWindAngle = new wxCheckBox( this, wxID_ANY, wxT("Show Wind Direction"), wxDefaultPosition, wxDefaultSize, 0 );
	bSizer1->Add( chkWindAngle, 0, wxALL, 5 );

	chkStartLine = new wxCheckBox( this, wxID_ANY, wxT("Show Start Line"), wxDefaultPosition, wxDefaultSize, 0 );
	bSizer1->Add( chkStartLine, 0, wxALL, 5 );

	chkLayLines = new wxCheckBox( this, wxID_ANY, wxT("Show Lay Lines"), wxDefaultPosition, wxDefaultSize, 0 );
	bSizer1->Add( chkLayLines, 0, wxALL, 5 );

	chkMultiCanvas = new wxCheckBox(this, wxID_ANY, wxT("Multi Canvas"), wxDefaultPosition, wxDefaultSize, 0);
	bSizer1->Add(chkMultiCanvas, 0, wxALL, 5);


	this->SetSizer( bSizer1 );
	this->Layout();

	// Connect Events
	spinCountdownTimer->Connect( wxEVT_COMMAND_SPINCTRL_UPDATED, wxSpinEventHandler( RacingToolboxBase::OnCountdownTimerChanged ), NULL, this );
	spinTackingAngle->Connect( wxEVT_COMMAND_SPINCTRL_UPDATED, wxSpinEventHandler( RacingToolboxBase::OnTackingAngleChanged ), NULL, this );
	chkWindAngle->Connect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( RacingToolboxBase::OnWindAngleChanged ), NULL, this );
	chkStartLine->Connect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( RacingToolboxBase::OnStartLineChanged ), NULL, this );
	chkLayLines->Connect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( RacingToolboxBase::OnLayLinesChanged ), NULL, this );
	chkMultiCanvas->Connect(wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler(RacingToolboxBase::OnCanvasChanged), NULL, this);
}

RacingToolboxBase::~RacingToolboxBase()
{
	// Disconnect Events
	spinCountdownTimer->Disconnect( wxEVT_COMMAND_SPINCTRL_UPDATED, wxSpinEventHandler( RacingToolboxBase::OnCountdownTimerChanged ), NULL, this );
	spinTackingAngle->Disconnect( wxEVT_COMMAND_SPINCTRL_UPDATED, wxSpinEventHandler( RacingToolboxBase::OnTackingAngleChanged ), NULL, this );
	chkWindAngle->Disconnect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( RacingToolboxBase::OnWindAngleChanged ), NULL, this );
	chkStartLine->Disconnect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( RacingToolboxBase::OnStartLineChanged ), NULL, this );
	chkLayLines->Disconnect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( RacingToolboxBase::OnLayLinesChanged ), NULL, this );
	chkMultiCanvas->Disconnect(wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler(RacingToolboxBase::OnCanvasChanged), NULL, this);
}
