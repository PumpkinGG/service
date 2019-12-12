#pragma once

#include <Windows.h>

extern SERVICE_STATUS g_ServiceStatus;
extern SERVICE_STATUS_HANDLE g_ServiceStatusHandle;
extern char g_SERVICE_NAME[];

void initTask();
void doTask();
void WINAPI serviceMain(DWORD argc, LPCWSTR* argv);
void WINAPI serviceCtrlHandler(DWORD opcode);
bool createService(SC_HANDLE schSCManager, LPSTR szPath, LPSTR szServiceName);
bool deleteService(SC_HANDLE schSCManager, LPSTR szServiceName);
bool startService(SC_HANDLE schSCManager, LPSTR szServiceName);
bool controlService(SC_HANDLE schSCManager, DWORD fdwControl, LPSTR szServiceName);
bool stopService(SC_HANDLE schSCManager, LPSTR szServiceName, DWORD dwTimeout);