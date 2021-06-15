#include "DataProc.h"
#include "../HAL/HAL.h"
#include "lvgl/lvgl.h"

uint32_t DataProc::GetTick()
{
    return lv_tick_get();
}

uint32_t DataProc::GetTickElaps(uint32_t prevTick)
{
    return lv_tick_elaps(prevTick);
}

const char* DataProc::ConvTime(uint64_t ms, char* buf, uint16_t len)
{
    uint64_t ss = ms / 1000;
    uint64_t mm = ss / 60;
    uint32_t hh = (uint32_t)(mm / 60);

    lv_snprintf(
        buf, len,
        "%d:%02d:%02d",
        hh,
        (uint32_t)(mm % 60),
        (uint32_t)(ss % 60)
    );

    return buf;
}

static void onTimer(Account* account)
{
    HAL::GPS_Info_t gps;
    account->Pull("GPS", &gps, sizeof(gps));

    if(gps.isVaild)
    {
        HAL::Clock_Info_t clock;
        account->Pull("TzConv", &clock, sizeof(clock));
        HAL::Clock_SetInfo(&clock);
        account->SetTimerPeriod(0);
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

    if (param->size != sizeof(HAL::Clock_Info_t))
    {
        return Account::ERROR_SIZE_MISMATCH;
    }

    HAL::Clock_Info_t* info = (HAL::Clock_Info_t*)param->data_p;
    HAL::Clock_GetInfo(info);

    return 0;
}

DATA_PROC_INIT_DEF(Clock)
{
    account->Subscribe("TzConv");
    account->Subscribe("GPS");
    account->SetEventCallback(onEvent);
    account->SetTimerPeriod(500);
}
