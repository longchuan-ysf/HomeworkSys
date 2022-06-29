#include "PageHandle.h"
#include "ImageDisplay.h"
#include "HomeworkGUI.h"
#include "jpegdisplay.h"
#include "camera_app.h"
#include "buttonbmp.h"
#include "delay.h"
uint8_t CurrentPage=0;//当前页面
uint8_t PreviousPage=0;//之前页面
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
		Close_Camera();//之前页面是拍照上传页面，切换页面要关闭摄像头
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
//			while(1)
//			{
//				if(rgb565_data_ok)
//				{
//					IMAGE_SetBitmap(ImageCamera,&bm2_c);
//					break;
//				}
//				delay_ms(10);
//			}
		}
		case PAGE_SYS:
		{
			
		}
		break;
	
	}
}