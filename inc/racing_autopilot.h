// Copyright(C) 2024 by Steven Adler
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

#ifndef RACING_AUTOPILOT_H
#define RACING_AUTOPILOT_H

// Pre compiled headers 
#include "wx/wxprec.h"

#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif

#include <wx/graphics.h>
#include <wx/dcbuffer.h>
#include <wx/filename.h>

#include "ocpn_plugin.h"
#include "version.h"

class RacingAutopilot : public wxControl {
public:
	RacingAutopilot(wxWindow* parent);
	virtual ~RacingAutopilot();

	// The vessels heading
	void SetHeading(double compassHeading);
	// The bearing to teh next waypoint
	void SetBearing(double bearing);
	// Cross Track Error
	void SetCrossTrackError(double xte);
	// Distance to next waypoint
	void SetDistance(double distance);
	// Speed Over Ground
	void SetSpeedOverGround(double sog);
	// Rudder Angle
	void SetRudderAngle(double angle);
	// Waypoint name
	void SetDestinationName(wxString name);
	// Change the colours
	void SetNightMode(bool mode);

protected:
	void OnPaint(wxPaintEvent& evt);
	void OnEraseBackground(wxEraseEvent& evt);
	void OnSize(wxSizeEvent& evt);

private:
	wxWindow* parentWindow;
	double compassHeading = 0.0;
	double bearingToWaypoint = 0.0;
	double speedOverGround = 0.0;
	double crossTrackError = 0.0;
	double distanceToWaypoint = 0.0;
	double rudderAngle = 0.0;
	bool nightMode = false;
	wxString destinationName;
	wxString CreateLabel(double value, wxString units);
	int NormalizeHeading(int heading);
	unsigned int ComputeTickHeight(const unsigned int value, const unsigned int height);
	void DrawCompass(wxAutoBufferedPaintDC& dc, const wxPoint& origin, const wxSize& size, wxFont& font);
	void DrawRudder(wxAutoBufferedPaintDC& dc, wxPoint& origin, wxSize& size, wxFont& font);
	void DrawHighway(wxAutoBufferedPaintDC& dc, wxPoint& origin, wxSize& size, wxFont& font);
};
#endif
