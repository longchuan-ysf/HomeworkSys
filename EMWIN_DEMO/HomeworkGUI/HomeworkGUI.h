#ifndef  HOMEWORK_GUI_H
#define  HOMEWORK_GUI_H
#include "DIALOG.h"
#include "keypad.h"

extern GUI_BITMAP buttonbmp_tab[2];
extern WM_HWIN WM_Camera;
extern WM_HWIN WM_Picture;//��ʾ��Ƭ
extern WM_HWIN WM_WIFIList;//
extern WM_HWIN WM_WIFIConnect;//��ʾ����wifi

extern WM_HWIN EDIT_ServerIP;
extern WM_HWIN EDIT_ServerPort;
extern WM_HWIN BUTTON_ServerSwitch;

extern u8 ButtonFlag_wifi;
extern u8 ButtonFlag_sever;

extern Dialog_MSG DisplayDialogMsg;

extern WM_HWIN ViewHomework;
extern WM_HWIN UploadHomework;
extern WM_HWIN DialogSelectWiFi;


extern WM_HWIN CreateFramewin(void);

#endif
