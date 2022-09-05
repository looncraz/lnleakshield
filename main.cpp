/*

	Next acts:

	Setup to skip known entries and record all changes for uknown indices over several days.
	Log to a file all the changes and include frame #s so you can see what changes happened together.
	Probably can actually use the time field...

	Create a way to explore the differences quickly, preferably graphically, probably a simmple
	playback with colors or something...
*/

#include <cstdio>
#include <iostream>
#include <chrono>
#include <set>
#include <string>
#include <thread>

#include <mutex>
#include <sys/stat.h>
#include <fcntl.h>

#include "BigT.h"
#include "Utils.h"

#include "LeakshieldData.h"

#define BUILDDATE __DATE__ " " __TIME__

std::recursive_mutex gPrintMutex;

#define DPRINT(X, Y...) //	gPrintMutex.lock();	printf(X, Y); gPrintMutex.unlock();


size_t ReadFileWithTimeout(SString path, int seconds, int size, void* buffer) {
	int fd = open(path.c_str(), O_RDONLY);
	fd_set rfds;	FD_ZERO(&rfds);	FD_SET(fd, &rfds);
	struct timeval tv;	tv.tv_sec = 5;	tv.tv_usec = 0;
	FILE* file = fdopen(fd, "r");

	if (file == nullptr)
		return 0;

	fseek(file, 0, SEEK_SET);
	size_t read = 0;

	if (select(fd + 1, &rfds, NULL, NULL, &tv))
		read = fread(buffer, 1, size, file);

	fclose(file);
	return read;
}


SStringList FindHIDPaths() {
	SStringList result;
//	result.push_back("/dev/hidraw1");
//	return result;
	SStringList hidPaths;

	// Get list of hidraw devices
	for (auto& entry : fs::directory_iterator("/dev/")) {
		if (!entry.is_directory()) {
			SString path = entry.path();
			if (path.rfind("/dev/hidraw", 0) == std::string::npos) continue;
			hidPaths.push_back(path);
			// TESTING ONLY:
	//		hidPaths.push_back(path);	// duplicating entries so there is more than one match
		}
	}

	// Start a unique thread per hidraw device, read 32 bytes to check for header

	struct hidData {
		SString			path;
		uint32			cookie;
		bool			done;
		std::thread*	thread;
	};

	std::deque<hidData*> threads;
	for (auto& path : hidPaths) {
		hidData* data = new (hidData){path.c_str(), 0, false, nullptr};

		data->thread = new std::thread([data](){
			ReadFileWithTimeout(data->path, 5, 4, &data->cookie);
			data->done = true;
		});

		threads.push_back(data);
	}


	for (auto& data : threads) {
		data->thread->join();
		if (data->done && data->cookie == 140771329)
				result.push_back(data->path);

		delete data->thread;
	}
	return result;
}


LeakshieldData* ReadLeakshield(const std::string& path)
{
	uint8_t* buf = (uint8_t*)malloc(LEAKSHIELD_DATA_SIZE);

	if (buf == nullptr) {
		fprintf(stderr, "Malloc failed! Out of memory?\n");
		return nullptr;
	}
	auto read = ReadFileWithTimeout(path.c_str(), 5, LEAKSHIELD_DATA_SIZE, buf);

	if (read != LEAKSHIELD_DATA_SIZE) {
		fprintf(stderr, "Read %lu bytes instead of %u bytes!\n", read, LEAKSHIELD_DATA_SIZE);
		fflush(stderr);
		free(buf);
		return nullptr;
	}

	return ((LeakshieldData*)buf);
}


void PrintHelp()
{
	printf("lnleakshield - utility to probe Aquacomputer Leakshield\n");
	printf("Built %s\n", BUILDDATE);

	printf("\t%-18s\t-\tFind Leakshield HID device(s)\n", "search");
	printf("\t%-18s\t-\tRead data from specific device\n", "probe /dev/hidraw#");
}


#define BUILD_FOR_HWINFO 0


struct CmdOption {
		std::string							command;
		int									requiredArgumentCount;
		SStringMap<SString>					options;
		std::function<int(SStringList)>	invoke;
};


