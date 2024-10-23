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


// Project: Racing Plugin
// Description: Race Start Display plugin for OpenCPN
// Owner: twocanplugin@hotmail.com
// Date: 6/1/2020
// Version History: 
// 1.0 Initial Release
//

#include "racing_window.h"

// Constructor and destructor implementation
RacingWindow::RacingWindow( wxWindow* parent, wxEvtHandler *handler) : RacingWindowBase(parent) {
	
	// Set the dialog's icon
	wxIcon icon;
	icon.CopyFromBitmap(pluginBitmap);
	RacingWindow::SetIcon(icon);
	parent->GetSize(&parentWidth, &parentHeight);

	// Maintain a reference to the parent event handler
	eventHandlerAddress = handler;

	Initialize();
}

RacingWindow::~RacingWindow() {
	// Nothing to do in the destructor
}

void RacingWindow::Initialize(void) {
	// Set larger and more readable fonts
	wxFont bigFont = labelSpeed->GetFont();
	bigFont.SetPointSize( 20 );
	bigFont.SetWeight(wxFONTWEIGHT_BOLD);
	labelSpeed->SetFont( bigFont );
	labelTimer->SetFont( bigFont );
	labelDistance->SetFont( bigFont );
	labelTTG->SetFont( bigFont );
		
	// Ensure the dialog is sized correctly	
	Fit();

	// And move to bottom right of the screen
	int dialogWidth;
	int dialogHeight;
	this->GetSize(&dialogWidth, &dialogHeight);
	this->SetPosition(wxPoint(parentWidth-dialogWidth, parentHeight - dialogHeight));
		
	// Initialize the timer
	stopWatch = new wxTimer();
	stopWatch->Connect(stopWatch->GetId(), wxEVT_TIMER, wxTimerEventHandler(RacingWindow::OnTimer), NULL, this);
	ResetTimer();

	// Initialize state of whether we have pinged the port & starboard ends of the start line
	portMark = FALSE;
	starboardMark = FALSE;
}

void RacingWindow::OnClose(wxCloseEvent& event) {
	if (stopWatch->IsRunning()) {
		stopWatch->Stop();
	}

	stopWatch->Disconnect(stopWatch->GetId(), wxEVT_TIMER, wxTimerEventHandler(RacingWindow::OnTimer));

	if (stopWatch != nullptr) {
		delete stopWatch;
	}

	// Notify the parent we have closed, so that it can update the toolbar state
	racingWindowVisible = false;

	wxCommandEvent *closeEvent = new wxCommandEvent(wxEVT_RACE_DIALOG_EVENT, RACE_DIALOG_CLOSED);
	wxQueueEvent(eventHandlerAddress, closeEvent);

	event.Skip();
}


void RacingWindow::OnTimer(wxTimerEvent& event) {
	// Update the countdown timer
	totalSeconds -= 1;
	
	minutes = trunc(totalSeconds / 60);
	seconds = totalSeconds - (minutes * 60);
	
	// Display the count down timer
	labelTimer->SetLabel(wxString::Format("%1d:%02d",minutes, seconds));

	// Display our current speed
	// BUG BUG irrespective of the unit preferences knots, metres per second etc. OpenCPN always sends knots
	labelSpeed->SetLabel(wxString::Format("%02.2f Kts",speedOverGround));
	
	// If we've pinged each end of the start line
	if ((portMark) && (starboardMark)) {
			
		// Determine if our current course crosses the start line
		if (CalculateIntersection(starboardLatitude, starboardLongitude, startLineBearing, currentLatitude, currentLongitude, courseOverGround, &intersectLatitude, &intersectLongitude)) {
			
			if (IsWithinBoundingBox(intersectLatitude, intersectLongitude)) {
				// Calculate distance from our current position to where we intersect with the start line
				double distance = HaversineFormula(currentLatitude, currentLongitude, intersectLatitude, intersectLongitude);
				// BUG BUG currently only displays but perhaps should honour OpenCPN units ?
				labelDistance->SetLabel(wxString::Format("%5.2f m",distance * 1852));
				// Calculate how long to cross the start line, given our current speed.
				if (wxFinite(distance / speedOverGround) != 0) {
					labelTTG->SetLabel(wxString::Format("%5.2f s", (distance / speedOverGround) * 60));
				}
				else {
					// Not crossing start line at current speed
					labelTTG->SetLabel("...");
				}
			}
			else {
				 // Not crosing start line between port & starboard marks
				labelTTG->SetLabel("---");
				// Calculate distance from our current position to starborad mark
				double distance = HaversineFormula(currentLatitude, currentLongitude, starboardLatitude, starboardLongitude);
				// display in metres
				labelDistance->SetLabel(wxString::Format("%5.2f m",distance * 1852));
			}
		}
		else {
			// Current course does not cross start line 
			labelTTG->SetLabel("xxx");
			// Calculate distance from our current position to starborad mark
			double distance = HaversineFormula(currentLatitude, currentLongitude, starboardLatitude, starboardLongitude);
			// display in metres
			labelDistance->SetLabel(wxString::Format("%5.2f m",distance * 1852));
		}
	}
	else {
		// Have yet to ping each end of the start line
		labelTTG->SetLabel("<->");
		labelDistance->SetLabel("<->");
	}
}

