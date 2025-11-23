// Copyright(C) 2024 by Steven Adler
//
// This file is part of Racing Plugin, a plugin for OpenCPN.
//
// Racing Plugin is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// Racing Plugin is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with Racing Plugin. If not, see <https://www.gnu.org/licenses/>.
//

//
// Project: Racing Plugin
// Description: Autopilot display, rolling road, heading and rudder gauge
// Owner: twocanplugin@hotmail.com
// Date: 13/9/2025
// Version History: 
// 1.0 Initial Release

#include "racing_autopilot.h"

RacingAutopilot::RacingAutopilot(wxWindow* parent)
	: wxControl(parent, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxFULL_REPAINT_ON_RESIZE) {

#if defined (__WXMSW__)
	this->SetBackgroundStyle(wxBG_STYLE_PAINT); // is/was needed for Microsoft Windows
#endif

	parentWindow = parent;

	Connect(wxEVT_PAINT, wxPaintEventHandler(RacingAutopilot::OnPaint), NULL, this);
	Connect(wxEVT_ERASE_BACKGROUND, wxEraseEventHandler(RacingAutopilot::OnEraseBackground), NULL, this);
	Connect(wxEVT_SIZE, wxSizeEventHandler(RacingAutopilot::OnSize),NULL,this);

	SetMinSize(wxSize(250, 250));

}

RacingAutopilot::~RacingAutopilot() {
	Disconnect(wxEVT_SIZE, wxSizeEventHandler(RacingAutopilot::OnSize));
	Disconnect(wxEVT_PAINT, wxPaintEventHandler(RacingAutopilot::OnPaint));
	Disconnect(wxEVT_ERASE_BACKGROUND, wxEraseEventHandler(RacingAutopilot::OnEraseBackground));
}

void RacingAutopilot::OnEraseBackground(wxEraseEvent& WXUNUSED(evt)) {
	// intentionally a null op to avoid flickering
}

void RacingAutopilot::OnSize(wxSizeEvent& event) {
	//this->SetSize(event.GetSize());
}

void RacingAutopilot::SetHeading(double heading) {
	compassHeading = heading;
}

void RacingAutopilot::SetBearing(double bearing) {
	bearingToWaypoint = NormalizeHeading(bearing);
}

void RacingAutopilot::SetCrossTrackError(double xte) {
	crossTrackError = xte;
}

void RacingAutopilot::SetDistance(double distance) {
	distanceToWaypoint = distance;
}

void RacingAutopilot::SetSpeedOverGround(double sog) {
	speedOverGround = sog;
}

void RacingAutopilot::SetRudderAngle(double angle) {
	rudderAngle = angle;
}

void RacingAutopilot::SetDestinationName(wxString name) {
	destinationName = name;
}

void RacingAutopilot::SetNightMode(bool mode) {
	nightMode = mode;
}

// BUG BUG Should use some constants and only calculate values once
//const int clientW = GetClientSize().GetWidth();
//const int clientH = GetClientSize().GetHeight();

