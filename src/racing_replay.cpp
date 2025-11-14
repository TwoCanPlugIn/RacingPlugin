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
// Description: Replay candump logfile, converting to "Actisense Raw" format for loopback driver 
// and transmit via loopback
// Owner: twocanplugin@hotmail.com
// Date: 10/10/2025

#include "racing_replay.h"

Racing_Replay::Racing_Replay( DriverHandle driver, wxFileName fileName) : wxThread(wxTHREAD_DETACHED) {
	commsInterface = driver;
	logFile = fileName;
}

Racing_Replay::~Racing_Replay() {

}

// Check if a PGN is a fast message
bool Racing_Replay::IsFastMessage(const unsigned int pgn) {

	// List of Fast messages

	static const std::vector<unsigned int> nmeaFastMessages = { 65240,
		126208,	126983, 126984, 126985, 126464, 126720, 126996, 126998,
		127233,	127237, 127489, 127496, 127506, 128275, 129029, 129038,
		129039,	129040, 129041,	129044, 129284, 129285, 129538, 129540,
		129542,	129547, 129793, 129794, 129795,	129797, 129798, 129799,
		129801,	129802, 129808, 129809, 129810, 130064, 130065,	130066,
		130067,	130068, 130069, 130070, 130071, 130072, 130074, 130323,
		130577,	130817, 130820, 130821, 130822, 130823, 130824, 130827,
		130830,	130834, 130840,	130842, 130844, 130845, 130846, 130848,
		130850,	130851, 130856, 130860, 130896,	130916, 130918, 130919,
		130938, 130944,	130945, 130948 };

	auto p = std::find(nmeaFastMessages.begin(), nmeaFastMessages.end(), pgn);
	if (p != nmeaFastMessages.end()) {
		return true;
	}
	return false;
}


// Determine if message is a single frame message (if so parse it) otherwise
// assemble the sequence of frames into a multi-frame Fast Message
void Racing_Replay::AssembleFastMessage(const CanHeader header, std::vector<unsigned char> payload) {
	if (IsFastMessage(header.pgn)) {
		int position;
		position = MapFindMatchingEntry(header, payload.at(0));
		// No matching entry, so assume it is the first message
		if (position == NOT_FOUND) {
			// Find a free slot
			position = MapFindFreeEntry();
			if (position == NOT_FOUND) {
				// No free slots, not even with garbage collector, Log an error and exit
				std::printf("Unable to free an entry\n");
				return;
			}
			else {
				// Found a free slot so insert the first message
				MapInsertEntry(header, payload, position);
			}
		}
		// We have an existing entry, so append it
		else {
			MapAppendEntry(header, payload, position);
		}
	}
	// It's a single frame message so process it
	else {
		ProcessData(header, payload, 8);
	}
}

// Initialize each entry in the Fast Message Map
void Racing_Replay::MapInitialize(void) {
	for (size_t i = 0; i < CONST_MAX_MESSAGES; i++) {
		fastMessages[i].isFree = true;
		fastMessages[i].data.clear();
	}
}

// Lock a range of entries
// BUG BUG Remove for production, just used for testing
void Racing_Replay::MapLockRange(const unsigned int start, const unsigned int end) {
	if (start < end) {
		for (size_t i = start; i < end; i++) {
			fastMessages[i].isFree = false;
			fastMessages[i].timeArrived = time(NULL);
		}
	}

}

// Find first free entry in fastMessages
int Racing_Replay::MapFindFreeEntry(void) {
	for (size_t i = 0; i < CONST_MAX_MESSAGES; i++) {
		if (fastMessages[i].isFree == true) {
			return i;
		}
	}
	// Could also run the garbageCollection routine in a separate thread, would require locking etc.
	// But this will look for stale entries in case there are no free entries
	// If there are no free entries, then indicative that we are receiving more Fast messages
	// than I anticipated. As someone said, "Assumptions are the mother of all fuckups"
	int staleEntries;
	staleEntries = MapGarbageCollector();
	// No free slots that could be recovered
	if (staleEntries == 0) {
		return NOT_FOUND;
	}
	else {
		return MapFindFreeEntry();
	}
}

