#pragma once

#include <Windows.h>

extern SERVICE_STATUS g_ServiceStatus;
extern SERVICE_STATUS_HANDLE g_ServiceStatusHandle;
extern char g_SERVICE_NAME[];

void initTask();

void doTask();

/*功能：服务运行函数*/
void WINAPI serviceMain(DWORD argc, LPCWSTR* argv);

/*功能：服务控制函数*/
void WINAPI serviceCtrlHandler(DWORD opcode);

/*功能：注册服务函数*/
bool createService(SC_HANDLE schSCManager, LPSTR szPath, LPSTR szServiceName);

/*功能：删除服务函数*/
bool deleteService(SC_HANDLE schSCManager, LPSTR szServiceName);

/*功能：启动服务函数*/
bool startService(SC_HANDLE schSCManager, LPSTR szServiceName);

/*功能：控制服务函数*/
bool controlService(SC_HANDLE schSCManager, DWORD fdwControl, LPSTR szServiceName);

/*功能：停止服务函数*/
bool stopService(SC_HANDLE schSCManager, LPSTR szServiceName, DWORD dwTimeout);
