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
RacingPlugin::RacingPlugin(void *ppimgr) : opencpn_plugin_118(ppimgr), wxEvtHandler() {
	// Initialize the plugin bitmap
	wxString pluginFolder = GetPluginDataDir(PLUGIN_PACKAGE_NAME) + wxFileName::GetPathSeparator() + _T("data") + wxFileName::GetPathSeparator();

	pluginBitmap = GetBitmapFromSVGFile(pluginFolder + _T("racing_icon.svg"), 32, 32);
	
	// Initialize Advanced User Interface Manager (AUI)
	auiManager = GetFrameAuiManager();

	// One second timer updates the "Wind Wizard" gauge
	oneSecondTimer = new wxTimer();
	oneSecondTimer->Bind(wxEVT_TIMER, &RacingPlugin::OnTimerElapsed, this);
	oneSecondTimer->Start(1000, wxTIMER_CONTINUOUS);
}

// Destructor
RacingPlugin::~RacingPlugin(void) {

	if (oneSecondTimer->IsRunning()) {
		oneSecondTimer->Stop();
	}
	oneSecondTimer->Unbind(wxEVT_TIMER, &RacingPlugin::OnTimerElapsed, this);
	delete oneSecondTimer;
}

int RacingPlugin::Init(void) {
	// Maintain a reference to the OpenCPN window to use as the parent for the Race Start Window
	parentWindow = GetOCPNCanvasWindow();

	// Maintain a reference to the OpenCPN configuration object 
	configSettings = GetOCPNConfigObject();

	// Initialize Localization catalogs
	AddLocaleCatalog(_T("opencpn-race_start_display_pi"));

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

	// Add the toolbar button 
	// Note that OpenCPN does not implement the rollover state
	racingToolbar = InsertPlugInToolSVG(_T(PLUGIN_COMMON_NAME), normalIcon, rolloverIcon, toggledIcon, wxITEM_CHECK, _("Race Start Display"), _T(""), NULL, -1, 0, this);

	racingWindowVisible = false;
	
	racingDialog = nullptr;
	racingWindow = nullptr;
	racingToolbox = nullptr;
	racingSettings = nullptr;

	// Example of adding a context menu item
	// This menu item is used to toggle the display of the "Wind Wizard" gauge
	wxMenuItem* myMenu = new wxMenuItem(NULL, wxID_HIGHEST + 1, _T("Wind Wizard"), wxEmptyString, wxITEM_NORMAL, NULL);
	racingContextMenuId = AddCanvasContextMenuItem(myMenu, this);

	// Set up the listeners. NMEA 0183, NMEA 2000 and SignalK are used to obtain data 
	// for boat speed, apparent wind angle & speed, NavData for position and heading

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

	// NMEA 0183 DPT Depth
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

	// SignalK Listerner
	wxDEFINE_EVENT(EVT_SIGNALK, ObservedEvt);
	SignalkId id_signalk = SignalkId("self");
	listener_SignalK = std::move(GetListener(id_signalk, EVT_SIGNALK, this));
	Bind(EVT_SIGNALK, [&](ObservedEvt ev) {
		HandleSignalK(ev);
		});

	// OpenCPN Core NavData
	wxDEFINE_EVENT(EVT_NAV_DATA, ObservedEvt);
	listener_nav = GetListener(NavDataId(), EVT_NAV_DATA, this);
	Bind(EVT_NAV_DATA, [&](ObservedEvt ev) {
		HandleNavData(ev);
		});

	// Retrieve a NMEA 2000 network interface which is used to transmit
	// PGN 130306 with the calculated True Wind Angles and Speed.
	// This is merely an example of writing to the NMEA 2000 Network
	n2kNetworkHandle = GetNetworkInterface("nmea2000");

	// Wire up the event handler to receive events from the race start dialog
	Connect(wxEVT_RACE_DIALOG_EVENT, wxCommandEventHandler(RacingPlugin::OnDialogEvent));


	// Notify OpenCPN what events we want to receive callbacks for
	return (WANTS_CONFIG | WANTS_PREFERENCES | INSTALLS_TOOLBOX_PAGE | 
		WANTS_TOOLBAR_CALLBACK | INSTALLS_TOOLBAR_TOOL | WANTS_NMEA_EVENTS |
		WANTS_PLUGIN_MESSAGING | USES_AUI_MANAGER | WANTS_LATE_INIT |
		WANTS_OVERLAY_CALLBACK | WANTS_OPENGL_OVERLAY_CALLBACK);
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

	// Cleanup the toolbox page here because OnSetupToolbox is only called once at Startup.
	// If we were to perform the cleanup in the OnCloseToolboxPane function, 
	// we can never initialize it again.
	DeleteOptionsPage(toolBoxWindow);
	delete toolBoxWindow;

	// Cleanup up the Count Down Timer Window
	if (racingWindowVisible) {
		//racingWindow->Finish();
		delete racingWindow;
	}

	// Unwire the handler for the Count Down Timer Window events 
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

// Add our own tab on the OpenCPN toolbox, under the "User" settings, requires INSTALLS_TOOLBOX_PAGE
// Ordinarily plugins would add their own settings dialog launched from the ShowPreferencesDialog method
void RacingPlugin::OnSetupOptions(void) {
	// Get a handle to our options page window, add a sizer to it, to which we will add our toolbox panel
	toolBoxWindow = AddOptionsPage(OptionsParentPI::PI_OPTIONS_PARENT_UI, _T(PLUGIN_COMMON_NAME));
	toolboxSizer = new wxBoxSizer(wxVERTICAL);
	toolBoxWindow->SetSizer(toolboxSizer);
	// Create our toolbox panel and add it to the toolbox via the sizer
	racingToolbox = new RacingToolbox(toolBoxWindow);
	toolboxSizer->Add(racingToolbox, 1, wxALL | wxEXPAND);
}

// I have no idea when this is called, supposedly when the plugin is first added
// but it seems like it is no longer implemented
void RacingPlugin::SetupToolboxPanel(int page_sel, wxNotebook* pnotebook) {
	//....wxMessageBox(wxString::Format(_T("SetupToolboxPanel: %d"), page_sel));
}

// Invoked when the OpenCPN Toolbox OK, Apply or Cancel buttons are pressed
// Requires INSTALLS_TOOLBOX_PAGE
void RacingPlugin::OnCloseToolboxPanel(int page_sel, int ok_apply_cancel) {
	// Why didn't they use standard enums like wxID_OK ??	
	if ((ok_apply_cancel == 0) || (ok_apply_cancel == 4)) {
		// Save the setttings
		if (configSettings) {
			configSettings->SetPath(_T("/PlugIns/RacingPlugin"));
			configSettings->Write(_T("StartLineBias"), showStartline);
			configSettings->Write(_T("Laylines"), showLayLines);
			configSettings->Write(_T("DualCanvas"), showMultiCanvas);
			configSettings->Write(_T("WindAngles"), showWindAngles);
			configSettings->Write(_T("StartTimer"), defaultTimerValue);
		}
	}
}

// Display Plugin preferences dialog. 
// This is probably the preferable way for plugins to configure their settings
void RacingPlugin::ShowPreferencesDialog(wxWindow* parent) {
	racingSettings = new RacingSettings(parent);

	// Kind of redundant as the settings are saved during deinit
	// However this demonstrates the use of plugin preferences
	// Could use getters & setters, or as I'm lazy, global variables
	if (racingSettings->ShowModal() == wxID_OK) {
		if (configSettings) {
			configSettings->SetPath(_T("/PlugIns/RacingPlugin"));
			configSettings->Write(_T("SendNMEA2000Wind"), generatePGN130306);
			configSettings->Write(_T("SendNMEA0183Wind"), generateMWVSentence);
		}
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
		wxLogDebug("Race Plugin, Canvas DC not OK");
		return false;
	}
	wxLogDebug("Race Plugin, OpenCPN not in legacy mode, %d", priority);
	return false;
}

bool RacingPlugin::RenderGLOverlayMultiCanvas(wxGLContext* pcontext, PlugIn_ViewPort* vp,
	int canvasIndex, int priority) {

	// BUG BUG No idea what the other priorities do?? OVERLAY_OVER_SHIPS, OVERLAY_OVER_UI,
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

						rc->SetPen(wxPen(wxColor(255, 153, 51), 1, wxPENSTYLE_SOLID));
						rc->SetBrush(wxColor(255, 153, 51));
						// Trying this just to see what it does...
						rc->DrawPolygonTessellated(WXSIZEOF(windArrow), windArrow);
					}
				}
				if (showLayLines) {
					// BUG BUG ToDo
				}
			}

			return true;
		}
		else {
			wxLogDebug("Race Plugin, Canvas GLContext not OK");
			return false;
		}
	}
	else {
		wxLogDebug("Race Plugin, OpenCPN not in Emboss Mode, Current Mode: %d", priority);
		return false;
	}
}


