﻿#include "stdafx.h"
#include "LuaEngine.h"
#include "../shared/RWLock.h"

#include "User.h"
#include "Npc.h"

// Define global functions to be called from Lua (e.g. myrand())
// These should not be associated with an object instance; those are bound separately.
DEFINE_LUA_FUNCTION_TABLE(
	g_globalFunctions,
	MAKE_LUA_FUNCTION(CheckPercent)
	MAKE_LUA_FUNCTION(RollDice)
	MAKE_LUA_FUNCTION(CheckBeefRoastVictory)
	MAKE_LUA_FUNCTION(CheckWarVictory)
);

CLuaEngine::CLuaEngine() : m_lock(new RWLock())
{
}

CLuaScript::CLuaScript() : m_luaState(nullptr)
{
}

/**
* @brief	Initialise Lua scripts.
*
* @return	true if it succeeds, false if it fails.
*/
bool CLuaEngine::Initialise()
{
	printf("Started up Lua engine (built with %s)\n", LUA_RELEASE);
	// TODO: Initialise a pool of scripts (enough for 1 per worker thread).
	return m_luaScript.Initialise();
}

/**
* @brief	Initialises a Lua script state.
*
* @return	true if it succeeds, false if it fails.
*/
bool CLuaScript::Initialise()
{
	Guard lock(m_lock);

	// Lua already initialised?
	if (m_luaState != nullptr)
	{
		printf("ERROR: Lua script already initialised. Cannot reinitialise.\n");
		return false;
	}

	// Create a new state.
	m_luaState = luaL_newstate();
	if (m_luaState == nullptr)
	{
		printf("ERROR: Failed to initialise Lua script. Not enough memory.\n");
		return false;
	}

	// Expose scripts to Lua libraries
	// May be preferable to limit these, but for now we won't stress too much.
	luaL_openlibs(m_luaState);

	/* globals */

	// push the global table onto the stack so we can set our globals
	lua_pushglobaltable(m_luaState);

	// setup our global funcs...
	luaL_setfuncs(m_luaState, g_globalFunctions, 0);

	/* objects */

	// bind our classes
	lua_bindclass(m_luaState, CUser);
	lua_bindclass(m_luaState, CNpc);

	return true;
}

/**
* @brief	TODO: Pull an available script for use.
*
* @return	null if it fails, else.
*/
CLuaScript* CLuaEngine::SelectAvailableScript()
{
	return &m_luaScript;
}

/**
* @brief	Attempts to executes a Lua script.
* 			If it has not been compiled already, it will compile the script
* 			and cache it in the internal script map.
*
* @param	pUser		   	The user running the script.
* @param	pNpc		   	The NPC attached to the script.
* @param	iEventID	   	Identifier for the event.
* @param	filename	   	The script's filename.
*
* @return	true if it succeeds, false if it fails.
*/
bool CLuaEngine::ExecuteScript(
	CUser* pUser,
	CNpc* pNpc,
	int32_t iEventID,
	const char* filename)
{
	ScriptBytecodeMap::iterator itr;
	bool result = false;

	m_lock->AcquireReadLock();
	itr = m_scriptMap.find(filename);
	if (itr == m_scriptMap.end())
	{
		// Build full path to script
		std::string szPath = LUA_SCRIPT_DIRECTORY;
		szPath += filename;

		// Release the read lock (we're not reading anymore)
		m_lock->ReleaseReadLock();

		// Attempt to compile 
		BytecodeBuffer bytecode;
		bytecode.reserve(LUA_SCRIPT_BUFFER_SIZE);

		CLuaScript* pScript = SelectAvailableScript();
		if (pScript == nullptr)
		{
			// No available script
			// NOTE: Not technically possible in the current state, but we intend to pool them.
			return false;
		}

		if (!pScript->CompileScript(szPath.c_str(), bytecode))
		{
			pScript->LogError(
				"Could not compile Lua script.",
				pUser,
				pNpc,
				szPath.c_str());
			return false;
		}

		// Acquire the write lock (we're adding the compiled script)
		m_lock->AcquireWriteLock();

#if !defined(LUA_SCRIPT_CACHE_DISABLED)
		// Add the script to our map
		m_scriptMap[filename] = bytecode;
#endif

		// Now that we have the bytecode, we can use it.
		result = pScript->ExecuteScript(
			pUser,
			pNpc,
			iEventID,
			filename,
			bytecode);

		// Done using the lock.
		m_lock->ReleaseWriteLock();
	}
	else
	{
		// Already have the bytecode, so now we need to use it.
		result = SelectAvailableScript()->ExecuteScript(
			pUser,
			pNpc,
			iEventID,
			filename,
			itr->second);

		// Done using the lock.
		m_lock->ReleaseReadLock();
	}

	return result;
}

/**
* @brief	Attempts to compile a Lua script.
*
* @param	filename	Filename of the script.
* @param	buffer  	The buffer to store the script's compiled bytecode.
*
* @return	true if it succeeds, false if it fails.
*/
bool CLuaScript::CompileScript(const char * filename, BytecodeBuffer & buffer)
{
	// ensure that we wait until the last user's done executing their script.
	Guard lock(m_lock);

	/* Attempt to load the file */
	int err = luaL_loadfile(m_luaState, filename);

	// If something bad happened, try to find an error.
	if (err != LUA_OK)
	{
		RetrieveLoadError(err, filename);
		return false;
	}

	// Everything's OK so far, the script has been loaded, now we need to start dumping it to bytecode.
	err = lua_dump(m_luaState, (lua_Writer)LoadBytecodeChunk, &buffer);
	if (err
		|| buffer.empty())
	{
		printf("ERROR: Failed to dump the Lua script `%s` to bytecode.\n", filename);
		return false;
	}

	// Compiled!
	return true;
}

