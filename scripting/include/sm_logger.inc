/**
 * vim: set ts=4 :
 * =============================================================================
 * sm_logger
 * Provides a simple set of functions for logging and debugging.
 * GitHub: https://github.com/raziEiL/SM-Logger
 *
 * sm_logger (C)2018-2019 raziEiL [disawar1] <mr.raz4291@gmail.com>
 * SourceMod (C)2004-2008 AlliedModders LLC.  All rights reserved.
 * =============================================================================
 *
 * This program is free software; you can redistribute it and/or modify it under
 * the terms of the GNU General Public License, version 3.0, as published by the
 * Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more
 * details.
 *
 * You should have received a copy of the GNU General Public License along with
 * this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 * As a special exception, AlliedModders LLC gives you permission to link the
 * code of this program (as well as its derivative works) to "Half-Life 2," the
 * "Source Engine," the "SourcePawn JIT," and any Game MODs that run on software
 * by the Valve Corporation.  You must obey the GNU General Public License in
 * all respects for all other code used.  Additionally, AlliedModders LLC grants
 * this exception to all derivative works.  AlliedModders LLC defines further
 * exceptions, found in LICENSE.txt (as of this writing, version JULY-31-2007),
 * or <http://www.sourcemod.net/license.php>.
 */

#if defined _logger_included
 #endinput
#endif
#define _logger_included "1.3"

// Output log bits
enum SMLOutFlags
{
	SML_NONE = (0<<0), // Disable
	SML_FILE = (1<<0), // Logs to file
	SML_SERVER = (1<<1), // Prints to server
	SML_CHAT = (1<<2), // Prints to chat
	SML_ALL = SML_FILE|SML_SERVER|SML_CHAT // Spam!
}

// Preprocessor
#define SML_SAFETY_CODE	1 // 0: optimization, 1: don't throw an error if you have code mistake
#define SML_LOWERCASE		1 // 0: none, 1: sets logfile/cvar names to lowercase
// Core
#define SML_TAG_SIZE		96
#define SML_TAB_SIZE		10
#define SML_TAG				"[SM-LOGGER]"
// Private vars
static File g_fileLog;
static ArrayList g_hTagList;
static char g_sFormattedStr[1024], SML_TAB[SML_TAB_SIZE][SML_TAB_SIZE];
static int g_iTagIndex, g_iTabIndex, g_iCvarLogFlags;
static SMLOutFlags g_iCvarLogOutFlags;

/**
 * Initializes internal structure necessary for sm_logger.inc functions.
 * @remark IT NECESSARY TO RUN THIS FUNCTION OnPluginStart()!
 *
 * @param tags			2D string array, contain a log tags.
 * @param size			Number of string (first dimension size of tags string).
 * @param bitFlags		Default log bits.
 * @param outFlags		Default output bits.
 * @param truncate		True to delete the content of existing file, false otherwise.
 *
 * @noreturn
 */
