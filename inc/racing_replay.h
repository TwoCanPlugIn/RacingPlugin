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
// NMEA2000® is a registered trademark of the National Marine Electronics Association
// Actisense® is a registered trademark of Active Research Limited


#ifndef RACING_REPLAY_H
#define RACING_REPLAY_H

// Pre compiled headers 
#include "wx/wxprec.h"

#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif

#include <wx/thread.h>
#include <wx/filename.h>

#include <ocpn_plugin.h>
// STL headers
#include <iomanip> // fill, hex Io manipulation
#include <regex> // Regular Expression
#include <fstream> // File Reading
#include <sstream> // String Stream

// NMEA 2000 stuff pinched from TwoCan plugin
// Use this to parse candump logfiles and assemble NMEA 2000 Fast Messages

#define CONST_GLOBAL_ADDRESS 255
#define CONST_HEADER_LENGTH 4
#define CONST_PAYLOAD_LENGTH 8
#define CONST_FRAME_LENGTH CONST_HEADER_LENGTH + CONST_PAYLOAD_LENGTH
#define CONST_MAX_MESSAGES 20
#define NOT_FOUND -1

// CAN v2.0 29 bit header as used by NMEA 2000
typedef struct CanHeader {
	unsigned char priority;
	unsigned char source;
	unsigned char destination;
	unsigned int pgn;
} CanHeader;

// Fast Message Re-Assembly
typedef struct FastMessageEntry {
	bool isFree; // Indicate whether this entry is free
	unsigned long long timeArrived; // time of the first message. garbage collector will remove stale entries > 300 usec old
	CanHeader header; // the header of the message. Used to "map" the incoming fast message fragments
	unsigned char sid; // message sequence identifier, used to check if a received message is the next message in the sequence
	unsigned int expectedLength; // total data length obtained from first frame
	unsigned int dataOffset; // cursor into the current position for the data buffer
	std::vector<unsigned char> data; // pointer to memory allocated for the data. Note: must be freed when isFree is set to true.
} FastMessageEntry;

class Racing_Replay : public wxThread {
public:
	Racing_Replay(DriverHandle driver, wxFileName fileName);
	virtual ~Racing_Replay();
protected:
	// wxThread overridden functions
	virtual wxThread::ExitCode Entry() wxOVERRIDE;
	virtual void OnExit() wxOVERRIDE;
private:
	bool IsFastMessage(const unsigned int pgn);
	void AssembleFastMessage(const CanHeader header, std::vector<unsigned char> payload);
	void MapInitialize(void);
	void MapLockRange(const unsigned int start, const unsigned int end);
	int MapFindFreeEntry(void);
	void MapInsertEntry(const CanHeader header, const std::vector<unsigned char> data, const int position);
	bool MapAppendEntry(const CanHeader header, const std::vector<unsigned char> data, const int position);
	int MapFindMatchingEntry(const CanHeader header, const unsigned char sid);
	int MapGarbageCollector(void);
	bool ConvertHexStringToByteArray(const unsigned char* hexstr, const unsigned int len, std::vector<unsigned char>* data);
	void ProcessData(CanHeader header, std::vector<byte> payload, int payload_Length);
	void Play();
	void PlayNMEA0183();
	void PlayNMEA2000();
	
	// The log file to be replayed
	wxFileName logFile;
	std::fstream logFileStream;
	
	// BUG BUG Contemplate std::vector
	FastMessageEntry fastMessages[CONST_MAX_MESSAGES];
	
	// Get time, OS dependent
	unsigned long long GetTimeInMicroseconds();

	// Decodes a 29 bit CAN header
	bool DecodeCanHeader(const std::vector<unsigned char> buf, CanHeader* header);

	// The OpenCPN network interface. For replaying stuff, it will be the loopback Interface
	DriverHandle commsInterface;
};
#endif