#include "service/service.h"
#include "logger/logger.h"

using namespace doe;

SERVICE_STATUS g_ServiceStatus;
SERVICE_STATUS_HANDLE g_ServiceStatusHandle;
char g_SERVICE_NAME[] = "MyService";

// Init
void initTask()
{
    init_async_logger("D:\\logs");
}

// Main
void doTask()
{
    while (1) {
        Sleep(5000);
        LOG_INFO(1, "Service is alive!");
    }
}

/*功能：服务运行函数*/
void WINAPI serviceMain(DWORD argc, LPCWSTR* argv) {
    g_ServiceStatus.dwServiceType = SERVICE_WIN32;
    g_ServiceStatus.dwCurrentState = SERVICE_START_PENDING;
    g_ServiceStatus.dwControlsAccepted = SERVICE_ACCEPT_STOP | SERVICE_ACCEPT_SHUTDOWN;
    g_ServiceStatus.dwWin32ExitCode = 0;
    g_ServiceStatus.dwCheckPoint = 0;
    g_ServiceStatus.dwWaitHint = 0;

    g_ServiceStatusHandle = RegisterServiceCtrlHandlerA(g_SERVICE_NAME, serviceCtrlHandler);
    if (g_ServiceStatusHandle == (SERVICE_STATUS_HANDLE)0) {
        //register service ctrl handle error.
        printf("Register service ctrl handle failed, error code (%d).\n", GetLastError());
        return;
    }

    initTask();

    g_ServiceStatus.dwCurrentState = SERVICE_RUNNING;
    if (!SetServiceStatus(g_ServiceStatusHandle, &g_ServiceStatus)) {
        //set service status error.
        printf("Set service status failed, error code (%d).\n", GetLastError());
        return;
    }

    doTask();
}

/*功能：服务控制函数*/
void WINAPI serviceCtrlHandler(DWORD opcode) {
    switch (opcode) {
    case SERVICE_CONTROL_PAUSE:
        g_ServiceStatus.dwCurrentState = SERVICE_PAUSED;
        break;
    case SERVICE_CONTROL_CONTINUE:
        g_ServiceStatus.dwCurrentState = SERVICE_RUNNING;
        break;
    case SERVICE_CONTROL_SHUTDOWN:
    case SERVICE_CONTROL_STOP:
        g_ServiceStatus.dwCurrentState = SERVICE_STOPPED;
        break;
    case SERVICE_CONTROL_INTERROGATE:/*[ɪn'terə.ɡeɪt] v.审问；询问*/
        MessageBeep(MB_OK);
        break;
    default:
        break;
    }

    if (!SetServiceStatus(g_ServiceStatusHandle, &g_ServiceStatus)) {
        //set service status error.
        printf("Set service status failed, error code (%d).\n", GetLastError());
    }
}

bool createService(SC_HANDLE schSCManager, LPSTR szPath, LPSTR szServiceName) {
    SC_HANDLE schService = CreateServiceA(schSCManager, szServiceName, g_SERVICE_NAME,
        SERVICE_ALL_ACCESS,         //存取权限
        SERVICE_WIN32_OWN_PROCESS,  //服务类别
        SERVICE_DEMAND_START,       //启动类型
        SERVICE_ERROR_NORMAL,       //错误控制类别
        szPath, NULL, NULL, NULL, NULL, NULL);

    if (schService == NULL) {
        printf("Create service failed, error code (%d).\n", GetLastError());
        return false;
    }

    printf("Create service succeeded.\n");
    CloseServiceHandle(schService);
    schService = NULL;
    return true;
}

bool deleteService(SC_HANDLE schSCManager, LPSTR szServiceName) {
    SC_HANDLE schService = OpenServiceA(schSCManager, szServiceName, DELETE);
    if (schService == NULL) {
        printf("OpenService failed, error code (%d).\n", GetLastError());
        return false;
    }
    if (!DeleteService(schService)) {
        printf("Delete service failed, error code (%d).\n", GetLastError());
        return false;
    }
    printf("Delete service succeeded.\n");
    CloseServiceHandle(schService);
    schService = NULL;
    return true;
}

