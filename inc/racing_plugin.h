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
// NMEA2000® is a registered trademark of the National Marine Electronics Association
// Racing® is a registered trademark of Active Research Limited


#ifndef RACING_PLUGIN_H
#define RACING_PLUGIN_H

// Pre compiled headers 
#include "wx/wxprec.h"

#ifndef WX_PRECOMP
      #include <wx/wx.h>
#endif

// Defines version numbers, names etc. for this plugin
// This is automagically constructed via version.h.in from CMakeLists.txt, personally I think this is convoluted
#include "version.h"

// OpenCPN include file
#include "ocpn_plugin.h"

//NMEA 0183 
#include "nmea0183.h"

// NMEA 2000
#include "N2KParser.h"

// wxJSON (used for parsing SignalK data)
#include "wx/json_defs.h"
#include "wx/jsonreader.h"
#include "wx/jsonval.h"
#include "wx/jsonwriter.h"


// Race Start Dialog
#include "racing_dialog.h"
#include "racing_window.h"

// Plugin Settings dialog
#include "racing_settings.h"

// Plugin Toolbox dialog
#include "racing_toolbox.h"
// "Wind Wizard" gauge, similar to B&G SailSteer
#include "racing_gauge.h"

// wxWidgets include files

// AUI Manager
#include <wx/aui/aui.h>
#include <wx/aui/framemanager.h>

// Configuration
#include <wx/fileconf.h>

// Strings
#include <wx/string.h>


// Plugin receives events from the Race Start Display Window
const wxEventType wxEVT_RACE_DIALOG_EVENT = wxNewEventType();
const int RACE_DIALOG_CLOSED = wxID_HIGHEST + 1;
const int RACE_DIALOG_PORT = wxID_HIGHEST + 2;
const int RACE_DIALOG_STBD = wxID_HIGHEST + 3;

// Globally accessible variables used by the plugin, dialogs etc.

// Note speed, distance values are always in the users chosen units
double currentLatitude = 0.0f;
double currentLongitude = 0.0f;
double courseOverGround = 0.0f;
double speedOverGround = 0.0f;
double headingTrue = 0.0f;
double headingMagnetic = 0.0f;
double boatSpeed = 0.0f;
double waterDepth = 0.0f;
double apparentWindSpeed = 0.0f;
double apparentWindAngle = 0.0f;
double trueWindDirection = 0.0f;
double trueWindAngle = 0.0f;
double trueWindSpeed = 0.0f;
double driftSpeed = 0.0f;
double driftAngle = 0.0f;

// Protect access to lat & long read/write
//wxCriticalSection* lockPositionFix;
wxMutex lockPositionFix;

// Toolbar state
bool racingWindowVisible;

// "Wind Wizard" gauge state
bool isWindWizardVisible = false;

// Bitmap used for both the plugin and dialogs
wxBitmap pluginBitmap;

// Some configuration settings
bool showStartline;
bool showLayLines;
bool showWindAngles;
bool showMultiCanvas;
bool generatePGN130306;
bool generateMWVSentence;
int tackingAngle;
int defaultTimerValue;


// The Racing plugin
class RacingPlugin : public opencpn_plugin_116, public wxEvtHandler {

public:
	// The constructor
	RacingPlugin(void *ppimgr);
	
	// and destructor
	~RacingPlugin(void);

	// Overridden OpenCPN plugin methods
	int Init(void);
	void LateInit(void);
	bool DeInit(void);
	int GetAPIVersionMajor();
	int GetAPIVersionMinor();
	int GetPlugInVersionMajor();
	int GetPlugInVersionMinor();
	wxString GetCommonName();
	wxString GetShortDescription();
	wxString GetLongDescription();
	void SetPositionFixEx(PlugIn_Position_Fix_Ex& pfix);
	void SetActiveLegInfo(Plugin_Active_Leg_Info& pInfo);
	wxBitmap *GetPlugInBitmap();
	int GetToolbarToolCount(void);
	int GetToolbarItemId(void);
	void OnToolbarToolCallback(int id);
	void OnContextMenuItemCallback(int id);
	void OnSetupOptions(void);
	void SetupToolboxPanel(int page_sel, wxNotebook* pnotebook);
	void OnCloseToolboxPanel(int page_sel, int ok_apply_cancel);
	void ShowPreferencesDialog(wxWindow* parent);
	void SetPluginMessage(wxString& message_id, wxString& message_body);


