#pragma once
#include "DriverData.h"
#include "Operations.h"
#include "COM.h"

NTSTATUS DriverEntry(PDRIVER_OBJECT DriverObject, PUNICODE_STRING RegistryPath);
NTSTATUS DriverExit(FLT_FILTER_UNLOAD_FLAGS fltUnloadFlags);
NTSTATUS DriverInstanceSetup(PCFLT_RELATED_OBJECTS pFltObjects,
                             FLT_INSTANCE_SETUP_FLAGS flags,
                             DEVICE_TYPE volumeDeviceType,
                             FLT_FILESYSTEM_TYPE volumeFilesystemType);
NTSTATUS DriverQueryTeardown(PCFLT_RELATED_OBJECTS pFltObjects,
                             FLT_INSTANCE_QUERY_TEARDOWN_FLAGS Flags);

CONST FLT_OPERATION_REGISTRATION kFltOperations[] = {
    {IRP_MJ_CREATE, 0, CreateFilePreops, NULL, NULL}, {IRP_MJ_OPERATION_END}};

CONST FLT_REGISTRATION kFltRegistration = {sizeof(FLT_REGISTRATION),
                                           FLT_REGISTRATION_VERSION,
                                           FLTFL_REGISTRATION_SUPPORT_NPFS_MSFS,
                                           NULL,
                                           kFltOperations,
                                           DriverExit,
                                           DriverInstanceSetup,
                                           DriverQueryTeardown,
                                           NULL,
                                           NULL,
                                           NULL,
                                           NULL,
                                           NULL,
                                           NULL,
                                           NULL};