// Insert the first message of a sequence of fast messages
void Racing_Replay::MapInsertEntry(const CanHeader header, const std::vector<unsigned char> data, const int position) {
	// first message of fast packet 
	// data[0] Sequence Identifier (sid)
	// data[1] Length of data bytes
	// data[2..7] 6 data bytes

	if ((data.at(0) & 0x1F) == 0) {
		// ensure this is the first frame by checking the sid (note top 3 bytes are the id, lower 5 bytes the sequence)
		unsigned int totalDataLength; // will also include padding as we copy all of the frame, because I'm lazy
		totalDataLength = (unsigned int)data.at(1);
		totalDataLength += 7 - ((totalDataLength - 6) % 7);

		// Mark this slot as being used
		fastMessages[position].isFree = false;

		// Set the time, used to free slots if garbage collection is run
		fastMessages[position].timeArrived = GetTimeInMicroseconds();

		// Save the sequence identifier 
		fastMessages[position].sid = data.at(0);
		fastMessages[position].expectedLength = (unsigned int)data.at(1);
		fastMessages[position].header = header;

		// Remember to clear after we have processed the final frame
		fastMessages[position].data.reserve(totalDataLength);
		std::copy(data.begin() + 2, data.end(), std::back_inserter(fastMessages[position].data));

		// First frame of a multi-frame Fast Message contains six data bytes, position the cursor ready for next message
		fastMessages[position].dataOffset = 6;

		// Fucking Fusion, using fast messages to sends frames less than eight bytes
		// Same for B&G/Navico Autopilots
		if (fastMessages[position].expectedLength <= 6) {
			std::vector<unsigned char> payload;
			ProcessData(header, fastMessages[position].data, fastMessages[position].expectedLength);
			// Clear the entry
			fastMessages[position].data.clear();
			fastMessages[position].isFree = true;
		}
	}
	else {
		// This is not the first frame, somewhere we have dropped a start frame
		// we don't do any further processing if we never had the start frame
		std::printf("Dropped Start frame\n");
		std::printf("PGN: %d\n", header.pgn);
	}
}

// Append subsequent messages of a sequence of fast messages
// Subsequent messages of fast packet 
// data[0] Sequence Identifier (sid)
// data[1..7] 7 data bytes
bool Racing_Replay::MapAppendEntry(const CanHeader header, const std::vector<unsigned char> data, const int position) {
	// Check that this is the next message in the sequence
	// Note that it successfully will check for a rollover from say 0xED, 0xEE, 0xF0
	// as this check is made before the next check if ((SID & 0x1F) == 0) {....}
	if ((fastMessages[position].sid + 1) == data.at(0)) {
		fastMessages[position].sid = data.at(0);
		std::copy(data.begin() + 1, data.end(), std::back_inserter(fastMessages[position].data));
		fastMessages[position].timeArrived = GetTimeInMicroseconds();

		// Subsequent messages contains seven data bytes (last message may be padded with 0xFF)
		fastMessages[position].dataOffset += 7;
		// Is this the last message ?
		if (fastMessages[position].dataOffset >= fastMessages[position].expectedLength) {
			// Send for parsing
			ProcessData(header, fastMessages[position].data, fastMessages[position].expectedLength);
			// Clear the entry
			fastMessages[position].data.clear();
			fastMessages[position].isFree = true;
		}
		return true;
	}
	else if ((data[0] & 0x1F) == 0) {
		// We've found a matching entry, however this is a start frame, therefore
		// we've missed an end frame, and now we have a start frame with the same id (top 3 bits). 
		// The id has obviously rolled over. Should really double check that (data[0] & 0xE0) 
		std::printf("Dropped Last Frame\n");
		// Clear the entry as we don't want to leak memory
		fastMessages[position].data.clear();
		fastMessages[position].isFree = true;
		// And now insert it
		MapInsertEntry(header, data, position);
		return true;

	}
	else {
		// This is not the next frame in the sequence and not a start frame
		// We've dropped an intermedite frame, so free the slot and do no further processing
		std::printf("Dropped IntermediateFrame\n");
		fastMessages[position].data.clear();
		fastMessages[position].isFree = true;
		return false;
	}
}

