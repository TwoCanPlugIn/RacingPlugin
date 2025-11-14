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
// This is automagically constructed via version.h.in from CMakeLists.txt, a bit convoluted...
#include "version.h"

// OpenCPN Plugin header
#include "ocpn_plugin.h"

// OpenCPN OpenGL header 
#include "pidc.h"

// NMEA 0183, Refer to OpenCPN Libraries
#include "nmea0183.h"

// NMEA 2000, Refer to OpenCPN Libraries
#include "N2KParser.h"

// wxJSON, Refer to OpenCPN Libraries
// Used for parsing SignalK data
#include "wx/json_defs.h"
#include "wx/jsonreader.h"
#include "wx/jsonval.h"
#include "wx/jsonwriter.h"

// wxWidgets include files

// AUI Manager
#include <wx/aui/aui.h>
#include <wx/aui/framemanager.h>

// Configuration
#include <wx/fileconf.h>

// Strings
#include <wx/string.h>

// Get Special Folders
#include <wx/stdpaths.h>

// Notifications 
// Not to be confused with OpenCPN notifications
#include <wx/notifmsg.h>

// Sockets 
// Used for debug spew
#include <wx/socket.h>

// STL
#include <vector>

// Race Start Dialog
#include "racing_dialog.h"

// An alternative to the above
#include "racing_window.h"

// Plugin Settings dialog
#include "racing_settings.h"

// Visual Studio style preferences
#include "racing_options.h"

// Plugin Toolbox dialog
#include "racing_toolbox.h"

// "Wind Wizard" gauge, similar to B&G SailSteer
#include "racing_gauge.h"

// Replay candump log files
#include "racing_replay.h"

// OpenCPN Device Context Abstraction Layer
#include "racing_graphics.h"

// Globally defined variables used by plugin, dialogs etc.
#include "racing_globals.h"

// The Racing plugin
#if (OCPN_API_VERSION_MINOR == 17)
class RacingPlugin : public opencpn_plugin_117, public wxEvtHandler {
#elif (OCPN_API_VERSION_MINOR == 18)
class RacingPlugin : public opencpn_plugin_118, public wxEvtHandler {
#elif (OCPN_API_VERSION_MINOR == 19)
class RacingPlugin : public opencpn_plugin_119, public wxEvtHandler {
#elif (OCPN_API_VERSION_MINOR == 20)
class RacingPlugin : public opencpn_plugin_120, public wxEvtHandler {
#elif (OCPN_API_VERSION_MINOR == 21)
class RacingPlugin : public opencpn_plugin_121, public wxEvtHandler {
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
	void SetDefaults(void);
	void OnSetupOptions(void);
	int GetToolboxPanelCount(void);
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
	void SetCursorLatLon(double lat, double lon);
	void SetCurrentViewPort(PlugIn_ViewPort& vp);
	bool MouseEventHook(wxMouseEvent& event);
	void OnIconize(wxIconizeEvent& event);

	// Only supported in API 1.19
#if (OCPN_API_VERSION_MINOR >= 19)	
	void PreShutdownHook();
#endif
	// Only supported in API 1.20
#if (OCPN_API_VERSION_MINOR >= 21)
	void OnContextMenuItemCallbackExt(int id, std::string obj_ident, std::string obj_type, double lat, double lon);
#endif

	// Event Handler for events received from the Countdown timer dialog
	void OnDialogEvent(wxCommandEvent &event);
	
	// Event Handler for wxAUI Manager
	void OnPaneClose(wxAuiManagerEvent& event);

	// Restore AUI Saved State
	void UpdateAuiStatus(void);

private: 
	// Countdown Timer displayed in a non-modal dialog
	RacingWindow* racingWindow;

	// Replay candump log file
	Racing_Replay* replayThread;

	// Reference to the OpenCPN window handle
	wxWindow *parentWindow;
	
	// Reference to OpenCPN AUI Manager
	wxAuiManager* auiManager;