stock void SMLoggerInit(const char[][] tags, int size, int bitFlags = 0, SMLOutFlags outFlags = SML_NONE, bool truncate = false)
{
	static bool bIsInit;
	if (bIsInit)
		return;

	int i;
	char sSelf[PLATFORM_MAX_PATH], sTemp[PLATFORM_MAX_PATH];

	GetPluginFilename(GetMyHandle(), sSelf, PLATFORM_MAX_PATH);
	sSelf[strlen(sSelf) - 4] = 0; // removes '.smx'

	if ((i = FindCharInString(sSelf, '\\', true)) != -1 || (i = FindCharInString(sSelf, '/', true)) != -1)
		Format(sSelf, sizeof(sSelf), sSelf[i+1]); // removes sub-folder path
	// filter
	ReplaceString(sSelf, sizeof(sSelf), " ", "_");
#if SML_LOWERCASE
	i = strlen(sSelf);
	for (int c = 0; c < i; c++)
		sSelf[c] = CharToLower(sSelf[c]);
#endif
	FormatEx(sTemp, sizeof(sTemp), "logs/%s.log", sSelf);
	BuildPath(Path_SM, sTemp, sizeof(sTemp), sTemp);
	g_fileLog = OpenFile(sTemp, truncate ? "w" : "a");

	if (g_fileLog == null){
		LogError("%s File logs/%s.log could not be opened!", SML_TAG, sSelf);
		return;
	}

	SMLoggerTabChar('\t');

	char sCvar[PLATFORM_MAX_PATH], sValue[20];
	ConVar cVar;
	int iPowerOfTwo, iFlagsSum;

	sTemp = "Logs bit flags (add numbers together): 0=Disable";

	g_hTagList = new ArrayList(ByteCountToCells(SML_TAG_SIZE));
	for (i = 0; i < size; i++){
		g_hTagList.PushString(tags[i]);
		iPowerOfTwo = RoundFloat(Pow(2.0, float(i)));
		iFlagsSum += iPowerOfTwo;
		Format(sTemp, sizeof(sTemp), "%s, %d=%s", sTemp, iPowerOfTwo, tags[i]);
	}

	g_iCvarLogOutFlags = outFlags;
	g_iCvarLogFlags = bitFlags;
	bIsInit = true;

	IntToString(bitFlags, sValue, sizeof(sValue));
	FormatEx(sCvar, sizeof(sCvar), "%s_log_flags", sSelf);
	cVar = CreateConVar(sCvar, sValue, sTemp, FCVAR_NOTIFY, true, 0.0);
#if SOURCEMOD_V_MAJOR >= 1 && SOURCEMOD_V_MINOR >= 8
	cVar.SetInt(bitFlags);
	cVar.AddChangeHook(SML_ch_Flags);
#else
	SetConVarInt(cVar, bitFlags);
	HookConVarChange(cVar, SML_ch_Flags);
#endif

	//PrintToServer("'%s' '%s' '%s'", sCvar, sValue, sTemp);
	SMLogEx(SML_FILE, "%s SM-Logger v%s initialized", SML_TAG, _logger_included);
	SMLogEx(SML_FILE, "%s Defined flags (Sum=%d): %s", SML_TAG, iFlagsSum, sTemp[39]);
	FormatEx(sTemp, sizeof(sTemp), "ConVars: %s,", sCvar);

	IntToString(view_as<int>(outFlags), sValue, sizeof(sValue));
	FormatEx(sCvar, sizeof(sCvar), "%s_log_outputs", sSelf);
	cVar = CreateConVar(sCvar, sValue, "Outputs bit flags (add numbers together): 0=Disable, 1=Logs to file, 2=Prints to server, 4=Prints to chat, 7=all", FCVAR_NOTIFY, true, 0.0, true, float(view_as<int>(SML_ALL)));
#if SOURCEMOD_V_MAJOR >= 1 && SOURCEMOD_V_MINOR >= 8
	cVar.SetInt(view_as<int>(outFlags));
	cVar.AddChangeHook(SML_ch_OutFlags);
#else
	SetConVarInt(cVar, view_as<int>(outFlags));
	HookConVarChange(cVar, SML_ch_OutFlags);
#endif

	//PrintToServer("'%s' '%s'", sCvar, sValue);
	SMLogEx(SML_FILE, "%s %s %s", SML_TAG, sTemp, sCvar);
}

stock void SMLoggerTabChar(char c)
{
	if (c == SML_TAB[1][0])
		return;

	for (int i = 1; i < SML_TAB_SIZE; i++){
		SML_TAB[i] = "";
		SML_TAB[i][0] = c;
		if (i > 1)
			StrCat(SML_TAB[i], sizeof(SML_TAB[]), SML_TAB[i-1]);
	}
}

stock char SMLoggerGetTab(int tabsize)
{
	return SML_TAB[tabsize];
}

stock void SMLoggerSetTab(int tabsize)
{
#if SML_SAFETY_CODE
	if (tabsize >= SML_TAB_SIZE)
		g_iTabIndex = SML_TAB_SIZE-1;
	else if (tabsize < 0)
		g_iTabIndex = 0;
	else
#endif
		g_iTabIndex = tabsize;
}

