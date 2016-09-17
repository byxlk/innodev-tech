#ifndef __DOORDVR_HWCLOCK_H__
#define __DOORDVR_HWCLOCK_H__
/***************************************************************************************************
**文件:
**编写者:http://sunshengquan.taobao.com/
**编写日期:2012年01月02号
**简要描述:
**修改者:
**修改日期:
**:注:版权为鑫鑫旺商行所有，买家不得以任何形式转载，不得网上发布，
不得转送给他人，不得转卖给他人，只允许公司内部人员或够买者使用
，从鑫鑫旺商行够买的代码只是使用权，若买家未遵守以上规则而造成源码外泄，
买家至少赔偿给鑫鑫旺商行(http://sunshengquan.taobao.com)50万人民币的赔偿费用.
****************************************************************************************************/

I32_T  XW_Hwclock_RtcGetTime(DateTimeDef *dt);
I32_T  XW_Hwclock_RtcSetTime(CommonSet_t *dt);
void   XW_Sync_SystemTime(void);
void   XW_Timezone(void);
#endif