	// Event Handler for events received from the Race Start Display Window
	void OnDialogEvent(wxCommandEvent &event);
	
	// Event Handler for wxAUI Manager
	void OnPaneClose(wxAuiManagerEvent& event);

	// Restore AUI Saved State
	void UpdateAuiStatus(void);

private: 
	// Race Display modal dialog
	RacingDialog *racingDialog;
	// or
	// Race Display modeless dialog
	RacingWindow *racingWindow;

	// Reference to the OpenCPN window handle
	wxWindow *parentWindow;
	
	// Reference to wxAUI Manager
	wxAuiManager* auiManager;

	// OpenCPN Confuration Settings
	wxFileConfig* configSettings;

	// Toolbar id
	int racingToolbar;

	// Context Menu id
	int racingContextMenuId;

	// WindWizard gauge
	WindWizard* windWizard;

	// Settings Toolbox
	RacingToolbox* racingToolbox;

	// Settings Toolbox, container for the above
	wxScrolledWindow* toolBoxWindow;
	wxBoxSizer* toolboxSizer;

	// Plugin Preferences 
	RacingSettings* racingSettings;

	// The driver handle for the requested network protocol
	DriverHandle n2kNetworkHandle;
	wxString GetNetworkInterface(wxString protocol);

	// Send a NMEA0183 True Wind Sentence
	void GenerateTrueWindSentence(void);

	// Send a NMEA 2000 True Wind message
	void GenerateTrueWindMessage(void);

	// Calculate NMEA 0183 XOR Checksum
	wxString ComputeChecksum(wxString sentence);

	// Maintain positions for the next waypoint / racing mark, so that a bearing can be calculated
	bool waypointActive = false;
	double waypointBearing, waypointDistance;

	// The "old" mechanism for receiving NMEA 0183 sentences
	void SetNMEASentence(wxString& sentence);

	// NMEA 0183, NMEA 2000 and NavMsg Listeners

	// NMEA 0183 MWV Wind sentence
	void HandleMWV(ObservedEvt ev);
	std::shared_ptr<ObservableListener> listener_mwv;

	// NMEA 0183 DPT Depth sentence
	void HandleDPT(ObservedEvt ev);
	std::shared_ptr<ObservableListener> listener_dpt;

	// NMEA 0183 VHW Boat speed and direction
	void HandleVHW(ObservedEvt ev);
	std::shared_ptr<ObservableListener> listener_vhw;

	// OpenCPN's position, speed, heading etc.
	// Means we don't need to parse NMEA 0183, NMEA 2000 or Signalk position data
	void HandleNavData(ObservedEvt ev);
	std::shared_ptr<ObservableListener> listener_nav;

	// NMEA 2000 Wind Speed and Direction
	void HandleN2K_130306(ObservedEvt ev);
	std::shared_ptr<ObservableListener> listener_130306;

	// NMEA 2000 Boat speed
	void HandleN2K_128259(ObservedEvt ev);
	std::shared_ptr<ObservableListener> listener_128259;

	// NMEA 2000 Water Depth
	void HandleN2K_128267(ObservedEvt ev);
	std::shared_ptr<ObservableListener> listener_128267;

	// SignalK listener
	void HandleSignalK(ObservedEvt ev);
	std::shared_ptr<ObservableListener> listener_SignalK;

	// Parse the SignalK update messages
	void HandleSKUpdate(wxJSONValue& value);
	void HandleSKItem(wxJSONValue& item);

	// Calculate True Wind from boat speed and apparent wind speed and direction
	void CalculateTrueWind();

	// Another version
	void CalculateTrueWindVersion2(void);

	// Calculate Drift using difference between COG & Heading.
	void CalculateDrift();

	// One second timer to update the "Wind Wizard" gauge
	wxTimer* oneSecondTimer;
	void OnTimerElapsed(wxEvent& event);

	// Proof of concept for capturing an image of the screen
	// May have been of use for https://www.cruisersforum.com/forums/f134/two-things-288437.html
	void CreateScreenShot();

	// OpenCPN's Own Ship Heading Predictor Length
	int headingPredictorLength;

	// Start line marks
	wxString starboardMarkGuid;
	wxString portMarkGuid;

};

#endif 

