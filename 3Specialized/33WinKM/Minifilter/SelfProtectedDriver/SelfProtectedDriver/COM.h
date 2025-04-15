#include <fltKernel.h>

#include "DriverData.h"

 NTSTATUS COMPortConnect(PFLT_PORT pClientPort, PVOID pServerPortCookie,
                         PVOID pConnectionContext, ULONG ulSizeOfContext,
                         PVOID* pConnectionCookie);
 NTSTATUS COMPortDisconnect(PVOID pConnectionCookie);
 NTSTATUS COMPortMessageHandleRoutine(PFLT_GENERIC_WORKITEM pWorkItem,
                                      PVOID pFilterObject, PVOID pContext);