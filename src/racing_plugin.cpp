// https://stackoverflow.com/questions/65374455/wxthreaddelete-hangs-forever

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
// Description: Race Start display for OpenCPN
// Owner: twocanplugin@hotmail.com
// Date: 6/1/2020
// Version History: 
// 1.0 Initial Release
// 1.01 - 9/7/2020, Support for OpenCPN Plugin Manager/CI/Cloudsmith stuff
// 1.1 - 4/11/2024, Support for OpenCPM 5.8 Listener API's, "Wind Wizard gauge", Render Overlays
// 1.2 - 22/11/2024 OpenCPN API 1.19
// 1.3 - 13/02/2025 SignalK/Messaging Listeners.
// 1.4 - 1/6/2025 OpenCPN API 1.20, Notifications, ContextMenuExt
// 1.5 - 20/10/2025 OpenCPN API 1.21, loopback driver, other API's of dubious benefit
// BUG BUG Note Broken OCPN Methods in 5.11: SetDefaults, OnSetupOptions, SetupToolboxPanel
// BUG BUG Investigate changing new/delete to std::unique_ptr/std::make_unique

#include "racing_plugin.h"

#include <wx/window.h>

#include <wx/bitmap.h>
#include <wx/rawbmp.h>

// Used for testing Jon Gough's bitmap problem
#include <wx/msw/bitmap.h>


// Debug spew via UDP
wxDatagramSocket* debugSocket;
wxIPV4address addrLocal;
wxIPV4address addrPeer;

// Used for testing Antipole's Bug - Delete when closed
const wxChar* ParseGPXDateTime(wxDateTime& dt, const wxChar* datetime) {
	long sign, hrs_west, mins_west;
	const wxChar* end;

	// Skip any leading whitespace
	while (isspace(*datetime)) datetime++;

	// Skip (and ignore) leading hyphen
	if (*datetime == wxT('-')) datetime++;

	// Parse and validate ISO 8601 date/time string
	if ((end = dt.ParseFormat(datetime, wxT("%Y-%m-%dT%T"))) != NULL) {
		// Invalid date/time
		if (*end == 0) return NULL;

		// ParseFormat outputs in UTC if the controlling
		// wxDateTime class instance has not been initialized.

		// Date/time followed by UTC time zone flag, so we are done
		else if (*end == wxT('Z')) {
			end++;
			return end;
		}

		// Date/time followed by given number of hrs/mins west of UTC
		else if (*end == wxT('+') || *end == wxT('-')) {
			// Save direction from UTC
			if (*end == wxT('+'))
				sign = 1;
			else
				sign = -1;
			end++;

			// Parse hrs west of UTC
			if (isdigit(*end) && isdigit(*(end + 1)) && *(end + 2) == wxT(':')) {
				// Extract and validate hrs west of UTC
				wxString(end).ToLong(&hrs_west);
				if (hrs_west > 12) return NULL;
				end += 3;

				// Parse mins west of UTC
				if (isdigit(*end) && isdigit(*(end + 1))) {
					// Extract and validate mins west of UTC
					wxChar mins[3];
					mins[0] = *end;
					mins[1] = *(end + 1);
					mins[2] = 0;
					wxString(mins).ToLong(&mins_west);
					if (mins_west > 59) return NULL;

					// Apply correction
					dt -= sign * wxTimeSpan(hrs_west, mins_west, 0, 0);
					return end + 2;
				}
				else
					// Missing mins digits
					return NULL;
			}
			else
				// Missing hrs digits or colon
				return NULL;
		}
		else
			// Unknown field after date/time (not UTC, not hrs/mins
			//  west of UTC)
			return NULL;
	}
	else
		// Invalid ISO 8601 date/time
		return NULL;
}

// The class factories, used to create and destroy instances of the PlugIn
extern "C" DECL_EXP opencpn_plugin* create_pi(void *ppimgr) {
	return new RacingPlugin(ppimgr);
}

extern "C" DECL_EXP void destroy_pi(opencpn_plugin* p) {
	delete p;
}