void RacingWindow::OnReset(wxCommandEvent &event) {
	if (stopWatch->IsRunning()) {
		stopWatch->Stop();
	}
	ResetTimer();
}

void RacingWindow::OnStart(wxCommandEvent &event) {
	// BUG BUG Should have user definable count down timer setting, currently hardcoded to 5 minutes
	totalSeconds = 300;
	stopWatch->Start(1000, wxTIMER_CONTINUOUS);
} 

void RacingWindow::OnStarboard(wxCommandEvent &event) {
	// Save the position for the Starboard Mark
	starboardLatitude = currentLatitude; // Test data for my Menton log 43.75847
	starboardLongitude = currentLatitude; // Test data for my Menton log 7.49575
	starboardMark = TRUE;
	buttonStarboard->SetBackgroundColour(*wxGREEN);
	// Notify parent to drop a waypoint at the starboard end
	wxCommandEvent *commandEvent = new wxCommandEvent(wxEVT_RACE_DIALOG_EVENT, RACE_DIALOG_STBD);
	wxQueueEvent(eventHandlerAddress, commandEvent);
	// If we've pinged both ends calculate the bearing of the start line
	if (portMark && starboardMark) {
		startLineBearing = BearingBetweenPoints(starboardLatitude, starboardLongitude, portLatitude, portLongitude);
	}
}

void RacingWindow::OnPort(wxCommandEvent &event) {
	// Save the position for the Port Mark
	portLatitude = currentLatitude; // Test data for my Menton log 43.757188
	portLongitude = currentLongitude; // Test data for my Menton log 7.497963
	portMark = TRUE;
	buttonPort->SetBackgroundColour(*wxGREEN);	
	// Notify parent to drop a waypoint at the port end
	wxCommandEvent *commandEvent = new wxCommandEvent(wxEVT_RACE_DIALOG_EVENT, RACE_DIALOG_PORT);
	wxQueueEvent(eventHandlerAddress, commandEvent);
	// If we've pinged both ends calculate the bearing of the start line
	if (portMark && starboardMark) {
		startLineBearing = BearingBetweenPoints(starboardLatitude, starboardLongitude, portLatitude, portLongitude);
	}
}

void RacingWindow::OnCancel(wxCommandEvent &event) {
	this->Close();
}

void RacingWindow::ResetTimer(void) {
	// Reset the countdown timer for 5 minutes
	totalSeconds = 300;
	minutes = trunc(totalSeconds / 60);
	seconds = totalSeconds - (minutes * 60);
	labelTimer->SetLabel(wxString::Format("%1d:%02d",minutes, seconds));
}


