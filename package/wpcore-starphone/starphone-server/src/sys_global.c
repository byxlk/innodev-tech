#include "include/xw_export.h"


#if 0
static SPS_SYSTEM_INFO_T gSystemInfo ;

DVR_SYSTEM_INFO_T  *XW_Global_InitSystemInfo(void)
{
    int i;
    memset(&gSystemInfo, 0x0, sizeof(SPS_SYSTEM_INFO_T));

    gSystemInfo.bShow_ParkingLine = HI_TRUE;
    
    // preview status
    gSystemInfo.bDebugOpen = HI_FALSE;
    gSystemInfo.PreviewStart = HI_FALSE;
    for(i = 0; i < 8; i++)
    {
        gSystemInfo.ChnDisplaystatus[i] = HI_FALSE;
        gSystemInfo.bShowChn[i] = HI_FALSE;
        gSystemInfo.bShowFlag[i] = HI_FALSE;
    }

    gSystemInfo.ChnNamePos[0].s32X = 190;
    gSystemInfo.ChnNamePos[0].s32Y = 452;
    gSystemInfo.ChnNamePos[1].s32X = 510;
    gSystemInfo.ChnNamePos[1].s32Y = 452;
    gSystemInfo.ChnNamePos[2].s32X = 345;
    gSystemInfo.ChnNamePos[2].s32Y = 219;

    gSystemInfo.FlrtopPos[0].s32X = 144;
    gSystemInfo.FlrtopPos[0].s32Y = 452;
    gSystemInfo.FlrtopPos[1].s32X = 464;
    gSystemInfo.FlrtopPos[1].s32Y = 452;
    gSystemInfo.FlrtopPos[2].s32X = 299;
    gSystemInfo.FlrtopPos[2].s32Y = 219;
    
    gSystemInfo.Day_Night_Status = HI_TRUE;
    gSystemInfo.Day_Night_Pos.s32X = 8;
    gSystemInfo.Day_Night_Pos.s32Y = 8;

    gSystemInfo.IR_OnOff_Status = HI_FALSE;
    gSystemInfo.IR_OnOff_Pos.s32X = 640 - 8 - 64;
    gSystemInfo.IR_OnOff_Pos.s32Y = 8;

    gSystemInfo.Brightness_Value = 3;
    gSystemInfo.Brightness_Pos.s32X = 8;
    gSystemInfo.Brightness_Pos.s32Y = 8;

    gSystemInfo.LT_ParkingLine_Status = HI_TRUE;
    gSystemInfo.LT_ParkingLine_Pos.s32X = 0;
    gSystemInfo.LT_ParkingLine_Pos.s32Y = 240;
    gSystemInfo.LT_ParkingLine_Pos.u32Width = 320;
    gSystemInfo.LT_ParkingLine_Pos.u32Height = 240;
    
    gSystemInfo.RT_ParkingLine_Status = HI_TRUE;
    gSystemInfo.RT_ParkingLine_Pos.s32X = 320;
    gSystemInfo.RT_ParkingLine_Pos.s32Y = 240;
    gSystemInfo.RT_ParkingLine_Pos.u32Width = 320;
    gSystemInfo.RT_ParkingLine_Pos.u32Height = 240;

    gSystemInfo.TOP_ParkingLine_Status = HI_TRUE;
    gSystemInfo.TOP_ParkingLine_Pos.s32X = 160;
    gSystemInfo.TOP_ParkingLine_Pos.s32Y = 0;
    gSystemInfo.TOP_ParkingLine_Pos.u32Width = 320;
    gSystemInfo.TOP_ParkingLine_Pos.u32Height = 240;

    int pthread_ret = pthread_rwlock_init(&gSystemInfo.gSysInfo_rwlock, NULL);
    if(pthread_ret != 0)
    {
        _ERROR("rwlovk initialization failed");
    }
        
    return &gSystemInfo;
}

SPS_SYSTEM_INFO_T  *XW_Global_GetSystemGlobalContext(void)
{
    return &gSystemInfo;
}
#endif