// When a route or waypoint is active, OpenCPN provides distance, bearing etc. to the waypoint
void RacingPlugin::SetActiveLegInfo(Plugin_Active_Leg_Info& pInfo) {
	// These global variables are also set in the OCPN_WPT... and OCPN_RTE... messages
	waypointActive = true;
	waypointBearing = pInfo.Btw;
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
	wxLogMessage(_T("Pfix: %0.3f, %0.3f %0.2f"), pfix.Lat, pfix.Lon, pfix.Hdt);
}

// The listeners
// In this plugin, all speed and distance variables are received from the various data sources and 
// stored in OpenCPN's default units. They are then converted to the user's chosen display units.
// Note that NMEA2000 and SignalK use SI units
// When using fromUserSpeed_Plugin use the following enums found in navutil_base.h
// enum { DISTANCE_NMI = 0,DISTANCE_MI,DISTANCE_KM,DISTANCE_M,DISTANCE_FT,DISTANCE_FA,DISTANCE_IN,DISTANCE_CM};
// enum { SPEED_KTS = 0, SPEED_MPH, SPEED_KMH, SPEED_MS };
// enum { WSPEED_KTS = 0, WSPEED_MS, WSPEED_MPH, WSPEED_KMH };
// enum { DEPTH_FT = 0, DEPTH_M, DEPTH_FA };
// enum { TEMPERATURE_C = 0, TEMPERATURE_F = 1, TEMPERATURE_K = 2 };
// Bearings/headings are in degrees

