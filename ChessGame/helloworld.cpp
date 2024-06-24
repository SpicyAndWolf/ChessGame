#include "pch.h"

void helloworld()
{
	acutPrintf(_T("\n欢迎使用五子棋，请输入指令PLAYGAME开始"));
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