/**
* @brief	Callback for lua_dump() to read in each chunk of bytecode.
*
* @param	L	  	The associated lua_State.
* @param	bytes 	The chunk of bytecode being dumped.
* @param	len   	The number of bytes of bytecode in this chunk.
* @param	buffer	The buffer to store this chunk of bytecode in.
*
* @return	The bytecode chunk.
*/
int CLuaScript::LoadBytecodeChunk(
	lua_State* L,
	uint8_t* bytes,
	size_t len,
	BytecodeBuffer* buffer)
{
	for (size_t i = 0; i < len; i++)
		buffer->push_back(bytes[i]);

	return 0;
}

/**
* @brief	Executes the Lua script from bytecode.
*
* @param	pUser		   	The user running the script.
* @param	pNpc		   	The NPC attached to the script.
* @param	iEventID	   	Identifier for the event.
* @param	filename	   	The script's filename for debugging purposes.
* @param	bytecode	   	The script's compiled bytecode.
*
* @return	true if it succeeds, false if it fails.
*/
bool CLuaScript::ExecuteScript(
	CUser* pUser,
	CNpc* pNpc,
	int32_t iEventID,
	const char* filename,
	BytecodeBuffer& bytecode)
{
	// Ensure that we wait until the last user's done executing their script.
	Guard lock(m_lock);

	/* Attempt to run the script. */

	// Load the buffer with our bytecode.
	int err = luaL_loadbuffer(m_luaState, reinterpret_cast<const char *>(&bytecode[0]), bytecode.size(), nullptr);
	if (err != LUA_OK)
	{
		RetrieveLoadError(err, filename);
		return false;
	}

#if defined(_DEBUG)
	printf(
		"TEMP: EVENT ID = %d. NPC ID = %d.\n",
		iEventID,
		pNpc->GetProtoID());
#endif

	lua_tsetglobal(m_luaState, "nEventID", iEventID);

	lua_tsetglobal(m_luaState, "pNpc", pNpc);
	lua_tsetglobal(m_luaState, "pUser", pUser);

	// Try calling the script's entry point
	err = lua_pcall(m_luaState, 
		0,	// no arguments
		0,	// 0 returned values
		0);	// no error handler

	// Nothing returned, so we can finish up here.
	if (err == LUA_OK)
	{
		lua_settop(m_luaState, 0);
		return true;
	}

	// Attempt to provide somewhat informative errors to help the user figure out what's wrong.
	switch (err)
	{
	case LUA_ERRRUN:
		LogError(
			"A runtime error occurred within Lua script.",
			pUser,
			pNpc,
			filename);
		break;

	case LUA_ERRMEM:
		LogError(
			"Unable to allocate memory during execution of Lua script.",
			pUser,
			pNpc,
			filename);
		break;

	case LUA_ERRERR:
		LogError(
			"An error occurred during Lua script, Error handler failed.",
			pUser,
			pNpc,
			filename);
		break;

	default:
		LogError(
			"An unknown error occurred in Lua script.",
			pUser,
			pNpc,
			filename);
		break;
	}

	// Is there an error set? That can be more useful than our generic error.
	if (lua_isstring(m_luaState, -1))
	{
		printf("ERROR: [%s] The following error was provided.\n",filename);
		printf("MESSAGE: %s\n", lua_to<const char *>(m_luaState, -1));
		printf("-\n");
	}

	lua_settop(m_luaState, 0);

	return false;
}

void CLuaScript::LogError(
	const char* message,
	CUser* pUser,
	CNpc* pNpc,
	const char* filename)
{
	// TODO: Log (this just condenses the temp errors that exist already)
	printf("ERROR: %s\n", message);
	printf("FILE: %s\n", filename);
	printf("USER: %s\n", pUser->GetName().c_str());
	printf("ZONE: %d\n", pUser->GetZoneID());
	printf("NPC ID: %d (proto: %d)\n", pNpc->GetID(), pNpc->GetProtoID());
	printf("-\n");
}

/**
* @brief	Retrieves the associated error for a script load operation.
*
* @param	err			The error.
* @param	filename	Filename of the file.
*/
void CLuaScript::RetrieveLoadError(int err, const char * filename)
{
	switch (err)
	{
	case LUA_ERRFILE:
		printf("ERROR: Unable to load Lua script `%s`.\n", filename);
		break;

	case LUA_ERRSYNTAX:
		printf("ERROR: There was a error with the syntax of Lua script `%s`.\n", filename);
		break;

	case LUA_ERRMEM:
		printf("ERROR: Unable to allocate memory for Lua script `%s`.\n", filename);
		break;

	default:
		printf("ERROR: An unknown error occurred while loading Lua script `%s`.\n", filename);
		break;
	}

	// Is there an error set? That can be more useful than our generic error.
	if (lua_isstring(m_luaState, -1))
	{
		printf("ERROR: %s", lua_to<const char *>(m_luaState, -1));
	}
}

/**
* @brief	Waits for & shuts down the current Lua script.
*/
void CLuaScript::Shutdown()
{
	Guard lock(m_lock);
	// Seems silly right now, but it ensures we wait
	// until a script is finished its execution before
	// we proceed. Cleanup will continue as normal.
}

/**
* @brief	Shuts down the Lua script pool.
*/
void CLuaEngine::Shutdown()
{
	m_lock->AcquireWriteLock();
	// TODO: Script pool.
	m_luaScript.Shutdown();
	m_lock->ReleaseWriteLock();
}

CLuaScript::~CLuaScript()
{
	Guard lock(m_lock);
	if (m_luaState != nullptr)
		lua_close(m_luaState);
}

CLuaEngine::~CLuaEngine()
{
	m_scriptMap.clear();
	delete m_lock;
}