// The old way of receiving NMEA 0183 sentences
void RacingPlugin::SetNMEASentence(wxString& sentence) {
	NMEA0183 parserNMEA0183;
	parserNMEA0183 << sentence;
	// We'll handle a few "older" style NMEA 0183 sentences using this method
	if (parserNMEA0183.PreParse()) {
		// $IIVWR,048,L,23.9,N,12.3,M,044.2,K*4F
		if (parserNMEA0183.LastSentenceIDReceived == _T("VWR")) {
			if (parserNMEA0183.Parse()) {
				apparentWindSpeed = fromUsrSpeed_Plugin(parserNMEA0183.Vwr.WindSpeedKnots, 0);
				apparentWindAngle = parserNMEA0183.Vwr.WindDirectionMagnitude;
				if (parserNMEA0183.Vwr.DirectionOfWind == LEFTRIGHT::Left) {
					apparentWindAngle = 360.0f - apparentWindAngle;
				}
			}
		}
		// $IIDBT,007.8,f,002.3,M,001.3,F*1D
		if (parserNMEA0183.LastSentenceIDReceived == _T("DBT")) {
			if (parserNMEA0183.Parse()) {
				// Following depends on PR #4098
				// waterDepth = fromUsrDepth_Plugin(parserNMEA0183.Dbt.DepthMeters, 1);
				waterDepth = parserNMEA0183.Dbt.DepthMeters;
			}
		}
	}
}

// Handler for Navigation Data events 
void RacingPlugin::HandleNavData(ObservedEvt ev) {
	PluginNavdata navdata = GetEventNavdata(ev);
	// Save our current position and heading
	//wxMutexLocker lock(lockPositionFix);
	//currentLatitude = navdata.lat;
	//currentLongitude = navdata.lon;
	headingTrue = navdata.hdt;
	headingMagnetic = navdata.hdt - navdata.var;
	wxLogMessage(_T("NavData: %0.3f, %0.3f %0.2f"), navdata.lat, navdata.lon, navdata.hdt);
}

