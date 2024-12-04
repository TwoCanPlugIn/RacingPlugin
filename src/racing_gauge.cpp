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
// Description: "Wind Wizard", duplicate look of B&G Sail Steer
// Owner: twocanplugin@hotmail.com
// Date: 6/8/2024
// Version History: 
// 1.0 Initial Release

#include "racing_gauge.h"

WindWizard::WindWizard(wxWindow* parent)
	: wxControl(parent, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxFULL_REPAINT_ON_RESIZE) {

#if defined (__WXMSW__)
	this->SetBackgroundStyle(wxBG_STYLE_PAINT); // is/was needed for Microsoft Windows
#endif

	parentWindow = parent;

	Connect(wxEVT_PAINT, wxPaintEventHandler(WindWizard::OnPaint), NULL, this);
	Connect(wxEVT_ERASE_BACKGROUND, wxEraseEventHandler(WindWizard::OnEraseBackground), NULL, this);
	Connect(wxEVT_SIZE, wxSizeEventHandler(WindWizard::OnSize),NULL,this);

	SetMinSize(wxSize(250, 250));

}

WindWizard::~WindWizard() {
	Disconnect(wxEVT_SIZE, wxSizeEventHandler(WindWizard::OnSize));
	Disconnect(wxEVT_PAINT, wxPaintEventHandler(WindWizard::OnPaint));
	Disconnect(wxEVT_ERASE_BACKGROUND, wxEraseEventHandler(WindWizard::OnEraseBackground));

}

void WindWizard::OnEraseBackground(wxEraseEvent& WXUNUSED(evt)) {
	// intentionally a null op to avoid flickering
}

void WindWizard::OnSize(wxSizeEvent& event) {
	//this->SetSize(event.GetSize());
}

void WindWizard::SetMagneticHeading(double heading) {
	magneticHeading = NormalizeHeading(heading);
}

void WindWizard::SetTrueHeading(double heading) {
	trueHeading = NormalizeHeading(heading);
}

void WindWizard::SetBearing(double bearing) {
	bearingToWaypoint = NormalizeHeading(bearing);
}

void WindWizard::SetApparentWindAngle(double windAngle) {
	apparentWindAngle = NormalizeHeading(windAngle);
}

void WindWizard::SetTrueWindAngle(double windAngle) {
	trueWindAngle = NormalizeHeading(windAngle);
}

void WindWizard::SetApparentWindSpeed(double windSpeed) {
	apparentWindSpeed = windSpeed;
}
void WindWizard::SetTrueWindSpeed(double windSpeed) {
	trueWindSpeed = windSpeed;
}
void WindWizard::SetWaterDepth(double depth) {
	waterDepth = depth;
}
void WindWizard::SetBoatSpeed(double speed) {
	boatSpeed = speed;
}
void WindWizard::SetCOG(double cog) {
	courseOverGround = NormalizeHeading(cog);
}
void WindWizard::SetSOG(double sog) {
	speedOverGround = sog;
}
void WindWizard::SetVMG(double vmg) {
	velocityMadeGood = vmg;
}
void WindWizard::SetDriftAngle(double angle) {
	driftAngle = angle;
}

void WindWizard::SetDriftSpeed(double speed) {
	driftSpeed = speed;
}

void WindWizard::ShowBearing(bool show) {
	displayBearingToWaypoint = show;
}

void WindWizard::SetNightMode(bool mode) {
	nightMode = mode;
}

double WindWizard::NormalizeHeading(double& heading) {
	if (!isnan(heading)) {
		if (heading < 0) {
			heading += 360.0f;
		}
		return fmod(heading, 360.0f);
	}
	else {
		return 0.0;
	}
}

