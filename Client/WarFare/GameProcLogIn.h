#pragma once

#if defined(LOGIN_SCENE_VERSION) && LOGIN_SCENE_VERSION == 1098
#include "GameProcLogIn_1098.h"
#elif !defined(LOGIN_SCENE_VERSION) || LOGIN_SCENE_VERSION == 1298
#include "GameProcLogIn_1298.h"
#else
#pragma error LOGIN_SCENE_VERSION value is unsupported. Expected values: 1098, 1298
#endif
