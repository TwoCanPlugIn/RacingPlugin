// Copyright(C) 2018-2020 by Steven Adler
//
// This file is part of Racing plugin for OpenCPN.
//
// Racing plugin for OpenCPN is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// Racing plugin for OpenCPN is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with the Racing plugin for OpenCPN. If not, see <https://www.gnu.org/licenses/>.
//


// Project: Racing Plugin
// Description: Race Start Display plugin for OpenCPN
// Owner: twocanplugin@hotmail.com
// Date: 6/1/2020
// Version History: 
// 1.0 Initial Release
//

#include "racing_options.h"

#include "racing_globals.h"

#include <map>
#include <wx/wxhtml.h>

// Constructor and destructor implementation
RacingOptions::RacingOptions(wxWindow* parent, wxWindowID id, const wxString& title, const wxSize& size) 
	: RacingOptionsBase(parent, id, title, wxDefaultPosition, size, wxCAPTION | wxCLOSE_BOX | wxDEFAULT_DIALOG_STYLE | wxMINIMIZE_BOX | wxSYSTEM_MENU) {
}

RacingOptions::~RacingOptions() {
	// Nothing to do in the destructor
}

void RacingOptions::OnInit(wxInitDialogEvent& event) {
	
	// Save the original settings in case user changes values, hits apply, but then cancels...
	originalGeneratePGN130306 = generatePGN130306;
	originalGenerateMWVSentence = generateMWVSentence;
	originalDefaultTimerValue = defaultTimerValue;
	originalShowLayLines = showLayLines;
	originalShowMultiCanvas = showMultiCanvas;
	originalShowStartline = showStartline;
	originalShowWindAngles = showWindAngles;
	originalTackingAngle = tackingAngle;

	root = treeOptions->AddRoot("Dummy");
	basic = treeOptions->AppendItem(root, "Basic Options");
	network = treeOptions->AppendItem(basic, "Network Options");
	misc = treeOptions->AppendItem(root, "Misc Options");

	// Set the selected item
	treeOptions->SelectItem(misc.GetID(), true);
	treeOptions->SetFocusedItem(misc.GetID());

	// So if the user hits the enter key, it will default be the OK button
	sizerButtonsOK->SetFocus();

}

void RacingOptions::OnOK(wxCommandEvent& event) {
	delete settingsTemplate;
	EndModal(wxID_OK);

}

void RacingOptions::OnCancel(wxCommandEvent& event) {
	// Restore the original settings
	generatePGN130306 = originalGeneratePGN130306;
	generateMWVSentence = originalGenerateMWVSentence;
	defaultTimerValue = originalDefaultTimerValue;
	tackingAngle = originalTackingAngle;
	showLayLines = originalShowLayLines;
	showMultiCanvas = originalShowMultiCanvas;
	showStartline = originalShowStartline;
	showWindAngles = originalShowWindAngles;

	delete settingsTemplate;
	EndModal(wxID_CANCEL);
}

void RacingOptions::OnApply(wxCommandEvent& event) {

}

void RacingOptions::OnChar(wxKeyEvent& event) { 
	wxMessageBox(wxString::Format("Key Code: %d", event.GetKeyCode()));
}

void RacingOptions::OnSearch(wxCommandEvent& event) {
	std::map<std::string, wxTreeItemId> m;

	// Need to associate search terms with the different settings panels 
	m["network"] = network.GetID();
	m["nmea0183"] = network.GetID();
	m["nmea2000"] = network.GetID();

	// Find the serch term and load the appropriate panel of settings
	std::map<std::string, wxTreeItemId>::iterator it = m.find(event.GetString().ToStdString());
	if (it != m.end()) {
		treeOptions->SelectItem(it->second, true);
		treeOptions->SetFocusedItem(it->second);
		wxTreeEvent* treeEvent = new wxTreeEvent();
		treeEvent->SetItem(it->second);
		sizerFlex->Clear(true);
		OnTreeSelected(*treeEvent);
	}
	else {
		// Search term was not found

		wxDialog* htmlDialog;
		htmlDialog = new wxDialog(this, wxID_ANY, "Help", wxDefaultPosition, wxSize(300, 300), wxDEFAULT_DIALOG_STYLE);
		wxHtmlWindow* htmlMessage;
		htmlMessage = new wxHtmlWindow(htmlDialog, wxID_ANY, wxDefaultPosition, wxSize(300,300), wxHW_SCROLLBAR_AUTO);
		
		// Pretty this up...
		wxString text = wxString::Format("Search Item %s not found", event.GetString());
		text.Prepend("<html><h1>");
		text.Append("</h1><body>Valid Search Items are:<p>Network<br>NMEA0183<br>NMEA2000<br></body></html>");
		htmlMessage->SetPage(text);
		htmlDialog->ShowModal();
	}
}

void RacingOptions::OnTreeChanging(wxTreeEvent& event) {
	sizerFlex->Clear(true);
}

void RacingOptions::OnTreeSelected(wxTreeEvent& event) {
	wxTreeItemId selectedItem = event.GetItem();

	if (selectedItem.GetID() == misc.GetID()) {
		
		settingsTemplate = new MiscSettings(scrollWindow, sizerFlex, treeOptions->GetItemText(selectedItem));
		
	}
	else if (selectedItem.GetID() == network.GetID()) {
		
		settingsTemplate = new NetworkSettings(scrollWindow, sizerFlex, treeOptions->GetItemText(selectedItem));

	}
	else if (selectedItem.GetID() == basic.GetID()) {
		
		settingsTemplate = new BasicSettings(scrollWindow, sizerFlex, treeOptions->GetItemText(selectedItem));

	}

	Layout();
}
