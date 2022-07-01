#include "PageHandle.h"
#include "ImageDisplay.h"
#include "HomeworkGUI.h"
#include "jpegdisplay.h"
#include "camera_app.h"
#include "buttonbmp.h"
#include "delay.h"
uint8_t CurrentPage=0;//��ǰҳ��
uint8_t PreviousPage=0;//֮ǰҳ��
void PageHnadle_main(PAGEHANDLE_PARA *para)	
{
	PreviousPage = CurrentPage;
	CurrentPage = para->page;
	printf("PreviousPage=%d CurrentPage=%d\r\n",PreviousPage,CurrentPage);
	if(PreviousPage==CurrentPage)
	{
		printf("not change page!\r\n");
		return;
	}
	if(PreviousPage == PAGE_CAMERA)
	{
		Close_Camera();//֮ǰҳ���������ϴ�ҳ�棬�л�ҳ��Ҫ�ر�����ͷ
	}
	switch(para->page)
	{
		case PAGE_VIEW:
		{
			Display_Image_byName(para->hItem,"3:/download/test.jpg");
		}
		break;
		case PAGE_CAMERA:
		{
			rgb565_test();
			Start_Camera();
		}
		case PAGE_SYS:
		{
			
		}
		break;
	
	}
}

