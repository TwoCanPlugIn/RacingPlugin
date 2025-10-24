///////////////////////////////////////////////////////////////////////////
// C++ code generated with wxFormBuilder (version 4.2.1-0-g80c4cb6)
// http://www.wxformbuilder.org/
//
// PLEASE DO *NOT* EDIT THIS FILE!
///////////////////////////////////////////////////////////////////////////

#include "racing_optionsbase.h"

///////////////////////////////////////////////////////////////////////////

RacingOptionsBase::RacingOptionsBase( wxWindow* parent, wxWindowID id, const wxString& title, const wxPoint& pos, const wxSize& size, long style ) : wxDialog( parent, id, title, pos, size, style )
{
	this->SetSizeHints( wxDefaultSize, wxDefaultSize );

	wxBoxSizer* sizerDialog;
	sizerDialog = new wxBoxSizer( wxVERTICAL );

	wxBoxSizer* sizerExplorer;
	sizerExplorer = new wxBoxSizer( wxHORIZONTAL );

	wxBoxSizer* sizerNavigation;
	sizerNavigation = new wxBoxSizer( wxVERTICAL );

	searchOptions = new wxSearchCtrl( this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
	#ifndef __WXMAC__
	searchOptions->ShowSearchButton( true );
	#endif
	searchOptions->ShowCancelButton( false );
	sizerNavigation->Add( searchOptions, 0, wxALL|wxEXPAND, 5 );

	treeOptions = new wxTreeCtrl( this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTR_DEFAULT_STYLE|wxTR_HIDE_ROOT );
	sizerNavigation->Add( treeOptions, 1, wxALL|wxEXPAND, 5 );


	sizerExplorer->Add( sizerNavigation, 1, wxEXPAND, 5 );

	wxBoxSizer* sizerScroll;
	sizerScroll = new wxBoxSizer( wxVERTICAL );

	scrollWindow = new wxScrolledWindow( this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxHSCROLL|wxVSCROLL );
	scrollWindow->SetScrollRate( 5, 5 );
	sizerFlex = new wxFlexGridSizer( 0, 1, 0, 0 );
	sizerFlex->SetFlexibleDirection( wxVERTICAL );
	sizerFlex->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );


	scrollWindow->SetSizer( sizerFlex );
	scrollWindow->Layout();
	sizerFlex->Fit( scrollWindow );
	sizerScroll->Add( scrollWindow, 1, wxEXPAND | wxALL, 5 );


	sizerExplorer->Add( sizerScroll, 2, wxEXPAND, 5 );


	sizerDialog->Add( sizerExplorer, 1, wxEXPAND, 5 );

	sizerButtons = new wxStdDialogButtonSizer();
	sizerButtonsOK = new wxButton( this, wxID_OK );
	sizerButtons->AddButton( sizerButtonsOK );
	sizerButtonsApply = new wxButton( this, wxID_APPLY );
	sizerButtons->AddButton( sizerButtonsApply );
	sizerButtonsCancel = new wxButton( this, wxID_CANCEL );
	sizerButtons->AddButton( sizerButtonsCancel );
	sizerButtons->Realize();

	sizerDialog->Add( sizerButtons, 0, wxALIGN_RIGHT|wxALL, 5 );


	this->SetSizer( sizerDialog );
	this->Layout();

	this->Centre( wxBOTH );

	// Connect Events
	this->Connect( wxEVT_CHAR, wxKeyEventHandler( RacingOptionsBase::OnChar ) );
	this->Connect( wxEVT_INIT_DIALOG, wxInitDialogEventHandler( RacingOptionsBase::OnInit ) );
	searchOptions->Connect( wxEVT_COMMAND_SEARCHCTRL_SEARCH_BTN, wxCommandEventHandler( RacingOptionsBase::OnSearch ), NULL, this );
	treeOptions->Connect( wxEVT_COMMAND_TREE_SEL_CHANGED, wxTreeEventHandler( RacingOptionsBase::OnTreeSelected ), NULL, this );
	treeOptions->Connect( wxEVT_COMMAND_TREE_SEL_CHANGING, wxTreeEventHandler( RacingOptionsBase::OnTreeChanging ), NULL, this );
	sizerButtonsApply->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( RacingOptionsBase::OnApply ), NULL, this );
	sizerButtonsCancel->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( RacingOptionsBase::OnCancel ), NULL, this );
	sizerButtonsOK->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( RacingOptionsBase::OnOk ), NULL, this );
}

RacingOptionsBase::~RacingOptionsBase()
{
	// Disconnect Events
	this->Disconnect( wxEVT_CHAR, wxKeyEventHandler( RacingOptionsBase::OnChar ) );
	this->Disconnect( wxEVT_INIT_DIALOG, wxInitDialogEventHandler( RacingOptionsBase::OnInit ) );
	searchOptions->Disconnect( wxEVT_COMMAND_SEARCHCTRL_SEARCH_BTN, wxCommandEventHandler( RacingOptionsBase::OnSearch ), NULL, this );
	treeOptions->Disconnect( wxEVT_COMMAND_TREE_SEL_CHANGED, wxTreeEventHandler( RacingOptionsBase::OnTreeSelected ), NULL, this );
	treeOptions->Disconnect( wxEVT_COMMAND_TREE_SEL_CHANGING, wxTreeEventHandler( RacingOptionsBase::OnTreeChanging ), NULL, this );
	sizerButtonsApply->Disconnect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( RacingOptionsBase::OnApply ), NULL, this );
	sizerButtonsCancel->Disconnect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( RacingOptionsBase::OnCancel ), NULL, this );
	sizerButtonsOK->Disconnect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( RacingOptionsBase::OnOk ), NULL, this );

}