// Gauge layout constants
//constexpr double GAUGE_TOP_FRACTION    = 0.1;
//constexpr double GAUGE_WIDTH_FRACTION  = 0.6;
//constexpr double GAUGE_XORIGIN_FRACTION = 0.2;
void RacingAutopilot::OnPaint(wxPaintEvent& evt) {
	// To avoid flickering....
	wxAutoBufferedPaintDC dc(this);

	if (dc.IsOk()) {

		dc.Clear();

		wxGraphicsContext* gc = wxGraphicsContext::Create(dc);

		if (gc) {

			// BUG BUG Needs work
			if (nightMode) {
				SetBackgroundColour(*wxBLACK);
			}
			else {
				SetBackgroundColour(*wxWHITE);
			}

			const int xCentre = static_cast<int>(GetClientSize().GetWidth() / 2);
			const int yCentre = static_cast<int>(GetClientSize().GetHeight() / 2);

			// BUG BUG Scaling of fonts as the gauge is resized
			wxFont textFont = wxFont(wxFontInfo(8).Family(wxFONTFAMILY_SWISS));
			dc.SetFont(textFont);

			// Used to calculate text widths and heights
			wxCoord textWidth;
			wxCoord textHeight;
			wxCoord textDescent = 0;
			wxCoord textLeading = 0;
			wxString label;

			// The rectangle for the gauge
			// BUG BUG Convert to wxPoint ?
			int xOrigin = static_cast<int>(GetClientSize().GetWidth() * 0.15);
			int yOrigin = static_cast<int>(GetClientSize().GetHeight() * 0.1);

			// BUG BUG Convert to wxSize ?
			int gaugeWidth = static_cast<int>(GetClientSize().GetWidth() * 0.7);
			int gaugeHeight = static_cast<int>(GetClientSize().GetHeight() * 0.1);

			DrawCompass(dc, wxPoint(xOrigin, yOrigin), wxSize(gaugeWidth, gaugeHeight), textFont);

			yOrigin = static_cast<int>(GetClientSize().GetHeight() * 0.8);

			DrawRudder(dc, wxPoint(xOrigin, yOrigin),
				wxSize(gaugeWidth, gaugeHeight), textFont);

			//wxPen savedPen = dc.GetPen();

			yOrigin = static_cast<int>(GetClientSize().GetHeight() * 0.3);
			gaugeHeight = GetClientSize().GetHeight() * 0.7;

			DrawHighway(dc, wxPoint(xOrigin, yOrigin), wxSize(gaugeWidth, gaugeHeight), textFont);

			// Labels
			// BUG BUG Could allow the user to select what fields to use
			dc.SetPen(*wxBLACK_PEN);
			// BUG BUG Bold or larger ?
			textFont.MakeBold().MakeLarger();

			// Distance To Go
			label = "Distance";
			dc.GetTextExtent(label, &textWidth, &textHeight, &textDescent, &textLeading, &textFont);
			dc.DrawText(label, xOrigin, yOrigin);

			label = CreateLabel(toUsrDistance_Plugin(distanceToWaypoint), getUsrDistanceUnit_Plugin());
			dc.GetTextExtent(label, &textWidth, &textHeight, &textDescent, &textLeading, &textFont);
			dc.DrawText(label, xOrigin, yOrigin + textHeight + 2);

			// Bearing To Waypoint
			label = "Bearing";
			dc.GetTextExtent(label, &textWidth, &textHeight, &textDescent, &textLeading, &textFont);
			dc.DrawText(label, xOrigin + gaugeWidth - textWidth, yOrigin);

			label = CreateLabel(bearingToWaypoint, "M");
			dc.GetTextExtent(label, &textWidth, &textHeight, &textDescent, &textLeading, &textFont);
			dc.DrawText(label, xOrigin + gaugeWidth - textWidth, yOrigin + textHeight + 2);

			// Cross Track Error
			label = CreateLabel(toUsrDistance_Plugin(abs(crossTrackError)), getUsrDistanceUnit_Plugin());
			dc.GetTextExtent(label, &textWidth, &textHeight, &textDescent, &textLeading, &textFont);
			dc.DrawText(label, crossTrackError > 0.0 ? xCentre - (2 * textWidth) : xCentre + textWidth,
				gaugeHeight - textHeight - 2);

			gc->Flush();
			delete gc;
		}
	}
}

