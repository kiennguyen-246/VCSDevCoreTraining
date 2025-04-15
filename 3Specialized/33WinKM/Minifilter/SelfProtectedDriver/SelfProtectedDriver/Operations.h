#pragma once
#include <fltKernel.h>

#include "DriverData.h"

FLT_PREOP_CALLBACK_STATUS CreateFilePreops(PFLT_CALLBACK_DATA pData,
                                           PCFLT_RELATED_OBJECTS pFltObjects,
                                           PVOID* pCompletionContext);
