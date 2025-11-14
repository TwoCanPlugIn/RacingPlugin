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

#include "racing_globals.h"

// Globally accessible variables used by the plugin, dialogs etc.

// Note speed, distance values are stored using OpenCPN defaut units,
// but should always be displayed in the user's chosen units
double currentLatitude = 0.0f;
double currentLongitude = 0.0f;
double courseOverGround = 0.0f;
double speedOverGround = 0.0f;
double headingTrue = 0.0f;
double headingMagnetic = 0.0f;
double boatSpeed = 0.0f;
double waterDepth = 0.0f;
double apparentWindSpeed = 0.0f;
double apparentWindAngle = 0.0f;
double trueWindDirection = 0.0f;
double trueWindAngle = 0.0f;
double trueWindSpeed = 0.0f;
double driftAngle = 0.0f;
double driftSpeed = 0.0f;

// Protect simultaneous read & write access to latitude and longitude
wxMutex lockPositionFix;

// Countdown timer dialog shown/hidden state
bool isCountdownTimerVisible = false;

// "Wind Wizard" gauge shown/hidden state
bool isWindWizardVisible = false;

// Bitmap used for both the plugin and dialogs
wxBitmap pluginBitmap;

// Some configuration settings
// If we draw the start line on the screen
bool showStartline = false;
// Not currently implemented
bool showLayLines = false;
// If we draw an arrow indicating apparent wind angle on the screen
bool showWindAngles = false;
// If we draw only on one canvas or on both when in multi canvas mode
bool showMultiCanvas = false;
// If we override the core Active Route vonsole with our own Autopilot Display
bool showAutopilot = false;
// If we calculate true wind angle and speed and transmit the NMEA 2000 Message
bool generatePGN130306 = false;
// If we calculate true wind angle and speed and transmit the NMEA MWV Sentence
bool generateMWVSentence = false;
// Not currently implemented
int tackingAngle = 0;
// Default value for the Countdown timer interval
int defaultTimerValue = 0;

// Events etc.
// Plugin receives events from the Countdown Timer dialog
wxEventType wxEVT_RACE_DIALOG_EVENT = wxNewEventType();
