#ifndef CLIENT_WARFARE_STDAFX_H
#define CLIENT_WARFARE_STDAFX_H

#pragma once

#include <WinSock2.h>
#include <N3Base/My_3DStruct.h>

#if !__has_include(<warfare_config.h>)
#error warfare_config.h missing - copy and rename warfare_config.h.default to warfare_config.h
#endif

#include <warfare_config.h>

#endif // CLIENT_WARFARE_STDAFX_H
