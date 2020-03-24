///////////////////////////////////////////////////////////////////////////
// C++ code generated with wxFormBuilder (version Oct 26 2018)
// http://www.wxformbuilder.org/
//
// PLEASE DO *NOT* EDIT THIS FILE!
///////////////////////////////////////////////////////////////////////////

#include "racing_windowbase.h"

///////////////////////////////////////////////////////////////////////////

RacingWindowBase::RacingWindowBase( wxWindow* parent, wxWindowID id, const wxString& title, const wxPoint& pos, const wxSize& size, long style ) : wxFrame( parent, id, title, pos, size, style )
{
	this->SetSizeHints( wxDefaultSize, wxDefaultSize );

	wxBoxSizer* sizerWindow;
	sizerWindow = new wxBoxSizer( wxVERTICAL );

	wxGridSizer* sizerGrid;
	sizerGrid = new wxGridSizer( 5, 2, 0, 0 );

	labelSpeed = new wxStaticText( this, wxID_ANY, wxT("Speed"), wxDefaultPosition, wxDefaultSize, 0 );
	labelSpeed->Wrap( -1 );
	sizerGrid->Add( labelSpeed, 0, wxALL, 5 );

	labelTimer = new wxStaticText( this, wxID_ANY, wxT("Timer"), wxDefaultPosition, wxDefaultSize, 0 );
	labelTimer->Wrap( -1 );
	sizerGrid->Add( labelTimer, 0, wxALL, 5 );

	labelTTG = new wxStaticText( this, wxID_ANY, wxT("TTG"), wxDefaultPosition, wxDefaultSize, 0 );
	labelTTG->Wrap( -1 );
	sizerGrid->Add( labelTTG, 0, wxALL, 5 );

	labelDistance = new wxStaticText( this, wxID_ANY, wxT("Distance"), wxDefaultPosition, wxDefaultSize, 0 );
	labelDistance->Wrap( -1 );
	sizerGrid->Add( labelDistance, 0, wxALL, 5 );

	buttonTimer = new wxButton( this, wxID_ANY, wxT("Start"), wxDefaultPosition, wxDefaultSize, 0 );
	sizerGrid->Add( buttonTimer, 0, wxALL, 5 );

	buttonreset = new wxButton( this, wxID_ANY, wxT("Reset"), wxDefaultPosition, wxDefaultSize, 0 );
	sizerGrid->Add( buttonreset, 0, wxALL, 5 );

	buttonPort = new wxButton( this, wxID_ANY, wxT("Port"), wxDefaultPosition, wxDefaultSize, 0 );
	sizerGrid->Add( buttonPort, 0, wxALL, 5 );

	buttonStarboard = new wxButton( this, wxID_ANY, wxT("Starboard"), wxDefaultPosition, wxDefaultSize, 0 );
	sizerGrid->Add( buttonStarboard, 0, wxALL, 5 );

	buttonCancel = new wxButton( this, wxID_ANY, wxT("Cancel"), wxDefaultPosition, wxDefaultSize, 0 );
	sizerGrid->Add( buttonCancel, 0, wxALL, 5 );


	sizerWindow->Add( sizerGrid, 1, wxEXPAND, 5 );


	this->SetSizer( sizerWindow );
	this->Layout();

	this->Centre( wxBOTH );

	// Connect Events
	this->Connect( wxEVT_CLOSE_WINDOW, wxCloseEventHandler( RacingWindowBase::OnClose ) );
	buttonTimer->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( RacingWindowBase::OnStart ), NULL, this );
	buttonreset->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( RacingWindowBase::OnReset ), NULL, this );
	buttonPort->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( RacingWindowBase::OnPort ), NULL, this );
	buttonStarboard->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( RacingWindowBase::OnStarboard ), NULL, this );
	buttonCancel->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( RacingWindowBase::OnCancel ), NULL, this );
}

RacingWindowBase::~RacingWindowBase()
{
	// Disconnect Events
	this->Disconnect( wxEVT_CLOSE_WINDOW, wxCloseEventHandler( RacingWindowBase::OnCloseWindow ) );
	buttonTimer->Disconnect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( RacingWindowBase::OnStart ), NULL, this );
	buttonreset->Disconnect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( RacingWindowBase::OnReset ), NULL, this );
	buttonPort->Disconnect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( RacingWindowBase::OnPort ), NULL, this );
	buttonStarboard->Disconnect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( RacingWindowBase::OnStarboard ), NULL, this );
	buttonCancel->Disconnect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( RacingWindowBase::OnCancel ), NULL, this );

}