void WindWizard::OnPaint(wxPaintEvent& evt) {
	// To avoid flickering....
	wxAutoBufferedPaintDC dc(this);

	if (dc.IsOk()) {

		dc.Clear();

		wxGraphicsContext* gc = wxGraphicsContext::Create(dc);

		if (gc) {

			if (nightMode) {
				SetBackgroundColour(*wxBLACK);
			}
			else {
				SetBackgroundColour(*wxWHITE);
			}

			// Note to self, investigate device independent pixels
			// eg. dc.FromDIP(wxSize(x, y));

			double xCentre = GetClientSize().GetWidth() / 2.0f;
			double yCentre = GetClientSize().GetHeight() / 2.0f;

			// Determine the maximum size of our dial
			double radius = wxMin(xCentre, yCentre) * 0.8f;

			// Co-ordinates etc.
			double radians, offset;
			wxCoord xPos, yPos;

			// Use the Swiss font as it is TrueType and can be rotated
			// BUG BUG Scaling of fonts as the gauge is resized
			wxFont textFont = wxFont(wxFontInfo(8).Family(wxFONTFAMILY_SWISS));
			dc.SetFont(textFont);
			    		
			// Specify font and "dummy" label in order to calculate text widths and heights
			wxCoord textWidth;
			wxCoord textHeight;
			wxCoord textDescent = 0;
			wxCoord textLeading = 0;
			wxString label = "000";
			dc.GetTextExtent(label, &textWidth, &textHeight, &textDescent,&textLeading, &textFont);

			double outerRing = radius + textHeight;
			double innerRing = radius - textHeight;

			// Make an annular ring for the compass rose
			if (nightMode) {
				gc->SetPen(wxPen(*wxWHITE, 2));
				gc->SetBrush(*wxGREY_BRUSH);
			}
			else {
				gc->SetPen(wxPen(*wxBLACK, 2));
				gc->SetBrush(*wxWHITE_BRUSH);
			}

			wxGraphicsPath compassRose = gc->CreatePath();
			compassRose.AddCircle(xCentre, yCentre, radius);
			compassRose.AddCircle(xCentre, yCentre, innerRing);
			gc->StrokePath(compassRose);
			gc->FillPath(compassRose);

			// And another pair of annular rings for the wind rose
			wxGraphicsPath starboardWindGauge = gc->CreatePath();
			wxGraphicsPath portWindGauge = gc->CreatePath();
			wxGraphicsGradientStops starboardGradient, portGradient;

			starboardGradient.SetStartColour(wxColour(60, 255, 120));
			starboardGradient.SetEndColour(wxColour(60, 150, 60));
			portGradient.SetStartColour(wxColour(250, 0, 0));
			portGradient.SetEndColour(wxColour(140, 0, 0));

			gc->SetBrush(gc->CreateLinearGradientBrush(xCentre, yCentre - innerRing,
				xCentre, yCentre + outerRing, starboardGradient));

			starboardWindGauge.MoveToPoint(xCentre, yCentre - radius);
			starboardWindGauge.AddArc(xCentre, yCentre, radius, (3.0 / 2.0f) * M_PI, M_PI / 2.0f, true);
			starboardWindGauge.AddLineToPoint(xCentre, yCentre + outerRing);
			starboardWindGauge.AddArc(xCentre, yCentre, outerRing, M_PI / 2.0f, (3.0 / 2.0f) * M_PI, false);
			starboardWindGauge.AddLineToPoint(xCentre, yCentre - radius);
			starboardWindGauge.CloseSubpath();

			gc->StrokePath(starboardWindGauge);
			gc->FillPath(starboardWindGauge);

			gc->SetBrush(gc->CreateLinearGradientBrush(xCentre, yCentre - innerRing,
				xCentre, yCentre + outerRing, portGradient));

			portWindGauge.MoveToPoint(xCentre, yCentre - radius);
			portWindGauge.AddArc(xCentre, yCentre, radius, (3.0 / 2.0f) * M_PI, M_PI / 2.0f, false);
			portWindGauge.AddLineToPoint(xCentre, yCentre + outerRing);
			portWindGauge.AddArc(xCentre, yCentre, outerRing, M_PI / 2.0f, (3.0 / 2.0f) * M_PI, true);
			portWindGauge.AddLineToPoint(xCentre, yCentre - radius);
			portWindGauge.CloseSubpath();

			gc->StrokePath(portWindGauge);
			gc->FillPath(portWindGauge);

			if (nightMode) {
				dc.SetPen(*wxWHITE_PEN);
				dc.SetTextForeground(*wxWHITE);
			}
			else {
				dc.SetPen(*wxBLACK_PEN);
				dc.SetTextForeground(*wxBLACK);
			}

			// Annotate the wind rose with tick marks and labels
			for (int degrees = 0; degrees < 360; degrees += 10) {

				radians = ((270 - degrees) * M_PI) / 180.0f;

				// Text labels are drawn at 30 degree intervals, except 0 & 180, otherwise just tick marks
				if (((degrees % 30) == 0) && (degrees != 0) && (degrees != 180) ) {
					// labels are 0 - 160 on both the port & starboard sides
					label = wxString::Format("%i", degrees < 180 ? degrees : 360 - degrees);
					dc.GetTextExtent(label, &textWidth, &textHeight, &textDescent, &textLeading, &textFont);

					offset = CalculateOffset(radius, textWidth / 2.0f);
					xPos = (cos(radians - offset) * outerRing) + xCentre;
					yPos = (sin(radians - offset) * outerRing) + yCentre;
					// Text rotation is from top left and in an anti clockwise direction for +ve values!!
					dc.DrawRotatedText(label, xPos, yPos, degrees);
				}
				else {
					radians = degrees * M_PI / 180.0f;
					wxCoord x1, y1, x2, y2;
					x1 = (cos(radians) * radius) + xCentre;
					y1 = (sin(radians) * radius) + yCentre;
					x2 = (cos(radians) * (outerRing)) + xCentre;
					y2 = (sin(radians) * (outerRing)) + yCentre;
					dc.DrawLine(x1, y1, x2, y2);
				}
			}

			// Similarly annotate the compass card with tick marks and labels
			// BUG BUG could refactor this together with the wind rose
			for (int degrees = 0; degrees < 360; degrees += 10) {

				radians = ((270 + degrees - magneticHeading) * M_PI) / 180.0f;

				// Text labels are drawn at 30 degree intervals, otherwise just tick marks
				if ((degrees % 30) == 0) {
					label = wxString::Format("%i", degrees);
					dc.GetTextExtent(label, &textWidth, &textHeight, &textDescent, &textLeading, &textFont);
					offset = CalculateOffset(radius, textWidth / 2.0f);
					xPos = (cos(radians - offset) * radius) + xCentre;
					yPos = (sin(radians - offset) * radius) + yCentre;
					dc.DrawRotatedText(label, xPos, yPos, magneticHeading - static_cast<double>(degrees));
				}
				else {
					wxCoord x1, y1, x2, y2;
					x1 = (cos(radians) * radius) + xCentre;
					y1 = (sin(radians) * radius) + yCentre;
					x2 = (cos(radians) * (innerRing)) + xCentre;
					y2 = (sin(radians) * (innerRing)) + yCentre;
					dc.DrawLine(x1, y1, x2, y2);
				}
			}
			
			// Draw the magnetic heading inside a rounded rectangle, located at 12 o'clock
			if (nightMode) {
				dc.SetTextForeground(*wxRED);
			}
			else {
				dc.SetTextForeground(*wxWHITE);
			}
			dc.SetBrush(*wxBLACK_BRUSH);
			dc.SetFont(textFont.Bold().MakeLarger());
			label = wxString::Format("%d", static_cast<int>(magneticHeading));
			dc.GetTextExtent(label, &textWidth, &textHeight, &textDescent, &textLeading, &dc.GetFont());
			// Note an extra 2 pixels space around the top, bottom & sides
			dc.DrawRoundedRectangle(xCentre - (textWidth / 2.0f) -2, yCentre - radius - 2, textWidth + 4, textHeight + 4, 3.0f);
			dc.DrawText(label, xCentre - (textWidth / 2.0f), yCentre - radius);
			// Restore normal font.
			if (nightMode) {
				dc.SetTextForeground(*wxRED);
			}
			else {
				dc.SetTextForeground(*wxBLACK);
			}
			dc.SetFont(textFont);

			// Draw an arrow to indicate Apparent Wind Angle
			double drawnAngle = apparentWindAngle;
			if (drawnAngle > 360) {
				drawnAngle -= 360;
			}

			// Remember 0 degrees is at 3 o'clock!
			drawnAngle -= 90; 

			radians = drawnAngle * M_PI / 180.0f;
			wxPoint2DDouble arrow[3];
			arrow[0].m_x = (cos(radians) * radius) + xCentre;
			arrow[0].m_y = (sin(radians) * radius) + yCentre;
			arrow[1].m_x = (cos(radians + 0.09f) * (outerRing)) + xCentre;
			arrow[1].m_y = (sin(radians + 0.09f) * (outerRing)) + yCentre;
			arrow[2].m_x = (cos(radians - 0.09f) * (outerRing)) + xCentre;
			arrow[2].m_y = (sin(radians - 0.09f) * (outerRing)) + yCentre;
			gc->SetPen(wxPen(wxColor(255, 153, 51), 1));
			gc->SetBrush(wxColor(255,153,51)); 
			gc->DrawLines(WXSIZEOF(arrow), arrow);

			// Similarly draw an arrow to indicate True Wind Angle
			drawnAngle = trueWindAngle;
			if (drawnAngle > 360) {
				drawnAngle -= 360;
			}

			drawnAngle -= 90;

			radians = drawnAngle * M_PI / 180.0f;
			
			arrow[0].m_x = (cos(radians) * radius) + xCentre;
			arrow[0].m_y = (sin(radians) * radius) + yCentre;
			arrow[1].m_x = (cos(radians + 0.09f) * (outerRing)) + xCentre;
			arrow[1].m_y = (sin(radians + 0.09f) * (outerRing)) + yCentre;
			arrow[2].m_x = (cos(radians - 0.09f) * (outerRing)) + xCentre;
			arrow[2].m_y = (sin(radians - 0.09f) * (outerRing)) + yCentre;
			gc->SetPen(wxPen(wxColor(51, 153, 255), 1));
			gc->SetBrush(wxColor(51, 153, 255));
			gc->DrawLines(WXSIZEOF(arrow), arrow);

			// Draw a yellow dot to indicate the bearing to the waypoint
			if (displayBearingToWaypoint) {
				drawnAngle = (bearingToWaypoint - magneticHeading - 90.0f);
				if (drawnAngle < 0) {
					drawnAngle = 360.0f + drawnAngle;
				}
				radians =  drawnAngle * M_PI / 180.0f;
				xPos = (cos(radians) * (outerRing)) + xCentre;
				yPos = (sin(radians) * (outerRing)) + yCentre;
				dc.SetBrush(*wxYELLOW_BRUSH);
				dc.DrawCircle(xPos, yPos, textHeight / 2.0f);
			}

			// Labels in the four corners of the gauge
			// BUG BUG Could allow the user to select what fields to use
			wxFont labelFont = dc.GetFont().Bold().Larger();
			dc.SetFont(labelFont);

			// Boat Speed
			label = CreateLabel(toUsrSpeed_Plugin(boatSpeed), getUsrSpeedUnit_Plugin());
			dc.GetTextExtent(label, &textWidth, &textHeight,0,0, &labelFont);
			dc.DrawText(label, 4, yCentre - radius - textHeight);

			label = "STW";
			dc.GetTextExtent(label, &textWidth, &textHeight, 0, 0, &labelFont);
			dc.DrawText(label, 4, yCentre - radius);

			// Velocity Made Good
			label = CreateLabel(toUsrSpeed_Plugin(velocityMadeGood), getUsrSpeedUnit_Plugin());
			dc.GetTextExtent(label, &textWidth, &textHeight, 0, 0, &labelFont);
			dc.DrawText(label, this->GetClientSize().GetWidth() - textWidth - 4, yCentre - radius - textHeight);

			label = "VMG";
			dc.GetTextExtent(label, &textWidth, &textHeight, 0, 0, &labelFont);
			dc.DrawText(label, this->GetClientSize().GetWidth() - textWidth - 4, yCentre - radius);

			// Speed Over Ground
			label = CreateLabel(toUsrSpeed_Plugin(speedOverGround), getUsrSpeedUnit_Plugin());
			dc.GetTextExtent(label, &textWidth, &textHeight, 0, 0, &labelFont);
			dc.DrawText(label, 4, yCentre + radius);

			label = "SOG";
			dc.GetTextExtent(label, &textWidth, &textHeight, 0, 0, &labelFont);
			dc.DrawText(label, 4, yCentre + radius - textHeight);

			// True Wind Speed
			label = CreateLabel(toUsrSpeed_Plugin(trueWindSpeed), getUsrSpeedUnit_Plugin());
			dc.GetTextExtent(label, &textWidth, &textHeight, 0, 0, &labelFont);
			dc.DrawText(label, this->GetClientSize().GetWidth() - textWidth - 4, yCentre + radius);

			label = "TWS";
			dc.GetTextExtent(label, &textWidth, &textHeight, 0, 0, &labelFont);
			dc.DrawText(label, this->GetClientSize().GetWidth() - textWidth - 4, yCentre + radius - textHeight);

			// Draw the Apparent Wind speed under the boat icon
			label = CreateLabel(toUsrSpeed_Plugin(apparentWindSpeed), wxEmptyString);
			dc.GetTextExtent(label, &textWidth, &textHeight, 0, 0, &labelFont);
			dc.DrawText(label, xCentre - (textWidth / 2.0f) , yCentre + (radius / 2.0f));
			dc.SetPen(*wxGREY_PEN);
			dc.SetBrush(*wxTRANSPARENT_BRUSH);
			dc.DrawRoundedRectangle(xCentre - (textWidth / 2.0f) - 2, yCentre + (radius / 2.0f) - 2, textWidth + 4, textHeight + 4, 3.0f);

			// Draw a boat icon
			wxGraphicsPath boatIcon = gc->CreatePath();
			double quarter = (radius / 4.0f);
			double threequarter = radius * 0.75f;
			double half = (radius / 2.0f);
			boatIcon.MoveToPoint(xCentre - quarter, yCentre + half);
			boatIcon.AddQuadCurveToPoint(xCentre - half, yCentre - quarter, xCentre, yCentre - threequarter);
			boatIcon.MoveToPoint(xCentre + quarter, yCentre + half);
			boatIcon.AddQuadCurveToPoint(xCentre + half, yCentre - quarter, xCentre, yCentre - threequarter);
			boatIcon.MoveToPoint(xCentre + quarter, yCentre + half);
			boatIcon.AddLineToPoint(xCentre - quarter, yCentre + half);
			if (nightMode) {
				gc->SetPen(wxPen(*wxWHITE, 2));
			}
			else {
				gc->SetPen(wxPen(*wxBLACK, 2));
			}
			gc->StrokePath(boatIcon);

			// Draw an arrow and label to indicate drift
			if (driftSpeed != 0) {
				wxPoint2DDouble currentArrow[7];
				double driftDirection = (driftAngle * M_PI / 180);

				currentArrow[0].m_x = xCentre + (radius * 0.4f * cos(driftDirection)); //.4
				currentArrow[0].m_y = yCentre + (radius * 0.4f * sin(driftDirection));
				currentArrow[1].m_x = xCentre + (radius * 0.2f * cos(driftDirection + 1.5)); //.18
				currentArrow[1].m_y = yCentre + (radius * 0.2f * sin(driftDirection + 1.5));
				currentArrow[2].m_x = xCentre + (radius * 0.1f * cos(driftDirection + 1.5));
				currentArrow[2].m_y = yCentre + (radius * 0.1f * sin(driftDirection + 1.5));

				currentArrow[3].m_x = xCentre + (radius * 0.3f * cos(driftDirection + 2.8)); //.3
				currentArrow[3].m_y = yCentre + (radius * 0.3f * sin(driftDirection + 2.8));
				currentArrow[4].m_x = xCentre + (radius * 0.3f * cos(driftDirection - 2.8));
				currentArrow[4].m_y = yCentre + (radius * 0.3f * sin(driftDirection - 2.8));

				currentArrow[5].m_x = xCentre + (radius * 0.1f * cos(driftDirection - 1.5));
				currentArrow[5].m_y = yCentre + (radius * 0.1f * sin(driftDirection - 1.5));
				currentArrow[6].m_x = xCentre + (radius * 0.2f * cos(driftDirection - 1.5)); //.18
				currentArrow[6].m_y = yCentre + (radius * 0.2f * sin(driftDirection - 1.5));

				wxGraphicsGradientStops driftGradient;
				driftGradient.SetStartColour(wxColour(0, 102, 255));
				driftGradient.SetEndColour(wxColour(179, 209, 255));
				gc->SetBrush(gc->CreateLinearGradientBrush(currentArrow[3].m_x, currentArrow[3].m_y,
					currentArrow[0].m_x, currentArrow[0].m_y, driftGradient));
				gc->SetPen(*wxTRANSPARENT_PEN);
				gc->DrawLines(WXSIZEOF(currentArrow), currentArrow);

				label = CreateLabel(driftSpeed, getUsrSpeedUnit_Plugin());
				dc.GetTextExtent(label, &textWidth, &textHeight, 0, 0, &labelFont);
				dc.DrawText(label, xCentre - (textWidth / 2.0f), yCentre);
			}
			
			gc->Flush();
			delete gc;
		}

	}
	
}

// Basic trig, given the radius & text extent width, calculate the internal angle
// So we can position the label correctly rotated and centred around the compass rose
double WindWizard::CalculateOffset(double radius, double halfWidth) {
	double hypotenuse;
	hypotenuse = sqrt((radius * radius) + (halfWidth * halfWidth));
	return acos(radius / hypotenuse); 
}

// Generate a label, dashes if the value is unavailable/invalid
wxString WindWizard::CreateLabel(double value, wxString units) {
	wxString result;
	if (isnan(value)) {
		result = wxString::Format("-- %s", units);
	}
	else {
		result = wxString::Format("%0.1f %s", value, units);
	}
	return result;
}