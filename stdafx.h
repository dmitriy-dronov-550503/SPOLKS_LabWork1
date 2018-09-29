// stdafx.h: включаемый файл для стандартных системных включаемых файлов
// или включаемых файлов для конкретного проекта, которые часто используются, но
// не часто изменяются
//

#pragma once

#ifdef WIN32
#include "targetver.h"
#include <tchar.h>
#endif

#include <stdio.h>
#include <time.h>
#include <string.h>

#ifndef WIN32
#define localtime_s(arg1, arg2) localtime_r(arg2, arg1)
#define strcpy_s(arg1, arg2, arg3) strcpy(arg1, arg3)
#define SYSTEM_CLOCK system_clock
#else
#define SYSTEM_CLOCK steady_clock
#endif

// TODO: Установите здесь ссылки на дополнительные заголовки, требующиеся для программы