// Determine whether an entry with a matching header and sequence id exists . 
// If not, then assume this is the first frame of a multi-frame Fast Message
int Racing_Replay::MapFindMatchingEntry(const CanHeader header, const unsigned char sid) {
	for (size_t i = 0; i < CONST_MAX_MESSAGES; i++) {
		if (((sid & 0xE0) == (fastMessages[i].sid & 0xE0)) && (fastMessages[i].isFree == false) && (fastMessages[i].header.pgn == header.pgn) && \
			(fastMessages[i].header.source == header.source) && (fastMessages[i].header.destination == header.destination)) {
			return i;
		}
	}
	return NOT_FOUND;
}

// BUG BUG if this gets run in a separate thread, need to lock the fastMessages 
// returns the number of stale entries that have been freed
int Racing_Replay::MapGarbageCollector(void) {
	int staleEntries = 0;
	unsigned long long elapsedTime;
	elapsedTime = GetTimeInMicroseconds();

	for (size_t i = 0; i < CONST_MAX_MESSAGES; i++) {
		if ((fastMessages[i].isFree == false) && (elapsedTime - fastMessages[i].timeArrived > 500)) {
			staleEntries++;
			fastMessages[i].data.clear();
			fastMessages[i].isFree = true;
		}
	}
	return staleEntries;
}

void Racing_Replay::ProcessData(CanHeader header, std::vector<unsigned char> payload, int payload_Length) {
	
	// Write to the loopback adapter
		// NMEA 0183 works
		//std::string sentence = "nmea0183 COM5 WIMWV $WIMWV,295.6,R,007.2,N,A*2E";

		// This is the "bastardised" output from VDR plugin, not sure what they are thinking...
		//std::string sentence = "$PCDIN,130306,93130402FD01FF020000000008580202FE4AFAFFFFB1";

		// This the correct format for Chetco Seasmart, although not supported by the loopback driver 
		//std::string sentence = "$PCDIN,01FD02,000C8472,09,04C300487FF8FFFF*55";

		// This format works (and is documented somewhere)!
		//std::string sentence = "NMEA2000 COM5 130306 93 13 02 02 fd 01 ff 03 ff ff ff ff 08 24 cf 01 fe 4a fa ff ff 55";

		// This is the output from the Data Monitor but this format doesn't work
		//std::string sentence = "1760484669972,NMEA2000,COM5,130306,93 13 04 02 fd 01 ff 02 00 00 00 00 08 24 cf 01 fe 4a fa ff ff 55";

		// This is the CSV output from Data Monitor, also doesn't work
		//std::string sentence = "2025-10-15T01:11:07.765Z,NMEA2000,130306,93130402FD01FF0200000000081824cf01fe4afaffff";

		// This is from the spec, although it is the "wrong" spec, and doesn't work
		//std::string sentence = "2024-02-16T10:00:00.234Z,NMEA2000,130306,CAN0,15FD020A24cf01fe4afaffff ";


	std::stringstream stream;
	
	stream << "NMEA2000 COM99 ";
	stream << header.pgn;
	stream << " 93 ";
	stream << payload.size() + 11;
	stream << " ";
	stream << std::setfill('0') << std::setw(2) << std::hex << (int)header.source;
	stream << " ";
	stream << std::setfill('0') << std::setw(2) << std::hex << (int)(header.pgn & 0xFF);
	stream << " ";
	stream << std::setfill('0') << std::setw(2) << std::hex << (int)((header.pgn >> 8) & 0xFF);
	stream << " ";
	stream << std::setfill('0') << std::setw(2) << std::hex << (int)((header.pgn >> 16) & 0xFF);
	stream << " ";
	stream << std::setfill('0') << std::setw(2) << std::hex << (int)((header.pgn >> 24) & 0xFF);
	stream << " ";
	stream << std::setfill('0') << std::setw(2) << std::hex << (int)header.destination;
	stream << " ff ff ff ff "; // dummy time stamp
	stream << std::setfill('0') << std::setw(2) << std::hex << (int)payload.size();
	stream << " ";
	for (auto it : payload) {
		stream << std::setfill('0') << std::setw(2) << std::hex << (int)it;
		stream << " ";
	}
	stream << "55"; //dummy checksum.

	std::string sentence(stream.str());

	CommDriverResult result;

	auto sharedPointer = std::make_shared<std::vector<unsigned char>>(sentence.begin(), sentence.end());
	result = WriteCommDriver(commsInterface, sharedPointer);
}

