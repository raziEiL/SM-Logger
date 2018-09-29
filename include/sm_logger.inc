/**
 * vim: set ts=4 :
 * =============================================================================
 * sm_logger
 * Provides a simple set of functions for logging and debugging.
 * https://github.com/raziEiL/SM-Logger
 *
 * sm_logger (C)2018 raziEiL [disawar1] <mr.raz4291@gmail.com>
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
#define _logger_included "1.0"

// Output bits
enum (<<= 1)
{
	SML_TO_FILE = 1,
	SML_TO_SERVER,
	SML_TO_CHAT
}

#define SML_TAG_MAX_LEN		96
#define SML_TAG				"[SM LOGGER]"
#define SML_CVAR_DESCR		"Logs bit flags (add numbers together): 0=Disable"

static File g_fileLog;
static ArrayList g_hTagList;
static char g_sFormattedStr[1024];
static int g_iTagIndex, g_iCvarLogFlags, g_iCvarLogOutFlags;

stock void SMLoggerInit(const char[][] tags, int size, int bitFlags = 0, int outFlags = 0)
{
	static bool bIsInit;
	if (bIsInit)
		return;

	int i;
	char sSelf[PLATFORM_MAX_PATH], sTemp[PLATFORM_MAX_PATH];

	GetPluginFilename(GetMyHandle(), sSelf, PLATFORM_MAX_PATH);
	sSelf[strlen(sSelf) - 4] = 0; // removes '.smx'

	if ((i = FindCharInString(sSelf, '\\', true)) != -1)
		Format(sSelf, sizeof(sSelf), sSelf[i+1]); // removes sub-folder path
	// filter
	ReplaceString(sSelf, sizeof(sSelf), " ", "_");
	i = strlen(sSelf);
	for (int c = 0; c < i; c++)
		sSelf[c] = CharToLower(sSelf[c]);

	FormatEx(sTemp, sizeof(sTemp), "logs/%s.log", sSelf);
	BuildPath(Path_SM, sTemp, sizeof(sTemp), sTemp);
	g_fileLog = OpenFile(sTemp, "a");

	if (g_fileLog == null){
		LogError("%s File logs/%s.log could not be opened!", SML_TAG, sSelf);
		return;
	}

	char sCvar[PLATFORM_MAX_PATH], sValue[20];
	ConVar cVar;
	int iPowerOfTwo, iFlagsSum;

	sTemp = SML_CVAR_DESCR;
	
	g_hTagList = new ArrayList(SML_TAG_MAX_LEN);
	for (i = 0; i < size; i++){
		g_hTagList.PushString(tags[i]);
		iPowerOfTwo = RoundFloat(Pow(2.0, (float(i)+1.0)));
		iFlagsSum += iPowerOfTwo;
		Format(sTemp, sizeof(sTemp), "%s, %d=%s", sTemp, iPowerOfTwo, tags[i]);
	}
	
	g_iCvarLogOutFlags = outFlags;
	g_iCvarLogFlags = bitFlags;
	bIsInit = true;	

	IntToString(bitFlags, sValue, sizeof(sValue));
	FormatEx(sCvar, sizeof(sCvar), "%s_log_flags", sSelf);
	cVar = CreateConVar(sCvar, sValue, sTemp, FCVAR_NOTIFY, true, 0.0);
	cVar.SetInt(bitFlags);
	cVar.AddChangeHook(SML_ch_Flags);
	
	//PrintToServer("'%s' '%s' '%s'", sCvar, sValue, sTemp);
	SMLogEx(SML_TO_FILE, "%s SM-Logger v%s initialized", SML_TAG, _logger_included);
	SMLogEx(SML_TO_FILE, "%s Defined flags (Sum=%d): %s", SML_TAG, iFlagsSum, sTemp[39]);
	FormatEx(sTemp, sizeof(sTemp), "ConVars: %s,", sCvar);

	IntToString(outFlags, sValue, sizeof(sValue));
	FormatEx(sCvar, sizeof(sCvar), "%s_log_outputs", sSelf);
	cVar = CreateConVar(sCvar, sValue, "Outputs bit flags (add numbers together): 0=Disable, 1=Logs to file, 2=Prints to server, 4=Prints to chat", FCVAR_NOTIFY, true, 0.0);
	cVar.SetInt(outFlags);
	cVar.AddChangeHook(SML_ch_OutFlags);
	
	//PrintToServer("'%s' '%s'", sCvar, sValue);
	SMLogEx(SML_TO_FILE, "%s %s %s", SML_TAG, sTemp, sCvar);
}
// Respetc logger convars. Converts bit to tag.
stock void SMLogTag(int bit, const char[] format, any ...)
{
	if (!g_iCvarLogOutFlags || !IsLogAllowed(bit))
		return;

	VFormat(g_sFormattedStr, sizeof(g_sFormattedStr), format, 3);
	SMLogger(bit, g_iCvarLogOutFlags, 0);
}
// Uses SMLog behavor but overwrite outputs cvar flags value
stock void SMLogTagEx(int bit, int outFlags, const char[] format, any ...)
{
	if (!outFlags || !IsLogAllowed(bit))
		return;

	VFormat(g_sFormattedStr, sizeof(g_sFormattedStr), format, 4);
	SMLogger(bit, outFlags, 0);
}
// Uses SMLog behavor but prints to specific client insted of all chat
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
stock void SMLogEx(int outFlags, const char[] format, any ...)
{
	if (!outFlags)
		return;

	VFormat(g_sFormattedStr, sizeof(g_sFormattedStr), format, 3);
	SMLogger(0, outFlags, 0);
}

static stock void SMLogger(int bit, int outFlags, int client)
{
	static char sTemp[SML_TAG_MAX_LEN];

	if (bit){
		g_hTagList.GetString(g_iTagIndex, sTemp, sizeof(sTemp));
		Format(g_sFormattedStr, sizeof(g_sFormattedStr), "[%s] %s", sTemp, g_sFormattedStr);
	}
	if (outFlags & SML_TO_CHAT){

		if (!client)
			PrintToChatAll(g_sFormattedStr);
		else if (IsClientInGame(client))
			PrintToChat(client, g_sFormattedStr);
	}

	FormatTime(sTemp, sizeof(sTemp), "%m/%d/%Y - %H:%M:%S");
	Format(g_sFormattedStr, sizeof(g_sFormattedStr), "%s %s", sTemp, g_sFormattedStr);

	if (outFlags & SML_TO_FILE){
		g_fileLog.WriteLine(g_sFormattedStr);
		FlushFile(g_fileLog);
	}
	if (outFlags & SML_TO_SERVER)
		PrintToServer(g_sFormattedStr);
}

static stock bool IsLogAllowed(int bit)
{
	if (!(g_iCvarLogFlags & bit))
		return false;
	if (!IsPowerOfTwo(bit)){
		LogError("%s Bit value %d is not power of two!", SML_TAG, bit);
		return false;
	}
	if ((g_iTagIndex = GetIndexByBit(bit)-1) >= g_hTagList.Length){
		LogError("%s Tag array index out of bound %d/%d", SML_TAG, g_iTagIndex, g_hTagList.Length);
		return false;
	}
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

static stock void SML_ch_Flags(ConVar convar, const char[] oldValue, const char[] newValue)
{
	if (!StrEqual(oldValue, newValue))
		g_iCvarLogFlags = convar.IntValue;
}

static stock void SML_ch_OutFlags(ConVar convar, const char[] oldValue, const char[] newValue)
{
	if (!StrEqual(oldValue, newValue))
		g_iCvarLogOutFlags = convar.IntValue;
}
