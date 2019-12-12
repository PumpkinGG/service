#include "service/service.h"
#include <stdio.h>

void print_help_info(char* arg0);

int main(int argc, char* argv[])
{
    if (argc < 2) {
        // Dispatch
        SERVICE_TABLE_ENTRYA DispatchTable[] = {
            { g_SERVICE_NAME, (LPSERVICE_MAIN_FUNCTIONA)serviceMain },
            { NULL, NULL }
        };
        if (!StartServiceCtrlDispatcherA(DispatchTable)) {
            //start service ctrl dispather error.
            //printf("Start service ctrl dispatcher failed.\n");
        }
        print_help_info(argv[0]);
        return 0;
    }

    SC_HANDLE schSCManager = OpenSCManager(NULL, NULL, SC_MANAGER_ALL_ACCESS);
    if (schSCManager == NULL) {
        printf("Open SCManager failed, error code (%d).\n", GetLastError());
        return -1;
    }

    int retCode = 0;
    do {
        if ((strcmp(argv[1], "--help") == 0) || (strcmp(argv[1], "-h") == 0) || (strcmp(argv[1], "/?") == 0)) {
            print_help_info(argv[0]);
        }
        else if (strcmp(argv[1], "install") == 0) {
            bool ret = createService(schSCManager, argv[0], g_SERVICE_NAME);
            if (!ret) {
                //printf("Create service failed (%d).\n", GetLastError());
                retCode = -2;
                break;
            }
        }
        else if (strcmp(argv[1], "start") == 0) {
            bool ret = startService(schSCManager, g_SERVICE_NAME);
            if (!ret) {
                //printf("Start service failed (%d).\n", GetLastError());
                retCode = -3;
                break;
            }
        }
        else if (strcmp(argv[1], "stop") == 0) {
            bool ret = stopService(schSCManager, g_SERVICE_NAME, 1000);
            if (!ret) {
                //printf("Stop service failed (%d).\n", GetLastError());
                retCode = -4;
                break;
            }
        }
        else if (strcmp(argv[1], "delete") == 0) {
            bool ret = deleteService(schSCManager, g_SERVICE_NAME);
            if (ret) {
                //printf("Delete service failed (%d).\n", ret);
                retCode = -5;
                break;
            }
        }
        else {
            print_help_info(argv[0]);
        }
    } while (0);
    CloseServiceHandle(schSCManager);
    schSCManager = NULL;

    return retCode;
}

void print_help_info(char* arg0)
{
    char* exe_file = strrchr(arg0, '\\');
    if (exe_file != NULL) {
        exe_file++;
    }
    else {
        exe_file = arg0;
    }
    printf("Please add command:\n");
    printf("  .\\%s install -- Install server service.\n", exe_file);
    printf("  .\\%s start   -- Start server service.\n", exe_file);
    printf("  .\\%s stop    -- Stop server service.\n", exe_file);
    printf("  .\\%s delete  -- Delete server service.\n", exe_file);
}