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

//
// Project: Racing Plugin
// Description: Race Start display for OpenCPN
// Owner: twocanplugin@hotmail.com
// Date: 6/1/2020
// Version History: 
// 1.0 Initial Release
// 1.01 - 9/7/2020, Support for OpenCPN Plugin Manager/CI/Cloudsmith stuff
//

#include "racing_plugin.h"

// The class factories, used to create and destroy instances of the PlugIn
extern "C" DECL_EXP opencpn_plugin* create_pi(void *ppimgr) {
	return new RacingPlugin(ppimgr);
}

extern "C" DECL_EXP void destroy_pi(opencpn_plugin* p) {
	delete p;
}

// Constructor
RacingPlugin::RacingPlugin(void *ppimgr) : opencpn_plugin_116(ppimgr), wxEvtHandler() {
	// Initialize the plugin bitmap
	wxString pluginFolder = GetPluginDataDir(PLUGIN_PACKAGE_NAME) + wxFileName::GetPathSeparator() + _T("data") + wxFileName::GetPathSeparator();

	pluginBitmap = GetBitmapFromSVGFile(pluginFolder + _T("racing_icon.svg"), 32, 32);
	
	// Initialize Advanced User Interface Manager (AUI)
	auiManager = GetFrameAuiManager();
}

// Destructor
RacingPlugin::~RacingPlugin(void) {
}

int RacingPlugin::Init(void) {
	// Maintain a reference to the OpenCPN window to use as the parent for the Race Start Window
	parentWindow = GetOCPNCanvasWindow();

	// Maintain a reference to the OpenCPN configuration object 
	configSettings = GetOCPNConfigObject();

	// Load Configuration Settings
	if (configSettings) {
		configSettings->SetPath(_T("/PlugIns/RacingPlugin"));
		configSettings->Read(_T("StartLineBias"), &showStartline, false);
		configSettings->Read(_T("Laylines"), &showLayLines, false);
		configSettings->Read(_T("WindAngles"), &showWindAngles, false);
		configSettings->Read(_T("DualCanvas"), &showMultiCanvas, false);
		configSettings->Read(_T("StartTimer"), &defaultTimerValue, 300);
		configSettings->Read(_T("Visible"), &isWindWizardVisible, false);
		configSettings->Read(_T("SendNMEA2000Wind"), &generatePGN130306, false);
		configSettings->Read(_T("SendNMEA0183Wind"), &generateMWVSentence, false);
		// Get the length of OpenCPN's Ship's Heading Predictor Length
		configSettings->SetPath(_T("Settings"));
		configSettings->Read(_T("OwnshipHDTPredictorMiles"), &headingPredictorLength, 1);
	}

	// Load plugin icons for the toolbar
	wxString pluginFolder = GetPluginDataDir(PLUGIN_PACKAGE_NAME) + wxFileName::GetPathSeparator() + _T("data") + wxFileName::GetPathSeparator();

	// This assume the plugin is using Scaled Vector Graphics (SVG)
	wxString normalIcon = pluginFolder + _T("racing_icon_normal.svg");
	wxString toggledIcon = pluginFolder + _T("racing_icon_toggled.svg");
	wxString rolloverIcon = pluginFolder + _T("racing_icon_rollover.svg");

	// Insert the toolbar icon
	// Note that OpenCPN does not implement the rollover state
	racingToolbar = InsertPlugInToolSVG(_T(PLUGIN_COMMON_NAME), normalIcon, rolloverIcon, toggledIcon, wxITEM_CHECK, _("Race Start Display"), _T(""), NULL, -1, 0, this);

	// Wire up the event handler to receive events from the race start dialog
	// BUG BUG For some reason couldn't use wxAUI (Advanced User Interface), casting error ) to handle the close event  ??
	Connect(wxEVT_RACE_DIALOG_EVENT, wxCommandEventHandler(RacingPlugin::OnDialogEvent));

	racingWindowVisible = false;

	// Example of adding a context menu item
	// This menu item is used to toggle the display of the "Wind Wizard" gauge
	wxMenuItem* myMenu = new wxMenuItem(NULL, wxID_HIGHEST + 1, _T("Wind Wizard"), wxEmptyString, wxITEM_NORMAL, NULL);
	racingContextMenuId = AddCanvasContextMenuItem(myMenu, this);


	// Notify OpenCPN what events we want to receive callbacks for
	return (WANTS_CONFIG | WANTS_TOOLBAR_CALLBACK | INSTALLS_TOOLBAR_TOOL | WANTS_NMEA_EVENTS |
		USES_AUI_MANAGER | WANTS_LATE_INIT);
}