bool RacingWindow::CalculateIntersection(double latitude1, double longitude1, double  bearing1, double latitude2, double longitude2, double bearing2, double *latitude3, double *longitude3) {
 // Returns the point of intersection of two paths defined by point and bearing.
 //
 // @param   {LatLon}      p1 - First point.
 // @param   {number}      brng1 - Initial bearing from first point.
 // @param   {LatLon}      p2 - Second point.
 // @param   {number}      brng2 - Initial bearing from second point.
 // @returns {LatLon|null} Destination point (null if no unique intersection defined).
 //
 // @example
 // const p1 = new LatLon(51.8853, 0.2545), brng1 = 108.547;
 // const p2 = new LatLon(49.0034, 2.5735), brng2 =  32.435;
 // const pInt = LatLon.intersection(p1, brng1, p2, brng2); // 50.9078°N, 004.5084°E

 // see www.edwilliams.org/avform.htm#Intersection

	double lat1 = latitude1 * M_PI / 180;
	double lon1 = longitude1 * M_PI / 180;
	double lat2 = latitude2 * M_PI / 180;
	double lon2 = longitude2 * M_PI / 180;
	double b1 = bearing1 * M_PI / 180;
	double b2 = bearing2 * M_PI / 180;
	double deltaLat = lat2 - lat1;
	double	deltaLon = lon2 - lon1;

	// angular distance point 1 - point 2
	double angularDistance = 2 * asin(sqrt(sin(deltaLat / 2) * sin(deltaLat / 2) + cos(lat1) * cos(lat2) * sin(deltaLon / 2) * sin(deltaLon / 2)));

	if (abs(angularDistance) < std::numeric_limits<double>::epsilon()) {
		return FALSE;//new LatLonSpherical(p1.lat, p1.lon); // coincident points
	}

	// initial/final bearings between points
	double initialBearingA = (sin(lat2) - sin(lat1)*cos(angularDistance)) / (sin(angularDistance)*cos(lat1));
	double initialBearingB = (sin(lat1) - sin(lat2)*cos(angularDistance)) / (sin(angularDistance)*cos(lat2));
	
	double finalBearingA = acos(std::min(std::max(initialBearingA, (double)-1), (double)1)); // protect against rounding errors
	double finalBearingB = acos(std::min(std::max(initialBearingB, (double)-1), (double)1)); // protect against rounding errors
	
	double bearingA = sin(lon2 - lon1) > 0 ? finalBearingA : 2 * M_PI - finalBearingA;
	double bearingB = sin(lon2 - lon1) > 0 ? 2 * M_PI - finalBearingB : finalBearingB;

	double angleA = b1 - bearingA; // angle 2-1-3
	double angleB = bearingB - b2; // angle 1-2-3

	if ((sin(angleA) == 0) && (sin(angleB) == 0)) {
		return FALSE; // infinite intersections
	}

	if ((sin(angleA) * sin(angleB) < 0)) {
		return FALSE; // ambiguous intersection (antipodal?)
	}

	double coslat3 = -cos(angleA)*cos(angleB) + sin(angleA)*sin(angleB)*cos(angularDistance);

	double tanlat3 = atan2(sin(angularDistance)*sin(angleA)*sin(angleB), cos(angleB) + cos(angleA)*coslat3);

	double lat3 = asin(sin(lat1)*cos(tanlat3) + cos(lat1)*sin(tanlat3)*cos(b1));

	double deltaLon3 = atan2(sin(b1)*sin(tanlat3)*cos(lat1), cos(tanlat3) - sin(lat1)*sin(lat3));
	double lon3 = lon1 + deltaLon3;

	*latitude3 = lat3 * 180 / M_PI;
	*longitude3 = lon3 * 180 / M_PI;

	return TRUE;

}
	

	
	// BUG BUG UNUSED
	//bearing
	//const p1 = new LatLon(52.205, 0.119);
	//const p2 = new LatLon(48.857, 2.351);
	//const b1 = p1.initialBearingTo(p2); // 156.2°
	//
	double RacingWindow::BearingBetweenPoints(double lat1, double lon1, double lat2, double lon2) {

		// tanθ = sindeltaLon⋅coslat2 / coslat1⋅sinlat2 − sinlat1⋅coslat2⋅cosdeltaLon
		// see mathforum.org/library/drmath/view/55417.html for derivation

		double theta1 = lat1 * M_PI / 180;
		double theta2 = lat2 * M_PI / 180;
		double delta = (lon2 - lon1) * M_PI / 180;;

		double x = cos(theta1) * sin(theta2) - sin(theta1) * cos(theta2) * cos(delta);
		double y = sin(delta) * cos(theta2);
		double bearing = atan2(y, x);

		return bearing * 180 / M_PI;;

		// or
		//return fmod(atan2(sin((lon1 * M_PI / 180 )- (lon2 * M_PI / 180))*cos((lat2 * M_PI / 180)), cos((lat1* M_PI / 180))*sin((lat2* M_PI / 180)) - sin((lat1* M_PI / 180))*cos((lat2* M_PI / 180))*cos((lon1* M_PI / 180) - (lon2* M_PI / 180))), (2 * M_PI));
	}

// Determine distamce between two points using the Haversine Formula
// Good reference http://www.movable-type.co.uk/scripts/latlong.html
double RacingWindow::HaversineFormula(double latutude1, double longitude1, double latitude2, double longitude2) {
	const double EARTHRADIUS = 3440;  //6371 Km, 3440 Nm
	double deltaLatitude = (latutude1 - latitude2) * M_PI / 180;
	double deltaLongitude = (longitude1 - longitude2) * M_PI / 180;
	double a = sin(deltaLatitude / 2) * sin(deltaLatitude / 2) + cos(latutude1 * M_PI / 180) * cos(latitude2 * M_PI / 180) * sin(deltaLongitude / 2) * sin(deltaLongitude / 2);
	double c = 2 * atan2(sqrt(a), sqrt(1 - a));
	return EARTHRADIUS * c;
}

