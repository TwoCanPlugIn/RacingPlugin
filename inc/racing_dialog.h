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

#ifndef RACING_DIALOG_H
#define RACING_DIALOG_H

// wxWidgets Precompiled Headers
#include "wx/wxprec.h"

#ifndef  WX_PRECOMP
#include "wx/wx.h"
#endif

// The dialog base class from which we are derived
// Note wxFormBuilder used to generate UI
#include "racing_dialogbase.h"


#include <wx/timer.h>

#include <wx/log.h>

// image for dialog icon
extern wxBitmap *_img_racing_logo_32;

// Position data from OpenCPN
extern double currentLatitude;
extern double currentLongitude;
extern double courseOverGround;
extern double speedOverGround;

class RacingDialog : public RacingDialogBase {
	
public:
	RacingDialog(wxWindow* parent);
	~RacingDialog();
	// wxTimer used as countdown timer 
	wxTimer *stopWatch;
	void OnTimer(wxTimerEvent& event);
	
	
protected:
	//overridden methods from the base class
	void OnInit(wxInitDialogEvent& event);
	void OnClose(wxCloseEvent& event);
	void OnReset(wxCommandEvent &event);
	void OnStart(wxCommandEvent &event);
	void OnPort(wxCommandEvent &event);
	void OnStbd(wxCommandEvent &event);
	void OnCancel(wxCommandEvent &event);
	
private:
	void ResetTimer(void);
	// Race start countdown timer
	int totalSeconds;
	int seconds;
	int minutes;
	// Port and Starboard ends of the start line
	double starboardLatitude;
	double starboardLongitude;
	double portLatitude;
	double portLongitude;
	// Whether the marks have been "pinged".
	bool portMark;
	bool starboardMark;
	// Where we are projected to cross the start line.
	double intersectLatitude;
	double intersectLongitude;
	// Distance & Bearing functions
	bool CalculateIntersection(double latitude1, double longitude1, double  bearing1, double latitude2, double longitude2, double bearing2, double *lat3, double *lon3);
	double BearingBetweenPoints(double latitude1, double longitude1, double latitude2, double longitude2);
	double HaversineFormula(double latutude1, double longitude1, double latitude2, double longitude2);
	double SphericalCosines(double latitude1, double longitude1, double latitude2, double longitude2);
	double Bearing(double latitude1, double longitude1, double latitude2, double longitude2);
	bool IsWithinBoundingBox(double latitude, double longitude);
	
	// Parent Windows Size
	int parentWidth;
	int parentHeight;
};

#endif