std::deque<CmdOption> gCommandOptions = {
	{{"search"}, 0, {}, [](SStringList list){
		auto paths = FindHIDPaths();
		if (paths.size() > 0)
			printf("%s\n", paths[0].c_str());
		return 0;
	}},

	{{"probe"}, 1, {{{"/dev/hidraw#", "Path to device"}}}, [](SStringList list){
		uint8 buf[420];
		ReadFileWithTimeout(list[0].c_str(), 5, 420, &buf);
		printf("%i\n", ((int32)(buf[286] << 0 | buf[285] << 8)));
		return 0;
	}},

	{{"dump"}, 1, {{{"/dev/hidraw#", "Path to device"}}}, [](SStringList list){
		auto lsdata = ReadLeakshield(list[0]);
		lsdata->PrintToStream();
		return 0;
	}},

	{{"watch"}, 1, {{{"/dev/hidraw#", "Path to device"}}}, [](SStringList list){
		while (true) {
			auto lsdata = ReadLeakshield(list[0]);
			// TODO: this is very crude!
			system("clear");
			lsdata->PrintToStream();
		}
		return 0;
	}},


	{{"monitor-deltas"}, 1, {{{"/dev/hidraw#", "Path to device"}}}, [](SStringList list){
		printf("Currently unimplemented, sorry!\n");
		return 0;
	}},

};



int main(int argc, char** argv) {

	bool found = false;
	int retVal = 1;
	SStringList parmList;
	for (int i = 1; i < argc; ++i)
		parmList.push_back(std::string(argv[i]));


	for (int i = 1; i < argc; ++i) {
		for (auto& cmdOpt : gCommandOptions) {
			if (cmdOpt.command == argv[i]) {
				parmList.pop_front();
				if (parmList.size() < cmdOpt.requiredArgumentCount) {
					fprintf(stderr, "\nWarning: Insufficient arguments for command\n\n");
					PrintHelp();
					return 2;
				}
				retVal = cmdOpt.invoke(parmList);
				found = true;
			}
		}
	}

	if (found) return retVal;

	PrintHelp();
	return 0;

/*
	Old cruft, need to create monitor-deltas command from this and show altered offsets

*/

#if BUILD_FOR_HWINFO

	if (argc >= 2) {
		if (argv[1] == std::string("search")) {
			auto paths = FindHIDPaths();
			if (paths.size() > 0)
				printf("%s\n", paths[0].c_str());
			return 0;
		} else if (argv[1] == std::string("probe") && argc == 3) {
			uint8 buf[420];
			ReadFileWithTimeout(argv[2], 5, 420, &buf);
			printf("%li\n", ((int32)(buf[286] << 0 | buf[285] << 8)));
			return 0;
		} else PrintHelp();
	} else PrintHelp();

	return 0;

#else

//	printf("Searching for Leakshield...");
//	fflush(stdout);
//	auto paths = FindHIDPaths(); // this takes FIVE seconds

// *** Comment out above 3 lines and uncomment the following two using the Leakshield path
// so you can use "sudo watch -n 0,1 ./object/lnleakshield"
// ***
	SStringList paths;
	paths.push_back(SString("/dev/hidraw4"));
//	printf("%lu found!\n", paths.size());

	if (paths.size() == 0) {
		std::cerr << "Could not find Leakshield!\n";
		return 1;
	}


	std::map<int, std::map<int, uint8>> changed;

	uint8_t* lastBuf = nullptr;

//	std::deque<int> known = {};

	for (auto& shield : paths) {

//		printf("%s\n", shield.c_str());

// Watch for a good long while for changes...
#define LOOP_COUNT 50

		std::set<int> known = {23, 33, 38, 39, 275, 276, 279, 280, 280, 281, 282, 283, 284, 285,
			286, 287, 288, 305, 306, 307, 308, 311, 312, 313, 314};

		for (int count = 0; count < LOOP_COUNT; count++) {
			auto lsdata = ReadLeakshield(shield);
			if (!lsdata) {
				fprintf(stderr, "Skip %s\n", shield.c_str());
				break;
			}

			lsdata->PrintToStream();

break; // remove to watch changes (comment out lsdata->PrintToStream()!)

			printf("\rWatching changes... %i / %i    ", count + 1, LOOP_COUNT);
			fflush(stdout);
			uint8_t* buf = (uint8_t*)lsdata;
			if (lastBuf == nullptr) {
				lastBuf = (uint8_t*)malloc(LEAKSHIELD_DATA_SIZE);
				memcpy(lastBuf, buf, LEAKSHIELD_DATA_SIZE);
			} else {
				for (int i = 0; i < LEAKSHIELD_DATA_SIZE; ++i) {
					if (buf[i] != lastBuf[i]) {
						if (known.count(i) <= 0)
							changed[count][i] = buf[i];
					}
				}
				memcpy(lastBuf, buf, LEAKSHIELD_DATA_SIZE);
			}


			free(lsdata);
		}

		for (auto& [frame, values] : changed) {
			printf("Frame: %i\n\t", frame);
			for (auto& [index, value] : values) {
				printf("%i = %i, ", index, value);
			}
			printf("\n");
		}
	}

#endif
}