void RacingPlugin::LateInit(void) {

	// For some reason unbeknownst to me, the aui manager fails to wire up correctly if done
	// in the constructor or init. Seems to wire up correctly here though....

	// Load the "Wind Wizard" gauge into the AUI Manager
	windWizard = new WindWizard(parentWindow);

	// Initialize AUI, it is used to display the "Wind Wizard" gauge
	wxAuiPaneInfo paneInfo;
	paneInfo.Name(_T(PLUGIN_COMMON_NAME));
	paneInfo.Caption(_T("Wind Wizard"));
	paneInfo.CloseButton(true);
	paneInfo.Float();
	paneInfo.Dockable(false);
	paneInfo.FloatingSize(windWizard->GetMinSize());
	paneInfo.MinSize(windWizard->GetMinSize());
	paneInfo.Show(isWindWizardVisible);
	auiManager->AddPane(windWizard, paneInfo);
	auiManager->Connect(wxEVT_AUI_PANE_CLOSE, wxAuiManagerEventHandler(RacingPlugin::OnPaneClose), NULL, this);
	auiManager->Update();

}

// OpenCPN is either closing down, or we have been disabled from the Preferences Dialog
bool RacingPlugin::DeInit(void) {

	// Save the current settings
	if (configSettings) {
		configSettings->SetPath(_T("/PlugIns/RacingPlugin"));
		configSettings->Write(_T("StartLineBias"), showStartline);
		configSettings->Write(_T("Laylines"), showLayLines);
		configSettings->Write(_T("DualCanvas"), showMultiCanvas);
		configSettings->Write(_T("WindAngles"), showWindAngles);
		configSettings->Write(_T("StartTimer"), defaultTimerValue);
		configSettings->Write(_T("Visible"), isWindWizardVisible);
		configSettings->Write(_T("SendNMEA2000Wind"), generatePGN130306);
		configSettings->Write(_T("SendNMEA0183Wind"), generateMWVSentence);
	}

	// Disconnect the Advanced User Interface manager
	auiManager->Disconnect(wxEVT_AUI_PANE_CLOSE, wxAuiManagerEventHandler(RacingPlugin::OnPaneClose), NULL, this);
	auiManager->UnInit();
	auiManager->DetachPane(windWizard);
	delete windWizard;
	Disconnect(wxEVT_RACE_DIALOG_EVENT, wxCommandEventHandler(RacingPlugin::OnDialogEvent));
	return TRUE;
}

// UpdateAUI Status is invoked by OpenCPN when the saved AUI perspective is loaded
void RacingPlugin::UpdateAuiStatus(void) {

	auiManager->GetPane(_T(PLUGIN_COMMON_NAME)).Show(isWindWizardVisible);
	auiManager->Update();
	SetCanvasContextMenuItemGrey(racingContextMenuId, isWindWizardVisible);

}

// Keep the context menu synchronized with the AUI pane state
// The context menu is used to toggle the display the "Wind Wizard" gauge
void RacingPlugin::OnPaneClose(wxAuiManagerEvent& event) {

	wxAuiPaneInfo* paneInfo = event.GetPane();
	if (paneInfo->name == _T(PLUGIN_COMMON_NAME)) {
		isWindWizardVisible = false;
		// Toggle the context menu item
		SetCanvasContextMenuItemGrey(racingContextMenuId, isWindWizardVisible);
	}
	else {
		event.Skip();
	}
}

// OpenCPN Plugin "housekeeping" methods. All plugins MUST implement these
// Indicate what version of the OpenCPN Plugin API we support
int RacingPlugin::GetAPIVersionMajor() {
	return OCPN_API_VERSION_MAJOR;
}

int RacingPlugin::GetAPIVersionMinor() {
	return OCPN_API_VERSION_MINOR;
}

// The plugin version numbers. 
int RacingPlugin::GetPlugInVersionMajor() {
	return PLUGIN_VERSION_MAJOR;
}

int RacingPlugin::GetPlugInVersionMinor() {
	return PLUGIN_VERSION_MINOR;
}

