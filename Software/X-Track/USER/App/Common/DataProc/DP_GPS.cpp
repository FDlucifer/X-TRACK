#include "DataProc.h"
#include "../HAL/HAL.h"

typedef enum{
    GPS_STATUS_DISCONNECT,
    GPS_STATUS_UNSTABLE,
    GPS_STATUS_CONNECT,
}GPS_Status_t;

static void onTimer(Account* account)
{
    HAL::GPS_Info_t gpsInfo;
    bool isValid = GPS_GetInfo(&gpsInfo);

    int satellites = gpsInfo.satellites;

    static GPS_Status_t lastStatus = GPS_STATUS_DISCONNECT;

    GPS_Status_t nowStatus;

    if (satellites > 6)
    {
        nowStatus = GPS_STATUS_CONNECT;
    }
    else if (satellites >= 3)
    {
        nowStatus = GPS_STATUS_UNSTABLE;
    }
    else
    {
        nowStatus = GPS_STATUS_DISCONNECT;
    }

    if (nowStatus != lastStatus)
    {
        const char* music[] = {
            "Disconnect",
            "UnstableConnect",
            "Connect"
        };

        HAL::Audio_PlayMusic(music[nowStatus]);
        lastStatus = nowStatus;
    }
}

static int onEvent(Account* account, Account::EventParam_t* param)
{
    if (param->event == Account::EVENT_TIMER)
    {
        onTimer(account);
        return 0;
    }

    if (param->event != Account::EVENT_SUB_PULL)
    {
        return Account::ERROR_UNSUPPORTED_REQUEST;
    }

    if (param->size != sizeof(HAL::GPS_Info_t))
    {
        return Account::ERROR_SIZE_MISMATCH;
    }

    HAL::GPS_GetInfo((HAL::GPS_Info_t*)param->data_p);

    return 0;
}

DATA_PROC_INIT_DEF(GPS)
{
    account->SetEventCallback(onEvent);
    account->SetTimerPeriod(500);
}