// Parse NMEA 0183 Wind sentence
void RacingPlugin::HandleMWV(ObservedEvt ev) {
	NMEA0183Id id_183_mwv("MWV");
	NMEA0183 parserNMEA0183;
	wxString sentence = GetN0183Payload(id_183_mwv, ev);
	parserNMEA0183 << sentence;

	// BUG BUG Really annoying that OpenCPN doesn't expose enums for the units in ocpn_plugin.h
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
		// Convert from m/s
		boatSpeed = fromUsrSpeed_Plugin(boatSpeedWaterReferenced, 3);
	}
}

// Parse NMEA 2000 Water Depth message
void RacingPlugin::HandleN2K_128267(ObservedEvt ev) {
	NMEA2000Id id_128267(128267);
	std::vector<uint8_t> payload = GetN2000Payload(id_128267, ev);

	unsigned char sid;
	double depthBelowTransducer;
	double transducerOffset;
	double maxRange;

	if (ParseN2kPGN128267(payload, sid, depthBelowTransducer, transducerOffset, maxRange)) {
		// Convert from cm, to metres, then to user's units
		waterDepth = depthBelowTransducer * 100;
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
		// Convert from m/s and radians
		apparentWindSpeed = fromUsrSpeed_Plugin(windSpeed, 3);
		apparentWindAngle = windAngle * 180 / M_PI;
	}
}

// Parse Signalk. Core OpenCPN has not yet implemented GetSignalKPayload
// See below for OCPN Messaging
void RacingPlugin::HandleSignalK(ObservedEvt ev) {
	//auto payload = GetSignalkPayload(ev);
	//const auto msg = *std::static_pointer_cast<const wxJSONValue>(payload);
}

