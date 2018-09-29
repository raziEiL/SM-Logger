#pragma semicolon 1
#pragma newdecls required
#include <sourcemod>
#include <profiler>
#include <sm_logger>

// ------------------------------DEBUG DEFINE-------------------------------

#define LOG_FLAGS	(SML_CORE|SML_EXAMPLE|SML_THROW_ERROR)
#define OUT_FLAGS	(SML_TO_FILE|SML_TO_SERVER|SML_TO_CHAT)

char LOG_TAGS[][] =	 {"CORE", "EXAMPLE"}; // <- adds new tag here

// Bitwise values definitions for logger
enum (<<= 1)
{
	SML_CORE = 1,
	SML_EXAMPLE,
	SML_THROW_ERROR // Throw an error due enum size not equal to LOG_TAGS size
	// <- adds new bit here
}

// -------------------------------------------------------------------------

public Plugin myinfo =
{
	name = "SM Logger Demonstration",
	author = "raziEiL [disawar1]",
	description = "blah",
	version = "1.0",
	url = "http://steamcommunity.com/id/raziEiL"
}

public void OnPluginStart()
{
	SMLoggerInit(LOG_TAGS, sizeof(LOG_TAGS), LOG_FLAGS, OUT_FLAGS); // setup debug
	RegServerCmd("sm_logtest", LogTest_Command); // test cmd
	RegServerCmd("sm_bench", Benchmark_Command); // benchmark test
}

public Action Log_Command(int argc)
{
	SMLog("----------Part 1-----------");

	// Good eg.
	SMLog("SMLogTag(\"...\")"); // Expected Behavior: Logs everywhere|Without tags. Actual Behavior: Okay!
	SMLogEx(SML_TO_SERVER, "SMLogEx(SML_TO_SERVER, \"...\")"); // Expected Behavior: Logs to server|Without tags. Actual Behavior: Okay!
	SMLogTag(SML_CORE, "SMLogTag(SML_CORE, \"...\")"); // Expected Behavior: Logs everywhere|Tags:CORE. Actual Behavior: Okay!
	SMLogTag(SML_EXAMPLE, "SMLogTag(SML_EXAMPLE, \"...\")"); // Expected Behavior: Logs everywhere|Tags:EXAMPLE. Actual Behavior: Okay!
	SMLogTagEx(SML_CORE, SML_TO_SERVER, "SMLogTagEx(SML_CORE, SML_TO_SERVER, \"...\")"); // Expected Behavior: Logs to server|Tags:CORE. Actual Behavior: Okay!
	SMLogTagEx(SML_EXAMPLE, SML_TO_FILE, "SMLogTagEx(SML_EXAMPLE, SML_TO_FILE, \"...\")"); // Expected Behavior: Logs to file|Tags:EXAMPLE. Actual Behavior: Okay!
	
	SMLog("----------Part 2-----------");
 	ConVar cVar = FindConVar("logger_test_log_outputs"), cVar2 = FindConVar("logger_test_log_flags");
	cVar.SetInt(0);
	cVar2.SetInt(0);
	// Expected Behavior: Nothing.
	SMLog("1. SMLogTag(\"...\")"); // Okay!
	SMLogTag(SML_CORE, "1. SMLogTag(SML_CORE, \"...\")");
	SMLogTag(SML_EXAMPLE, "1. SMLogTag(SML_EXAMPLE, \"...\")"); 
	SMLogTagEx(SML_CORE, SML_TO_SERVER, "1. SMLogTagEx(SML_CORE, SML_TO_SERVER, \"...\")"); 
	SMLogTagEx(SML_EXAMPLE, SML_TO_FILE, "1. SMLogTagEx(SML_EXAMPLE, SML_TO_FILE, \"...\")");
	// Expected Behavior: Prints
	SMLogEx(SML_TO_SERVER, "1. SMLogEx(SML_TO_SERVER, \"...\")");
	// Expected Behavior: Nothing.
	cVar2.RestoreDefault();
	SMLogTag(SML_CORE, "2. SMLogTag(SML_CORE, \"...\")");
	SMLogTag(SML_EXAMPLE, "2. SMLogTag(SML_EXAMPLE, \"...\")");
	// Expected Behavior: Prints
	SMLogEx(SML_TO_SERVER, "2. SMLogEx(SML_TO_SERVER, \"...\")");
	SMLogTagEx(SML_CORE, SML_TO_SERVER, "2. SMLogTagEx(SML_CORE, SML_TO_SERVER, \"...\")"); 
	SMLogTagEx(SML_EXAMPLE, SML_TO_FILE, "2. SMLogTagEx(SML_EXAMPLE, SML_TO_FILE, \"...\")");
	cVar.RestoreDefault();

	SMLog("----------Part 3-----------");
	// Bad eg.
	SMLogTag(3, "This is not power of two!");
	SMLogTag(SML_THROW_ERROR, "I'm so bad as programmer...");
}
//LogMessage benchmark: 1.014046 seconds
//SMLogTagEx benchmark: 0.018186 seconds
//SMLogEx benchmark: 0.017643 seconds
public Action Benchmark_Command(int argc)
{
	if (!argc){
		PrintToServer("sm_bench <1-3>");
		return;
	}
	int i, MAX_LINES = 1000;
	char sSelf[PLATFORM_MAX_PATH+1], sArg[2];
	for (i = 0; i < PLATFORM_MAX_PATH; i++)
		sSelf[i] = 'a';

	GetCmdArg(argc, sArg, 2);
	argc = StringToInt(sArg);

	Handle hProf = CreateProfiler();
	if (argc == 1){
		StartProfiling(hProf);
		for (i = 0; i != MAX_LINES; i++)
			LogMessage(sSelf);
		StopProfiling(hProf);
		PrintToServer("LogMessage benchmark: %f seconds", GetProfilerTime(hProf));
	}
	else if (argc == 2){
		StartProfiling(hProf);
		for (i = 0; i != MAX_LINES; i++)
			SMLogTagEx(SML_CORE, SML_TO_FILE, sSelf);
		StopProfiling(hProf);
		PrintToServer("SMLogTagEx benchmark: %f seconds", GetProfilerTime(hProf));
	}
	else if (argc == 3){
		StartProfiling(hProf);
		for (i = 0; i != MAX_LINES; i++)
			SMLogEx(SML_TO_FILE, sSelf);
		StopProfiling(hProf);
		PrintToServer("SMLogEx benchmark: %f seconds", GetProfilerTime(hProf));
	}
	delete hProf;
}
