
// Copyright(C) 2018-2025 by Steven Adler
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

//
// Project: Racing Plugin
// Description: New layout for settings, Similar to Visual Studio
// Owner: twocanplugin@hotmail.com
// Date: 10/10/2025
#include "racing_optionpages.h"

SettingsTemplate::SettingsTemplate(wxScrolledWindow* parent, wxFlexGridSizer* sizer, wxString name)  {
	
	parentWindow = parent;
	
	panelSizer = sizer;

	sizer->SetVGap(10);
}

SettingsTemplate::~SettingsTemplate() {
	
}

MiscSettings::MiscSettings(wxScrolledWindow* parent, wxFlexGridSizer* sizer, wxString name) : SettingsTemplate(parent, sizer, name) {

	checkWindAngle = new wxCheckBox(parent, wxID_ANY, _("Show Wind Direction"), wxDefaultPosition, wxDefaultSize, 0);
	sizer->Add(checkWindAngle, 0, wxALL, 5);

	checkStartLine = new wxCheckBox(parent, wxID_ANY, _("Show Startline"), wxDefaultPosition, wxDefaultSize, 0);
	sizer->Add(checkStartLine, 0, wxALL, 5);

	checkLayline = new wxCheckBox(parent, wxID_ANY, _("Show Laylines"), wxDefaultPosition, wxDefaultSize, 0);
	sizer->Add(checkLayline, 0, wxALL, 5);

	checkMultiCanvas = new wxCheckBox(parent, wxID_ANY, _("Multi Canvas"), wxDefaultPosition, wxDefaultSize, 0);
	sizer->Add(checkMultiCanvas, 0, wxALL, 5);

	check1 = new wxCheckBox(parent, wxID_ANY, _("One"), wxDefaultPosition, wxDefaultSize, 0);
	sizer->Add(check1, 0, wxALL, 5);

	check2 = new wxCheckBox(parent, wxID_ANY, _("Two"), wxDefaultPosition, wxDefaultSize, 0);
	sizer->Add(check2, 0, wxALL, 5);

	check3 = new wxCheckBox(parent, wxID_ANY, _("Three"), wxDefaultPosition, wxDefaultSize, 0);
	sizer->Add(check3, 0, wxALL, 5);

	check4 = new wxCheckBox(parent, wxID_ANY, _("Four"), wxDefaultPosition, wxDefaultSize, 0);
	sizer->Add(check4, 0, wxALL, 5);

	check5 = new wxCheckBox(parent, wxID_ANY, _("Five"), wxDefaultPosition, wxDefaultSize, 0);
	sizer->Add(check5, 0, wxALL, 5);

	check6 = new wxCheckBox(parent, wxID_ANY, _("Six"), wxDefaultPosition, wxDefaultSize, 0);
	sizer->Add(check6, 0, wxALL, 5);

	checkWindAngle->Connect(wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler(MiscSettings::OnWindAngleChanged), NULL, this);
	checkStartLine->Connect(wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler(MiscSettings::OnStartlineChanged), NULL, this);
	checkLayline->Connect(wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler(MiscSettings::OnLaylineChanged), NULL, this);
	checkMultiCanvas->Connect(wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler(MiscSettings::OnCanvasChanged), NULL, this);

	checkWindAngle->SetValue(showWindAngles);
	checkStartLine->SetValue(showStartline);
	checkLayline->SetValue(showLayLines);
	checkMultiCanvas->SetValue(showMultiCanvas);
}

MiscSettings::~MiscSettings() {
	checkWindAngle->Disconnect(wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler(MiscSettings::OnWindAngleChanged), NULL, this);
	checkStartLine->Disconnect(wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler(MiscSettings::OnStartlineChanged), NULL, this);
	checkLayline->Disconnect(wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler(MiscSettings::OnLaylineChanged), NULL, this);
	checkMultiCanvas->Disconnect(wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler(MiscSettings::OnCanvasChanged), NULL, this);

}

void MiscSettings::OnWindAngleChanged(wxCommandEvent& event) {
	showWindAngles = checkWindAngle->GetValue();
}

void MiscSettings::OnStartlineChanged(wxCommandEvent& event) {
	showStartline = checkStartLine->GetValue();
}

void MiscSettings::OnLaylineChanged(wxCommandEvent& event) {
	showLayLines = checkLayline->GetValue();
}