// Respect logger convars. Converts bit to tag.
stock void SMLogTag(int bit, const char[] format, any ...)
{
	if (!g_iCvarLogOutFlags || !IsLogAllowed(bit))
		return;

	VFormat(g_sFormattedStr, sizeof(g_sFormattedStr), format, 3);
	SMLogger(bit, g_iCvarLogOutFlags, 0);
}
// Uses SMLog behavior but overwrite outputs cvar flags value
stock void SMLogTagEx(int bit, SMLOutFlags outFlags, const char[] format, any ...)
{
	if (!outFlags || !IsLogAllowed(bit))
		return;

	VFormat(g_sFormattedStr, sizeof(g_sFormattedStr), format, 4);
	SMLogger(bit, outFlags, 0);
}
// Uses SMLog behavior but prints to specific client instead of all chat
stock void SMLogClient(int bit, int client, const char[] format, any ...)
{
	if (!g_iCvarLogOutFlags || !IsLogAllowed(bit))
		return;

	VFormat(g_sFormattedStr, sizeof(g_sFormattedStr), format, 4);
	SMLogger(bit, g_iCvarLogOutFlags, client);
}
// Logging without tagging. Respect cvar flags value
stock void SMLog(const char[] format, any ...)
{
	if (!g_iCvarLogOutFlags)
		return;

	VFormat(g_sFormattedStr, sizeof(g_sFormattedStr), format, 2);
	SMLogger(0, g_iCvarLogOutFlags, 0);
}
// Logging without tagging. Overwrite outputs cvar flags value
stock void SMLogEx(SMLOutFlags outFlags, const char[] format, any ...)
{
	if (!outFlags)
		return;

	VFormat(g_sFormattedStr, sizeof(g_sFormattedStr), format, 3);
	SMLogger(0, outFlags, 0);
}
/*
|--------------------------------------------------------------------------
| SM_LOGGER PRIVATE STRUCTURE
|--------------------------------------------------------------------------
*/
static stock void SMLogger(int bit, SMLOutFlags outFlags, int client)
{
	static char sTemp[SML_TAG_SIZE];

	if (g_iTabIndex)
		Format(g_sFormattedStr, sizeof(g_sFormattedStr), "%s%s", SML_TAB[g_iTabIndex], g_sFormattedStr);
	if (bit){
		g_hTagList.GetString(g_iTagIndex, sTemp, sizeof(sTemp));
		Format(g_sFormattedStr, sizeof(g_sFormattedStr), "[%s] %s", sTemp, g_sFormattedStr);
	}
	if (outFlags & SML_CHAT){

		if (!client)
			PrintToChatAll(g_sFormattedStr);
		else if (IsClientInGame(client))
			PrintToChat(client, g_sFormattedStr);
	}

	FormatTime(sTemp, sizeof(sTemp), "%m/%d/%Y - %H:%M:%S");
	Format(g_sFormattedStr, sizeof(g_sFormattedStr), "%s %s", sTemp, g_sFormattedStr);

	if (outFlags & SML_FILE){
		g_fileLog.WriteLine(g_sFormattedStr);
		FlushFile(g_fileLog);
	}
	if (outFlags & SML_SERVER)
		PrintToServer(g_sFormattedStr);
}

static stock bool IsLogAllowed(int bit)
{
	if (!(g_iCvarLogFlags & bit))
		return false;

	g_iTagIndex = GetIndexByBit(bit)-1;

#if SML_SAFETY_CODE
	if (!IsPowerOfTwo(bit)){
		LogError("%s Bit value %d is not power of two!", SML_TAG, bit);
		return false;
	}
	if (g_iTagIndex >= g_hTagList.Length){
		LogError("%s Tag array index out of bound %d/%d", SML_TAG, g_iTagIndex, g_hTagList.Length);
		return false;
	}
#endif
	return true;
}

static stock int GetIndexByBit(int n)
{
	static int pos;
	for (pos = 0; n; pos++)
		n >>= 1;
	return pos;
}

static stock bool IsPowerOfTwo(int n)
{
	return n && (!(n & (n - 1)));
}

#if SOURCEMOD_V_MAJOR >= 1 && SOURCEMOD_V_MINOR >= 8
static stock void SML_ch_Flags(ConVar convar, const char[] oldValue, const char[] newValue)
#else
public void SML_ch_Flags(ConVar convar, const char[] oldValue, const char[] newValue)
#endif
{
	if (!StrEqual(oldValue, newValue))
		g_iCvarLogFlags = convar.IntValue;
}

#if SOURCEMOD_V_MAJOR >= 1 && SOURCEMOD_V_MINOR >= 8
static stock void SML_ch_OutFlags(ConVar convar, const char[] oldValue, const char[] newValue)
#else
public void SML_ch_OutFlags(ConVar convar, const char[] oldValue, const char[] newValue)
#endif
{
	if (!StrEqual(oldValue, newValue))
		g_iCvarLogOutFlags = view_as<SMLOutFlags>(convar.IntValue);
}