// Constructor. Edit CMakeLists.txt to select what OpenCPN Plugin API version to support
#if (OCPN_API_VERSION_MINOR == 17)
RacingPlugin::RacingPlugin(void* ppimgr) : opencpn_plugin_117(ppimgr), wxEvtHandler() {
#elif (OCPN_API_VERSION_MINOR == 18)
RacingPlugin::RacingPlugin(void *ppimgr) : opencpn_plugin_118(ppimgr), wxEvtHandler() {
#elif (OCPN_API_VERSION_MINOR == 19)
	RacingPlugin::RacingPlugin(void* ppimgr) : opencpn_plugin_119(ppimgr), wxEvtHandler() {
#elif (OCPN_API_VERSION_MINOR == 20)
	RacingPlugin::RacingPlugin(void* ppimgr) : opencpn_plugin_120(ppimgr), wxEvtHandler() {
#elif (OCPN_API_VERSION_MINOR == 21)
	RacingPlugin::RacingPlugin(void* ppimgr) : opencpn_plugin_121(ppimgr), wxEvtHandler() {
#endif
	
	// Dialogs displayed by the plugin
	windWizard = nullptr;
	racingWindow = nullptr;
	racingToolbox = nullptr;
	racingSettings = nullptr;
	replayThread = nullptr;

	// Initialize the plugin bitmap
	wxString pluginFolder = GetPluginDataDir(PLUGIN_PACKAGE_NAME) + wxFileName::GetPathSeparator() + "data" + wxFileName::GetPathSeparator();
	pluginBitmap = GetBitmapFromSVGFile(pluginFolder + "racing_icon_toggled.svg", 32, 32);
	
	// Initialize a UDP socket for debug spew
	addrLocal.Hostname();
	addrPeer.Hostname("127.0.0.1");
	addrPeer.Service(6666);

	debugSocket = new wxDatagramSocket(addrLocal, wxSOCKET_NONE);

	if (!debugSocket->IsOk()) {
		wxLogMessage(_T("Racing Plugin, Error, failed to create UDP peer socket"));
	}
}

// Destructor
RacingPlugin::~RacingPlugin(void) {
}

int RacingPlugin::Init(void) {

	// Maintain a reference to the OpenCPN window to use as the parent Window for plugin dialogs
	parentWindow = GetOCPNCanvasWindow();

	// Maintain a reference to OpenCPN's Advanced User Interface Manager (AUI)
	auiManager = GetFrameAuiManager();

	// Load localized strings. Note to self, strings to be localized are prefixed with the _() macro
	//AddLocaleCatalog("opencpn-race_start_display_pi");

	// Load Configuration Settings
	LoadSettings();

	// Dump some of the OpenCPN's special folders
	wxLogMessage("Racing Plugin, OpenCPN Program Path (opencpn.exe): %s", GetOCPN_ExePath());
	wxLogMessage("Racing Plugin, OpenCPN Plugin Path (built-in plugins): %s", *GetpPlugInLocation());
	wxLogMessage("Racing Plugin, OpenCPN Private Data Path (logs, config): %s", *GetpPrivateApplicationDataLocation());
	wxLogMessage("Racing Plugin, OpenCPN Shared Data Path: %s", *GetpSharedDataLocation());
	wxLogMessage("Racing Plugin, Any Plugin's Writeable Documents Path: %s", GetWritableDocumentsDir());
	wxLogMessage("Racing Plugin, This Plugin's Data Path: %s", GetPluginDataDir(PLUGIN_PACKAGE_NAME));
	wxLogMessage("Racing Plugin, This Plugin's Library Path: %s", GetPlugInPath(this));

	// Dump some of the config Options for display units
	wxLogMessage("Racing Plugin, Temperature Units: %s", getUsrTempUnit_Plugin());
	wxLogMessage("Racing Plugin, Speed Units: %s", getUsrSpeedUnit_Plugin());
#if (OCPN_API_VERSION_MINOR >= 20)
	wxLogMessage("Racing Plugin, Wind Speed Units: %s", getUsrWindSpeedUnit_Plugin());
	wxLogMessage("Racing Plugin, Depth Units: %s", getUsrDepthUnit_Plugin());
#endif

#if (OCPN_API_VERSION_MINOR >= 20)
	// New OCPN Helper method for displaying/formatting date/time
	wxDateTime now = wxDateTime::Now();
	DateTimeFormatOptions timeOptions;
	timeOptions.SetTimezone("Local Time");
	timeOptions.SetShowTimezone(false);

	timeOptions.SetFormatString("$weekday_short_date_time");
	wxLogMessage("Racing Plugin, Time Format (%s): %s", timeOptions.format_string, toUsrDateTimeFormat_Plugin(now, timeOptions));
	
	timeOptions.SetFormatString("$short_date");
	wxLogMessage("Racing Plugin, Time Format (%s): %s", timeOptions.format_string, toUsrDateTimeFormat_Plugin(now, timeOptions));
	
	timeOptions.SetFormatString("$short_date_time");
	wxLogMessage("Racing Plugin, Time Format (%s): %s", timeOptions.format_string, toUsrDateTimeFormat_Plugin(now, timeOptions));
	
	timeOptions.SetFormatString("$long_date_time");
	wxLogMessage("Racing Plugin, Time Format (%s): %s", timeOptions.format_string, toUsrDateTimeFormat_Plugin(now, timeOptions));
	
	timeOptions.SetFormatString("$long_date");
	wxLogMessage("Racing Plugin, Time Format (%s): %s", timeOptions.format_string, toUsrDateTimeFormat_Plugin(now, timeOptions));
	
	// Note, the helper API is kind of "unsafe"
	timeOptions.SetFormatString("long_date forgot leading $"); 
	wxLogMessage("Racing Plugin, Time Format (%s): %s", timeOptions.format_string, toUsrDateTimeFormat_Plugin(now, timeOptions));
#endif
	
	// Load icons for the toolbar
	// The race start dialog
	wxString pluginFolder = GetPluginDataDir(PLUGIN_PACKAGE_NAME) + wxFileName::GetPathSeparator() + "data" + wxFileName::GetPathSeparator();

	// This assumes the plugin is using Scaled Vector Graphics (SVG)
	wxString normalIcon = pluginFolder + "racing_icon_normal.svg";
	wxString toggledIcon = pluginFolder + "racing_icon_toggled.svg";
	wxString rolloverIcon = pluginFolder + "racing_icon_rollover.svg";

	// Add the toolbar button 
	// BUG BUG Note that OpenCPN does not implement the rollover state
	// This toolbar button toggles the display of the race start dialog
	racingToolbarId = InsertPlugInToolSVG(PLUGIN_COMMON_NAME, normalIcon, 
		rolloverIcon, toggledIcon, wxITEM_CHECK, PLUGIN_COMMON_NAME, PLUGIN_SHORT_DESCRIPTION, NULL, -1, 0, this);

	// Log File Play/Stop ToolbarId
	// Uses the loopback interface to replay NMEA 0183, NMEA 2000 & SignalK logs
	// Invokes racing_replay class which converts candump logs to the loopback "format"
	normalIcon = pluginFolder + "vdr_play.svg";
	toggledIcon = pluginFolder + "vdr_stop.svg";
	rolloverIcon = pluginFolder + "vdr_play_rollover.svg";

	playLogFileToolbarId = InsertPlugInToolSVG("Play", normalIcon,
		rolloverIcon, toggledIcon, wxITEM_CHECK, "Play", "Play Candump Log File", NULL, -1, 0, this);

	// Default state for displaying the Countdown Timer dialog
	isCountdownTimerVisible = false;

	// Examples of adding a context menu items
	// This menu item is used to display the "Wind Wizard" gauge
	wxMenuItem* wizardMenu = new wxMenuItem(NULL, WINDWIZARD_MENU, "Wind Wizard", "a funky gauge", wxITEM_NORMAL, NULL);
	windWizardContextMenuId = AddCanvasContextMenuItem(wizardMenu, this);

	// This menu item raises core OpenCPN notifications
	wxMenuItem* alertMenu = new wxMenuItem(NULL, ALERT_MENU, "Raise Alert", "OCPN Notifications", wxITEM_NORMAL, NULL);
	alertContextMenuId = AddCanvasContextMenuItem(alertMenu, this);

	// I just use this menu is used to test different things
	wxMenuItem* playbackMenu = new wxMenuItem(NULL, TEST_MENU, "Test", "Adhoc Testing", wxITEM_NORMAL, NULL);
	testContextMenuId = AddCanvasContextMenuItem(playbackMenu, this);

#if (OCPN_API_VERSION_MINOR >= 20)
	// Valid Sub Menu Names "Route", " Waypoint", "Track", "AIS")

	// This menu item is added to the AIS Sub Menu
	wxMenuItem* dscMenu = new wxMenuItem(NULL, DSC_MENU, "DSC Call", "Call target om VHF", wxITEM_NORMAL, NULL);
	dscContextMenuId = AddCanvasContextMenuItemExt(dscMenu, this, "AIS");

	// This menu item is added to the Route Sub Menu
	wxMenuItem* rteMenu = new wxMenuItem(NULL, RTE_MENU, "To NMEA 2000", "Send to NMEA 2000", wxITEM_NORMAL, NULL);
	rteContextMenuId = AddCanvasContextMenuItemExt(rteMenu, this, "Route");

	// One of the new 1.21 API's enables toggling display of route waypoint names
	wxMenuItem* wptMenu = new wxMenuItem(NULL, RTE_MENU, "Toggle Waypoint Names", "Show or Hide Route Waypoints", wxITEM_NORMAL, NULL);
	wptContextMenuId = AddCanvasContextMenuItemExt(wptMenu, this, "Route");

#endif

	// Set up the listeners. NMEA 0183, NMEA 2000 and SignalK are used to obtain data 
	// for boat speed, apparent wind angle & speed and NavData for position and heading
	// BUG BUG Should ensure that the connections exist before adding the listeners

	// NMEA 0183 MWV Wind Sentence
	wxDEFINE_EVENT(EVT_183_MWV, ObservedEvt);
	NMEA0183Id id_mwv = NMEA0183Id("MWV");
	listener_mwv = std::move(GetListener(id_mwv, EVT_183_MWV, this));
	Bind(EVT_183_MWV, [&](ObservedEvt ev) {
		HandleMWV(ev);
		});

	// NMEA 0183 VHW Boat Speed Sentence
	wxDEFINE_EVENT(EVT_183_VHW, ObservedEvt);
	NMEA0183Id id_vhw = NMEA0183Id("VHW");
	listener_vhw = std::move(GetListener(id_vhw, EVT_183_VHW, this));
	Bind(EVT_183_VHW, [&](ObservedEvt ev) {
		HandleVHW(ev);
		});

	// NMEA 0183 DPT Depth Sentence
	wxDEFINE_EVENT(EVT_183_DPT, ObservedEvt);
	NMEA0183Id id_dpt = NMEA0183Id("DPT");
	listener_dpt = std::move(GetListener(id_dpt, EVT_183_DPT, this));
	Bind(EVT_183_DPT, [&](ObservedEvt ev) {
		HandleDPT(ev);
		});

	// PGN 130306 Wind
	wxDEFINE_EVENT(EVT_N2K_130306, ObservedEvt);
	NMEA2000Id id_130306 = NMEA2000Id(130306);
	listener_130306 = std::move(GetListener(id_130306, EVT_N2K_130306, this));
	Bind(EVT_N2K_130306, [&](ObservedEvt ev) {
		HandleN2K_130306(ev);
		});

	// PGN 128267 Depth
	wxDEFINE_EVENT(EVT_N2K_128267, ObservedEvt);
	NMEA2000Id id_128267 = NMEA2000Id(128267);
	listener_128267 = std::move(GetListener(id_128267, EVT_N2K_128267, this));
	Bind(EVT_N2K_128267, [&](ObservedEvt ev) {
		HandleN2K_128267(ev);
		});

	// PGN 128259 Boat Speed
	wxDEFINE_EVENT(EVT_N2K_128259, ObservedEvt);
	NMEA2000Id id_128259 = NMEA2000Id(128259);
	listener_128259 = std::move(GetListener(id_128259, EVT_N2K_128259, this));
	Bind(EVT_N2K_128259, [&](ObservedEvt ev) {
		HandleN2K_128259(ev);
		});

	// SignalK, Observer Listener now supported in API 1.19
#if (OCPN_API_VERSION_MINOR >= 19)
	wxDEFINE_EVENT(EVT_SIGNALK, ObservedEvt);
	SignalkId id_signalk = SignalkId("self");
	listener_SignalK = std::move(GetListener(id_signalk, EVT_SIGNALK, this));
	Bind(EVT_SIGNALK, [&](ObservedEvt ev) {
		HandleSignalK(ev);
		});
#endif

	// OpenCPN Core NavData
	wxDEFINE_EVENT(EVT_NAV_DATA, ObservedEvt);
	listener_nav = std::move(GetListener(NavDataId(), EVT_NAV_DATA, this));
	Bind(EVT_NAV_DATA, [&](ObservedEvt ev) {
		HandleNavData(ev);
		});

	// OpenCPN Messaging
#if (OCPN_API_VERSION_MINOR >= 19)
	wxDEFINE_EVENT(EVT_OCPN_MSG, ObservedEvt);
	PluginMsgId msg_id = PluginMsgId("OCPN_WPT_ACTIVATED");
	listener_msg = std::move(GetListener(msg_id, EVT_OCPN_MSG, this));
	Bind(EVT_OCPN_MSG, [&](ObservedEvt ev) {
		HandleMsgData(ev);
		});
#endif

	// Wire up the event handler to receive events from the Countdown Timer dialog
	Connect(wxEVT_RACE_DIALOG_EVENT, wxCommandEventHandler(RacingPlugin::OnDialogEvent));

	// Get handles for the different network interfaces
	// BUG BUG Broken, GetActiveDrivers is not valid until afterwards 
	//InitializeNetwork();
	
	// Instantiate the "Wind Wizard" gauge
	windWizard = new WindWizard(parentWindow);

	// Add the "Wind Wizard" gauge to the AUI Manager
	wxAuiPaneInfo paneInfo;
	paneInfo.Name(PLUGIN_COMMON_NAME);
	paneInfo.Caption("Wind Wizard");
	paneInfo.CloseButton(true);
	paneInfo.GripperTop(true);
	paneInfo.Float();
	paneInfo.MinSize(windWizard->GetMinSize());
	paneInfo.Show(isWindWizardVisible);
	auiManager->AddPane(windWizard, paneInfo);
	auiManager->Update();
	auiManager->Connect(wxEVT_AUI_PANE_CLOSE, wxAuiManagerEventHandler(RacingPlugin::OnPaneClose), NULL, this);

	// Wire up the event handler to receive events from the Countdown Timer dialog
	Connect(wxEVT_RACE_DIALOG_EVENT, wxCommandEventHandler(RacingPlugin::OnDialogEvent));

	// Notify OpenCPN what events we want to receive callbacks for
	return (WANTS_CONFIG | WANTS_PREFERENCES | INSTALLS_TOOLBOX_PAGE |
		WANTS_TOOLBAR_CALLBACK | INSTALLS_TOOLBAR_TOOL | WANTS_NMEA_EVENTS | WANTS_MOUSE_EVENTS |
		WANTS_PLUGIN_MESSAGING | USES_AUI_MANAGER | WANTS_LATE_INIT | WANTS_CURSOR_LATLON |
		WANTS_OVERLAY_CALLBACK | WANTS_OPENGL_OVERLAY_CALLBACK | WANTS_ONPAINT_VIEWPORT 
#if (OCPN_API_VERSION_MINOR >= 19)
		| WANTS_PRESHUTDOWN_HOOK
#endif
		);
}

// Late init allows plugins to perform additional actions well after the plugin has been loaded
void RacingPlugin::LateInit(void) {

	// One second timer updates the "Wind Wizard" gauge
	// Optionally constructs and transmits NMEA 0183 & NMEA 2000 True Wind data
	oneSecondTimer = new wxTimer();
	oneSecondTimer->Connect(wxEVT_TIMER, wxTimerEventHandler(RacingPlugin::OnTimerElapsed), NULL, this);
	oneSecondTimer->Start(1000, wxTIMER_CONTINUOUS);

	// BUG BUG Broken, GetActiveDrivers is not valid until afterwards 
	//InitializeNetwork();

}

// OpenCPN is either closing down, or we have been disabled from the Preferences Dialog
bool RacingPlugin::DeInit(void) {

	// Save the current settings
	SaveSettings();

	// Cleanup the One Second Timer
	if (oneSecondTimer->IsRunning()) {
		oneSecondTimer->Stop();
	}
	// Disconnect the timer event handler
	oneSecondTimer->Disconnect(wxEVT_TIMER, wxTimerEventHandler(RacingPlugin::OnTimerElapsed), NULL, this);


	// If replaying a log file, cleanup
	if (replayThread != nullptr) {
		if (replayThread->IsRunning()) {
			wxThread::ExitCode exitCode;
			replayThread->Delete(&exitCode, wxTHREAD_WAIT_BLOCK);
		}
	}

	// Disconnect the Advanced User Interface manager
	auiManager->DetachPane(windWizard);
	auiManager->Disconnect(wxEVT_AUI_PANE_CLOSE, wxAuiManagerEventHandler(RacingPlugin::OnPaneClose), NULL, this);
	delete windWizard;

	// Cleanup the toolbox page here because OnSetupToolbox is only called once at Startup.
	// If we were to perform the cleanup in the OnCloseToolboxPane method, we can never initialize it again.
	DeleteOptionsPage(toolBoxWindow);

	// Cleanup up the Countdown Timer dialog
	if (isCountdownTimerVisible) {
		racingWindow->Close();
		delete racingWindow;
	}

	// Cleanup the handler for the Countdown Timer dialog events 
	Disconnect(wxEVT_RACE_DIALOG_EVENT, wxCommandEventHandler(RacingPlugin::OnDialogEvent));

	// OpenCPN doesn't care about the return value
	return bShutdown; 
}

// UpdateAUI Status is invoked by OpenCPN when the saved AUI perspective is loaded
// Use this to synch the context menu state when the AUI perspective is initially loaded
void RacingPlugin::UpdateAuiStatus(void) {

	if (auiManager->GetPane(PLUGIN_COMMON_NAME).IsOk()) {
		auiManager->GetPane(PLUGIN_COMMON_NAME).Show(isWindWizardVisible);
		SetCanvasContextMenuItemGrey(windWizardContextMenuId, isWindWizardVisible);
	}
}

// Keep the context menu synchronized when the AUI pane is closed
void RacingPlugin::OnPaneClose(wxAuiManagerEvent& event) {

	if (event.GetPane()->name ==  PLUGIN_COMMON_NAME) {
		isWindWizardVisible = !isWindWizardVisible;
		// Toggle the context menu item
		SetCanvasContextMenuItemGrey(windWizardContextMenuId, isWindWizardVisible);
	}
	else {
		event.Skip();
	}
}

// Get the latitude & longitude where the cursor is located on the canvas.  Can also another 
// API to perform the reverse, converting from latitide & longitude to pixel points.
// Requires WANTS_CURSOR_LATLON 
void RacingPlugin::SetCursorLatLon(double lat, double lon) {

	wxPoint wxP;
	GetCanvasPixLL(&viewPort, &wxP, lat, lon);
	// BUG BUG Do something useful with the current cursor position
}

// BUG BUG Iconize appears not to be called
// Show/Hide my Windows if OpenCPN is iconized
void RacingPlugin::OnIconize(wxIconizeEvent& event) {
	
	wxMessageBox("Iconize");

	if (event.IsIconized()) {
		if (isCountdownTimerVisible) {
			racingWindow->Hide();
		}
		if (auiManager->GetPane(PLUGIN_COMMON_NAME).IsOk()) {
			auiManager->GetPane(PLUGIN_COMMON_NAME).Hide();
		}
	}
	else {
		if (isCountdownTimerVisible) {
			racingWindow->Restore();
		}
		if (auiManager->GetPane(PLUGIN_COMMON_NAME).IsOk()) {
			auiManager->GetPane(PLUGIN_COMMON_NAME).Show();
		}
	}
	
	event.Skip();
}

// Requires WANTS_MOUSE_EVENTS
bool RacingPlugin::MouseEventHook(wxMouseEvent& event) {
	int canvasIndex = GetCanvasIndexUnderMouse();
	double lat, lon;

	// Touch interface devices do not provide cursor tracking...
	// We only get button click events here.
	// Compute the cursor lat/lon on each such event received,

	if (IsTouchInterface_PlugIn()) {
		wxPoint wxP(event.GetX(), event.GetY());
		GetCanvasLLPix(&viewPort, wxP, &lat, &lon);
	}

	//GetCanvasCount() {
	
	// Make sure we are not beyond the bounds
	// CheckMUIEdgePan_PlugIn(g_cursor_x, g_cursor_y, event.Dragging(), g_EdgePanSensitivity, 2, m_drawing_canvas_index);
	
	if (event.LeftDClick()) {
		wxPoint wxP;
		wxP = event.GetPosition();
		GetCanvasLLPix(&viewPort, wxP, &lat, &lon);

		wxString message = wxString::Format("Left Double Click at Position: %0.2f, %0.2f", lat, lon);
		SendSystemNotification(message);
	}

	else if (event.LeftDown()) {
	
	}

	else if (event.LeftUp()) {
	
	}

	else if (event.Dragging()) {
		if (event.LeftIsDown()) {
		
		}
	}
	else if (event.RightDown()) {
		
	}

	// false allows the event to also be handled by OpenCPN (eg. chart object properties)
	// true means the event is not propogated
	return false; 
}

// The Viewort allows us to determine the chart extents
// Requires WANTS_ONPAINT_VIEWPORT 
void RacingPlugin::SetCurrentViewPort(PlugIn_ViewPort& vp) {

	// Persist the viewport as it is used by GetCanvasLLPix 
	viewPort = vp;
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

int RacingPlugin::GetPlugInVersionPatch() {
	return PLUGIN_VERSION_PATCH;
}

// Return descriptions for the Plugin
wxString RacingPlugin::GetCommonName() {
	return PLUGIN_COMMON_NAME;
}

wxString RacingPlugin::GetShortDescription() {
	return PLUGIN_SHORT_DESCRIPTION;
}

wxString RacingPlugin::GetLongDescription() {
	return PLUGIN_LONG_DESCRIPTION;
}

// Most plugins use a 32x32 pixel PNG file converted to xpm by pgn2wx.pl perl script
// However easier to use a SVG file as it means I can maintain one image format rather than several.
wxBitmap* RacingPlugin::GetPlugInBitmap() {
	return &pluginBitmap;
}

// Optional OpenCPN Plugin Methods

// Pre shutdown, Perhaps do some cleanup here. 
// However, doesn't allow a plugin to veto shutdown which would be useful for autopilots
// Note for Windows, wxWidgets Yes/No dialogs do not display Question Mark icon
#if (OCPN_API_VERSION_MINOR >= 19)
void RacingPlugin::PreShutdownHook() {

	bShutdown = true;
	if (wxMessageBox("OK to shutdown?", "Shutdown", wxYES_NO | wxICON_QUESTION) == wxID_NO) {
		bShutdown = false;
	}
}
#endif

// BUG BUG I've never seen this called
int RacingPlugin::GetToolbarToolCount(void) {

	SendSystemNotification("GetToolbarToolCount called");

 return 3;
}

// BUG BUG I've never seen this called. 
// What happens if a plugin installs multiple toolbar buttons?
int RacingPlugin::GetToolbarItemId() {

	SendSystemNotification("GetToolbarItemId called");
	return racingToolbarId; 
}

// What to perfom when the toolbar button is presssed
void RacingPlugin::OnToolbarToolCallback(int id) {

	if (id == racingToolbarId) {
		// Display the non-modal Countdown Timer dialog
		if (!isCountdownTimerVisible) {
			racingWindow = new RacingWindow(parentWindow, this);
			isCountdownTimerVisible = true;
			SetToolbarItemState(id, isCountdownTimerVisible);
			racingWindow->Show(true);
		}
		else {
			racingWindow->Close();
			delete racingWindow;
			isCountdownTimerVisible = false;
			SetToolbarItemState(id, isCountdownTimerVisible);
		}
	}
	else if (id == playLogFileToolbarId) {
		if (replayThread == nullptr) {

			// BUG BUG Temporary Workaround
			InitializeNetwork();

			// Create a file open dialog, and replay a candump file using the loopback driver
			wxString fileName;

			wxFileDialog fileOpenDialog;
			fileOpenDialog.SetDirectory(wxStandardPaths::Get().GetDocumentsDir());
			fileOpenDialog.SetMessage("Open Log File");
			fileOpenDialog.SetPosition(wxDefaultPosition);
			fileOpenDialog.SetSize(wxSize(300, 300));
			fileOpenDialog.SetExtraStyle(wxFD_OPEN | wxFD_FILE_MUST_EXIST);
			fileOpenDialog.SetWildcard("Candump (*.log)|*.log|Comma Separated Variable (*.csv)|*.csv|Text files (*.txt)|*.txt");
			fileOpenDialog.EnableFullScreenView(false);

			if (fileOpenDialog.ShowModal() == wxID_OK) {

				wxFileName filePath(fileOpenDialog.GetPath());

				wxMessageBox(wxString::Format("File Path: %s\nFile Name: %s\nFile Ext: %s\nCmp: %d",
					filePath.GetFullPath(), filePath.GetFullName(), filePath.GetExt(),
					filePath.GetExt().compare("log")));

				if (filePath.IsOk()) {
					// BUG BUG Contemplate parsing either NMEA 2000 or NMEA 0183
					// BUG BUG Consider using std::unique_ptr, std::make_unique
					replayThread = new Racing_Replay(loopbackHandle, filePath);

					replayThread->Run();
				}
			}
		}
		else {
			// Toggle the play button
			SetToolbarItemState(playLogFileToolbarId, false);

			// And stop the replay thread
			if (replayThread != nullptr) {
				if (replayThread->IsRunning()) {

					wxThread::ExitCode exitCode;
					replayThread->Delete(&exitCode, wxTHREAD_WAIT_BLOCK);
					if (exitCode == (wxThread::ExitCode)wxTHREAD_NO_ERROR) {
						wxLogMessage("Racing Plugin, Replay thread exit code: %d", exitCode);
					}
				}
				replayThread = nullptr;
			}
		}
	}
}

// What to do when the context menu items are selected
void RacingPlugin::OnContextMenuItemCallback(int id) {
	
	// Toggle the visibility of the Race Start timer dialog
	if (id == windWizardContextMenuId) {
		isWindWizardVisible = !isWindWizardVisible;
		SetCanvasContextMenuItemGrey(windWizardContextMenuId, isWindWizardVisible);
		auiManager->GetPane(PLUGIN_COMMON_NAME).Show(isWindWizardVisible);
		auiManager->Update();
	}

	else if (id == testContextMenuId) {
		// Testing icons for Jon Gough's bug
		wxArrayString iconNames = GetIconNameArray();

		wxString icons;
		for (auto it : iconNames) {
			icons += it;
			icons += "\n";

		}
		wxMessageBox(icons,	"Icon Names");

		auto p = std::find(iconNames.begin(), iconNames.end(), "mob"); //"1st-Active-Waypoint");
		if (p != iconNames.end()) {
			wxMessageBox(wxString::Format("Found Icon: %s", p->data()));
			wxBitmap markBitmap = GetIcon_PlugIn(p->data());
			markBitmap.SaveFile(wxString::Format("d:\\logs\\%s.bmp", p->data()), 
				wxBitmapType::wxBITMAP_TYPE_BMP);
			wxMessageBox(wxString::Format("Bitmap is %s, Width: %d, Height: %d", 
				markBitmap.IsOk() ? "OK" : "Not OK",
				markBitmap.GetWidth(),
				markBitmap.GetHeight()));	
			
			wxNativePixelData data(markBitmap);

			if (!data) {
				wxMessageBox("No Native Pixel Data", "Native Pixel");
			}
			else {
				wxMessageBox(wxString::Format("Native Pixel Data\nHeight: %d, Width %d", data.GetHeight(),
					data.GetWidth()), 
					"Native Pixel");
			}

			wxNativePixelData::Iterator it(data);
			bool NonZero = true;
			wxString pixelString;
			for (int y = 0; y < data.GetHeight(); ++y) {
				wxNativePixelData::Iterator rowStart = it;

				for (int x = 0; x < data.GetWidth(); ++x, ++it) {

					pixelString += wxString::Format("%d, %d: %d %d %d\n",
						y, x, it.Red(), it.Green(), it.Blue());

					if ((it.Red() != 0) || (it.Green() !=0 ) || (it.Blue() != 0)) {
						NonZero = false;
					}
				}

				it = rowStart;
				it.OffsetY(data, 1);
			}

			wxMessageBox(pixelString, "Native Pixel Data");


			wxMessageBox(wxString::Format("BitMap is %s", NonZero ? "Dummy Bitmap" : "Really Valid", "Native Pixel Data"));


			// Convert To Image

			wxImage markImage = markBitmap.ConvertToImage();

			
			unsigned char* ptr = markImage.GetData();
			bool validImage = false;
			for (int h = 0; h < markImage.GetHeight(); h++) {
				for (int w = 0; w < markImage.GetWidth(); w++) {
					if (*ptr != 0) {
						validImage = true;
					}
					ptr++;
				}
				ptr++;
			}

			wxMessageBox(wxString::Format("Image is %s", validImage ? "Valid" : "Invalid"), "Image Data");

		}
		else {
			wxMessageBox("Icon Not found");
		}
	}

	// Raise an OpenCPN Alert (different to a platform OS notification)
	else if (id == alertContextMenuId) {

#if (OCPN_API_VERSION_MINOR >= 20)
		// Note converting from std::string to wxString using wxWidgets wxString constructor
		wxString notificationResponse(RaiseNotification(PI_NotificationSeverity::PI_kWarning, "Racing Plugin Demo Notification"));
		// Display the resulting GUID created by OpenCPN to identify the alert
		wxMessageBox(wxString::Format("Notification GUID: %s", notificationResponse), "Notification");
#else
		//wxMessageBox(wxString::Format("Notifications not supported on API level: %d", OCPN_API_VERSION_MINOR));
		wxDialog* falseDialog = new wxDialog(parentWindow, wxID_ANY, "Notification Error");
		wxBoxSizer* falseDialogSizer = new wxBoxSizer(wxOrientation::wxVERTICAL);
		wxStaticText* label = new wxStaticText(falseDialog, wxID_ANY, "This is an error message");
		falseDialogSizer->Add(label);

		wxStdDialogButtonSizer* buttonSizer = new wxStdDialogButtonSizer();
		buttonSizer->SetOrientation(wxOrientation::wxHORIZONTAL);

		wxButton* okButton = new wxButton(falseDialog, wxID_OK, "OK");
		wxButton* cancelButton = new wxButton(falseDialog, wxID_CANCEL, "Cancel");
		//buttonSizer->AddButton(okButton);
		//buttonSizer->AddButton(cancelButton);
		buttonSizer->AddStretchSpacer(2);
		buttonSizer->Add(okButton,0, wxALL | wxBORDER, 5);
		buttonSizer->Add(cancelButton, 0, wxALL | wxBORDER , 5);

		//buttonSizer->SetAffirmativeButton(okButton);
		//buttonSizer->SetCancelButton(cancelButton);
		falseDialogSizer->AddStretchSpacer(2);
		falseDialogSizer->Add(buttonSizer);
		
		falseDialog->SetSizer(falseDialogSizer);
		if (wxID_OK == falseDialog->ShowModal()) {
			wxMessageBox("OK", "Notification Dialog Result");
		}

#endif
	}
}

#if (OCPN_API_VERSION_MINOR >= 21)
// Context MenuItems that may be added to AIS, Routes, Tracks and Waypoints context menu sub-items
void RacingPlugin::OnContextMenuItemCallbackExt(int id, std::string obj_ident, std::string obj_type, double lat, double lon) {

	// Handle the Context Menu call back if the object was an AIS target
	// Could use this to initiate a DSC VHF call
	if (id == dscContextMenuId) {
		wxMessageBox(wxString::Format("Object Id: %d\nObject Identifier (MMSI): %s\nObject Type: %s\nLatitude: %s\nLongitude: %s",
			id, obj_ident.c_str(), obj_type.c_str(),
			toSDMM_PlugIn(1, lat, true), toSDMM_PlugIn(2, lon, true)), "AIS Target Information", wxOK | wxICON_INFORMATION);
	}

	// Show or Hide Route Waypoint Names
	else if (id == wptContextMenuId) {
		toggleWaypointNames = !toggleWaypointNames;
		ShowRouteWaypointNames(obj_ident, toggleWaypointNames);
	}
	// Handle the Context Menu call back if the object was a Route
	else if (id == rteContextMenuId) {

		wxMessageBox(wxString::Format("Object Id: %d\nObject Identifier (Route): %s\nObject Type: %s",
			id, obj_ident.c_str(), obj_type.c_str()), "Route Information", wxOK | wxICON_INFORMATION);
		
		// And if it is an active route, display the route & waypoints
		wxString currentRoute = GetActiveRouteGUID();

		if (!currentRoute.IsEmpty()) {
			std::unique_ptr<PlugIn_Route> activeRoute;
			activeRoute = GetRoute_Plugin(currentRoute);
			wxMessageBox(wxString::Format("Route Name: %s\n", activeRoute->m_NameString), "Route Details", wxOK);

			std::unique_ptr<PlugIn_Route_ExV3> activeRouteV3;
			activeRouteV3 = GetRouteExV3_Plugin("698dffff-03be-4b5f-9c4d-edcf3d230000");
			wxMessageBox(wxString::Format("Name: %s\nSpeed: %f\nColour: %s\nETD: %s\nTime Format: %s", 
				activeRouteV3->m_NameString,
				activeRouteV3->m_PlannedSpeed,
				activeRouteV3->m_Colour, activeRouteV3->m_PlannedDeparture.FormatISOCombined(' '),
				activeRouteV3->m_TimeDisplayFormat, "Route ExV3"));

			Plugin_WaypointList* list = activeRoute->pWaypointList;

			for (Plugin_WaypointList::iterator it = list->begin(); it != list->end(); ++it) {
				auto wp = *it;
				wxMessageBox(wxString::Format("GUID: %s\nName: %s\nLat: %0.2f\nLon: %0.2f\nDate: %s", wp->m_GUID, wp->m_MarkName,
					wp->m_lat, wp->m_lon, wp->m_CreateTime.FormatISOCombined()),"Waypoint Details", wxOK);

			}
		}

		// A "new: API, can be useful to determining if a route is Active rather than
		// using OCPN Messaging, eg. OCPN_ROUTE_ACTIVATED
		wxString currentWaypoint = GetActiveWaypointGUID();
		if (!currentWaypoint.IsEmpty()) {

			wxMessageBox(wxString::Format("Current Waypoint (GUID)%s", currentWaypoint), "Active Waypoint", wxOK);
		}

		// This was for Tony (Antipole's) bug.
		// Testing Navobj db & waypoint errors for date time, creation date etc.
		// T'was something weird with the display of date time thingies.
		wxString point_time_string = "2023-12-23T10:07:26Z";
		if (point_time_string.size()) {
			wxString sdt(point_time_string.c_str());
			wxDateTime dt;
			ParseGPXDateTime(dt, sdt);
			wxMessageBox(dt.FormatISOCombined(),"Antipole's Date Time bug");

		}

	}
}
#endif


// Set default values when the plugin is installed
void RacingPlugin::SetDefaults(void) {

	showStartline = false;
	showLayLines = false;
	showWindAngles, true;
	showMultiCanvas, false;
	defaultTimerValue, 300; // 5 minutes
	isWindWizardVisible, false;
	generatePGN130306 = false;
	generateMWVSentence, false;

	SendSystemNotification("SetDefaults invoked");
}

// Add our own tab on the OpenCPN toolbox, under the "User" settings. Ordinarily plugins 
// would add their own settings dialog launched using the ShowPreferencesDialog method
// OnSetupOptions seems to be invoked at Startup. In 5.13 is it broken ?
void RacingPlugin::OnSetupOptions(void) {

	// Get a handle to our options page window, add a sizer to it, to which we will add our toolbox panel
	toolBoxWindow = AddOptionsPage(OptionsParentPI::PI_OPTIONS_PARENT_UI, PLUGIN_COMMON_NAME);
	toolboxSizer = new wxBoxSizer(wxVERTICAL);
	toolBoxWindow->SetSizer(toolboxSizer);
	// Create our toolbox panel and add it to the toolbox via the sizer
	racingToolbox = new RacingToolbox(toolBoxWindow);
	toolboxSizer->Add(racingToolbox, 1, wxALL | wxEXPAND);

	SendSystemNotification("OnSetupOptions invoked");
}

// Get the number of panels that we install in the toolbox.Never seen this invoked.
int RacingPlugin::GetToolboxPanelCount(void) {

	SendSystemNotification("GetToolboxPanelCount invoked");
	// The plugin installs a single Toolbar item
	return 1;
}

// I have no idea when this is called, supposedly when the plugin is initially installed
// but it seems like it is no longer implemented
void RacingPlugin::SetupToolboxPanel(int page_sel, wxNotebook* pnotebook) {

	SendSystemNotification(wxString::Format("SetupToolboxPanel invoked: %d", page_sel));
}

// Invoked when the OpenCPN Toolbox OK, Apply or Cancel buttons are pressed
void RacingPlugin::OnCloseToolboxPanel(int page_sel, int ok_apply_cancel) {

	SendSystemNotification("OnCloseToolboxPanel called");
	// BUG BUG Why didn't they use standard enums like wxID_OK ??	
	if ((ok_apply_cancel == 0) || (ok_apply_cancel == 4)) {
		// Save the setttings
		SaveSettings();
	}
}

// Display Plugin preferences dialog. 
// This is the preferable way for plugins to display UI to configure their settings
void RacingPlugin::ShowPreferencesDialog(wxWindow* parent) {

	//racingSettings = new RacingSettings(parent);
	//if (racingSettings->ShowModal() == wxID_OK) {
	//	SaveSettings();
	//}
	//delete racingSettings;

	// BUG BUG Testing new settings UI design, similar to Visual Studio
	// Investigate how to perform the search stuff
	racingOptions = new RacingOptions(parent,wxID_ANY, "Settings" , parent->FromDIP(wxSize(500, 400)));
	if (wxID_OK == racingOptions->ShowModal()) {
		// BUG BUG Could also implement persisting the settings here
		wxMessageBox("Settings to save", "Racing Plugin Options");
	}
	if (racingOptions != nullptr) {
		delete racingOptions;
	}
}

// Handle changes to OpenCPN Colour scheme
void RacingPlugin::SetColorScheme(PI_ColorScheme cs) {

	if ((cs == PI_GLOBAL_COLOR_SCHEME_DUSK) || (cs == PI_GLOBAL_COLOR_SCHEME_NIGHT)) {
		windWizard->SetNightMode(true);
	}
	else {
		windWizard->SetNightMode(false);
	}
}

// Drawing on the canvas when in multi canvas mode and not using OpenGL
bool RacingPlugin::RenderOverlayMultiCanvas(wxDC& dc, PlugIn_ViewPort* vp,
	int canvasIndex, int priority) {

	// Only draw in legacy mode
	if (priority == OVERLAY_LEGACY) {

		if (dc.IsOk()) {

			if ((canvasIndex == 0) || ((canvasIndex == 1) && (showMultiCanvas))) {

				if (showStartline) {

					// Draw a line on the chart to indicate the start line
					if ((!starboardMarkGuid.IsEmpty()) && (!portMarkGuid.IsEmpty())) {
						// Obtain the screen cordinates for the starboard and port ends of the start line
						wxPoint starboardScreenPoint, portScreenPoint;
						GetCanvasPixLL(vp, &starboardScreenPoint, starboardMarkLatitude, starboardMarkLongitude);
						GetCanvasPixLL(vp, &portScreenPoint, portMarkLatitude, portMarkLongitude);

						dc.SetPen(*wxBLACK_PEN);
						dc.DrawLine(starboardScreenPoint, portScreenPoint);
					}
				}

				if (showWindAngles) {

					// Example of using a graphics context, so we can use alpha channels, rotate text etc.
					wxMemoryDC* memoryDC;
					memoryDC = wxDynamicCast(&dc, wxMemoryDC);
					wxGraphicsContext* graphicsContext = wxGraphicsContext::Create(*memoryDC);

					// Draw an annular ring centred around the boat
					graphicsContext->SetPen(*wxBLACK_PEN);
					// The following specifies a light grey brush with an alpha channel (opacity/transparency)
					graphicsContext->SetBrush(wxColour(100, 100, 100, 50));
					wxGraphicsPath graphicsPath = graphicsContext->CreatePath();
					// Convert our current position to screen co-ordinates
					wxPoint boatLocation, ringLocation;
					GetCanvasPixLL(vp, &boatLocation, currentLatitude, currentLongitude);
					// Draw a transparent circle around the boat, the radius equal to the heading predictor length
					// Seems like there is no way to calculate a fixed length so given 1' of latitude = 1NM
					double oneMinuteAway = currentLatitude + (headingPredictorLength * 0.0166f);
					GetCanvasPixLL(vp, &ringLocation, oneMinuteAway, currentLongitude);
					graphicsPath.AddCircle(boatLocation.x, boatLocation.y, abs(boatLocation.y - ringLocation.y));
					graphicsContext->FillPath(graphicsPath);

					// Draw apparent wind angle centred around the boat
					if ((!isnan(apparentWindAngle)) && (!isnan(headingMagnetic))) {
						double drawnAngle = apparentWindAngle + headingMagnetic;
						if (drawnAngle < 0) {
							drawnAngle += 360.0;
						}
						drawnAngle = fmod(drawnAngle, 360.0);

						// Remember, 0 degress is at 3'oclock on the screen !
						drawnAngle -= 90;

						double radians;
						radians = drawnAngle * M_PI / 180;

						wxPoint2DDouble windArrow[3];
						windArrow[0].m_x = (cos(radians) * 10) + boatLocation.x;
						windArrow[0].m_y = (sin(radians) * 10) + boatLocation.y;
						windArrow[1].m_x = (cos(radians + 0.088) * abs(boatLocation.y - ringLocation.y)) + boatLocation.x;
						windArrow[1].m_y = (sin(radians + 0.088) * abs(boatLocation.y - ringLocation.y)) + boatLocation.y;
						windArrow[2].m_x = (cos(radians - 0.088) * abs(boatLocation.y - ringLocation.y)) + boatLocation.x;
						windArrow[2].m_y = (sin(radians - 0.088) * abs(boatLocation.y - ringLocation.y)) + boatLocation.y;

						// An example of drawing using a gradient.
						wxGraphicsGradientStops stops;
						stops.SetStartColour(wxColor(255, 153, 51)); // Orangle
						stops.SetEndColour(wxColour(255, 229, 204)); // Light Orange
						graphicsContext->SetBrush(graphicsContext->CreateLinearGradientBrush(windArrow[0].m_x, windArrow[0].m_y,
							windArrow[2].m_x, windArrow[2].m_y, stops));
						graphicsContext->SetPen(wxPen(wxColor(255, 153, 51), 1, wxPENSTYLE_SOLID));
						graphicsContext->DrawLines(WXSIZEOF(windArrow), windArrow);
						graphicsContext->Flush();
					}
				}

				if (showLayLines) {
					// BUG BUG ToDo
				}

			}
			return true;
		}
		wxLogDebug("Racing Plugin, Debug, Canvas DC not OK");
		return false;
	}
	wxLogDebug("Racing Plugin, Debug, OpenCPN not in legacy mode, %d", priority);
	return false;
}

bool RacingPlugin::RenderGLOverlayMultiCanvas(wxGLContext* pcontext, PlugIn_ViewPort* vp,
	int canvasIndex, int priority) {

	// BUG BUG No idea what the other priorities do?? OVERLAY_OVER_SHIPS, OVERLAY_OVER_UI, OVERLAY_OVER_EMBOSS
	if (priority == OVERLAY_OVER_EMBOSS) {

		if (pcontext->IsOK()) {

			RacingGraphics* rc = new RacingGraphics();

			if ((canvasIndex == 0) || ((canvasIndex == 1) && (showMultiCanvas))) {

				if (showStartline) {

					// Draw a line on the chart to indicate the start line
					if ((!starboardMarkGuid.IsEmpty()) && (!portMarkGuid.IsEmpty())) {
						// Obtain the screen cordinates for the starboard and port ends of the start line
						wxPoint starboardScreenPoint, portScreenPoint;
						GetCanvasPixLL(vp, &starboardScreenPoint, starboardMarkLatitude, starboardMarkLongitude);
						GetCanvasPixLL(vp, &portScreenPoint, portMarkLatitude, portMarkLongitude);
						rc->SetPen(*wxBLACK_PEN);
						rc->DrawLine(starboardScreenPoint.x, starboardScreenPoint.y,
							portScreenPoint.x, portScreenPoint.y, true);

						// Draw a true wind direction arrow centred on the start boat
						if (!isnan(trueWindDirection)) {
							double drawnAngle = trueWindDirection;
							if (drawnAngle < 0) {
								drawnAngle += 360.0;
							}
							drawnAngle = fmod(drawnAngle, 360.0);

							drawnAngle -= 90;

							double radians;
							radians = drawnAngle * M_PI / 180;

							wxPoint windArrow[4];
							windArrow[0].x = (cos(radians) * 10) + starboardScreenPoint.x;
							windArrow[0].y = (sin(radians) * 10) + starboardScreenPoint.y;
							windArrow[1].x = (cos(radians + 0.088) * 70) + starboardScreenPoint.x;
							windArrow[1].y = (sin(radians + 0.088) * 70) + starboardScreenPoint.y;
							windArrow[2].x = (cos(radians - 0.088) * 70) + starboardScreenPoint.x;
							windArrow[2].y = (sin(radians - 0.088) * 70) + starboardScreenPoint.y;
							windArrow[3].x = (cos(radians) * 10) + starboardScreenPoint.x;
							windArrow[3].y = (sin(radians) * 10) + starboardScreenPoint.y;

							rc->SetPen(wxPen(*wxBLUE, 1, wxPENSTYLE_SOLID));
							rc->SetBrush(*wxBLUE_BRUSH);
							rc->DrawLines(WXSIZEOF(windArrow), windArrow);
						}
					}
				}

				if (showWindAngles) {

					// Draw an annular ring centred around the boat with apparent wind direction indication
					rc->SetPen(*wxBLACK_PEN);
					rc->SetBrush(wxColour(100, 100, 100, 50));
					// Convert our current position to screen co-ordinates
					wxPoint boatLocation, ringLocation;
					GetCanvasPixLL(vp, &boatLocation, currentLatitude, currentLongitude);
					// Draw a transparent circle around the boat
					double oneMinuteAway = currentLatitude + (headingPredictorLength * 0.0166f);
					GetCanvasPixLL(vp, &ringLocation, oneMinuteAway, currentLongitude);
					rc->StrokeCircle(boatLocation.x, boatLocation.y, abs(boatLocation.y - ringLocation.y));

					// Draw apparent wind angle centred around the boat
					if ((!isnan(apparentWindAngle)) && (!isnan(headingMagnetic))) {
						double drawnAngle = apparentWindAngle + headingMagnetic;
						if (drawnAngle < 0) {
							drawnAngle += 360.0f;
						}
						drawnAngle = fmod(drawnAngle, 360.0f);

						drawnAngle -= 90;

						double radians;
						radians = drawnAngle * M_PI / 180;

						wxPoint windArrow[4];
						windArrow[0].x = (cos(radians) * 10) + boatLocation.x;
						windArrow[0].y = (sin(radians) * 10) + boatLocation.y;
						windArrow[1].x = (cos(radians + 0.088) * abs(boatLocation.y - ringLocation.y)) + boatLocation.x;
						windArrow[1].y = (sin(radians + 0.088) * abs(boatLocation.y - ringLocation.y)) + boatLocation.y;
						windArrow[2].x = (cos(radians - 0.088) * abs(boatLocation.y - ringLocation.y)) + boatLocation.x;
						windArrow[2].y = (sin(radians - 0.088) * abs(boatLocation.y - ringLocation.y)) + boatLocation.y;
						windArrow[3].x = (cos(radians) * 10) + boatLocation.x;
						windArrow[3].y = (sin(radians) * 10) + boatLocation.y;
						// Use different colours for different wind speed ranges
						// Previously used a single colour; orange 253, 153, 51
						//rc->SetPen(wxPen(wxColor(253, 153, 51), 1, wxPENSTYLE_SOLID));
						//rc->SetBrush(wxColor(253, 153, 51));
						
						if (apparentWindSpeed < 10) {
							rc->SetPen(wxPen(wxColor(255, 255, 155), 1, wxPENSTYLE_SOLID));
							rc->SetBrush(wxColor(255, 255, 155));
						}
						else if ((apparentWindSpeed >= 10) && (apparentWindSpeed < 15)) {
							rc->SetPen(wxPen(wxColor(0, 255, 0), 1, wxPENSTYLE_SOLID));
							rc->SetBrush(wxColor(0, 255, 0));
						}
						else if ((apparentWindSpeed >= 15) && (apparentWindSpeed < 20)) {
							rc->SetPen(wxPen(wxColor(0, 255, 255), 1, wxPENSTYLE_SOLID));
							rc->SetBrush(wxColor(0, 255, 255));
						}
						else if ((apparentWindSpeed >= 20) && (apparentWindSpeed < 25)) {
							rc->SetPen(wxPen(wxColor(0, 0, 255), 1, wxPENSTYLE_SOLID));
							rc->SetBrush(wxColor(0, 0, 255));
						}
						else {
							rc->SetPen(wxPen(wxColor(255, 155, 128), 1, wxPENSTYLE_SOLID));
							rc->SetBrush(wxColor(255, 155, 128));
						}
												
						// Draw the wind arrow
						rc->DrawPolygonTessellated(WXSIZEOF(windArrow), windArrow);
						//rc->DrawLines(WXSIZEOF(windArrow), windArrow);
					}
				}
				if (showLayLines) {
					// BUG BUG ToDo
				}
			}

			return true;
		}
		else {
			wxLogDebug("Racing Plugin, Debug, Canvas GLContext not OK");
			return false;
		}
	}
	else {
		wxLogDebug("Racing Plugin, Debug, OpenCPN not in Emboss Mode, Current Mode: %d", priority);
		return false;
	}
}

// When a route or waypoint is active, OpenCPN provides distance, bearing, waypoint name etc. to the waypoint
void RacingPlugin::SetActiveLegInfo(Plugin_Active_Leg_Info& pInfo) {
	waypointBearing = pInfo.Btw;
	// We also set this variable upon reception of OCPN_WPT... and OCPN_RTE... messages
	isWaypointActive = true;
	// Could be useful for the autopilot plugin
	wxLogMessage("Racing Plugin, SetActiveLegInfo: %s", pInfo.wp_name);
}

// Receive Position, Course, Speed and Heading from OpenCPN
// This has now probably been superceded by NavMsg listener
void RacingPlugin::SetPositionFixEx(PlugIn_Position_Fix_Ex& pfix) {

	//wxMutexLocker lock(lockPositionFix);
	currentLatitude = pfix.Lat;
	currentLongitude = pfix.Lon;
	courseOverGround = pfix.Cog;
	speedOverGround = pfix.Sog;
	headingTrue = pfix.Hdt;
	headingMagnetic = pfix.Hdm;
}

// The "old way" of receiving NMEA 0183 sentences
void RacingPlugin::SetNMEASentence(wxString& sentence) {

	NMEA0183 parserNMEA0183;
	parserNMEA0183 << sentence;
	// We'll handle a few "older" style NMEA 0183 sentences using this method
	if (parserNMEA0183.PreParse()) {
		// $IIVWR,048,L,23.9,N,12.3,M,044.2,K*4F
		if (parserNMEA0183.LastSentenceIDReceived == "VWR") {
			if (parserNMEA0183.Parse()) {
				apparentWindSpeed = fromUsrSpeed_Plugin(parserNMEA0183.Vwr.WindSpeedKnots, 0);
				apparentWindAngle = parserNMEA0183.Vwr.WindDirectionMagnitude;
				if (parserNMEA0183.Vwr.DirectionOfWind == LEFTRIGHT::Left) {
					apparentWindAngle = 360.0f - apparentWindAngle;
				}
			}
		}
		// $IIDBT,007.8,f,002.3,M,001.3,F*1D
		if (parserNMEA0183.LastSentenceIDReceived == "DBT") {
			if (parserNMEA0183.Parse()) {
				// Following depends on PR #4098
				// waterDepth = fromUsrDepth_Plugin(parserNMEA0183.Dbt.DepthMeters, 1);
				waterDepth = parserNMEA0183.Dbt.DepthMeters;
			}
		}
	}
}

// The Observable Listener Handlers
// In this plugin, all speed and distance variables are received from the various data sources and 
// stored in OpenCPN's default units. 
// They are then converted to the user's chosen display units in the "Wind Wizard" and Countdown Timer dialg
// Note that NMEA2000 and SignalK use SI units.

// When converting to the users display units such as fromUserSpeed_Plugin use the following enums
// which are found in navutil_base.h
// enum { SPEED_KTS = 0, SPEED_MPH, SPEED_KMH, SPEED_MS };
// enum { DISTANCE_NMI = 0,DISTANCE_MI,DISTANCE_KM,DISTANCE_M,DISTANCE_FT,DISTANCE_FA,DISTANCE_IN,DISTANCE_CM};
// enum { WSPEED_KTS = 0, WSPEED_MS, WSPEED_MPH, WSPEED_KMH };
// enum { DEPTH_FT = 0, DEPTH_M, DEPTH_FA };
// enum { TEMPERATURE_C = 0, TEMPERATURE_F = 1, TEMPERATURE_K = 2 };

// Handler for Navigation Data events 
void RacingPlugin::HandleNavData(ObservedEvt ev) {

	PluginNavdata navdata = GetEventNavdata(ev);
	// Save our current position and heading
	//wxMutexLocker lock(lockPositionFix);
	currentLatitude = navdata.lat;
	currentLongitude = navdata.lon;
	headingTrue = navdata.hdt;
	headingMagnetic = navdata.hdt - navdata.var;
}

// The "new" way of receiving NMEA 0183 sentences
// Parse NMEA 0183 Wind sentence
void RacingPlugin::HandleMWV(ObservedEvt ev) {

	NMEA0183Id id_183_mwv("MWV");
	NMEA0183 parserNMEA0183;
	wxString sentence = GetN0183Payload(id_183_mwv, ev);
	parserNMEA0183 << sentence;

	if (parserNMEA0183.Parse()) {
		if (parserNMEA0183.Mwv.WindSpeedUnits == 'N') { //Knots
			apparentWindSpeed = fromUsrSpeed_Plugin(parserNMEA0183.Mwv.WindSpeed, 0);
		}
		else if (parserNMEA0183.Mwv.WindSpeedUnits == 'K') { // Kilometres/hour
			apparentWindSpeed = fromUsrSpeed_Plugin(parserNMEA0183.Mwv.WindSpeed, 2);
		}
		else if (parserNMEA0183.Mwv.WindSpeedUnits == 'M') { //metres per second
			apparentWindSpeed = fromUsrSpeed_Plugin(parserNMEA0183.Mwv.WindSpeed, 3);
		}
		apparentWindAngle = parserNMEA0183.Mwv.WindAngle;
	}
}

// Parse NMEA 0183 Depth sentence
void RacingPlugin::HandleDPT(ObservedEvt ev) {

	NMEA0183Id id_183_dpt("DPT");
	NMEA0183 parserNMEA0183;
	wxString sentence = GetN0183Payload(id_183_dpt, ev);
	parserNMEA0183 << sentence;
	if (parserNMEA0183.Parse()) {
		// Following depends on PR #4098
		// waterDepth = fromUsrDepth_Plugin(parserNMEA0183.Dbt.DepthMeters, 1);
		waterDepth = parserNMEA0183.Dbt.DepthMeters;
	}
}

// Parse NMEA 0183 Speed through Water sentence
void RacingPlugin::HandleVHW(ObservedEvt ev) {

	NMEA0183Id id_183_vhw("VHW");
	NMEA0183 parserNMEA0183;
	wxString sentence = GetN0183Payload(id_183_vhw, ev);
	parserNMEA0183 << sentence;

	if (parserNMEA0183.Parse()) {
		// Convert from knots
		boatSpeed = fromUsrSpeed_Plugin(parserNMEA0183.Vhw.Knots, 0);
	}
}

// Parse NMEA 2000 Speed Through Water message
void RacingPlugin::HandleN2K_128259(ObservedEvt ev) {

	NMEA2000Id id_128259(128259);
	std::vector<uint8_t> payload = GetN2000Payload(id_128259, ev);
	unsigned char sid;
	double boatSpeedWaterReferenced;
	double boatSpeedGroundReferenced;
	tN2kSpeedWaterReferenceType waterReferenceType; // 0 = Paddlewheel

	if (ParseN2kPGN128259(payload, sid, boatSpeedWaterReferenced, boatSpeedGroundReferenced, waterReferenceType)) {
		// Convert from m/s to OpenCPN's core units
		boatSpeed = fromUsrSpeed_Plugin(boatSpeedWaterReferenced, 3);
	}
}

// Parse NMEA 2000 Water Depth message
void RacingPlugin::HandleN2K_128267(ObservedEvt ev) {

	NMEA2000Id id = ev.GetId();
	wxLogMessage(wxString::Format("Racing Plugin, NMEA 2000 PGN 128267 Id: %d", id.id));

	NMEA2000Id id_128267(128267);
	std::vector<uint8_t> payload = GetN2000Payload(id_128267, ev);
	unsigned char sid;
	double depthBelowTransducer;
	double transducerOffset;
	double maxRange;

	if (ParseN2kPGN128267(payload, sid, depthBelowTransducer, transducerOffset, maxRange)) {
		// Convert from metres to OpenCPN's core units
		// Following depends on PR #4098
		// waterDepth = fromUsrDepth_Plugin(depthBelowTransducer, 1);
		waterDepth = depthBelowTransducer;
	}
}

// Parse NMEA 2000 Wind message
void RacingPlugin::HandleN2K_130306(ObservedEvt ev) {

	NMEA2000Id id_130306(130306);
	std::vector<uint8_t> payload = GetN2000Payload(id_130306, ev);
	unsigned char sid;
	double windSpeed;
	double windAngle;
	tN2kWindReference windReferenceType;

	if (ParseN2kPGN130306(payload, sid, windSpeed, windAngle, windReferenceType)) {
		// Convert from m/s and radians to OpenCPN's core units
		apparentWindSpeed = fromUsrSpeed_Plugin(windSpeed, 3);
		apparentWindAngle = windAngle * 180 / M_PI;
	}
}

// Parse OpenCPN Core Messaging, New API in 1.19 Now working for both REST and Plugin Messaging
#if (OCPN_API_VERSION_MINOR >= 19)
void RacingPlugin::HandleMsgData(ObservedEvt ev) {
	PluginMsgId msg_id = PluginMsgId("OCPN_WPT_ACTIVATED");
	std::string message = GetPluginMsgPayload(msg_id, ev);
	isWaypointActive = true;
}
#endif

//
// Parse Signalk. 
#if (OCPN_API_VERSION_MINOR >= 19)
void RacingPlugin::HandleSignalK(ObservedEvt ev) {

	auto payload = GetSignalkPayload(ev);
	const auto msg = *std::static_pointer_cast<const wxJSONValue>(payload);
	// BUG BUG Should we bail out on errors ?
	auto errorCount = msg.ItemAt("ErrorCount");
	if (errorCount.AsInt() > 0) {
		wxLogMessage("Racing Plugin, SignalK Error Count: %d", errorCount.AsInt());
		// BUG BUG Should we log the error message
		return;
	}

	// Retrieve the Self Context and the SignalK Data
	wxJSONValue self = msg.ItemAt("ContextSelf");
	wxJSONValue data = msg.ItemAt("Data");
	
	// Only interested in displaying data for our own vessel
	if (data.HasMember("context") && data["context"].IsString()) {
		wxString context = data["context"].AsString();
		if (context != self.AsString()) {
			return;
		}

		// Parse the data
		if (data.HasMember("updates") && data["updates"].IsArray()) {
			wxJSONValue updates = data["updates"];
			for (int i = 0; i < updates.Size(); ++i) {
				HandleSKUpdate(updates[i]);
			}
		}
	}
}
#endif

// Receive & handle OpenCPN Messaging, the "Old" mechanism
void RacingPlugin::SetPluginMessage(wxString& message_id, wxString& message_body) {

	// Some known OCPN Messages
	// GLOBAL_SETTINGS_UPDATED
	// OpenCPN Config
	// OCPN_CORE_SIGNALK
	// OCPN_OPENGL_CONFIG
	// WMM_VARIATION_BOAT
	// WMM_VARIATION_BOAT_REQUEST
	// WMM_WINDOW_SHOWN
	// WMM_WINDOW_HIDDEN
	// AIS
	// OCPN_WPT_ACTIVATED
	// OCPN_WPT_DEACTIVATED
	// OCPN_WPT_ARRIVED
	// OCPN_RTE_ACTIVATED
	// OCPN_RTE_DEACTIVATED
	// OCPN_RTE_ENDED
	// OCPN_RTE_REQUEST
	// GRIB_VALUES

	wxJSONReader jsonReader;
	wxJSONValue root;
	// Parse navigation related messages to determine whether to 
	// display bearing to waypoint etc. 

	if (message_id == "OpenCPN Config") {

	}
	else if (message_id == "OCPN_WPT_CREATED") {
		// I think this was for testing a PR I had contemplated when a waypoint was added
		SendSystemNotification(message_body);
	}
	else if (message_id == "OCPN_OPENGL_CONFIG") {
	}
	else if (message_id == "WMM_VARIATION_BOAT") {
	}
	else if (message_id == "AIS") {
	}
	else if (message_id == "OCPN_RTE_ACTIVATED") {
		isWaypointActive = true;
	}
	else if (message_id == "OCPN_RTE_DEACTIVATED") {
		isWaypointActive = false;
	}
	else if (message_id == "OCPN_RTE_ENDED") {
		isWaypointActive = false;
	}
#if (OCPN_API_VERSION_MINOR <= 18)
	// For API 1.19 I'm now handling this with the new Msg Listener
	else if (message_id == "OCPN_WPT_ACTIVATED") {
		isWaypointActive = true;
	}
#endif
	else if (message_id == "OCPN_WPT_DEACTIVATED") {
		isWaypointActive = false;
	}
	else if (message_id == "OCPN_WPT_ARRIVED") {
		isWaypointActive = false;
	}
#if (OCPN_API_VERSION_MINOR <= 18)
	// Process SignalK messages, the "Old" way to receive SignalK data
	else if (message_id == "OCPN_CORE_SIGNALK") {
		if (jsonReader.Parse(message_body, &root) > 0) {
			wxLogMessage("Racing Plugin, JSON Error in following");
			wxLogMessage("%s", message_body);
			wxArrayString jsonErrors = jsonReader.GetErrors();
			for (auto it : jsonErrors) {
				wxLogMessage(it);
			}
			return;
		}

		// Upon initial connection, SignalK identifies the vessels for which it stores information
		// It identifies "self" with a unique id. We only want to receive updates for the "self" context
		// "self":"urn:mrn:signalk:uuid:1cb1a66a-814c-4478-8b84-701eec9524bb" which then is used to match
		// "context":"vessels.urn:mrn:signalk:uuid:1cb1a66a-814c-4478-8b84-701eec9524bb"
		if (root.HasMember("self")) {
			if (root["self"].AsString().StartsWith("vessels."))
				selfURN = (root["self"].AsString());  // for Java server, and OpenPlotter node.js server 1.20
			else
				selfURN = "vessels." + (root["self"].AsString()); // for Node.js server
		}

		if (root.HasMember("context") && root["context"].IsString()) {
			auto context = root["context"].AsString();
			if (context != selfURN) {
				return;
			}
		}

		if (root.HasMember("updates") && root["updates"].IsArray()) {
			wxJSONValue updates = root["updates"];
			for (int i = 0; i < updates.Size(); ++i) {
				HandleSKUpdate(updates[i]);
			}
		}
	}

#endif
	else {
		wxLogMessage("Racing Plugin, Debug, SetPluginMessage: %s, %s", message_id, message_body);
	}
}

// Parse SignalK updates
void RacingPlugin::HandleSKUpdate(wxJSONValue& update) {
	if (update.HasMember("values") && update["values"].IsArray()) {
		for (int j = 0; j < update["values"].Size(); ++j) {
			wxJSONValue& item = update["values"][j];
			HandleSKItem(item);
		}
	}
}

// Extract the SignalK values for apparent wind and boat speed
// SignalK uses SI units and radians
void RacingPlugin::HandleSKItem(wxJSONValue& item) {
	if (item.HasMember("path") && item.HasMember("value")) {
		const wxString& update_path = item["path"].AsString();
		wxJSONValue& value = item["value"];

		if (update_path.StartsWith("environment")) {
			if (update_path == "environment.wind.angleApparent") {
				// SignalK uses +/- Pi, convert to 0 - 360
				apparentWindAngle = value.AsDouble() * 180 / M_PI;
				if (apparentWindAngle < 0) {
					apparentWindAngle += 360.0f;
				}
			}
			if (update_path == "environment.wind.speedApparent") {
				apparentWindSpeed = fromUsrSpeed_Plugin(value.AsDouble(), 3);
			}
			if (update_path == "environment.depth.belowTransducer") {
				// Following depends on PR #4098
				// waterDepth = fromUsrDepth_Plugin(value.AsDouble(), 1);
				waterDepth = value.AsDouble();
			}
		}
		else if (update_path.StartsWith("navigation")) {
			if (update_path == "navigation.speedThroughWater") {
				boatSpeed = fromUsrSpeed_Plugin(value.AsDouble(), 3);
			}
		}
	}
}

// Generate NMEA 0183 MWV sentence with True Wind
void RacingPlugin::GenerateTrueWindSentence(void) {

	wxString sentence;
	// Generate the MWV sentence
	sentence = wxString::Format("$IIMWV,%.2f,T,%.2f,N,A", trueWindAngle, trueWindSpeed);
	// Append the checksum
	wxString checksum = ComputeChecksum(sentence);
	sentence.Append("*");
	sentence.Append(checksum);
	sentence.Append("\r\n");
	// Transmit the NMEA 0183 sentence, the "old" method
	// PushNMEABuffer(sentence);
	// The "new" method
	if (!n183NetworkHandle.empty()) {
		SendNMEA0183(sentence);
	}
}

// Transmit onto NMEA 0183 connection
void RacingPlugin::SendNMEA0183(wxString sentence) {
	CommDriverResult result;
	std::vector<uint8_t> payload;
	for (size_t i = 0; i < sentence.length(); i++) {
		payload.push_back(sentence.at(i));
	}

	debugSocket->SendTo(addrPeer, payload.data(), payload.size());

	auto sharedPointer = std::make_shared<std::vector<uint8_t> >(std::move(payload));
	result = WriteCommDriver(n183NetworkHandle, sharedPointer);
	
}

#if (OCPN_API_VERSION_MINOR >= 19)	
// FYI Note that the payload for PluginMessaging consists of id<space>message
// BUG BUG Doesn't work, see below
void RacingPlugin::SendOCPNMessage(PluginMsgId msg_id, wxString message) {
	CommDriverResult result;
	std::vector<uint8_t> payload;
	for (auto it : msg_id.id) {
		payload.push_back(it);
	}
	payload.push_back(' '); // Not the space between the id and the message is the delimiter
	for (auto it : message) {
		payload.push_back(it);
	}

	// BUG BUG No idea how to get the Comm Driver handle, "internal" does not appear in the list of drivers
	auto sharedPointer = std::make_shared<std::vector<uint8_t> >(std::move(payload));
	result = WriteCommDriver(ocpnMessageHandle, sharedPointer);
	wxLogMessage(_T("Racing Plugin, Debug, Send OCPN Message %s, %s, %d"), ocpnMessageHandle.c_str(), message, result);
}

#endif

// Generate NMEA 2000 PGN 130306 message with True Wind
void RacingPlugin::GenerateTrueWindMessage(void) {

	tN2kMsg N2kMsg;
	// Only Transmit if we have a valid NMEA 2000 connection
	if (!n2kNetworkHandle.empty()) {
		CommDriverResult result;
		SetN2kWindSpeed(N2kMsg, 1, trueWindSpeed, trueWindAngle, tN2kWindReference::N2kWind_True_boat);
		std::vector<uint8_t> payload(N2kMsg.Data, N2kMsg.Data + N2kMsg.GetAvailableDataLength());
		auto sharedPointer = std::make_shared<std::vector<uint8_t>>(payload);
		result = WriteCommDriverN2K(n2kNetworkHandle, 130306, 255, 5, sharedPointer);
		wxLogMessage(_T("Racing Plugin, Debug, Send NMEA 2000 PGN 130306, %s: %d"), n2kNetworkHandle.c_str(), result);
	}
}

// Generate the NMEA 0183 XOR checksum to be appended to an NMEA 0183 sentence
wxString RacingPlugin::ComputeChecksum(wxString sentence) {

	unsigned char calculatedChecksum = 0;
	for (wxString::const_iterator it = sentence.begin() + 1; it != sentence.end(); ++it) {
		calculatedChecksum ^= static_cast<unsigned char> (*it);
	}
	return(wxString::Format("%02X", calculatedChecksum));
}

// Update the "Wind Wizard" every second and generate True Wind messages/sentences
// BUG BUG This is where a pub/sub model would be interesting....
void RacingPlugin::OnTimerElapsed(wxTimerEvent& ev) {

	if (oneSecondTimer->IsRunning()) {
		CalculateTrueWind();
		CalculateDrift();
		if (windWizard != nullptr) {
			windWizard->SetTrueWindAngle(trueWindAngle);
			windWizard->SetTrueWindSpeed(trueWindSpeed);
			windWizard->SetApparentWindAngle(apparentWindAngle);
			windWizard->SetApparentWindSpeed(apparentWindSpeed);
			windWizard->SetBoatSpeed(boatSpeed);
			windWizard->SetWaterDepth(waterDepth);
			windWizard->SetMagneticHeading(headingMagnetic);
			windWizard->SetTrueHeading(headingTrue);
			windWizard->SetCOG(courseOverGround);
			windWizard->SetSOG(speedOverGround);
			// BUG BUG Ideological question; VMG or CMG
			windWizard->SetVMG(boatSpeed * cos(trueWindAngle));
			// course made good = boatSpeed * cos(headingTrue - headingMagnetic); 
			windWizard->SetDriftAngle(driftSpeed);
			windWizard->SetDriftSpeed(driftAngle);
			windWizard->ShowBearing(isWaypointActive);
			if (isWaypointActive) {
				windWizard->SetBearing(waypointBearing);
			}

			// Update the Gauge
			windWizard->Refresh();
		}

		// Generate NMEA 0183 and NMEA 2000 True Wind Messages
		if (generateMWVSentence) {
			GenerateTrueWindSentence();
		}
		if (generatePGN130306) {
			GenerateTrueWindMessage();
		}

		// Write to the loopback adapter
		// NMEA 0183 works
		//std::string sentence = "nmea0183 COM5 WIMWV $WIMWV,295.6,R,007.2,N,A*2E";
		
		// NMEA 2000 must be in Actisense format
		// Opportunity for candump to actisense utility, Finish the replay class

		// This is the "bastardised" output from VDR plugin, not sure what they are thinking...
		//std::string sentence = "$PCDIN,130306,93130402FD01FF020000000008580202FE4AFAFFFFB1";
		
		// This the correct format for Chetco Seasmart, although not supported by the loopback driver 
		//std::string sentence = "$PCDIN,01FD02,000C8472,09,04C300487FF8FFFF*55";

		// This format works (and is documented somewhere)!
		std::string sentence = "NMEA2000 COM5 130306 93 13 02 02 fd 01 ff 03 ff ff ff ff 08 24 cf 01 fe 4a fa ff ff 55";
		
		// This is the output from the Data Monitor but this format doesn't work
		//std::string sentence = "1760484669972,NMEA2000,COM5,130306,93 13 04 02 fd 01 ff 02 00 00 00 00 08 24 cf 01 fe 4a fa ff ff 55";
		
		// This is the CSV output from Data Monitor, also doesn't work
		//std::string sentence = "2025-10-15T01:11:07.765Z,NMEA2000,130306,93130402FD01FF0200000000081824cf01fe4afaffff";

		// This is from the spec, although it is the "wrong" spec, and doesn't work
		// std::string sentence = "2024-02-16T10:00:00.234Z,NMEA2000,130306,CAN0,15FD020A24cf01fe4afaffff ";
		
		CommDriverResult result;
		auto sharedPointer = std::make_shared<std::vector<unsigned char>>(sentence.begin(), sentence.end());
		result = WriteCommDriver(loopbackHandle, sharedPointer);
		wxLogMessage(_T("Racing Plugin, Debug, Send Loopback %s, %s, %d"), loopbackHandle.c_str(), sentence, result);

		// Every second could also take a screen capture
		// CreateScreenShot();
	}
}

// Perhaps of use to the folks investigating the use of a marine radar to track weather
// May have been of use for https://www.cruisersforum.com/forums/f134/two-things-288437.html
void RacingPlugin::CreateScreenShot() {

	wxClientDC clientDC(GetOCPNCanvasWindow());

	wxCoord screenWidth, screenHeight;
	clientDC.GetSize(&screenWidth, &screenHeight);

	// Create a bitmap to hold the screenshot image
	wxBitmap bitMap(screenWidth, screenHeight, -1);

	// Create a memory DC that will capture the screen
	wxMemoryDC memDC;

	memDC.SelectObject(bitMap);

	// Copy it
	memDC.Blit(0, 0, screenWidth, screenHeight, &clientDC, 0, 0);

	memDC.SelectObject(wxNullBitmap);

	bitMap.SaveFile(GetWritableDocumentsDir() + wxFileName::GetPathSeparators() + "screenshot.jpg", wxBITMAP_TYPE_JPEG);
}

void RacingPlugin::LoadSettings(void) {

	wxFileConfig* configSettings = GetOCPNConfigObject();

	if (configSettings) {
		configSettings->SetPath("/PlugIns/RacingPlugin");
		configSettings->Read("StartLineBias", &showStartline, false);
		configSettings->Read("Laylines", &showLayLines, false);
		configSettings->Read("WindAngles", &showWindAngles, false);
		configSettings->Read("DualCanvas", &showMultiCanvas, false);
		configSettings->Read("Autopilot", &showAutopilot, false);
		configSettings->Read("StartTimer", &defaultTimerValue, 300);
		configSettings->Read("TackingAngle", &tackingAngle, 45);
		configSettings->Read("Visible", &isWindWizardVisible, false);
		configSettings->Read("SendNMEA2000Wind", &generatePGN130306, false);
		configSettings->Read("SendNMEA0183Wind", &generateMWVSentence, false);
		// Get the length of OpenCPN's Ship's Heading Predictor Length
		// The plugin uses it for determining the length of the apparent wind arrow on the canvas
		configSettings->SetPath("Settings");
		configSettings->Read("OwnshipHDTPredictorMiles", &headingPredictorLength, 1);
	}
}

void RacingPlugin::SaveSettings(void) {

	wxFileConfig* configSettings = GetOCPNConfigObject();

	if (configSettings) {
		configSettings->SetPath("/PlugIns/RacingPlugin");
		configSettings->Write("StartLineBias", showStartline);
		configSettings->Write("Laylines", showLayLines);
		configSettings->Write("DualCanvas", showMultiCanvas);
		configSettings->Write("Autopilot", showAutopilot);
		configSettings->Write("WindAngles", showWindAngles);
		configSettings->Write("TackingAngle", tackingAngle);
		configSettings->Write("StartTimer", defaultTimerValue);
		configSettings->Write("Visible", isWindWizardVisible);
		configSettings->Write("SendNMEA2000Wind", generatePGN130306);
		configSettings->Write("SendNMEA0183Wind", generateMWVSentence);
	}
}

// Handle events from the Countdown Timer dialog
void RacingPlugin::OnDialogEvent(wxCommandEvent& event) {

	switch (event.GetId()) {

	// https://stackoverflow.com/questions/22867654/enum-vs-constexpr-for-actual-static-constants-inside-classes

	// Keep the toolbar & canvas in sync with the display of the Countdown Timer dialog
	case RACE_DIALOG_CLOSED:
		if (!starboardMarkGuid.IsEmpty()) {
			DeleteSingleWaypoint(starboardMarkGuid);
		}
		if (!portMarkGuid.IsEmpty()) {
			DeleteSingleWaypoint(portMarkGuid);
		}

		isCountdownTimerVisible = false;
		SetToolbarItemState(racingToolbarId, isCountdownTimerVisible);
		break;

		// Drop temporary waypoints to represent port & starboard ends of the start line
		// Waypoint icons are found in \uidata\markicons
	case RACE_DIALOG_STBD: {
		PlugIn_Waypoint waypoint;
		waypoint.m_IsVisible = true;
		waypoint.m_MarkName = "Starboard";
		waypoint.m_IconName = "Marks-Race-Committee-Start-Boat";
		starboardMarkGuid = GetNewGUID();
		waypoint.m_GUID = starboardMarkGuid;
		waypoint.m_lat = currentLatitude; 
		waypoint.m_lon = currentLongitude;
		starboardMarkLatitude = currentLatitude;
		starboardMarkLongitude = currentLongitude;
		AddSingleWaypoint(&waypoint, false);
		break;
	}
	case (int)RACE_DIALOG_PORT: {
		PlugIn_Waypoint waypoint;
		waypoint.m_IsVisible = true;
		waypoint.m_MarkName = "Port";
		waypoint.m_IconName = "Marks-Race-Start";
		portMarkGuid = GetNewGUID();
		waypoint.m_GUID = portMarkGuid;
		waypoint.m_lat = currentLatitude;
		waypoint.m_lon = currentLongitude;
		portMarkLatitude = currentLatitude;
		portMarkLongitude = currentLongitude;
		AddSingleWaypoint(&waypoint, false);
		break;
	}
	default:
		event.Skip();
	}
}

void RacingPlugin::InitializeNetwork() {

	wxLogMessage("Racing Plugin, Initializing Network");

	// Retrieve the "new" loopback interface
	// Supercedes PushNMEABuffer and also supports NMEA 2000 and SignalK as inputs
	loopbackHandle = GetNetworkInterfaceV2("loopback");

	// Retrieve a NMEA 2000 network interface which is used to transmit
	// PGN 130306 with the calculated True Wind Angles and Speed.
	// This is an example of writing to the NMEA 2000 Network
	n2kNetworkHandle = GetNetworkInterfaceV2("nmea2000");

	// Retrieve a NMEA 0183 network interface which is used to transmit
	// MWV sentences with True Wind Angles and Speed.
	// This is an example of writing to the NMEA 0183 Network
	// in addition to PushNMEABuffer and to the loopback driver
	n183NetworkHandle = GetNetworkInterfaceV2("nmea0183");

	// Get an interface for internal OpenCPN messaging
	// BUG BUG Not implemented ??
	ocpnMessageHandle = GetNetworkInterfaceV2("internal");

	// For the NMEA 2000 interface plugins need to register what NMEA 2000 PGN's they transmit.
	// This is required for Actisense NGT-1 Adapters
	// Presumably results in a null operation (NOP) for other NMEA 2000 adapters
	if (!n2kNetworkHandle.empty()) {
		std::vector<int> transmittedPGN = { 130306 };
		RegisterTXPGNs(n2kNetworkHandle, transmittedPGN);
	}
}

// Retrieves the first interface for the selected protocol
DriverHandle RacingPlugin::GetNetworkInterfaceV2(std::string desiredProtocol) {

	wxLogMessage("Racing Plugin, Number of Active Drivers: %d", GetActiveDrivers().size());

	for (const auto& driver : GetActiveDrivers()) {
		const auto& attributes = GetAttributes(driver);
		// Debug Dump out all of teh key value pairs
		for (auto it : attributes) {
			wxLogMessage("Debug: Key: %s, Value: %s", it.first, it.second);
		}
		// If none of the std::map entries have a protocol attribute, do the next for loop iteration
		if (attributes.find("protocol") == attributes.end())
			continue;
		wxLogMessage("Network Interface V2, Protocol: %s", attributes.at("protocol"));
		if (attributes.at("protocol") == desiredProtocol) {
			// Found our requested protocol
			if (attributes.find("ioDirection")!= attributes.end()) {
				// Found a driver that supports output
				if ((attributes.at("ioDirection") == "IN/OUT") || (attributes.at("ioDirection") == "OUT")) {
					wxLogMessage("Network Interface V2, Using %s for %s", driver, desiredProtocol);
					return driver;
				}
			}
		}
	}
	wxLogMessage("Racing Plugin, No driver found supporting %s", desiredProtocol);
	return "";
}

// Attempting to find the internal message driver interface
// Using find_if 
void RacingPlugin::FindInternalMessageDriver(std::string infernalDriverName) {
	const auto& drivers = GetActiveDrivers();
	auto found = std::find_if(
		drivers.begin(), drivers.end(), [infernalDriverName](const DriverHandle& h) {
			return GetAttributes(h).at("protocol") == infernalDriverName;
		});
	if (found != drivers.end()) {
		wxMessageBox(wxString::Format("%s driver found", infernalDriverName));
	}
	else {
		wxMessageBox(wxString::Format("%s driver not found", infernalDriverName));
	}
}

/*
// An example using a lambda expression
DriverHandle RacingPlugin::GetNetworkInterfaceV3(std::string selectedProtocol) {

	bool foundProtocol = false;
	std::vector<DriverHandle> activeDrivers;
	activeDrivers = GetActiveDrivers();

	auto it1 = std::find_if(activeDrivers.begin(), activeDrivers.end(),
		[selectedProtocol](const auto& p) { return p.first == "Protocol" && p.second == selectedProtocol; });

	if (it1 != activeDrivers.end()) {
		foundProtocol = true;
	}

	auto it2 = std::find_if(activeDrivers.begin(), activeDrivers.end(),
		[](const auto& p) { return p.first == "ioDirection" && ((p.second == "OUT") || (p.second == "IN/OUT")); });
	if (foundProtocol) {
		if (it2 != activeDrivers.end()) {
			return *it2;
		}
		foundProtocol = false;
	}
}
*/

// Retrieves the first interface for the selected protocol, Dumps all the key/value pairs
// For NMEA 0183 it should also check if interface is an "output" interface
// Available protocols include "nmea2000", "SignalK", "nmea0183", 
// Note to self, Writing to SignalK is unsupported
DriverHandle RacingPlugin::GetNetworkInterfaceV1(std::string selectedProtocol) {
	wxLogMessage("Racing Plugin, Searching for: %s", selectedProtocol.c_str());
	bool foundProtocol = false;
	std::vector<DriverHandle> activeDrivers;
	activeDrivers = GetActiveDrivers();
	for (auto const& driver : activeDrivers) {
		wxLogMessage("Racing Plugin, Interface: %s", driver);
		for (auto const& attributes : GetAttributes(driver)) {
			// We've found a key named protocol
			if (attributes.first.compare("protocol") == 0) {
				// Now check the values
				wxLogMessage("Racing Plugin, Key: %s, Value: %s, Compare: %d, Find: %d", 
					attributes.first, attributes.second,
					selectedProtocol.compare(attributes.second), 
					selectedProtocol.find(attributes.second));
				// We've found a matching protocol
				if (selectedProtocol.find(attributes.second) != std::string::npos) {
					wxLogMessage("Racing Plugin, Selected Interface: %s for %s", driver, selectedProtocol);
					foundProtocol = true;
				}
			}

			else if (attributes.first == "netAddress") {
				wxLogMessage("Racing Plugin, Key: %s, IP Address: %s", attributes.first, attributes.second);
			}
			else if (attributes.first == "netPort") {
				wxLogMessage("Racing Plugin, Key: %s, Port: %s", attributes.first, attributes.second);
			}
			else if (attributes.first == "commPort") {
				wxLogMessage("Racing Plugin, Key: %s, Comm Port: %s", attributes.first, attributes.second);
			}
			else if (attributes.first == "ioDirection") {
				wxLogMessage("Racing Plugin, Key: %s, IO Direction: %s", attributes.first, attributes.second);
				// Check to see if it is an output
				if (foundProtocol) {
					if ((attributes.second == "OUT") || (attributes.second == "IN/OUT")) {
						wxLogMessage("Racing Plugin, Network Protocol %s using %s", selectedProtocol, driver);
						return driver;
					}
					foundProtocol = false;
				}
			}
			else if (attributes.first == "userComment") {
				wxLogMessage("Racing Plugin, Key: %s, User Comment: %s", attributes.first, attributes.second);
			}
			else if (attributes.first == "canAddress") {
				wxLogMessage("Racing Plugin, Key: %s, CAN Address: %s", attributes.first, attributes.second);
			}
			else { // An undefined attribute ?
				wxLogMessage("Racing Plugin, Unknown Key: %s, Value: %s", attributes.first, attributes.second);
			}
		}
	}

	wxLogMessage("Racing Plugin, No Networks supporting %s were found", selectedProtocol);
	return ""; 
}

// Adopted from Dashboard Tactics
void RacingPlugin::CalculateTrueWind() {

	if (apparentWindAngle < 180.0f) {
		trueWindAngle = 90.0f - (180.0f / M_PI * atan((apparentWindSpeed * cos(apparentWindAngle * M_PI /  180.0f) - boatSpeed) / (apparentWindSpeed * sin(apparentWindAngle * M_PI /  180.0f))));
	}
	else if (apparentWindAngle > 180.0f) {
		trueWindAngle = 360.0f - (90.0f - (180.0f / M_PI * atan((apparentWindSpeed * cos((180.0f- (apparentWindAngle -  180.0f)) * M_PI /  180.0f) - boatSpeed) / (apparentWindSpeed * sin((180.0f - (apparentWindAngle - 180.0f)) * M_PI / 180.0f)))));
	}
	else {
		trueWindAngle = 180.0f;
	}
	trueWindSpeed = sqrt(pow((apparentWindSpeed * cos(apparentWindAngle * M_PI / 180.0f)) - boatSpeed, 2) + pow(apparentWindSpeed * sin(apparentWindAngle * M_PI /  180.0f), 2));

	trueWindDirection = fmod(trueWindAngle + headingTrue, 360.0f);
}

// Another algorithm for calculating True Wind
void RacingPlugin::CalculateTrueWindV2() {

	double u, v;
	u = (boatSpeed * sin(headingTrue * M_PI / 180)) - (apparentWindSpeed * sin(apparentWindAngle * M_PI / 180.0f));
	v = (boatSpeed * cos(headingTrue * M_PI / 180.0f)) - (apparentWindSpeed * cos(apparentWindAngle * M_PI / 180.0f));
	trueWindSpeed = sqrt((u * u) + (v * v));
	trueWindAngle = atan(u / v) * 180 / M_PI;
}

void RacingPlugin::CalculateDrift() {
	// The diffference between COG, SOG and STW and HDG
	// Two ways of calculating, one using difference between projected positions from STW/HDG and COG/SOG
	// the other using law of cosines

	// https://sailing-blog.nauticed.org/coastal-navigation-the-math-behind-it/

	double gpsLatitude;
	double gpsLongitude;
	double headingLatitude;
	double headingLongitude;

	// Calculate the projected positions
	PositionBearingDistanceMercator_Plugin(currentLatitude, currentLongitude, headingTrue,
		boatSpeed, &headingLatitude, &headingLongitude);

	PositionBearingDistanceMercator_Plugin(currentLatitude, currentLongitude, courseOverGround,
		speedOverGround, &gpsLatitude, &gpsLongitude);

	// Calculate the direction and speed of the current
	// Note this doesn't take into account leeway nor the effect of heel
	DistanceBearingMercator_Plugin(headingLatitude, headingLongitude, gpsLatitude,
		gpsLongitude, &driftSpeed, &driftAngle);

	//wxLogMessage("Racing Plugin, Set: %0.02f, Drift: %0.02f", driftAngle, driftSpeed);
	
}


// Raise the wx32 Operating Specific platform specific notification
void RacingPlugin::SendSystemNotification(wxString message) {
	wxNotificationMessage* myNotification;
	myNotification = new wxNotificationMessage("Notification",
		"Message: " + message, parentWindow, wxICON_INFORMATION);
#ifdef __WXMSW__
	myNotification->MSWUseToasts();
#endif
	myNotification->Show();
}
