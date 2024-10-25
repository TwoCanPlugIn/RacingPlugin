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

#include "racing_toolbox.h"

// Constructor and destructor implementation
RacingToolbox::RacingToolbox( wxWindow* parent) : RacingToolboxBase(parent) {
	spinCountdownTimer->SetValue(defaultTimerValue / 60);
	spinTackingAngle->SetValue(tackingAngle);
	chkWindAngle->SetValue(showWindAngles);
	chkLayLines->SetValue(showLayLines);
	chkStartLine->SetValue(showStartline);
	chkMultiCanvas->SetValue(showMultiCanvas);
	// Not used here, Would normally only save the settings if they have been changed
	settingsDirty = false;
}

RacingToolbox::~RacingToolbox() {
	// Nothing to do in the destructor
}

void RacingToolbox::OnCountdownTimerChanged(wxSpinEvent& event) {
	defaultTimerValue = spinCountdownTimer->GetValue() * 60;
	settingsDirty = true;
}

void RacingToolbox::OnTackingAngleChanged(wxSpinEvent& event) {
	tackingAngle = spinTackingAngle->GetValue();
	settingsDirty = true;
}

void RacingToolbox::OnWindAngleChanged(wxCommandEvent& event) {
	showWindAngles = chkWindAngle->IsChecked();
	settingsDirty = true;
}

void RacingToolbox::OnStartLineChanged(wxCommandEvent& event) {
	showStartline = chkStartLine->IsChecked();
	settingsDirty = true;
}

void RacingToolbox::OnLayLinesChanged(wxCommandEvent& event) {
	showLayLines = chkLayLines->IsChecked();
	settingsDirty = true;
}

void RacingToolbox::OnCanvasChanged(wxCommandEvent& event) {
	showMultiCanvas = chkMultiCanvas->IsChecked();
	settingsDirty = true;
}