// Draws a linear compass gauge
void RacingAutopilot::DrawCompass(wxAutoBufferedPaintDC& dc, const wxPoint& origin, const wxSize& size, wxFont& font) {

	wxSize textSize;
	constexpr unsigned int compassExtent = 40; // +/- 20 degree scale
	unsigned int centreX = origin.x + (size.GetWidth() / 2 );
	unsigned int pixelsPerDegree = size.GetWidth() / compassExtent;
	unsigned int xOffset = std::round((size.GetWidth() - (compassExtent * pixelsPerDegree)) / 2);
	int displayHeading = NormalizeHeading(static_cast<unsigned int>(compassHeading));

	/*
	if (nightMode) {
		dc.SetTextForeground(*wxWHITE);
		dc.SetTextBackground(*wxBLACK);
		dc.SetPen(*wxBLACK_PEN);
	}
	else {
		dc.SetTextForeground(*wxBLACK);
		dc.SetTextBackground(*wxWHITE);
		dc.SetPen(*wxBLACK_PEN);
	}
	*/

	// The bounding rectangle
	dc.DrawRectangle(origin.x,origin.y,size.GetWidth(),size.GetHeight());

	// The size of the text to be drawn
	textSize = dc.GetTextExtent(wxString::Format("%d", displayHeading));

	
	dc.SetTextForeground(*wxWHITE);
	dc.SetBrush(*wxBLACK_BRUSH);

	// The vessel's heading, drawn in a rounded rectangle
	dc.DrawRoundedRectangle(centreX - (textSize.GetWidth() / 2) - 2, origin.y - textSize.GetHeight() - 2, 
		textSize.GetWidth() + 4, textSize.GetHeight() + 2, 3.0f);

	// The vessel's heading
	dc.DrawText(wxString::Format("%d", displayHeading),
		centreX - (textSize.GetWidth() / 2.0f), origin.y - textSize.GetHeight());

	dc.SetTextForeground(*wxBLACK);
	dc.SetBrush(*wxWHITE_BRUSH);

	// In case some labels are off the screen
	dc.SetClippingRegion(origin.x, origin.y, size.x, size.y);

	// Draw labels for headings from -20 to +20 degrees relative to Magnetic Heading
	for (int i = 0; i < compassExtent + 2; i++) {

		unsigned int degreeValue = displayHeading - 20 + i;
		unsigned int dashHeight = ComputeTickHeight(degreeValue, size.y);
		unsigned int x = origin.x + xOffset + (i * pixelsPerDegree);

		// Draw tick marks
		dc.DrawLine(x, origin.y + size.GetHeight(), x, origin.y + dashHeight);

		// Measure the text extent for the font
		textSize = dc.GetTextExtent(wxString::Format("%03d", degreeValue));

		// Draw text label every 10 degrees
		if (degreeValue % 10 == 0) {
			dc.DrawText(wxString::Format("%03d", degreeValue),  
				x - (textSize.GetWidth() / 2),  origin.y);
		}
	}
	dc.DestroyClippingRegion();	
}

// A Linear Rudder Angle display
void RacingAutopilot::DrawRudder(wxAutoBufferedPaintDC& dc, wxPoint& origin, wxSize& size, wxFont& font) {
	
	// The bounding rectangle
	dc.DrawRectangle(origin.x, origin.y, size.x, size.y);

	// Rudder Angle display extends from +/- 40 degrees (labels only at +/- 30)
	constexpr int rudderExtent = 40;

	// length representing the rudder angle
	double length = (size.x / 2) * (rudderAngle / rudderExtent);

	if (rudderAngle < 0) {
		dc.SetPen(*wxRED_PEN);
		dc.SetBrush(*wxRED_BRUSH);

		dc.DrawRectangle(origin.x + ((size.x * 0.5) - abs(length)), origin.y + (size.y / 2),
			abs(length), size.y / 2 );
	}
	else {
		dc.SetPen(*wxGREEN_PEN);
		dc.SetBrush(*wxGREEN_BRUSH);
		dc.DrawRectangle(origin.x + (size.x * 0.5), origin.y + (size.y / 2), length, size.y / 2);
	}

	// Draw a scale at 10 degree intervals from +/- 30
	wxSize textSize;
	int pixelsPerDegree = size.x / 8;

	dc.SetPen(*wxBLACK_PEN);
	
	for (int i = 1; i <= 7; i++) {
		// Draw the tick marks
		dc.DrawLine(origin.x + (i * pixelsPerDegree), origin.y + (size.y / 2),
			origin.x + (i * pixelsPerDegree), origin.y + size.y);

		// Measure the text extent for the font
		textSize = GetTextExtent(wxString::Format("%d", abs((i * 10) - 40)));

		// Draw the labels
		dc.DrawText(wxString::Format("%d", abs((i * 10) - 40)), 
			origin.x + (i * pixelsPerDegree) - (textSize.GetWidth() / 2), origin.y + 2);
	}
}

