#include "pch.h"

void helloworld()
{
	acutPrintf(_T("\n��ӭʹ�������壬������ָ��PLAYGAME��ʼ"));
}


int showhello(struct resbuf *rb)
{
	ads_printf(_T("hello"));
	ads_retvoid();
	return RTNORM;
}

int showhellocmd(struct resbuf *rb)
{
	ads_printf(_T("hello"));
	ads_retvoid();
	return RTNORM;
}
