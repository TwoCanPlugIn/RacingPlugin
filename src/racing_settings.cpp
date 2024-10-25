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

#include "racing_settings.h"

// Constructor and destructor implementation
RacingSettings::RacingSettings(wxWindow* parent, wxWindowID id, const wxString& title, const wxPoint& pos, const wxSize& size, long style) : RacingSettingsBase(parent, id, title, pos, size, style) {
}

RacingSettings::~RacingSettings() {
	// Nothing to do in the destructor
}

void RacingSettings::OnInit(wxInitDialogEvent& event) {
	checkNMEA2000->SetValue(generatePGN130306);
	checkNMEA0183->SetValue(generateMWVSentence);
	// Save the original settings so that if the user has changed the values, but then cancels...
	originalGeneratePGN130306 = generatePGN130306;
	originalGenerateMWVSentence = generateMWVSentence;
}

void RacingSettings::OnOK(wxCommandEvent& event) {
	generatePGN130306 = checkNMEA2000->IsChecked();
	generateMWVSentence = checkNMEA0183->IsChecked();
	EndModal(wxID_OK);
}

void RacingSettings::OnCancel(wxCommandEvent& event) {
	generatePGN130306 = originalGeneratePGN130306;
	generateMWVSentence = originalGenerateMWVSentence;
	EndModal(wxID_CANCEL);
}

void RacingSettings::OnApply(wxCommandEvent& event) {
	generatePGN130306 = checkNMEA2000->IsChecked();
	generateMWVSentence = checkNMEA0183->IsChecked();
}