// Another formula to determine distance between two points
double RacingWindow::SphericalCosines(double latitude1, double longitude1, double latitude2, double longitude2) {
	const double EARTHRADIUS = 3440;
	return acos(sin(latitude1 * M_PI / 180) * sin(latitude2 * M_PI / 180) + cos(latitude1 * M_PI / 180) * cos(latitude2 * M_PI / 180) * cos((longitude2 - longitude1) * M_PI / 180)) * EARTHRADIUS;
}

// Determine the bearing between two points.
double	RacingWindow::Bearing(double latitude1, double longitude1, double latitude2, double longitude2) {
	double deltaLongitude = (longitude2 - longitude1) * M_PI / 180;
	double y = sin(deltaLongitude) * cos(latitude2 * M_PI / 180);
	double x = cos(latitude1 * M_PI / 180) * sin(latitude2 * M_PI / 180) - sin(latitude1 * M_PI / 180) * cos(latitude2 * M_PI / 180) * cos(deltaLongitude);
	double z = atan2(y, x) * 180 / M_PI;
	return fmod((z + 360.0f),360.0f);
}

// When we have an intersection with the start line, calculated as a point & bearing, determine if the interstection is actually within the port & starboard marks
bool RacingWindow::IsWithinBoundingBox(double latitude, double longitude) {
	double startLineLength = HaversineFormula(portLatitude, portLongitude, starboardLatitude, starboardLongitude);
	if ((HaversineFormula(portLatitude, portLongitude, latitude, longitude) <= startLineLength) && (HaversineFormula(starboardLatitude, starboardLongitude, latitude, longitude) <= startLineLength)) {
		return TRUE;
	}
	else {
		return FALSE;
	}
}

// Some Test Data which appear to test correctly
	
	/*
	double result;

	result = Bearing(41.22000, 2.00000,41.22206,2.00250);
	wxMessageBox(wxString::Format(" A to B: %f", result));

	result = HaversineFormula(41.22000, 2.00000, 41.22206, 2.00250);
	wxMessageBox(wxString::Format("A To B: %f Nm, %f metres", result, result * 1852));

	result = Bearing(41.22000, 2.00000, 41.22000, 2.00249);
	wxMessageBox(wxString::Format(" A to F: %f", result));

	result = HaversineFormula(41.22000, 2.00000, 41.22000, 2.00249);
	wxMessageBox(wxString::Format("A To F: %f Nm, %f metres", result, result * 1852));

	result = Bearing(41.22000, 2.00000, 41.21890, 2.00123);
	wxMessageBox(wxString::Format(" A to E: %f", result));

	result = HaversineFormula(41.22000, 2.00000, 41.21890, 2.00123);
	wxMessageBox(wxString::Format("A To E: %f Nm, %f metres", result, result * 1852));

	result = Bearing(41.22206, 2.00250, 41.2200, 2.00500);
	wxMessageBox(wxString::Format(" B to C: %f", result));

	result = HaversineFormula(41.22206, 2.00250, 41.2200, 2.00500);
	wxMessageBox(wxString::Format("B To C: %f Nm, %f metres", result, result * 1852));

	result = Bearing(41.22206, 2.00250, 41.21890, 2.00123);
	wxMessageBox(wxString::Format(" B to E: %f", result));

	result = HaversineFormula(41.22206, 2.00250, 41.2189, 2.00123);
	wxMessageBox(wxString::Format("B To E: %f Nm, %f metres", result, result * 1852));

	result = Bearing(41.21890, 2.00123, 41.22000, 2.00500);
	wxMessageBox(wxString::Format("E to C: %f", result));

	result = HaversineFormula(41.2189, 2.00123, 41.22000, 2.00500);
	wxMessageBox(wxString::Format("E To C: %f Nm, %f metres", result, result * 1852));
	
	double bearingAtoF = Bearing(41.22000, 2.00000, 41.22000, 2.00249);

	double bearingBtoE = Bearing(41.22206, 2.00250, 41.21890, 2.00123);

	if (CalculateIntersection(41.22000, 2.00000, bearingAtoF, 41.22206, 2.00250, bearingBtoE, &intersectLatitude, &intersectLongitude)) {
		wxMessageBox(wxString::Format("Intersects at: %f,%f", intersectLatitude, intersectLongitude));
		double result = HaversineFormula(41.22206, 2.00250, intersectLatitude, intersectLongitude);
		wxMessageBox(wxString::Format("Distance to Start Line:  %f Nm, %f metres", result, result * 1852));
		if (IsWithinBoundingBox(intersectLatitude, intersectLongitude)) {
			wxMessageBox("Is Within Start Line");
		}
	}
	else {
		wxMessageBox("No intersection");
	}
	
	*/