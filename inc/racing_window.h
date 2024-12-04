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

#ifndef RACING_WINDOW_H
#define RACING_WINDOW_H

// The wxFrame base class from which we are derived
// Note wxFormBuilder used to generate UI
#include "racing_windowbase.h"

// For the Stopwatch/Countdown timer
#include <wx/timer.h>

#include <algorithm>

// For OpenCPN User's display units
#include <ocpn_plugin.h>

// image for dialog icon
extern wxBitmap pluginBitmap;

// Events posted to plugin
extern const int RACE_DIALOG_CLOSED;
extern const int RACE_DIALOG_PORT;
extern const int RACE_DIALOG_STBD;
extern const wxEventType wxEVT_RACE_DIALOG_EVENT;

// Position data from OpenCPN
extern double currentLatitude;
extern double currentLongitude;
extern double courseOverGround;
extern double speedOverGround;

// Countdown Timer Value
extern int defaultTimerValue;

class RacingWindow : public RacingWindowBase {
	
public:
	RacingWindow(wxWindow* parent, wxEvtHandler *handler);
	~RacingWindow();

	// wxTimer used as countdown timer 
	wxTimer *stopWatch;
	void OnTimer(wxTimerEvent& event);
	
	// The plugin event handler to which we post events
	wxEvtHandler *eventHandlerAddress;

	// Cleanup
	void Close();

	
protected:
	//overridden methods from the base class
	void OnClose(wxCloseEvent& event);
	void OnReset(wxCommandEvent &event);
	void OnStart(wxCommandEvent &event);
	void OnPort(wxCommandEvent &event);
	void OnStarboard(wxCommandEvent &event);
	void OnCancel(wxCommandEvent &event);
		
private:
	void Initialize(void);
	void ResetTimer(void);
	// Countdown timer
	int totalSeconds;
	// Port and Starboard ends of the start line
	double starboardLatitude;
	double starboardLongitude;
	double portLatitude;
	double portLongitude;
	// Whether the ends of the start line have been "pinged".
	bool portMark;
	bool starboardMark;
	// Bearing of the start Line (from starboard to port end of the line)
	double startLineBearing;
	// Where we are projected to cross the start line.
	double intersectLatitude;
	double intersectLongitude;
	// Navigation Formula functions
	bool CalculateIntersection(double latitude1, double longitude1, double  bearing1, double latitude2, double longitude2, double bearing2, double *lat3, double *lon3);
	double BearingBetweenPoints(double latitude1, double longitude1, double latitude2, double longitude2);
	double HaversineFormula(double latutude1, double longitude1, double latitude2, double longitude2);
	double SphericalCosines(double latitude1, double longitude1, double latitude2, double longitude2);
	double Bearing(double latitude1, double longitude1, double latitude2, double longitude2);
	bool IsWithinBoundingBox(double latitude, double longitude);
	
	// Parent Window size
	int parentWidth;
	int parentHeight;
};

#endif
