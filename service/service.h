#pragma once

#include <Windows.h>

extern SERVICE_STATUS g_ServiceStatus;
extern SERVICE_STATUS_HANDLE g_ServiceStatusHandle;
extern char g_SERVICE_NAME[];

void initTask();

void doTask();

/*���ܣ��������к���*/
void WINAPI serviceMain(DWORD argc, LPCWSTR* argv);

/*���ܣ�������ƺ���*/
void WINAPI serviceCtrlHandler(DWORD opcode);

/*���ܣ�ע�������*/
bool createService(SC_HANDLE schSCManager, LPSTR szPath, LPSTR szServiceName);

/*���ܣ�ɾ��������*/
bool deleteService(SC_HANDLE schSCManager, LPSTR szServiceName);

/*���ܣ�����������*/
bool startService(SC_HANDLE schSCManager, LPSTR szServiceName);

/*���ܣ����Ʒ�����*/
bool controlService(SC_HANDLE schSCManager, DWORD fdwControl, LPSTR szServiceName);

/*���ܣ�ֹͣ������*/
bool stopService(SC_HANDLE schSCManager, LPSTR szServiceName, DWORD dwTimeout);