	// Toolbar Id's
	int racingToolbarId;
	int playLogFileToolbarId;
	int stopLogFileToolbarId;

	// Context Menu id's
	int windWizardContextMenuId;
	int alertContextMenuId;
	int dscContextMenuId;
	int rteContextMenuId;
	int wptContextMenuId;
	int testContextMenuId;

	// When the route context menu is displayed, show/hide waypoint names
	bool toggleWaypointNames = false;

	// Toolbar bitmaps
	wxBitmap playLogFile;
	wxBitmap stopLogFile;

	// "Wind Wizard" gauge
	WindWizard* windWizard;

	// Settings Toolbox dialog
	RacingToolbox* racingToolbox;

	// Container for the Settings Toolbox
	wxScrolledWindow* toolBoxWindow;
	wxBoxSizer* toolboxSizer;

	// Plugin Preferences dialog
	RacingSettings* racingSettings;

	// Testing a new style of prefeences
	RacingOptions* racingOptions;

	// The interface for the OpenCPN loopback connection
	DriverHandle loopbackHandle;

	// The interface for a NMEA 2000 connection
	DriverHandle n2kNetworkHandle;

	// The interface for NMEA 0183 connection
	DriverHandle n183NetworkHandle;

	void InitializeNetwork();

	// The interface for OpenCPN Plugin Messaging
	DriverHandle ocpnMessageHandle;

	// Attempting to find the "internal" driver.
	void FindInternalMessageDriver(std::string name);

	// Returns the first interface for a requested protocol;
	// either NMEA 2000 or NMEA 0183
	DriverHandle GetNetworkInterfaceV1(std::string protocol);

	// Same as above but using std::find
	DriverHandle GetNetworkInterfaceV2(std::string protocol);

	// Same as above but using std::find_if
	//DriverHandle GetNetworkInterfaceV3(std::string protocol);

	// Transmit data over the NMEA 0183 connection
	void SendNMEA0183(wxString sentence);

#if (OCPN_API_VERSION_MINOR >= 19)	
	// Send an OpenCPN Message
	// Not clear whether this is supported
	void SendOCPNMessage(PluginMsgId msg_id, wxString message);
#endif

	// Send a NMEA0183 True Wind Sentence
	void GenerateTrueWindSentence(void);

	// Send a NMEA 2000 True Wind message
	void GenerateTrueWindMessage(void);

	// Calculate NMEA 0183 XOR Checksum
	wxString ComputeChecksum(wxString sentence);

	// Maintain positions for the next waypoint/racing mark, so that a bearing can be calculated
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
#if (OCPN_API_VERSION_MINOR >= 19)
	void HandleSignalK(ObservedEvt ev);
	std::shared_ptr<ObservableListener> listener_SignalK;
#endif

	// OpenCPN Core Messaging
#if (OCPN_API_VERSION_MINOR >= 19)
	void HandleMsgData(ObservedEvt ev);
	std::shared_ptr<ObservableListener> listener_msg;
#endif

	// Parse the SignalK update messages, either from GetSignalKPayload or OCPN Messaging
	wxString selfURN;
	void HandleSKUpdate(wxJSONValue& value);
	void HandleSKItem(wxJSONValue& item);

	// Variable to handle OpenCPN Shutdown, Supposedly the new API gives plugin's an opportunity to clean up?
	bool bShutdown = false;

	// Calculate True Wind from boat speed and apparent wind speed and direction
	void CalculateTrueWind();

	// Another version of above
	void CalculateTrueWindV2();

	// Calculate Drift using difference between COG & Heading.
	void CalculateDrift();

	// One second timer to update the "Wind Wizard" gauge and its event handler
	wxTimer* oneSecondTimer;
	void OnTimerElapsed(wxTimerEvent& event);

	// Proof of concept for capturing an image of the screen
	void CreateScreenShot();

	// Proof of concept for notifications. Different to OpenCPN notifications.
	void SendSystemNotification(wxString message);

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

	// The Chart ViewPort
	PlugIn_ViewPort viewPort;
};

#endif 