// Decodes a 29 bit CAN header
bool Racing_Replay::DecodeCanHeader(const std::vector<unsigned char> buf, CanHeader* header) {
	if ((buf.size() > 0) && (header != NULL)) {
		header->source = buf.at(0);
		header->destination = buf.at(2) < 240 ? buf.at(1) : CONST_GLOBAL_ADDRESS;
		header->pgn = (buf.at(3) & 0x01) << 16 | (buf.at(2) << 8) | (buf.at(2) < 240 ? 0 : buf.at(1));
		header->priority = (buf.at(3) & 0x1c) >> 2;
		return true;
	}
	else {
		return false;
	}
}

bool Racing_Replay::ConvertHexStringToByteArray(const unsigned char* hexstr, const unsigned int len, std::vector<unsigned char> *data) {
	// Must be a even number of hexadecimal characters
	// BUG BUG Another check is to perform an isxdigit on each character
	if ((len % 2 == 0) && (hexstr != NULL)) {
		unsigned char pair[2];
		for (unsigned int i = 0; i < len; i++) {
			pair[0] = hexstr[i * 2];
			pair[1] = hexstr[(i * 2) + 1];
			data->push_back((unsigned char)strtoul((char*)pair, NULL, 16));
		}
		return true;
	}
	else {
		return false;
	}
}

// Could probably replace this with std::chrono
unsigned long long Racing_Replay::GetTimeInMicroseconds() {
#ifdef __linux__
	struct timeval currentTime;
	gettimeofday(&currentTime, NULL);
	return (currentTime.tv_sec * 1e6) + currentTime.tv_usec;
#elif (_WIN32) 
	FILETIME currentTime;
	GetSystemTimeAsFileTime(&currentTime);
	unsigned long long totalTime = currentTime.dwHighDateTime;
	totalTime <<= 32;
	totalTime |= currentTime.dwLowDateTime;
	totalTime /= 10; // Windows file time is expressed in tenths of microseconds (or 100 nanoseconds)
	totalTime -= 11644473600000000ULL; // convert from Windows epoch (1/1/1601) to Posix Epoch 1/1/1970	
	return totalTime;
#endif
}

void Racing_Replay::PlayNMEA0183() {

	std::vector<wxString> sentences = {
		"$GPGGA,175244.43,3824.17,S,14435.86,E,1,10,0.98,1113.0,M,-21.3,M,25,10*5C"
		"$GPGLL,3824.17,S,14435.86,E,175244.43.00,A,A*7E",
		"$HCHDG,213.0,7.5,E,0.0,W*52",
		"$GPRMC,175244.43,A,3824.17,S,14435.86,E,012.00,213.00,071125,8.5,E,A*22",
		"$WIMWV,190.0,R,11.4,N,A*1F",
		"$SDDPT,3.0,-0.3,100.0*79",
		"$SDVHW,,T,,M,012.0,N,006.5,K*42",
		"$IIRSA,10,A,,V*56",
		"$ERRPM,E,0,2000,,A*42",
		"$ERXDR,P,300000,P,ENGINEOIL#0,C,89.5,C,ENGINE#0,U,13.4,V,ALTERNATOR#0,G,250,,Engine#0*47",
		"$ERXDR,E,15,P,Fuel#0,E,80,P,FRESHWATER#0#0,E,50,P,FRESHWATER#1,E,75,P,FRESHWATER#2*67"
	};

	CommDriverResult result;

	while (IsRunning()) {
		if (TestDestroy()) {
			break;
		}
		// This is the format for NMEA 0183 
		// nmea0183 loopback WIMWV $WIMWV,295.6,R,007.2,N,A*2E		
		// Don't have any VDR files, so just send stuff
		for (auto sentence : sentences) {
			// Need to extract the talkerid and  from the nmea sentence to
			// construct the nmea 0183 loopback format
			sentence.Prepend(" ");
			size_t commaPos = sentence.find(',');
			sentence.Prepend(sentence.substr(2, commaPos - 2));
			sentence.Prepend("nmea0183 com66 ");

			std::string sentenceToSend = sentence.ToStdString();
			// Send it
			auto sharedPointer = std::make_shared<std::vector<unsigned char>>(sentenceToSend.begin(), sentenceToSend.end());
			result = WriteCommDriver(commsInterface, sharedPointer);

			wxLogMessage("Racing Plugin, Result: %d, %s", result, sentence);
			
		}
		// Wait one second before resending
		Sleep(1000);
	}
}


