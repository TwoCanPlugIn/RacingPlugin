#ifndef RACING_GLOBALS_H
#define RACING_GLOBALS_H

// wxWidgets Precompiled Headers
#include "wx/wxprec.h"

#ifndef  WX_PRECOMP
#include "wx/wx.h"
#endif

#include "wx/thread.h"

#include "wx/event.h"

// Globally accessible variables used by the plugin, dialogs etc.

// Note speed, distance values are stored using OpenCPN defaut units,
// but should always be displayed in the user's chosen units
extern double currentLatitude;
extern double currentLongitude;
extern double courseOverGround;
extern double speedOverGround;
extern double headingTrue;
extern double headingMagnetic;
extern double boatSpeed;
extern double waterDepth;
extern double apparentWindSpeed;
extern double apparentWindAngle;
extern double trueWindDirection;
extern double trueWindAngle;
extern double trueWindSpeed;
extern double driftAngle;
extern double driftSpeed;

// Protect simultaneous read & write access to latitude and longitude
extern wxMutex lockPositionFix;

// Countdown timer dialog shown/hidden state
extern bool isCountdownTimerVisible;

// "Wind Wizard" gauge shown/hidden state
extern bool isWindWizardVisible;

// Autopilot Also used to determine if dialog shown/hidden
extern bool isWaypointActive;

// Bitmap used for both the plugin and dialogs
extern wxBitmap pluginBitmap;

// Some configuration settings
// If we draw the start line on the screen
extern bool showStartline;
// Not currently implemented
extern bool showLayLines;
// If we draw an arrow indicating apparent wind angle on the screen
extern bool showWindAngles;
// If we draw only on one canvas or on both when in multi canvas mode
extern bool showMultiCanvas;
// If we calculate true wind angle and speed and transmit the NMEA 2000 Message
extern bool generatePGN130306;
// If we calculate true wind angle and speed and transmit the NMEA MWV Sentence
extern bool generateMWVSentence;
// If we override the core Active Route console with our own Autopilot Display
extern bool showAutopilot;;
// Not currently implemented
extern int tackingAngle;
// Default value for the Countdown timer interval
extern int defaultTimerValue;

// Autopilot AUI Pane
#define AUTOPILOT "AUTOPILOT_AUI_DIALOG"

// Events etc.
// Plugin receives events from the Countdown Timer dialog
extern wxEventType wxEVT_RACE_DIALOG_EVENT;

#define RACE_DIALOG_CLOSED  wxID_HIGHEST + 1 
#define RACE_DIALOG_PORT  wxID_HIGHEST + 2
#define RACE_DIALOG_STBD  wxID_HIGHEST + 3
#define WINDWIZARD_MENU  wxID_HIGHEST + 4
#define ALERT_MENU  wxID_HIGHEST + 5
#define DSC_MENU  wxID_HIGHEST + 6
#define RTE_MENU  wxID_HIGHEST + 7
#define TEST_MENU wxID_HIGHEST + 8

#endif