bool startService(SC_HANDLE schSCManager, LPSTR szServiceName) {
    SC_HANDLE schService;
    SERVICE_STATUS_PROCESS ssp;
    DWORD dwOldCheckPoint;
    ULONGLONG dwStartTickCount;
    DWORD dwWaitTime;
    DWORD dwBytesNeeded;
    schService = OpenServiceA(schSCManager, szServiceName, SERVICE_ALL_ACCESS);
    if (schService == NULL) {
        printf("Open service failed, error code (%d).\n", GetLastError());
        return false;
    }
    int ret = StartServiceA(schService,
        0,    //argc
        NULL  //argv
    );
    if (!ret) {
        printf("Service start failed, error code (%d).\n", GetLastError());
        return false;
    }
    printf("Service start pending.\n");
    //验证状态
    if (!QueryServiceStatusEx(schService, SC_STATUS_PROCESS_INFO, (LPBYTE)&ssp,
        sizeof(SERVICE_STATUS_PROCESS), &dwBytesNeeded)) {
        printf("Query service status failed, error code (%d).\n", GetLastError());
        return false;
    }
    dwStartTickCount = GetTickCount64();
    dwOldCheckPoint = ssp.dwCheckPoint;
    //查询状态，确定Pending状态结束
    while (ssp.dwCurrentState == SERVICE_START_PENDING) {
        dwWaitTime = ssp.dwWaitHint / 10;
        if (dwWaitTime < 1000) {
            dwWaitTime = 1000;
        }
        else if (dwWaitTime > 10000) {
            dwWaitTime = 10000;
        }
        Sleep(dwWaitTime);
        //再次查询
        if (!QueryServiceStatusEx(schService, SC_STATUS_PROCESS_INFO, (LPBYTE)&ssp,
            sizeof(SERVICE_STATUS_PROCESS), &dwBytesNeeded)) {
            printf("Query service status failed, error code (%d).\n", GetLastError());
            break;
        }
        if (ssp.dwCheckPoint > dwOldCheckPoint) {
            //进程创建中
            dwStartTickCount = GetTickCount64();
            dwOldCheckPoint = ssp.dwCheckPoint;
        }
        else if (GetTickCount64() - dwStartTickCount > ssp.dwWaitHint) {
            //WaitHint时间到
            break;
        }
    }
    CloseServiceHandle(schService);
    schService = NULL;
    if (ssp.dwCurrentState != SERVICE_RUNNING) {
        printf("Start service failed, error code (%d).\n", GetLastError());
        return false;
    }
    printf("Start service success.\n");
    return true;
}

bool controlService(SC_HANDLE schSCManager, DWORD fdwControl, LPSTR szServiceName) {
    SC_HANDLE schService;
    SERVICE_STATUS status;
    DWORD fdwAccess;
    switch (fdwControl) {
    case SERVICE_CONTROL_STOP:
        fdwAccess = SERVICE_STOP;
        break;
    case SERVICE_CONTROL_PAUSE:
    case SERVICE_CONTROL_CONTINUE:
        fdwAccess = SERVICE_PAUSE_CONTINUE;
        break;
    default:
        fdwAccess = SERVICE_INTERROGATE;
        break;
    }
    schService = OpenServiceA(schSCManager, szServiceName, fdwAccess);
    if (schService == NULL) {
        printf("Open service failed, error code (%d).\n", GetLastError());
        return false;
    }
    //发送控制码
    if (!ControlService(schService, fdwControl, &status)) {
        printf("Control service failed, error code (%d).\n", GetLastError());
        return false;
    }
    printf("Control service success.\n");
    return true;
}

bool stopService(SC_HANDLE schSCManager, LPSTR szServiceName, DWORD dwTimeout) {
    SERVICE_STATUS_PROCESS ssp;
    SERVICE_STATUS status;
    ULONGLONG dwStartTime = GetTickCount64();
    DWORD dwBytesNeeded;
    SC_HANDLE schService = OpenServiceA(schSCManager, szServiceName, SERVICE_ALL_ACCESS);
    if (schService == NULL) {
        printf("Open service failed, error code (%d).\n", GetLastError());
        return false;
    }
    if (!QueryServiceStatusEx(schService, SC_STATUS_PROCESS_INFO, (LPBYTE)&ssp,
        sizeof(SERVICE_STATUS_PROCESS), &dwBytesNeeded)) {
        printf("Query service status failed, error code (%d).\n", GetLastError());
        return false;
    }
    if (ssp.dwCurrentState == SERVICE_STOPPED) {
        printf("Stop service success.\n");
        return true;
    }
    //正在结束，只需等待即可
    while (ssp.dwCurrentState == SERVICE_STOP_PENDING) {
        Sleep(ssp.dwWaitHint);
        if (!QueryServiceStatusEx(schService, SC_STATUS_PROCESS_INFO, (LPBYTE)&ssp,
            sizeof(SERVICE_STATUS_PROCESS), &dwBytesNeeded)) {
            printf("Query service status failed, error code (%d).\n", GetLastError());
            return false;
        }
        if (ssp.dwCurrentState == SERVICE_STOPPED) {
            printf("Stop service success.\n");
            return true;
        }
        if (GetTickCount64() - dwStartTime > dwTimeout) {
            printf("Time Out.\n");
            return false;
        }
    }
    //结束指定服务
    if (!ControlService(schService, SERVICE_CONTROL_STOP, &status)) {
        printf("Control service failed, error code (%d).\n", GetLastError());
        return false;
    }
    while (status.dwCurrentState != SERVICE_STOPPED) {
        Sleep(status.dwWaitHint);
        if (!QueryServiceStatusEx(schService, SC_STATUS_PROCESS_INFO,
            (LPBYTE)&ssp, sizeof(SERVICE_STATUS_PROCESS), &dwBytesNeeded)) {
            printf("Query service status failed, error code (%d).\n", GetLastError());
            return false;
        }
        if (status.dwCurrentState == SERVICE_STOPPED) {
            printf("Stop service success.\n");
            break;
        }
        if (GetTickCount64() - dwStartTime > dwTimeout) {
            printf("Time Out.\n");
            return false;
        }
    }
    if (status.dwCurrentState == SERVICE_STOPPED) {
        printf("Stop service success.\n");
        return true;
    }
    return false;
}