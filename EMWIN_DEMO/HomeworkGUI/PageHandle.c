#include "PageHandle.h"
#include "ImageDisplay.h"
#include "HomeworkGUI.h"
#include "jpegdisplay.h"
#include "camera_app.h"
#include "buttonbmp.h"
#include "delay.h"
uint8_t CurrentPage=0;//��ǰҳ��
uint8_t PreviousPage=0;//֮ǰҳ��
void PageHnadle_main(uint8_t PageIndex)	
{
	PreviousPage = CurrentPage;
	CurrentPage =PageIndex;
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
	switch(PageIndex)
	{
		case PAGE_VIEW:
		{
			PictureIndex=0;
            PaintPic=1;
			WM_InvalidateWindow(WM_Picture);//����ͼƬ
			//Display_Image_byIndex(WM_Picture,PictureIndex);
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

