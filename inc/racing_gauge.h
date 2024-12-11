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

#ifndef WIND_WIZARD_H
#define WIND_WIZARD_H

// Pre compiled headers 
#include "wx/wxprec.h"

#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif

#include <wx/graphics.h>
#include <wx/dcbuffer.h>

#include "ocpn_plugin.h"

class WindWizard : public wxControl {
public:
	WindWizard(wxWindow* parent);
	virtual ~WindWizard();
	void SetMagneticHeading(double heading);
	void SetTrueHeading(double heading);
	void SetBearing(double bearing);
	void SetApparentWindAngle(double windAngle);
	void SetTrueWindAngle(double windAngle);
	void SetApparentWindSpeed(double windSpeed);
	void SetTrueWindSpeed(double windSpeed);
	void SetBoatSpeed(double boatSpeed);
	void SetWaterDepth(double waterDepth);
	void SetCOG(double cog);
	void SetSOG(double sog);
	void SetVMG(double vmg);
	void SetDriftSpeed(double driftSpeed);
	void SetDriftAngle(double driftAngle);
	void ShowBearing(bool show);
	void SetNightMode(bool mode);

protected:
	void OnPaint(wxPaintEvent& evt);
	void OnEraseBackground(wxEraseEvent& evt);
	void OnSize(wxSizeEvent& evt);

private:
	wxWindow* parentWindow;
	double CalculateOffset(double radius, double halfWidth);
	wxString CreateLabel(double value, wxString units);
	double NormalizeHeading(double& heading);
	double magneticHeading = 0.0f;
	double trueHeading = 0.0f;
	double bearingToWaypoint = 0.0f;
	double apparentWindAngle = 0.0f;
	double trueWindAngle = 0.0f;
	double trueWindDirection = 0.0f;
	double apparentWindSpeed = 0.0f;
	double trueWindSpeed = 0.0f;
	double boatSpeed = 0.0f;
	double waterDepth = 0.0f;
	double courseOverGround = 0.0f;
	double speedOverGround = 0.0f;
	double driftAngle = 0.0f;
	double driftSpeed = 0.0f;
	double velocityMadeGood;
	bool nightMode = false;
	bool displayBearingToWaypoint = false;
};
#endif