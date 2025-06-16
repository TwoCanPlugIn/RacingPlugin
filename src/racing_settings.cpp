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
	checkLayline->SetValue(showStartline);
	checkStartLine->SetValue(showStartline);
	checkMultiCanvas->SetValue(showMultiCanvas);
	checkWindAngle->SetValue(showWindAngles);
	checkNMEA2000->SetValue(generatePGN130306);
	checkNMEA0183->SetValue(generateMWVSentence);
	spinCountdown->SetValue(defaultTimerValue / 60);
	spinTackingAngle->SetValue(tackingAngle);

	// Save the original settings in case user changes values, hits apply, but then cancels...
	originalGeneratePGN130306 = generatePGN130306;
	originalGenerateMWVSentence = generateMWVSentence;
	originalDefaultTimerValue = defaultTimerValue;
	originalShowLayLines = showLayLines;
	originalShowMultiCanvas = showMultiCanvas;
	originalShowStartline = showStartline;
	originalShowWindAngles = showWindAngles;
	originalTackingAngle = tackingAngle;

}

void RacingSettings::OnOK(wxCommandEvent& event) {

	defaultTimerValue = spinCountdown->GetValue() * 60; // In seconds
	tackingAngle = spinTackingAngle->GetValue();
	generatePGN130306 = checkNMEA2000->IsChecked();
	generateMWVSentence = checkNMEA0183->IsChecked();
	showLayLines = checkLayline->IsChecked();
	showMultiCanvas = checkMultiCanvas->IsChecked();
	showStartline = checkStartLine->IsChecked();
	showWindAngles = checkWindAngle->IsChecked();

	EndModal(wxID_OK);
}

void RacingSettings::OnCancel(wxCommandEvent& event) {
	// Restore the original settings
	generatePGN130306 = originalGeneratePGN130306;
	generateMWVSentence = originalGenerateMWVSentence;
	defaultTimerValue = originalDefaultTimerValue;
	tackingAngle = originalTackingAngle;
	showLayLines = originalShowLayLines;
	showMultiCanvas = originalShowMultiCanvas;
	showStartline = originalShowStartline;
	showWindAngles = originalShowWindAngles;

	EndModal(wxID_CANCEL);
}

void RacingSettings::OnApply(wxCommandEvent& event) {

	defaultTimerValue = spinCountdown->GetValue() * 60;
	tackingAngle = spinTackingAngle->GetValue();
	generatePGN130306 = checkNMEA2000->IsChecked();
	generateMWVSentence = checkNMEA0183->IsChecked();
	showLayLines = checkLayline->IsChecked();
	showMultiCanvas = checkMultiCanvas->IsChecked();
	showStartline = checkStartLine->IsChecked();
	showWindAngles = checkWindAngle->IsChecked();

}