// Rolling Highway
void RacingAutopilot::DrawHighway(wxAutoBufferedPaintDC& dc, wxPoint& origin, wxSize& size, wxFont& font) {
		
	dc.SetPen(*wxBLACK_PEN);
	dc.SetBrush(*wxLIGHT_GREY_BRUSH);
	wxPoint centre;

	centre.x = origin.x + (size.GetWidth() / 2);
	centre.y = origin.y + (size.GetHeight() / 2);

	// A polygon to give a 3D effect for the rolling highway
	std::vector<wxPoint>  points;

	points.push_back(wxPoint(origin.x + size.GetWidth(), size.GetHeight()));

	points.push_back(wxPoint(origin.x + (size.GetWidth() - (size.GetWidth() * 0.3)),
		origin.y));

	points.push_back(wxPoint(origin.x + (size.GetWidth() * 0.3),
		origin.y));

	points.push_back(wxPoint(origin .x, size.GetHeight()));
	
	dc.DrawPolygon(points.size(), points.data());

	dc.SetPen(wxPen(wxColor(250, 50,0), 2, wxSOLID));
	dc.DrawLine(points.at(2), points.at(3));

	dc.SetPen(wxPen(wxColor(0, 160, 0), 2, wxSOLID));
	dc.DrawLine(points.at(0), points.at(1));

	// Middle Line
	dc.SetPen(wxPen(*wxWHITE, 2, wxLONG_DASH));
	dc.DrawLine(origin.x + (size.GetWidth() / 2),
		origin.y,
		origin.x + (size.GetWidth() / 2),
		size.GetHeight());

	// Port Side
	dc.SetPen(wxPen(wxColor(250, 50, 0), 2, wxLONG_DASH));
	dc.DrawLine(origin.x + (size.GetWidth() * 0.4), origin.y,
		(origin.x + (size.GetWidth() * 0.25)), size.GetHeight());

	// Starboard Side
	dc.SetPen(wxPen(wxColor(0,160,0), 2, wxLONG_DASH));
	dc.DrawLine(origin.x + (size.GetWidth() * 0.6), origin.y ,
		origin.x + (size.GetWidth() * 0.75), size.GetHeight());

	// Draw boat
	wxBitmap ownShip;
	wxString pluginFolder = GetPluginDataDir(PLUGIN_PACKAGE_NAME) + wxFileName::GetPathSeparator() + "data" + wxFileName::GetPathSeparator();
	ownShip = GetBitmapFromSVGFile(pluginFolder + "ownship.svg", 32, 48);

	// An artificial scale of +/- 2Nm
	int offset;
	if (crossTrackError == 0.0) {
		offset = origin.x + (size.GetWidth() / 2);
	}
	else if (abs(crossTrackError) > 2.0) {
		crossTrackError > 0.0 ? offset = origin.x : offset = origin.x + size.GetWidth();
	}
	else {
		offset = centre.x - ((crossTrackError / 2) * (size.GetWidth() / 2));
	}
	dc.DrawBitmap(ownShip, wxPoint( offset,	centre.y - 48));
}

// Determine height for a tick mark (actually it is the amount of whitespace)
unsigned int RacingAutopilot::ComputeTickHeight(const unsigned int value, const unsigned int height) {
	if (value % 10 == 0) {
		return static_cast<unsigned int>(height * 0.4);
	}
	if (value % 5 == 0) {
		return static_cast<unsigned int>(height * 0.6);
	}
	return static_cast<unsigned int>(height * 0.8);
}

// Generate a label, dashes if the value is unavailable/invalid
wxString RacingAutopilot::CreateLabel(double value, wxString units) {
	wxString result;
	if (isnan(value)) {
		result = wxString::Format("-- %s", units);
	}
	else {
		result = wxString::Format("%0.1f %s", value, units);
	}
	return result;
}

// Ensure headings are between 0 to 360
int RacingAutopilot::NormalizeHeading(int heading) {
	if (!isnan(heading)) {
		if (heading < 0) {
			heading += 360;
		}
		return heading % 360;
	}
	else {
		return 0;
	}
}