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
// Actisense® is a registered trademark of Active Research Limited


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

// OpenGL 
#include "pidc.h"

// OpenCPN Device Context Abstraction Layer
#include "racing_graphics.h"

// wxWidgets include files

// AUI Manager
#include <wx/aui/aui.h>
#include <wx/aui/framemanager.h>

// Configuration
#include <wx/fileconf.h>

// Strings
#include <wx/string.h>

// STL
#include <vector>

// Plugin receives events from the Countdown Timer dialog
const wxEventType wxEVT_RACE_DIALOG_EVENT = wxNewEventType();
const int RACE_DIALOG_CLOSED = wxID_HIGHEST + 1;
const int RACE_DIALOG_PORT = wxID_HIGHEST + 2;
const int RACE_DIALOG_STBD = wxID_HIGHEST + 3;

// Globally accessible variables used by the plugin, dialogs etc.

// Note speed, distance values are stored using OpenCPN defaut units,
// but always displayed in the user's chosen units
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

// Protect simultaneous read & write access to latitude and longitude
wxMutex lockPositionFix;

// Toolbar state
bool isCountdownTimerVisible;

// "Wind Wizard" gauge state
bool isWindWizardVisible = false;

// Bitmap used for both the plugin and dialogs
wxBitmap pluginBitmap;

// Some configuration settings
// If we draw the start line on the screen
bool showStartline;
// Not currently implemented
bool showLayLines;
// If we draw an arrow indicating apparent wind angle on the screen
bool showWindAngles;
// If we draw only on one canvas or on both when in multi canvas mode
bool showMultiCanvas;
// If we calculate true wind angle and speed and transmit the NMEA 2000 Message
bool generatePGN130306;
// If we calculate true wind angle and speed and transmit the NMEA MWV Sentence
bool generateMWVSentence;
// Not currently implemented
int tackingAngle;
// Default value for the Countdown timer interval
int defaultTimerValue;

// The Racing plugin
#if (OCPN_API_VERSION_MINOR == 18)
class RacingPlugin : public opencpn_plugin_118, public wxEvtHandler {
#endif
#if (OCPN_API_VERSION_MINOR == 19)
	class RacingPlugin : public opencpn_plugin_119, public wxEvtHandler {
#endif
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
	int GetPlugInVersionPatch();
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
	void SetNMEASentence(wxString& sentence);
	void SetColorScheme(PI_ColorScheme cs);
	void SetPluginMessage(wxString& message_id, wxString& message_body);
	bool RenderGLOverlayMultiCanvas(wxGLContext* pcontext, PlugIn_ViewPort* vp,
		int canvasIndex, int priority);
	bool RenderOverlayMultiCanvas(wxDC& dc, PlugIn_ViewPort* vp,
		int canvasIndex, int priority);
	
	// Only supported in API 1.19
#if (OCPN_API_VERSION_MINOR == 19)	
	void PreShutdownHook();
#endif

	// Event Handler for events received from the Race Start Display Window
	void OnDialogEvent(wxCommandEvent &event);
	
	// Event Handler for wxAUI Manager
	void OnPaneClose(wxAuiManagerEvent& event);

	// Restore AUI Saved State
	void UpdateAuiStatus(void);

private: 
	// Countdown Timer non-modal dialog
	RacingWindow* racingWindow;

	// Reference to the OpenCPN window handle
	wxWindow *parentWindow;
	
	// Reference to OpenCPN AUI Manager
	wxAuiManager* auiManager;

	// Toolbar id
	int racingToolbarId;

	// Context Menu id
	int racingContextMenuId;

	// "Wind Wizard" gauge
	WindWizard* windWizard;

	// Settings Toolbox
	RacingToolbox* racingToolbox;

	// Container for the Settings Toolbox
	wxScrolledWindow* toolBoxWindow;
	wxBoxSizer* toolboxSizer;

	// Plugin Preferences 
	RacingSettings* racingSettings;

	// The driver handle for the requested network protocol
	DriverHandle n2kNetworkHandle;
	DriverHandle GetNetworkInterface(std::string protocol);

	// Send a NMEA0183 True Wind Sentence
	void GenerateTrueWindSentence(void);

	// Send a NMEA 2000 True Wind message
	void GenerateTrueWindMessage(void);

	// Calculate NMEA 0183 XOR Checksum
	wxString ComputeChecksum(wxString sentence);

	// Maintain positions for the next waypoint / racing mark, so that a bearing can be calculated
	bool isWaypointActive = false;
	double waypointBearing, waypointDistance;

	// NMEA 0183, NMEA 2000 and NavMsg Listener Handlers

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
	// Used instead of parsing NMEA 0183, NMEA 2000 or Signalk position data
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
#if (OCPN_API_VERSION_MINOR == 19)
	void HandleSignalK(ObservedEvt ev);
	std::shared_ptr<ObservableListener> listener_SignalK;
#endif

	// OpenCPN Core Messaging
#if (OCPN_API_VERSION_MINOR == 19)
	void HandleMsgData(ObservedEvt ev);
	std::shared_ptr<ObservableListener> listener_msg;
#endif

	// Parse the SignalK update messages, either from GetSignalKPayload or OCPN Messaging
	wxString selfURN;
	void HandleSKUpdate(wxJSONValue& value);
	void HandleSKItem(wxJSONValue& item);

	// Variable to handle OpenCPN Shutdown, doesn't do anything
	bool bShutdown = false;

	// Calculate True Wind from boat speed and apparent wind speed and direction
	void CalculateTrueWind();

	// Another version
	void CalculateTrueWindV2();

	// Calculate Drift using difference between COG & Heading.
	void CalculateDrift();

	// One second timer to update the "Wind Wizard" gauge
	wxTimer* oneSecondTimer;
	void OnTimerElapsed(wxTimerEvent& event);

	// Proof of concept for capturing an image of the screen
	void CreateScreenShot();

	// OpenCPN Configuration Settings
	void SaveSettings(void);
	void LoadSettings(void);

	// OpenCPN's Own Ship Heading Predictor Length
	int headingPredictorLength;

	// Start line marks
	wxString starboardMarkGuid;
	wxString portMarkGuid;
	double starboardMarkLatitude;
	double starboardMarkLongitude;
	double portMarkLatitude;
	double portMarkLongitude;
};

#endif 