// Receive & handle OpenCPN Messaging
// 
// As Core OpenCPN does not yet support GetSignalKPayload, obtain SignalK deltas from OCPN messaging
void RacingPlugin::SetPluginMessage(wxString& message_id, wxString& message_body) {
	wxJSONReader jsonReader;
	wxJSONValue root;

	// Process SignalK messages
	if (message_id == _T("OCPN_CORE_SIGNALK")) {
		wxString self;
		if (jsonReader.Parse(message_body, &root) > 0) {
			wxLogMessage("Race Plugin, JSON Error in following");
			wxLogMessage("%s", message_body);
			wxArrayString jsonErrors = jsonReader.GetErrors();
			for (auto it : jsonErrors) {
				wxLogMessage(it);
			}
			return;
		}

		if (root.HasMember("self")) {
			if (root["self"].AsString().StartsWith(_T("vessels.")))
				self = (root["self"].AsString());  // for Java server, and OpenPlotter node.js server 1.20
			else
				self = _T("vessels.") + (root["self"].AsString()); // for Node.js server
		}

		if (root.HasMember("context") && root["context"].IsString()) {
			auto context = root["context"].AsString();
			if (context != self) {
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
	// Parse navigation related messages to control whether to display laylines, bearing to waypoint etc.
	else if (message_id == _T("OCPN_RTE_ACTIVATED")) {
		waypointActive = true;
	}
	else if (message_id == _T("OCPN_RTE_DEACTIVATED")) {
		waypointActive = false;
	}
	else if (message_id == _T("OCPN_RTE_ENDED")) {
		waypointActive = false;
	}
	else if (message_id == _T("OCPN_WPT_ACTIVATED")) {
		waypointActive = true;
	}
	else if (message_id == _T("OCPN_WPT_DEACTIVATED")) {
		waypointActive = false;
	}
	else if (message_id == _T("OCPN_WPT_ARRIVED")) {
		waypointActive = false;
	}
}

// Process SignalK updates
void RacingPlugin::HandleSKUpdate(wxJSONValue& update) {
	if (update.HasMember("values") && update["values"].IsArray()) {
		for (int j = 0; j < update["values"].Size(); ++j) {
			wxJSONValue& item = update["values"][j];
			HandleSKItem(item);
		}
	}
}

// Extract the SignalK values for apparent wind and boat speed
// Note SignalK uses SI units and radians
// BUG BUG Check scaling factor
void RacingPlugin::HandleSKItem(wxJSONValue& item) {
	if (item.HasMember("path") && item.HasMember("value")) {
		const wxString& update_path = item["path"].AsString();
		wxJSONValue& value = item["value"];

		if (update_path.StartsWith("environment")) {
			if (update_path == _T("environment.wind.angleApparent")) {
				apparentWindAngle = value.AsDouble() * 180 / M_PI;
			}
			if (update_path == _T("environment.wind.speedApparent")) {
				apparentWindSpeed = fromUsrSpeed_Plugin(value.AsDouble(), 3);
			}
			if (update_path == _T("environment.depth.belowTransducer")) {
				// Following depends on PR #4098
				// waterDepth = fromUsrDepth_Plugin(100 * value.AsDouble(), 1);
				waterDepth = 100 * value.AsDouble();
			}
		}
		else if (update_path.StartsWith("navigation")) {
			if (update_path == "navigation.speedThroughWater") {
				boatSpeed = fromUsrSpeed_Plugin(value.AsDouble(), 3);
			}
		}
	}
}

// Generate True Wind NMEA 0183 Sentence
void RacingPlugin::GenerateTrueWindSentence(void) {
	wxString sentence;
	// Generate the MWV sentence
	sentence = wxString::Format("$IIMWV,%.2f,T,%.2f,N,A", trueWindAngle, trueWindSpeed);
	// Append the checksum
	wxString checksum = ComputeChecksum(sentence);
	sentence.Append(wxT("*"));
	sentence.Append(checksum);
	sentence.Append(wxT("\r\n"));
	// Send it to OpenCPN
	PushNMEABuffer(sentence);
	// Alternatively could have used the WriteCommDriver.
	// But need to check if it is received by other plugins
}

// Generate True Wind NMEA 2000 PGN 130306 message
void RacingPlugin::GenerateTrueWindMessage(void) {
	tN2kMsg N2kMsg;
	// Only Transmit if we have a valid NMEA 2000 connection
	if (n2kNetworkHandle != wxEmptyString) {
		SetN2kWindSpeed(N2kMsg, 1, trueWindSpeed, trueWindAngle, tN2kWindReference::N2kWind_True_boat);
		std::vector<uint8_t> payload(N2kMsg.Data, N2kMsg.Data + N2kMsg.GetAvailableDataLength());
		auto sharedPointer = std::make_shared<std::vector<uint8_t>>(payload);
		WriteCommDriverN2K(n2kNetworkHandle, 130306, 255, 5, sharedPointer);
	}
}

// Generate the NMEA 0183 XOR checksum to be appended to an NMEA 0183 sentence
wxString RacingPlugin::ComputeChecksum(wxString sentence) {
	unsigned char calculatedChecksum = 0;
	for (wxString::const_iterator it = sentence.begin() + 1; it != sentence.end(); ++it) {
		calculatedChecksum ^= static_cast<unsigned char> (*it);
	}
	return(wxString::Format(wxT("%02X"), calculatedChecksum));
}
// Update the "Wind Wizard" every second and generate True Wind messages/sentences
// BUG BUG This is where a pub/sub model would be interesting....
void RacingPlugin::OnTimerElapsed(wxEvent& ev) {
	if (oneSecondTimer->IsRunning()) {
		CalculateTrueWind();
		//CalculateDrift();

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
		windWizard->SetVMG(boatSpeed * cos(trueWindAngle));
		// course made good = boatSpeed * cos(headingTrue - headingMagnetic); 
		windWizard->SetDriftAngle(driftAngle);
		windWizard->SetDriftSpeed(driftSpeed);
		windWizard->ShowBearing(waypointActive);
		if (waypointActive) {
			windWizard->SetBearing(waypointBearing);
		}

		// Update the Gauge
		windWizard->Refresh();

		// Generate NMEA 0183 and NMEA 2000 True Wind Messages
		if (generateMWVSentence) {
			GenerateTrueWindSentence();
		}
		if (generatePGN130306) {
			GenerateTrueWindMessage();
		}
	}
	// Every second could also take a screen capture
	// CreateScreenShot();
}

// Perhaps of use to the folks investigating the use of a marine radar to track weather
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

// Handle events from the Race Start Dialog

void RacingPlugin::OnDialogEvent(wxCommandEvent& event) {
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
		waypoint.m_IconName = _T("Marks-Race-Committee-Start-Boat");
		starboardMarkGuid = GetNewGUID();
		waypoint.m_GUID = starboardMarkGuid;
		waypoint.m_lat = currentLatitude; 
		waypoint.m_lon = currentLongitude;
		starboardMarkLatitude = currentLatitude;
		starboardMarkLongitude = currentLongitude;
		AddSingleWaypoint(&waypoint, false);
		break;
	}
	case RACE_DIALOG_PORT: {
		PlugIn_Waypoint waypoint;
		waypoint.m_IsVisible = true;
		waypoint.m_MarkName = _T("Port");
		waypoint.m_IconName = _T("Marks-Race-Start");
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

wxString RacingPlugin::GetNetworkInterface(wxString selectedProtocol) {
	// Retrieves the first interface for the selected protocol
	// BUG BUG Ignores multiple interfaces. It should also check if interface is an "output" interface
	// Available protocols include "nmea2000", "SignalK", "nmea0183"
	// Note that writing to SignalK is unsupported
	std::vector<DriverHandle> activeDrivers;
	activeDrivers = GetActiveDrivers();
	for (auto const& activeDriver : activeDrivers) {
		wxLogMessage(_T("Race Plugin, Interface: %s"), activeDriver);
		for (auto const& driver : GetAttributes(activeDriver)) {
			if (driver.first == "protocol") {
				wxLogMessage(_T("Race Plugin, Type: %s, Protocol: %s"),
					driver.first, driver.second);
			}
			if (driver.first == "netAddress") {
				wxLogMessage(_T("Race Plugin, Type: %s, IP Address: %s"),
					driver.first, driver.second);
			}
			if (driver.first == "netPort") {
				wxLogMessage(_T("Race Plugin, Type: %s, Port: %s"),
					driver.first, driver.second);
			}
			if (driver.first == "commPort") {
				wxLogMessage(_T("Race Plugin, Type: %s, Comm Port: %s"),
					driver.first, driver.second);
			}
			if (driver.second == selectedProtocol) {
				wxLogMessage(_T("Race Plugin, Protocol %s using %s"), selectedProtocol, activeDriver);
				return activeDriver;
			}
		}
	}
	return wxEmptyString;
}

// Adopted from Dashboard Tactics
void RacingPlugin::CalculateTrueWind() {
	if (apparentWindAngle < 180.0f) {
		trueWindAngle = 90.0f - (180.0f / M_PI * atan((apparentWindSpeed * cos(apparentWindAngle * M_PI / 180.) - boatSpeed) / (apparentWindSpeed * sin(apparentWindAngle * M_PI / 180.))));
	}
	else if (apparentWindAngle > 180.0f) {
		trueWindAngle = 360.0f - (90.0f - (180.0f / M_PI * atan((apparentWindSpeed * cos((180. - (apparentWindAngle - 180.)) * M_PI / 180.) - boatSpeed) / (apparentWindSpeed * sin((180.0f - (apparentWindAngle - 180.0f)) * M_PI / 180.0f)))));
	}
	else {
		trueWindAngle = 180.0f;
	}
	trueWindSpeed = sqrt(pow((apparentWindSpeed * cos(apparentWindAngle * M_PI / 180.)) - boatSpeed, 2) + pow(apparentWindSpeed * sin(apparentWindAngle * M_PI / 180.), 2));

	trueWindDirection = fmod(trueWindAngle + headingTrue, 360.0f);
}

void RacingPlugin::CalculateDrift() {
	// The diffference between COG, SOG and STW and HDG
	// Two ways of calculating, one using difference between projected positions from STW/HDG and COG/SOG
	// the other using vector addition

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
		gpsLongitude, &driftAngle, &driftSpeed);

	wxLogMessage(_T("Racing Plugin, Drift: Angle %0.02f, Speed: %0,02f"), driftAngle, driftSpeed);

}
