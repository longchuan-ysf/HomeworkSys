#include "PageHandle.h"
#include "ImageDisplay.h"
#include "HomeworkGUI.h"
#include "jpegdisplay.h"
#include "camera_app.h"
#include "buttonbmp.h"
#include "delay.h"
uint8_t CurrentPage=0;//当前页面
uint8_t PreviousPage=0;//之前页面
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
		Close_Camera();//之前页面是拍照上传页面，切换页面要关闭摄像头
	}
	switch(PageIndex)
	{
		case PAGE_VIEW:
		{
			PictureIndex=0;
			WM_InvalidateWindow(WM_Picture);//绘制图片
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