// Return descriptions for the Plugin
wxString RacingPlugin::GetCommonName() {
	return _T(PLUGIN_COMMON_NAME);
}

// BUG BUG Should use XML_SUMMARY & DESCRIPTION to avoid duplication
wxString RacingPlugin::GetShortDescription() {
	return _T(PLUGIN_SHORT_DESCRIPTION);
}

wxString RacingPlugin::GetLongDescription() {
	return _T(PLUGIN_LONG_DESCRIPTION);
}

// Most plugins use a 32x32 pixel PNG file converted to xpm by pgn2wx.pl perl script
// However easier just to use a SVG file as it means we can just use one image format
// rather than maintaining several (png, bmp, ico)
wxBitmap* RacingPlugin::GetPlugInBitmap() {
	return &pluginBitmap;
}

// Receive Position, Course & Speed from OpenCPN
void RacingPlugin::SetPositionFix(PlugIn_Position_Fix &pfix) {
	currentLatitude = pfix.Lat; 
	currentLongitude = pfix.Lon; 
	courseOverGround = pfix.Cog; 
	speedOverGround = pfix.Sog; 
}

// We only install a singe toolbar item
int RacingPlugin::GetToolbarToolCount(void) {
 return 1;
}

int RacingPlugin::GetToolbarItemId() { 
	return racingToolbar; 
}

void RacingPlugin::OnToolbarToolCallback(int id) {
	// Display modal Race Start Window
	//RacingDialog *racingDialog = new RacingDialog(parentWindow);
	//racingDialog->ShowModal();
	//delete racingDialog;
	//SetToolbarItemState(id, false);

	// Display a non-modal Race Start Window
	if (!racingWindowVisible) {
		racingWindow = new RacingWindow(parentWindow, this);
		racingWindowVisible = true;
		SetToolbarItemState(id, racingWindowVisible);
		racingWindow->Show(true);
	}
	else {
		racingWindow->Close();
		delete racingWindow;
		SetToolbarItemState(id, racingWindowVisible);
	}
}

// What to do when our context menu item is selected
void RacingPlugin::OnContextMenuItemCallback(int id) {

	if (racingContextMenuId == id) {
		isWindWizardVisible = !isWindWizardVisible;
		SetCanvasContextMenuItemGrey(racingContextMenuId, isWindWizardVisible);
		auiManager->GetPane(_T(PLUGIN_COMMON_NAME)).Show(isWindWizardVisible);
		auiManager->Update();
	}
}


// Handle events from the Race Start Dialog
void RacingPlugin::OnDialogEvent(wxCommandEvent &event) {
	switch (event.GetId()) {
		// Keep the toolbar & canvas in sync with the display of the race start dialog
		case RACE_DIALOG_CLOSED:
			if (!starboardMarkGuid.IsEmpty()) {
				DeleteSingleWaypoint(starboardMarkGuid);
			}
			if (!portMarkGuid.IsEmpty()) {
				DeleteSingleWaypoint(portMarkGuid);
			}
			SetToolbarItemState(racingToolbar, racingWindowVisible);
			break;
		// drop temporary waypoints to represent port & starboard ends of the start line
		case RACE_DIALOG_STBD: {
			PlugIn_Waypoint waypoint;
			waypoint.m_IsVisible = true;
			waypoint.m_MarkName = _T("Starboard");
			starboardMarkGuid = GetNewGUID();
			waypoint.m_GUID = starboardMarkGuid;
			waypoint.m_lat = currentLatitude; // Test data 43.75847; 
			waypoint.m_lon = currentLongitude; // Test data 7.49575; 
			AddSingleWaypoint(&waypoint, false);
			break;
		}
		case RACE_DIALOG_PORT: {
			PlugIn_Waypoint waypoint;
			waypoint.m_IsVisible = true;
			waypoint.m_MarkName = _T("Man Overboard");
			waypoint.m_IconName = _T("Mob");
			portMarkGuid = GetNewGUID();
			waypoint.m_GUID = portMarkGuid;
			//waypoint.m_lat = currentLatitude; // Test data 43.757188; 
			//waypoint.m_lon = currentLongitude; // Test data 7.497963;
			waypoint.m_lat = -38.1085;
			waypoint.m_lon = 144.3989;
			AddSingleWaypoint(&waypoint, false);
			break;
		}
		default:
			event.Skip();
	}
}