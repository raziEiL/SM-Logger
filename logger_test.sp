#pragma semicolon 1
#include <sourcemod>
#include <profiler>
#pragma newdecls required
#include <sm_logger>

// ------------------------------DEBUG DEFINE-------------------------------

#define LOG_FLAGS	SML_CORE|SML_EXAMPLE|SML_THROW_ERROR
#define OUT_FLAGS	SML_ALL

// Tags str definitions for logger
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
	version = "1.4",
	url = "http://steamcommunity.com/id/raziEiL"
}

public void OnPluginStart()
{
	SMLoggerInit(LOG_TAGS, sizeof(LOG_TAGS), LOG_FLAGS, OUT_FLAGS); // setup logger

	RegServerCmd("sm_logtest", LogTest_Command); // logger test
	RegServerCmd("sm_logcvar", LogCvar_Command); // cvar test
	RegServerCmd("sm_logbench", LogBench_Command); // benchmark test
}

// log result -> logger_test.log
public Action LogTest_Command(int argc)
{
	SMLog("----------Good eg.-----------");
	SMLog("SMLogTag(\"...\")"); // Expected Behavior: Logs everywhere|Without tags. Actual Behavior: Okay!
	SMLogEx(SML_SERVER, "SMLogEx(SML_SERVER, \"...\")"); // Expected Behavior: Logs to server|Without tags. Actual Behavior: Okay!
	SMLogTag(SML_CORE, "SMLogTag(SML_CORE, \"...\")"); // Expected Behavior: Logs everywhere|Tags:CORE. Actual Behavior: Okay!
	SMLogTag(SML_EXAMPLE, "SMLogTag(SML_EXAMPLE, \"...\")"); // Expected Behavior: Logs everywhere|Tags:EXAMPLE. Actual Behavior: Okay!
	SMLogTag(SML_EXAMPLE, "%sSMLogTag(SML_EXAMPLE, \"2 tabs here\")", SMLoggerGetTab(2));
	SMLogTagEx(SML_CORE, SML_SERVER, "SMLogTagEx(SML_CORE, SML_SERVER, \"...\")"); // Expected Behavior: Logs to server|Tags:CORE. Actual Behavior: Okay!
	SMLogTagEx(SML_EXAMPLE, SML_FILE, "SMLogTagEx(SML_EXAMPLE, SML_FILE, \"...\")"); // Expected Behavior: Logs to file|Tags:EXAMPLE. Actual Behavior: Okay!

	// See error logs
	LogError("----------Bad eg.-----------");
	SMLogTag(3, "This is not power of two!"); // Expected Behavior: Error. Actual Behavior: Okay!
	SMLogTag(SML_THROW_ERROR, "I'm so bad as programmer..."); // Expected Behavior: Error. Actual Behavior: Okay!

	SMLogEx(SML_FILE, "----------Tab test 1-----------");
	SMLogEx(SML_FILE, "deep 1");
	SMLogEx(SML_FILE, "{");
	SMLoggerSetTab(1);
	int x, i;
	for (i = 0; i < 3; i++){
		SMLogEx(SML_FILE, "index=%d", i);
		SMLogEx(SML_FILE, "tick=%d", GetGameTickCount());
		SMLogEx(SML_FILE, "gametime=%f", GetGameTime());

		SMLogEx(SML_FILE, "deep 2");
		SMLogEx(SML_FILE, "{");
		SMLoggerSetTab(2);
		for (x = 0; x < 5; x++){
			SMLogEx(SML_FILE, "index2=%d", x);
			SMLogEx(SML_FILE, "sum=%d", i+x);
		}
		SMLoggerSetTab(1);
		SMLogEx(SML_FILE, "}");
	}
	SMLoggerSetTab(0);
	SMLogEx(SML_FILE, "}");

	SMLogEx(SML_FILE, "----------Tab test 2-----------");
	SMLoggerTabChar('.');
	for (i = 0; i < SML_TAB_SIZE; i++){
		SMLoggerSetTab(i);
		SMLogEx(SML_FILE, "stairs");
	}
	SMLoggerTabChar('\t');
	SMLoggerSetTab(0);
}

public Action LogCvar_Command(int argc)
{
	SMLog("----------Cvar test-----------");
 	ConVar cVar = FindConVar("logger_test_log_outputs"), cVar2 = FindConVar("logger_test_log_flags");
	cVar.SetInt(0);
	cVar2.SetInt(0);
	// Expected Behavior: Nothing.
	SMLog("1. SMLogTag(\"...\")"); // Okay!
	SMLogTag(SML_CORE, "1. SMLogTag(SML_CORE, \"...\")"); // Okay!
	SMLogTag(SML_EXAMPLE, "1. SMLogTag(SML_EXAMPLE, \"...\")");  // Okay!
	SMLogTagEx(SML_CORE, SML_SERVER, "1. SMLogTagEx(SML_CORE, SML_SERVER, \"...\")");  // Okay!
	SMLogTagEx(SML_EXAMPLE, SML_FILE, "1. SMLogTagEx(SML_EXAMPLE, SML_FILE, \"...\")"); // Okay!
	// Expected Behavior: Prints
	SMLogEx(SML_SERVER, "1. SMLogEx(SML_SERVER, \"...\")"); // Okay!
	// Expected Behavior: Nothing.
	cVar2.RestoreDefault();
	SMLogTag(SML_CORE, "2. SMLogTag(SML_CORE, \"...\")"); // Okay!
	SMLogTag(SML_EXAMPLE, "2. SMLogTag(SML_EXAMPLE, \"...\")"); // Okay!
	// Expected Behavior: Prints
	SMLogEx(SML_SERVER, "2. SMLogEx(SML_SERVER, \"...\")");
	SMLogTagEx(SML_CORE, SML_SERVER, "2. SMLogTagEx(SML_CORE, SML_SERVER, \"...\")");  // Okay!
	SMLogTagEx(SML_EXAMPLE, SML_FILE, "2. SMLogTagEx(SML_EXAMPLE, SML_FILE, \"...\")"); // Okay!
	cVar.RestoreDefault();
}

//LogMessage benchmark: 1.014046 seconds
//SMLogTagEx benchmark: 0.018186 seconds
//SMLogEx benchmark: 0.017643 seconds
public Action LogBench_Command(int argc)
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
			SMLogTagEx(SML_CORE, SML_FILE, sSelf);
		StopProfiling(hProf);
		PrintToServer("SMLogTagEx benchmark: %f seconds", GetProfilerTime(hProf));
	}
	else if (argc == 3){
		StartProfiling(hProf);
		for (i = 0; i != MAX_LINES; i++)
			SMLogEx(SML_FILE, sSelf);
		StopProfiling(hProf);
		PrintToServer("SMLogEx benchmark: %f seconds", GetProfilerTime(hProf));
	}
	delete hProf;
}