void Racing_Replay::PlayNMEA2000() {

	// (1542794024.860693) can0 1CFF1906#419F010B00000000
	const std::regex candump("^\\(([0-9]+.[0-9]+)\\)\\s(slcan|vcan|can)[0-9]\\s([0-9A-F]{8})#([0-9A-F]{16})$");

	std::smatch match;

	double startTime = 0.0f;
	double endTime = 0.0f;

	std::vector<unsigned char> frame;
	CanHeader header;

	std::string inputLine;

	logFileStream.open(logFile.GetFullPath().ToStdString(), std::ios::in);

	while (!logFileStream.eof() && IsRunning()) {

		if (TestDestroy()) {
			break;
		}

		getline(logFileStream, inputLine);

		if (std::regex_match(inputLine, match, candump)) {

			endTime = atof(match[1].str().c_str());

			if (startTime > 0.0f) {

				// Time stamp (1542794024.860693)
				// Number of seconds: 1542794024
				// Number of tenths of seconds 860693 with microsecond precision
				// Some candump logs may have nanosecond precision....
				// Convert to milliseconds
				wxThread::Sleep(static_cast<unsigned int>(1000 * (endTime - startTime)));

				startTime = endTime;

			}
			else {
				// First line, no need to wait...
				startTime = endTime;
			}

			// Decode the Can header
			unsigned long temp = std::strtoul(match[3].str().c_str(), NULL, 16);
			unsigned char buf[4];
			buf[3] = (temp >> 24) & 0xFF;
			buf[2] = (temp >> 16) & 0xFF;
			buf[1] = (temp >> 8) & 0xFF;
			buf[0] = temp & 0xFF;
			header.source = buf[0];
			header.destination = buf[2] < 240 ? buf[1] : CONST_GLOBAL_ADDRESS;
			header.pgn = (buf[3] & 0x01) << 16 | (buf[2] << 8) | (buf[2] < 240 ? 0 : buf[1]);
			header.priority = (buf[3] & 0x1c) >> 2;

			// Decode the 8 byte payload
			frame.clear();
			ConvertHexStringToByteArray((const unsigned char*)match[4].str().c_str(), 8, &frame);
			AssembleFastMessage(header, frame);
		}
		// BUG BUG Should log that it is nt a candump log file
	}
	wxLogMessage("Racing Plugin, Replay Thread Exiting");
}

wxThread::ExitCode Racing_Replay::Entry() {
	// Determine from the file extension if it is a candump or nmea 0183 log file
	if (logFile.GetExt().compare("log") == 0) {
		wxLogMessage("Racing Plugin, Playing NMEA 2000 candump log file: %s", logFile.GetFullPath());
		PlayNMEA2000();
	}
	if (logFile.GetExt().compare("txt") == 0) {
		wxLogMessage("Racing Plugin, Playing NMEA 0183 text file: %s", logFile.GetFullPath());
		PlayNMEA0183();
	}

	return ExitCode(0);
}

void Racing_Replay::OnExit() {
	logFileStream.close();
	wxLogMessage("Racing Plugin, Replay thread exiting, log file closed");
}