void MiscSettings::OnCanvasChanged(wxCommandEvent& event) {
	showMultiCanvas = checkMultiCanvas->GetValue();
}



BasicSettings::BasicSettings(wxScrolledWindow* parent, wxFlexGridSizer* sizer, wxString name) : SettingsTemplate(parent, sizer, name) {

	labelCountdownTimer = new wxStaticText(parent, wxID_ANY, _("Countdown Timer (minutes)"), wxDefaultPosition, wxDefaultSize, 0);
	labelCountdownTimer->Wrap(-1);
	sizer->Add(labelCountdownTimer, 0, wxEXPAND, 0);

	spinCountdownTimer = new wxSpinCtrl(parent, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS, 0, 10, 0);
	sizer->Add(spinCountdownTimer, 0, wxEXPAND, 0);

	labelTackingAngle = new wxStaticText(parent, wxID_ANY, _("Tacking Angle (degrees)"), wxDefaultPosition, wxDefaultSize, 0);
	labelTackingAngle->Wrap(-1);
	sizer->Add(labelTackingAngle, 0, wxEXPAND, 0);

	spinTackingAngle = new wxSpinCtrl(parent, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS, 0, 90, 0);
	sizer->Add(spinTackingAngle, 0, wxEXPAND, 0);

	spinCountdownTimer->Connect(wxEVT_COMMAND_SPINCTRL_UPDATED, wxSpinEventHandler(BasicSettings::OnCountdownTimerUpdated), NULL, this);
	spinTackingAngle->Connect(wxEVT_COMMAND_SPINCTRL_UPDATED, wxSpinEventHandler(BasicSettings::OnTackingAngleUpdated), NULL, this);

	spinCountdownTimer->SetValue(defaultTimerValue / 60);
	spinTackingAngle->SetValue(tackingAngle);
}

void BasicSettings::OnCountdownTimerUpdated(wxSpinEvent& event) {
	defaultTimerValue = spinCountdownTimer->GetValue() * 60;
}

void BasicSettings::OnTackingAngleUpdated(wxSpinEvent& event) {
	tackingAngle = spinTackingAngle->GetValue();
}

BasicSettings::~BasicSettings() {
	spinCountdownTimer->Disconnect(wxEVT_COMMAND_SPINCTRL_UPDATED, wxSpinEventHandler(BasicSettings::OnCountdownTimerUpdated), NULL, this);
	spinTackingAngle->Disconnect(wxEVT_COMMAND_SPINCTRL_UPDATED, wxSpinEventHandler(BasicSettings::OnTackingAngleUpdated), NULL, this);

}

NetworkSettings::NetworkSettings(wxScrolledWindow* parent, wxFlexGridSizer* sizer, wxString name) : SettingsTemplate(parent, sizer, name) {

	checkNMEA2000 = new wxCheckBox(parent, wxID_ANY, _("Generate NMEA 2000 True Wind message"), wxDefaultPosition, wxDefaultSize, 0);
	sizer->Add(checkNMEA2000, 0, wxALL, 5);

	checkNMEA0183 = new wxCheckBox(parent, wxID_ANY, _("Generate NMEA 0183 True Wind sentence"), wxDefaultPosition, wxDefaultSize, 0);
	sizer->Add(checkNMEA0183, 0, wxALL, 5);

	checkNMEA2000->Connect(wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler(NetworkSettings::OnNMEA2000Changed), NULL, this);
	checkNMEA0183->Connect(wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler(NetworkSettings::OnNMEA0183Changed), NULL, this);

	checkNMEA0183->SetValue(generateMWVSentence);
	checkNMEA2000->SetValue(generatePGN130306);

}

NetworkSettings::~NetworkSettings() {

	checkNMEA2000->Disconnect(wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler(NetworkSettings::OnNMEA2000Changed), NULL, this);
	checkNMEA0183->Disconnect(wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler(NetworkSettings::OnNMEA0183Changed), NULL, this);

}

void NetworkSettings::OnNMEA2000Changed(wxCommandEvent& event) {
	generatePGN130306 = checkNMEA2000->GetValue();
}

void NetworkSettings::OnNMEA0183Changed(wxCommandEvent& event) {
	generateMWVSentence = checkNMEA0183->GetValue();
}

