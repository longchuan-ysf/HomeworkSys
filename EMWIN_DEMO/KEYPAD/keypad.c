#include <stddef.h>
#include "DIALOG.h"
#include "keypad.h"
#include "tftlcd.h"
#include "string.h"
#include "stdlib.h"
#include "usart.h"

#include "EmWinHZFont.h"
#include "led.h"
#include "WM.h"
#include "HomeworkGUI.h"
#include "malloc.h"

WM_HWIN hMulti=0;       //�����ı�
KEYPADStructTypeDef keypad_dev;		

static int _DrawSkinFlex_BUTTON(const WIDGET_ITEM_DRAW_INFO * pDrawItemInfo); 
	
typedef struct 
{
	int          xPos;              //��ťXλ��
	int          yPos;              //��ťYλ��
	int          xSize;             //��ť���
	int          ySize;             //��ť�߶�
	const char * acLabel;           //��ť��������ַ�
	void (* pfDraw)(WM_HWIN hWin);  //��ť�Ļ��ƺ���
    const char * ButString;         //��ť����ʾ���ַ�
} BUTTON_DATA;


//�ڰ�ť�ϻ���λͼ������س�����ɾ������
static void _DrawCentered(WM_HWIN hWin, const GUI_BITMAP * pBM) 
{
	int xSizeWin;
	int ySizeWin;
	int xSizeBMP;
	int ySizeBMP;
	int xPos;
	int yPos;

	xSizeWin = WM_GetWindowSizeX(hWin);
	ySizeWin = WM_GetWindowSizeY(hWin);
	xSizeBMP = pBM->XSize;
	ySizeBMP = pBM->YSize;
	xPos = (xSizeWin - xSizeBMP) >> 1;
	yPos = (ySizeWin - ySizeBMP) >> 1;
	GUI_DrawBitmap(pBM, xPos, yPos);
}

static void _DrawBkSpc (WM_HWIN hWin) {  _DrawCentered(hWin, &bmBackSpace); }	//�����˸��(ɾ����)
static void _DrawEnter (WM_HWIN hWin) {  _DrawCentered(hWin, &bmEnter); }		//���ƻس���
static void _DrawShift (WM_HWIN hWin) {  _DrawCentered(hWin, &bmShift); }		//����shift��
static void _DrawRetern(WM_HWIN hWin) {  _DrawCentered(hWin, &bmReturn); }		//���Ʒ��ؼ�


//Ӣ�ļ���λ��
static  BUTTON_DATA _aEngButtonData[] = {
	{ (ENG_BUTTON_XPAC*1+ENG_BUTTON_WIDTH*0+ENG_BUTTONLINE1_XOFFSET),  ENG_BUTTON_LINE1YPOS, ENG_BUTTON_WIDTH,ENG_BUTTON_HEIGHT, "Q",0,"Q" },
	{ (ENG_BUTTON_XPAC*2+ENG_BUTTON_WIDTH*1+ENG_BUTTONLINE1_XOFFSET),  ENG_BUTTON_LINE1YPOS, ENG_BUTTON_WIDTH,ENG_BUTTON_HEIGHT, "W",0,"W" },
	{ (ENG_BUTTON_XPAC*3+ENG_BUTTON_WIDTH*2+ENG_BUTTONLINE1_XOFFSET),  ENG_BUTTON_LINE1YPOS, ENG_BUTTON_WIDTH,ENG_BUTTON_HEIGHT, "E",0,"E" },
	{ (ENG_BUTTON_XPAC*4+ENG_BUTTON_WIDTH*3+ENG_BUTTONLINE1_XOFFSET),  ENG_BUTTON_LINE1YPOS, ENG_BUTTON_WIDTH,ENG_BUTTON_HEIGHT, "R",0,"R" },
	{ (ENG_BUTTON_XPAC*5+ENG_BUTTON_WIDTH*4+ENG_BUTTONLINE1_XOFFSET),  ENG_BUTTON_LINE1YPOS, ENG_BUTTON_WIDTH,ENG_BUTTON_HEIGHT, "T",0,"T" },
	{ (ENG_BUTTON_XPAC*6+ENG_BUTTON_WIDTH*5+ENG_BUTTONLINE1_XOFFSET),  ENG_BUTTON_LINE1YPOS, ENG_BUTTON_WIDTH,ENG_BUTTON_HEIGHT, "Y",0,"Y" },
	{ (ENG_BUTTON_XPAC*7+ENG_BUTTON_WIDTH*6+ENG_BUTTONLINE1_XOFFSET),  ENG_BUTTON_LINE1YPOS, ENG_BUTTON_WIDTH,ENG_BUTTON_HEIGHT, "U",0,"U" },
	{ (ENG_BUTTON_XPAC*8+ENG_BUTTON_WIDTH*7+ENG_BUTTONLINE1_XOFFSET),  ENG_BUTTON_LINE1YPOS, ENG_BUTTON_WIDTH,ENG_BUTTON_HEIGHT, "I",0,"I" },
	{ (ENG_BUTTON_XPAC*9+ENG_BUTTON_WIDTH*8+ENG_BUTTONLINE1_XOFFSET),  ENG_BUTTON_LINE1YPOS, ENG_BUTTON_WIDTH,ENG_BUTTON_HEIGHT, "O",0,"O" },
	{ (ENG_BUTTON_XPAC*10+ENG_BUTTON_WIDTH*9+ENG_BUTTONLINE1_XOFFSET), ENG_BUTTON_LINE1YPOS, ENG_BUTTON_WIDTH,ENG_BUTTON_HEIGHT, "P",0,"P" },

	{ (ENG_BUTTON_XPAC*1+ENG_BUTTON_WIDTH*0+ENG_BUTTONLINE2_XOFFSET),  ENG_BUTTON_LINE2YPOS, ENG_BUTTON_WIDTH,ENG_BUTTON_HEIGHT, "A",0,"A" },
	{ (ENG_BUTTON_XPAC*2+ENG_BUTTON_WIDTH*1+ENG_BUTTONLINE2_XOFFSET),  ENG_BUTTON_LINE2YPOS, ENG_BUTTON_WIDTH,ENG_BUTTON_HEIGHT, "S",0,"S" },
	{ (ENG_BUTTON_XPAC*3+ENG_BUTTON_WIDTH*2+ENG_BUTTONLINE2_XOFFSET),  ENG_BUTTON_LINE2YPOS, ENG_BUTTON_WIDTH,ENG_BUTTON_HEIGHT, "D",0,"D" },
	{ (ENG_BUTTON_XPAC*4+ENG_BUTTON_WIDTH*3+ENG_BUTTONLINE2_XOFFSET),  ENG_BUTTON_LINE2YPOS, ENG_BUTTON_WIDTH,ENG_BUTTON_HEIGHT, "F",0,"F" },
	{ (ENG_BUTTON_XPAC*5+ENG_BUTTON_WIDTH*4+ENG_BUTTONLINE2_XOFFSET),  ENG_BUTTON_LINE2YPOS, ENG_BUTTON_WIDTH,ENG_BUTTON_HEIGHT, "G",0,"G" },
	{ (ENG_BUTTON_XPAC*6+ENG_BUTTON_WIDTH*5+ENG_BUTTONLINE2_XOFFSET),  ENG_BUTTON_LINE2YPOS, ENG_BUTTON_WIDTH,ENG_BUTTON_HEIGHT, "H",0,"H" },
	{ (ENG_BUTTON_XPAC*7+ENG_BUTTON_WIDTH*6+ENG_BUTTONLINE2_XOFFSET),  ENG_BUTTON_LINE2YPOS, ENG_BUTTON_WIDTH,ENG_BUTTON_HEIGHT, "J",0,"J" },
	{ (ENG_BUTTON_XPAC*8+ENG_BUTTON_WIDTH*7+ENG_BUTTONLINE2_XOFFSET),  ENG_BUTTON_LINE2YPOS, ENG_BUTTON_WIDTH,ENG_BUTTON_HEIGHT, "K",0,"K" },
	{ (ENG_BUTTON_XPAC*9+ENG_BUTTON_WIDTH*8+ENG_BUTTONLINE2_XOFFSET),  ENG_BUTTON_LINE2YPOS, ENG_BUTTON_WIDTH,ENG_BUTTON_HEIGHT, "L",0,"L" },

	{ (ENG_BUTTON_XPAC*1+ENG_BUTTON_WIDTH*0+ENG_BUTTONLINE3_XOFFSET),  ENG_BUTTON_LINE3YPOS, ENG_BUTTON_WIDTH+ENG_BUTTONSHIFT_XINC,ENG_BUTTON_HEIGHT, "\x19", _DrawShift },	//shift�ļ�ֵΪ0X19
	{ (ENG_BUTTON_XPAC*2+ENG_BUTTON_WIDTH*1+ENG_BUTTONLINE3_XOFFSET+ENG_BUTTONSHIFT_XINC),  ENG_BUTTON_LINE3YPOS, ENG_BUTTON_WIDTH,ENG_BUTTON_HEIGHT, "Z",0,"Z" },
	{ (ENG_BUTTON_XPAC*3+ENG_BUTTON_WIDTH*2+ENG_BUTTONLINE3_XOFFSET+ENG_BUTTONSHIFT_XINC),  ENG_BUTTON_LINE3YPOS, ENG_BUTTON_WIDTH,ENG_BUTTON_HEIGHT, "X",0,"X" },
	{ (ENG_BUTTON_XPAC*4+ENG_BUTTON_WIDTH*3+ENG_BUTTONLINE3_XOFFSET+ENG_BUTTONSHIFT_XINC),  ENG_BUTTON_LINE3YPOS, ENG_BUTTON_WIDTH,ENG_BUTTON_HEIGHT, "C",0,"C" },
	{ (ENG_BUTTON_XPAC*5+ENG_BUTTON_WIDTH*4+ENG_BUTTONLINE3_XOFFSET+ENG_BUTTONSHIFT_XINC),  ENG_BUTTON_LINE3YPOS, ENG_BUTTON_WIDTH,ENG_BUTTON_HEIGHT, "V",0,"V" },
	{ (ENG_BUTTON_XPAC*6+ENG_BUTTON_WIDTH*5+ENG_BUTTONLINE3_XOFFSET+ENG_BUTTONSHIFT_XINC),  ENG_BUTTON_LINE3YPOS, ENG_BUTTON_WIDTH,ENG_BUTTON_HEIGHT, "B",0,"B" },
	{ (ENG_BUTTON_XPAC*7+ENG_BUTTON_WIDTH*6+ENG_BUTTONLINE3_XOFFSET+ENG_BUTTONSHIFT_XINC),  ENG_BUTTON_LINE3YPOS, ENG_BUTTON_WIDTH,ENG_BUTTON_HEIGHT, "N",0,"N" },
	{ (ENG_BUTTON_XPAC*8+ENG_BUTTON_WIDTH*7+ENG_BUTTONLINE3_XOFFSET+ENG_BUTTONSHIFT_XINC),  ENG_BUTTON_LINE3YPOS, ENG_BUTTON_WIDTH,ENG_BUTTON_HEIGHT, "M",0,"M" },
	{ (ENG_BUTTON_XPAC*9+ENG_BUTTON_WIDTH*8+ENG_BUTTONLINE3_XOFFSET+ENG_BUTTONSHIFT_XINC),  ENG_BUTTON_LINE3YPOS, ENG_BUTTON_WIDTH+ENG_BUTTONBACK_XINC,ENG_BUTTON_HEIGHT, "\x08",_DrawBkSpc},
	
	{ (ENG_BUTTON_XPAC*1+ENG_BUTTON_WIDTH*0+ENG_BUTTONLINE4_XOFFSET),  ENG_BUTTON_LINE4YPOS, ENG_BUTTON_WIDTH+ENG_BUTTONFIGURE_XINC,ENG_BUTTON_HEIGHT, "\x02",0,"��"},						    //�л��������				
	{ (ENG_BUTTON_XPAC*2+ENG_BUTTON_WIDTH*1+ENG_BUTTONLINE4_XOFFSET+ENG_BUTTONFIGURE_XINC),  ENG_BUTTON_LINE4YPOS, ENG_BUTTON_WIDTH+ENG_BUTTONSYMBOL_XINC,ENG_BUTTON_HEIGHT,  "\x03",0,"��"},	//�л����ż���
	{ (ENG_BUTTON_XPAC*3+ENG_BUTTON_WIDTH*2+ENG_BUTTONLINE4_XOFFSET+ENG_BUTTONFIGURE_XINC+ENG_BUTTONSYMBOL_XINC),  ENG_BUTTON_LINE4YPOS, ENG_BUTTON_WIDTH,ENG_BUTTON_HEIGHT,  ",",0,"," },
	{ (ENG_BUTTON_XPAC*4+ENG_BUTTON_WIDTH*3+ENG_BUTTONLINE4_XOFFSET+ENG_BUTTONFIGURE_XINC+ENG_BUTTONSYMBOL_XINC),  ENG_BUTTON_LINE4YPOS, ENG_BUTTON_WIDTH+ENG_BUTTONSPACE_XINC,ENG_BUTTON_HEIGHT,  " ",0,"����"},
	{ (ENG_BUTTON_XPAC*5+ENG_BUTTON_WIDTH*4+ENG_BUTTONLINE4_XOFFSET+ENG_BUTTONFIGURE_XINC+ENG_BUTTONSYMBOL_XINC+ENG_BUTTONSPACE_XINC),  ENG_BUTTON_LINE4YPOS, ENG_BUTTON_WIDTH,ENG_BUTTON_HEIGHT, "." ,0,"."},
	{ (ENG_BUTTON_XPAC*6+ENG_BUTTON_WIDTH*5+ENG_BUTTONLINE4_XOFFSET+ENG_BUTTONFIGURE_XINC+ENG_BUTTONSYMBOL_XINC+ENG_BUTTONSPACE_XINC),  ENG_BUTTON_LINE4YPOS, ENG_BUTTON_WIDTH+ENG_BUTTONENTER_XINC,ENG_BUTTON_HEIGHT, "\x0d",_DrawEnter},
};

//���ּ���λ��
static  BUTTON_DATA _aNumButtonData[] = {
	{ (NUM_BUTTON_XPAC*1+NUM_BUTTON_WIDTH*0+NUM_BUTTON_XOFFSET),  NUM_BUTTON_LINE1YPOS,(NUM_BUTTON_WIDTH-NUM_BUTTONSING_XINC),NUM_BUTTON_HEIGHT, "+",0,"+" },
	{ (NUM_BUTTON_XPAC*2+NUM_BUTTON_WIDTH*1+NUM_BUTTON_XOFFSET-NUM_BUTTONSING_XINC),  NUM_BUTTON_LINE1YPOS, NUM_BUTTON_WIDTH,NUM_BUTTON_HEIGHT, "1" ,0,"1"},
	{ (NUM_BUTTON_XPAC*3+NUM_BUTTON_WIDTH*2+NUM_BUTTON_XOFFSET-NUM_BUTTONSING_XINC),  NUM_BUTTON_LINE1YPOS, NUM_BUTTON_WIDTH,NUM_BUTTON_HEIGHT, "2" ,0,"2"},
	{ (NUM_BUTTON_XPAC*4+NUM_BUTTON_WIDTH*3+NUM_BUTTON_XOFFSET-NUM_BUTTONSING_XINC),  NUM_BUTTON_LINE1YPOS, NUM_BUTTON_WIDTH,NUM_BUTTON_HEIGHT, "3" ,0,"3"},
	{ (NUM_BUTTON_XPAC*5+NUM_BUTTON_WIDTH*4+NUM_BUTTON_XOFFSET-NUM_BUTTONSING_XINC),  NUM_BUTTON_LINE1YPOS, (NUM_BUTTON_WIDTH-NUM_BUTTONBACKSPACE_XINC),NUM_BUTTON_HEIGHT, "\x08",_DrawBkSpc},	//ɾ����
  
	{ (NUM_BUTTON_XPAC*1+NUM_BUTTON_WIDTH*0+NUM_BUTTON_XOFFSET),  NUM_BUTTON_LINE2YPOS,(NUM_BUTTON_WIDTH-NUM_BUTTONSING_XINC),NUM_BUTTON_HEIGHT, "-",0,"-" },
	{ (NUM_BUTTON_XPAC*2+NUM_BUTTON_WIDTH*1+NUM_BUTTON_XOFFSET-NUM_BUTTONSING_XINC),  NUM_BUTTON_LINE2YPOS, NUM_BUTTON_WIDTH,NUM_BUTTON_HEIGHT, "4" ,0,"4"},
	{ (NUM_BUTTON_XPAC*3+NUM_BUTTON_WIDTH*2+NUM_BUTTON_XOFFSET-NUM_BUTTONSING_XINC),  NUM_BUTTON_LINE2YPOS, NUM_BUTTON_WIDTH,NUM_BUTTON_HEIGHT, "5" ,0,"5"},
	{ (NUM_BUTTON_XPAC*4+NUM_BUTTON_WIDTH*3+NUM_BUTTON_XOFFSET-NUM_BUTTONSING_XINC),  NUM_BUTTON_LINE2YPOS, NUM_BUTTON_WIDTH,NUM_BUTTON_HEIGHT, "6" ,0,"6"},
	{ (NUM_BUTTON_XPAC*5+NUM_BUTTON_WIDTH*4+NUM_BUTTON_XOFFSET-NUM_BUTTONSING_XINC),  NUM_BUTTON_LINE2YPOS, (NUM_BUTTON_WIDTH-NUM_BUTTONSPACE_XINC),NUM_BUTTON_HEIGHT,  " ",0,"����" },	//�ո��
  
	{ (NUM_BUTTON_XPAC*1+NUM_BUTTON_WIDTH*0+NUM_BUTTON_XOFFSET),  NUM_BUTTON_LINE3YPOS,(NUM_BUTTON_WIDTH-NUM_BUTTONSING_XINC),NUM_BUTTON_HEIGHT, "/",0,"/" },
	{ (NUM_BUTTON_XPAC*2+NUM_BUTTON_WIDTH*1+NUM_BUTTON_XOFFSET-NUM_BUTTONSING_XINC),  NUM_BUTTON_LINE3YPOS, NUM_BUTTON_WIDTH,NUM_BUTTON_HEIGHT, "7" ,0,"7"},
	{ (NUM_BUTTON_XPAC*3+NUM_BUTTON_WIDTH*2+NUM_BUTTON_XOFFSET-NUM_BUTTONSING_XINC),  NUM_BUTTON_LINE3YPOS, NUM_BUTTON_WIDTH,NUM_BUTTON_HEIGHT, "8" ,0,"8"},
	{ (NUM_BUTTON_XPAC*4+NUM_BUTTON_WIDTH*3+NUM_BUTTON_XOFFSET-NUM_BUTTONSING_XINC),  NUM_BUTTON_LINE3YPOS, NUM_BUTTON_WIDTH,NUM_BUTTON_HEIGHT, "9" ,0,"9"},
	{ (NUM_BUTTON_XPAC*5+NUM_BUTTON_WIDTH*4+NUM_BUTTON_XOFFSET-NUM_BUTTONSING_XINC),  NUM_BUTTON_LINE3YPOS, (NUM_BUTTON_WIDTH-NUM_BUTTONENTER_XINC),NUM_BUTTON_HEIGHT, "\x0d",_DrawEnter},	//�س���
  
	{ (NUM_BUTTON_XPAC*1+NUM_BUTTON_WIDTH*0+NUM_BUTTON_XOFFSET),  NUM_BUTTON_LINE4YPOS,(NUM_BUTTON_WIDTH-NUM_BUTTONSING_XINC),NUM_BUTTON_HEIGHT, "@",0,"@" },
	{ (NUM_BUTTON_XPAC*2+NUM_BUTTON_WIDTH*1+NUM_BUTTON_XOFFSET-NUM_BUTTONSING_XINC),  NUM_BUTTON_LINE4YPOS, NUM_BUTTON_WIDTH,NUM_BUTTON_HEIGHT, "," ,0,","},
	{ (NUM_BUTTON_XPAC*3+NUM_BUTTON_WIDTH*2+NUM_BUTTON_XOFFSET-NUM_BUTTONSING_XINC),  NUM_BUTTON_LINE4YPOS, NUM_BUTTON_WIDTH,NUM_BUTTON_HEIGHT, "0" ,0,"0"},
	{ (NUM_BUTTON_XPAC*4+NUM_BUTTON_WIDTH*3+NUM_BUTTON_XOFFSET-NUM_BUTTONSING_XINC),  NUM_BUTTON_LINE4YPOS, NUM_BUTTON_WIDTH,NUM_BUTTON_HEIGHT, "." ,0,"."},
	{ (NUM_BUTTON_XPAC*5+NUM_BUTTON_WIDTH*4+NUM_BUTTON_XOFFSET-NUM_BUTTONSING_XINC),  NUM_BUTTON_LINE4YPOS, (NUM_BUTTON_WIDTH-NUM_BUTTONRETURN_XINC),NUM_BUTTON_HEIGHT, "\x04",_DrawRetern },//���ؼ�
};

//���ż��̲���
static  BUTTON_DATA _aSinButtonData[][20] = {
	{
		{ (SIGN_BUTTON_XPAC*1+SING_BUTTON_WIDTH*0+SIGN_BUTTON_XOFFSET),  SIGN_BUTTON_LINE1YPOS, SING_BUTTON_WIDTH,SIGN_BUTTON_HEIGHT, "~" ,0,"~"},
		{ (SIGN_BUTTON_XPAC*2+SING_BUTTON_WIDTH*1+SIGN_BUTTON_XOFFSET),  SIGN_BUTTON_LINE1YPOS, SING_BUTTON_WIDTH,SIGN_BUTTON_HEIGHT, "-" ,0,"-"},
		{ (SIGN_BUTTON_XPAC*3+SING_BUTTON_WIDTH*2+SIGN_BUTTON_XOFFSET),  SIGN_BUTTON_LINE1YPOS, SING_BUTTON_WIDTH,SIGN_BUTTON_HEIGHT, ":" ,0,":"},
		{ (SIGN_BUTTON_XPAC*4+SING_BUTTON_WIDTH*3+SIGN_BUTTON_XOFFSET),  SIGN_BUTTON_LINE1YPOS, SING_BUTTON_WIDTH,SIGN_BUTTON_HEIGHT, "_" ,0,"_"},
		{ (SIGN_BUTTON_XPAC*5+SING_BUTTON_WIDTH*4+SIGN_BUTTON_XOFFSET),  SIGN_BUTTON_LINE1YPOS, (SING_BUTTON_WIDTH-SIGN_BUTTONCONTR_XINC),SIGN_BUTTON_HEIGHT, "\x08",_DrawBkSpc},	//ɾ����
  
		{ (SIGN_BUTTON_XPAC*1+SING_BUTTON_WIDTH*0+SIGN_BUTTON_XOFFSET),  SIGN_BUTTON_LINE2YPOS, SING_BUTTON_WIDTH,SIGN_BUTTON_HEIGHT, "\"",0,"\"" },
		{ (SIGN_BUTTON_XPAC*2+SING_BUTTON_WIDTH*1+SIGN_BUTTON_XOFFSET),  SIGN_BUTTON_LINE2YPOS, SING_BUTTON_WIDTH,SIGN_BUTTON_HEIGHT, "," ,0,","},
		{ (SIGN_BUTTON_XPAC*3+SING_BUTTON_WIDTH*2+SIGN_BUTTON_XOFFSET),  SIGN_BUTTON_LINE2YPOS, SING_BUTTON_WIDTH,SIGN_BUTTON_HEIGHT, "<" ,0,"<"},
		{ (SIGN_BUTTON_XPAC*4+SING_BUTTON_WIDTH*3+SIGN_BUTTON_XOFFSET),  SIGN_BUTTON_LINE2YPOS, SING_BUTTON_WIDTH,SIGN_BUTTON_HEIGHT, ">" ,0,">"},
		{ (SIGN_BUTTON_XPAC*5+SING_BUTTON_WIDTH*4+SIGN_BUTTON_XOFFSET),  SIGN_BUTTON_LINE2YPOS, (SING_BUTTON_WIDTH-SIGN_BUTTONCONTR_XINC),SIGN_BUTTON_HEIGHT,  "\x05",0,"��"},	//��һҳ
  
		{ (SIGN_BUTTON_XPAC*1+SING_BUTTON_WIDTH*0+SIGN_BUTTON_XOFFSET),  SIGN_BUTTON_LINE3YPOS, SING_BUTTON_WIDTH,NUM_BUTTON_HEIGHT, "*",0,"*" },
		{ (SIGN_BUTTON_XPAC*2+SING_BUTTON_WIDTH*1+SIGN_BUTTON_XOFFSET),  SIGN_BUTTON_LINE3YPOS, SING_BUTTON_WIDTH,NUM_BUTTON_HEIGHT, "+",0,"+" },
		{ (SIGN_BUTTON_XPAC*3+SING_BUTTON_WIDTH*2+SIGN_BUTTON_XOFFSET),  SIGN_BUTTON_LINE3YPOS, SING_BUTTON_WIDTH,NUM_BUTTON_HEIGHT, ".",0,"." },
		{ (SIGN_BUTTON_XPAC*4+SING_BUTTON_WIDTH*3+SIGN_BUTTON_XOFFSET),  SIGN_BUTTON_LINE3YPOS, SING_BUTTON_WIDTH,NUM_BUTTON_HEIGHT, "#",0,"#" },
		{ (SIGN_BUTTON_XPAC*5+SING_BUTTON_WIDTH*4+SIGN_BUTTON_XOFFSET),  SIGN_BUTTON_LINE3YPOS, (SING_BUTTON_WIDTH-SIGN_BUTTONCONTR_XINC),SIGN_BUTTON_HEIGHT, "\x06",0,"��"},	//��һҳ
  
		{ (SIGN_BUTTON_XPAC*1+SING_BUTTON_WIDTH*0+SIGN_BUTTON_XOFFSET),  SIGN_BUTTON_LINE4YPOS, SING_BUTTON_WIDTH,NUM_BUTTON_HEIGHT, "%",0,"%"},
		{ (SIGN_BUTTON_XPAC*2+SING_BUTTON_WIDTH*1+SIGN_BUTTON_XOFFSET),  SIGN_BUTTON_LINE4YPOS, SING_BUTTON_WIDTH,NUM_BUTTON_HEIGHT, "@",0,"@" },
		{ (SIGN_BUTTON_XPAC*3+SING_BUTTON_WIDTH*2+SIGN_BUTTON_XOFFSET),  SIGN_BUTTON_LINE4YPOS, SING_BUTTON_WIDTH,NUM_BUTTON_HEIGHT, "!",0,"!" },
		{ (SIGN_BUTTON_XPAC*4+SING_BUTTON_WIDTH*3+SIGN_BUTTON_XOFFSET),  SIGN_BUTTON_LINE4YPOS, SING_BUTTON_WIDTH,NUM_BUTTON_HEIGHT, "/",0,"/" },
		{ (SIGN_BUTTON_XPAC*5+SING_BUTTON_WIDTH*4+SIGN_BUTTON_XOFFSET),  SIGN_BUTTON_LINE4YPOS, (SING_BUTTON_WIDTH-SIGN_BUTTONCONTR_XINC),SIGN_BUTTON_HEIGHT, "\x04",_DrawRetern },//���ؼ�
	},
	
	{
		{ (SIGN_BUTTON_XPAC*1+SING_BUTTON_WIDTH*0+SIGN_BUTTON_XOFFSET),  SIGN_BUTTON_LINE1YPOS, SING_BUTTON_WIDTH,SIGN_BUTTON_HEIGHT, "?",0,"?" },
		{ (SIGN_BUTTON_XPAC*2+SING_BUTTON_WIDTH*1+SIGN_BUTTON_XOFFSET),  SIGN_BUTTON_LINE1YPOS, SING_BUTTON_WIDTH,SIGN_BUTTON_HEIGHT, ";",0,";" },
		{ (SIGN_BUTTON_XPAC*3+SING_BUTTON_WIDTH*2+SIGN_BUTTON_XOFFSET),  SIGN_BUTTON_LINE1YPOS, SING_BUTTON_WIDTH,SIGN_BUTTON_HEIGHT, "`",0,"`" },
		{ (SIGN_BUTTON_XPAC*4+SING_BUTTON_WIDTH*3+SIGN_BUTTON_XOFFSET),  SIGN_BUTTON_LINE1YPOS, SING_BUTTON_WIDTH,SIGN_BUTTON_HEIGHT, "$",0,"$" },
		{ (SIGN_BUTTON_XPAC*5+SING_BUTTON_WIDTH*4+SIGN_BUTTON_XOFFSET),  SIGN_BUTTON_LINE1YPOS, (SING_BUTTON_WIDTH-SIGN_BUTTONCONTR_XINC),SIGN_BUTTON_HEIGHT, "\x08",_DrawBkSpc},	//ɾ����
  
		{ (SIGN_BUTTON_XPAC*1+SING_BUTTON_WIDTH*0+SIGN_BUTTON_XOFFSET),  SIGN_BUTTON_LINE2YPOS, SING_BUTTON_WIDTH,SIGN_BUTTON_HEIGHT, "^" ,0,"^"},
		{ (SIGN_BUTTON_XPAC*2+SING_BUTTON_WIDTH*1+SIGN_BUTTON_XOFFSET),  SIGN_BUTTON_LINE2YPOS, SING_BUTTON_WIDTH,SIGN_BUTTON_HEIGHT, "\\",0,"\\" },
		{ (SIGN_BUTTON_XPAC*3+SING_BUTTON_WIDTH*2+SIGN_BUTTON_XOFFSET),  SIGN_BUTTON_LINE2YPOS, SING_BUTTON_WIDTH,SIGN_BUTTON_HEIGHT, "(" ,0,"("},
		{ (SIGN_BUTTON_XPAC*4+SING_BUTTON_WIDTH*3+SIGN_BUTTON_XOFFSET),  SIGN_BUTTON_LINE2YPOS, SING_BUTTON_WIDTH,SIGN_BUTTON_HEIGHT, ")" ,0,")"},
		{ (SIGN_BUTTON_XPAC*5+SING_BUTTON_WIDTH*4+SIGN_BUTTON_XOFFSET),  SIGN_BUTTON_LINE2YPOS, (SING_BUTTON_WIDTH-SIGN_BUTTONCONTR_XINC),SIGN_BUTTON_HEIGHT,  "\x05",0,"��" },	//��һҳ
  
		{ (SIGN_BUTTON_XPAC*1+SING_BUTTON_WIDTH*0+SIGN_BUTTON_XOFFSET),  SIGN_BUTTON_LINE3YPOS, SING_BUTTON_WIDTH,NUM_BUTTON_HEIGHT, "&",0,"&" },
		{ (SIGN_BUTTON_XPAC*2+SING_BUTTON_WIDTH*1+SIGN_BUTTON_XOFFSET),  SIGN_BUTTON_LINE3YPOS, SING_BUTTON_WIDTH,NUM_BUTTON_HEIGHT, "=",0,"=" },
		{ (SIGN_BUTTON_XPAC*3+SING_BUTTON_WIDTH*2+SIGN_BUTTON_XOFFSET),  SIGN_BUTTON_LINE3YPOS, SING_BUTTON_WIDTH,NUM_BUTTON_HEIGHT, "'",0,"'" },
		{ (SIGN_BUTTON_XPAC*4+SING_BUTTON_WIDTH*3+SIGN_BUTTON_XOFFSET),  SIGN_BUTTON_LINE3YPOS, SING_BUTTON_WIDTH,NUM_BUTTON_HEIGHT, "|",0,"|" },
		{ (SIGN_BUTTON_XPAC*5+SING_BUTTON_WIDTH*4+SIGN_BUTTON_XOFFSET),  SIGN_BUTTON_LINE3YPOS, (SING_BUTTON_WIDTH-SIGN_BUTTONCONTR_XINC),SIGN_BUTTON_HEIGHT, "\x06",0,"��"},	//��һҳ
  
		{ (SIGN_BUTTON_XPAC*1+SING_BUTTON_WIDTH*0+SIGN_BUTTON_XOFFSET),  SIGN_BUTTON_LINE4YPOS, SING_BUTTON_WIDTH,NUM_BUTTON_HEIGHT, "[",0,"[" },
		{ (SIGN_BUTTON_XPAC*2+SING_BUTTON_WIDTH*1+SIGN_BUTTON_XOFFSET),  SIGN_BUTTON_LINE4YPOS, SING_BUTTON_WIDTH,NUM_BUTTON_HEIGHT, "]",0,"]" },
		{ (SIGN_BUTTON_XPAC*3+SING_BUTTON_WIDTH*2+SIGN_BUTTON_XOFFSET),  SIGN_BUTTON_LINE4YPOS, SING_BUTTON_WIDTH,NUM_BUTTON_HEIGHT, "{",0,"{" },
		{ (SIGN_BUTTON_XPAC*4+SING_BUTTON_WIDTH*3+SIGN_BUTTON_XOFFSET),  SIGN_BUTTON_LINE4YPOS, SING_BUTTON_WIDTH,NUM_BUTTON_HEIGHT, "}",0,"}" },
		{ (SIGN_BUTTON_XPAC*5+SING_BUTTON_WIDTH*4+SIGN_BUTTON_XOFFSET),  SIGN_BUTTON_LINE4YPOS, (SING_BUTTON_WIDTH-SIGN_BUTTONCONTR_XINC),SIGN_BUTTON_HEIGHT, "\x04",_DrawRetern },//���ؼ�
	},
};


//��Сд�л�����
void changecase(u8 change_flag,BUTTON_DATA *buttondata)
{
	u16 i;
	char	   	NumToStr;
	char buffer[5];
	if(keypad_dev.LowerCase)		//Сд,��ť�޸�ΪСд
	{
		//���İ�ť���ַ�
		for (i = 0; i < 27; i++) 	
		{
			if(i==19) continue;
			NumToStr=(char)((int)*(buttondata[i].acLabel)+0x20);  
            sprintf(buffer,"%c",NumToStr);              //ת��Ϊ�ַ�
			BUTTON_SetFont(keypad_dev.EngButton_Handle[i],&GUI_FontHZ24); 
			BUTTON_SetText(keypad_dev.EngButton_Handle[i],buffer);
			BUTTON_SetFocussable(keypad_dev.EngButton_Handle[i], 0);
        }
    }
	else 	//��д����ť�ϵ��ַ���Ϊ��д
	{
		//���İ�ť���ַ�
		for (i = 0; i < 27; i++) 	
		{
			if(i==19) continue;
			BUTTON_SetFont(keypad_dev.EngButton_Handle[i],&GUI_FontHZ24);
			BUTTON_SetText(keypad_dev.EngButton_Handle[i], buttondata[i].acLabel);
			BUTTON_SetFocussable(keypad_dev.EngButton_Handle[i], 0);
		}
	}
}

//����Ӣ�ļ���
//hWin:������
void drawenglish_pad(WM_HWIN hWin)
{
	u16 	i;
	
	//�������̰�ť
	BUTTON_SetDefaultSkin(_DrawSkinFlex_BUTTON); //����BUTTON��Ƥ��	
	for (i = 0; i < GUI_COUNTOF(_aEngButtonData); i++) 
	{
		//������ť
		keypad_dev.EngButton_Handle[i] = BUTTON_CreateEx(_aEngButtonData[i].xPos, _aEngButtonData[i].yPos, _aEngButtonData[i].xSize, _aEngButtonData[i].ySize, 
                             hWin, WM_CF_SHOW | WM_CF_HASTRANS, 0, ID_BUTTON + i);
		BUTTON_SetFont(keypad_dev.EngButton_Handle[i],&GUI_FontHZ24);
        BUTTON_SetText(keypad_dev.EngButton_Handle[i], _aEngButtonData[i].ButString);
		BUTTON_SetFocussable(keypad_dev.EngButton_Handle[i], 0);
	}
	changecase(keypad_dev.LowerCase,_aEngButtonData);
}

//�������ּ���
void drawnumber_pad(WM_HWIN hWin)
{
	u16 i;
	for(i=0;i<GUI_COUNTOF(_aNumButtonData);i++)								//�������ּ���
	{
		//������ť
		BUTTON_SetDefaultSkin(_DrawSkinFlex_BUTTON); //����BUTTON��Ƥ��
		keypad_dev.NumButton_Handle[i] = BUTTON_CreateEx(_aNumButtonData[i].xPos, _aNumButtonData[i].yPos, _aNumButtonData[i].xSize, _aNumButtonData[i].ySize, 
                             hWin, WM_CF_SHOW | WM_CF_HASTRANS, 0, ID_BUTTON + i);
		BUTTON_SetFont(keypad_dev.NumButton_Handle[i],&GUI_FontHZ24);
		BUTTON_SetText(keypad_dev.NumButton_Handle[i], _aNumButtonData[i].ButString);
		BUTTON_SetFocussable(keypad_dev.NumButton_Handle[i], 0);
	}
}

//���Ʒ��ż���
void drawsign_pad(WM_HWIN hWin)
{
	u16 i;
	for(i=0;i<GUI_COUNTOF(_aSinButtonData[keypad_dev.signpad_flag]);i++)					//�������ż���
	{
		//������ť
      	BUTTON_SetDefaultSkin(_DrawSkinFlex_BUTTON); //����BUTTON��Ƥ��
		keypad_dev.SignButton_Handle[i] = BUTTON_CreateEx(_aSinButtonData[keypad_dev.signpad_flag][i].xPos, _aSinButtonData[keypad_dev.signpad_flag][i].yPos,
											_aSinButtonData[keypad_dev.signpad_flag][i].xSize,_aSinButtonData[keypad_dev.signpad_flag][i].ySize,
											hWin, WM_CF_SHOW | WM_CF_HASTRANS, 0, ID_BUTTON + i);
		BUTTON_SetFont(keypad_dev.SignButton_Handle[i],&GUI_FontHZ24);
		BUTTON_SetText(keypad_dev.SignButton_Handle[i],_aSinButtonData[keypad_dev.signpad_flag][i].ButString);
		BUTTON_SetFocussable(keypad_dev.SignButton_Handle[i], 0);
	}
}



//Ӣ�ļ��̴�����
//buttondata:Ӣ�ļ��̰�ť����
//Id:��ťID
//*pMsg:ָ����Ϣ
void engkeypad_process(BUTTON_DATA *buttondata,int Id,WM_MESSAGE *pMsg)
{
	char 		c;
	WM_MESSAGE 	Msg;
    u8          KeyID;

    KeyID=Id-ID_BUTTON;
	if (buttondata[Id - ID_BUTTON].acLabel) 
	{
		c = buttondata[Id - ID_BUTTON].acLabel[0];
        if(KeyID == 19)                             //��Сд�л�
        {
            keypad_dev.LowerCase ^= 1;		        //��LowerCase����������
			changecase(keypad_dev.LowerCase,buttondata);                
        }
        else  if(KeyID == 28)                       //�л������ּ���
        {
            keypad_dev.padtype=NUMBER_KEYPAD;		//���ּ���
			Msg.MsgId=CHANGE_ENGLISHPADTO_NUMPAD;
			Msg.hWinSrc=pMsg->hWin;
			WM_SendMessage(keypad_dev.hKeypad,&Msg);
        }    
        else if(KeyID == 29)                        //�л������ż���
        {
            keypad_dev.padtype=SIGN_KEYPAD;		
			Msg.MsgId=CHANGE_ENGLISHPADTO_SIGNPAD1;
			Msg.hWinSrc=pMsg->hWin;
			WM_SendMessage(keypad_dev.hKeypad,&Msg);
        }
		else if(KeyID == 33)                        //enter��
        {
			mymemset(keypad_dev.FinalData,0,sizeof(keypad_dev.FinalData));
			EDIT_GetText(WM_GetDialogItem(DisplayDialogMsg.hFrame, GUI_ID_EDIT9),(char *)keypad_dev.FinalData,sizeof(keypad_dev.FinalData));
			keypad_dev.Finish = 1;
			WM_DeleteWindow(DisplayDialogMsg.hFrame);
        }
        else
        {
            if (keypad_dev.LowerCase && (c>=0X41) && (c<=0X5A)) //Сд
			{
				c += 0x20;
			}
			GUI_StoreKeyMsg(c, 1);	//����Ϣ������̻�����������״̬
			GUI_StoreKeyMsg(c, 0);	//����Ϣ������̻��������ɿ�״̬
        }
	}
}

//���ּ��̴�����
//buttondata:���ּ��̰�ť����
//Id:��ťID
//*pMsg:ָ����Ϣ
void numkeypad_process(BUTTON_DATA *buttondata,int Id,WM_MESSAGE *pMsg)	
{
	char 		c;
	WM_MESSAGE 	Msg;
    u8          KeyID;
    
    KeyID=Id-ID_BUTTON;
	if (buttondata[Id - ID_BUTTON].acLabel) 
	{
		c = buttondata[Id - ID_BUTTON].acLabel[0];

        if(KeyID == 19)                             //����Ӣ�ļ���
        {
            keypad_dev.padtype=ENGLISH_KEYPAD;		//Ӣ�ļ���
			Msg.MsgId=CHANGE_NUMPADTO_ENGLISHPAD;
			Msg.hWinSrc=pMsg->hWin;
			WM_SendMessage(keypad_dev.hKeypad,&Msg); 
        }
		else if(KeyID == 14)                        //enter��
        {
			mymemset(keypad_dev.FinalData,0,sizeof(keypad_dev.FinalData));
			EDIT_GetText(WM_GetDialogItem(DisplayDialogMsg.hFrame, GUI_ID_EDIT9),(char *)keypad_dev.FinalData,sizeof(keypad_dev.FinalData));
			keypad_dev.Finish = 1;
			WM_DeleteWindow(DisplayDialogMsg.hFrame);
        }
        else
        {
            GUI_StoreKeyMsg(c, 1);				//����Ϣ������̻�����������״̬
			GUI_StoreKeyMsg(c, 0);				//����Ϣ������̻��������ɿ�״̬ 
        }
	}
}
//���ż��̴�����
//buttondata:���ּ��̰�ť����
//Id:��ťID
//*pMsg:ָ����Ϣ
void signkeypad_process(BUTTON_DATA *buttondata,int Id,WM_MESSAGE *pMsg)	
{
	char 		c;
	WM_MESSAGE 	Msg;
    u8          KeyID;
    
    KeyID=Id-ID_BUTTON;
	if (buttondata[Id - ID_BUTTON].acLabel) 
	{
		c = buttondata[Id - ID_BUTTON].acLabel[0];
        if(KeyID == 19)                             //���ص�Ӣ�ļ���
        {
            keypad_dev.padtype=ENGLISH_KEYPAD;		//Ӣ�ļ���
			Msg.MsgId=CHANGE_SIGNPADTO_ENGLISHPAD;
			Msg.hWinSrc=pMsg->hWin;
			WM_SendMessage(keypad_dev.hKeypad,&Msg);  
        }
        else if(KeyID == 14)                        //��һ�����ż���
        {
            if(keypad_dev.signpad_flag<(keypad_dev.signpad_num-1))	
			{
				keypad_dev.signpad_flag++;	        //���ż���������һ
				Msg.MsgId=CHANGE_SIGNPAD;
				Msg.hWinSrc=pMsg->hWin;
				WM_SendMessage(keypad_dev.hKeypad,&Msg);
			}
        } 
        else if(KeyID == 9)                         //��һ�����ż���
        {
            if(keypad_dev.signpad_flag>0)	
			{
				keypad_dev.signpad_flag--;          //���ż���������1
				Msg.MsgId=CHANGE_SIGNPAD;
				Msg.hWinSrc=pMsg->hWin;
				WM_SendMessage(keypad_dev.hKeypad,&Msg);
			}  
        }
        else
        {
            GUI_StoreKeyMsg(c, 1);				//����Ϣ������̻�����������״̬
			GUI_StoreKeyMsg(c, 0);				//����Ϣ������̻��������ɿ�״̬ 
        }
	}
}



//����BUTTONҪʹ�õ�Ƥ��
static int _DrawSkinFlex_BUTTON(const WIDGET_ITEM_DRAW_INFO * pDrawItemInfo) 
{
	static GUI_MEMDEV_Handle ahMemEdges[4];
	GUI_MEMDEV_Handle        hMem;
	WM_HWIN                  hWin;
	unsigned                 i;
	int                      xPos;
	int                      yPos;
	int                      xSize;
	int                      ySize;
	int                      x;
	int                      y;
	int                      Id;
	int                      IsPressed;
    int                      butheigh;
	void (* pfDraw)(WM_HWIN hWin);

	x = 0;
	y = 0;
	switch (pDrawItemInfo->Cmd) 
	{
		case WIDGET_ITEM_CREATE:
			if (ahMemEdges[0] == 0) 
			{
				//����һ��С�İ������ڣ����ǲ��ɼ�
				hWin = WM_CreateWindowAsChild(0, 0, 16, 16, WM_HBKWIN, WM_CF_SHOW | WM_CF_STAYONTOP, NULL, 0);
				WM_SelectWindow(hWin);
				//Ϊbutton��ÿ����Ե�����洢�豸
				for (i = 0; i < GUI_COUNTOF(ahMemEdges); i++) 
				{
					switch (i) 
					{
						case 0:
							x = 0;
							y = 0;
							break;
						case 1:
							x = -16;
							y = 0;
							break;
						case 2:
							x = -16;
							y = -16;
							break;
						case 3:
							x = 0;
							y = -16;
							break;
					}
					//�����Ŵ��豸
					hMem = GUI_MEMDEV_CreateFixed(0, 0, 16, 16, GUI_MEMDEV_NOTRANS, GUI_MEMDEV_APILIST_32, GUI_COLOR_CONV_8888);
					GUI_MEMDEV_Select(hMem);
					GUI_SetBkColor(GUI_TRANSPARENT);
					GUI_Clear();
					GUI_SetColor(COLOR_BORDER);
					GUI_DrawRoundedFrame(x, y, x + 31, y + 31, 16, 4);	//���ƻ�����İ�ť�Ľǵ�Բ��

					//������ָ���Ĵ�С
					ahMemEdges[i] = GUI_MEMDEV_CreateFixed(0, 0, 4, 4, GUI_MEMDEV_NOTRANS, GUI_MEMDEV_APILIST_32, GUI_COLOR_CONV_8888);
					GUI_MEMDEV_Select(ahMemEdges[i]);
					GUI_SetBkColor(GUI_TRANSPARENT);
					GUI_Clear();
					GUI_MEMDEV_RotateHQ(hMem, ahMemEdges[i], -6, -6, 0, 250);
					GUI_MEMDEV_Delete(hMem);
					GUI_MEMDEV_Select(0);
				}
				WM_DeleteWindow(hWin);
			}
			break;
		case WIDGET_ITEM_DRAW_TEXT:
			//�ڰ�ť����ʾ�Զ������Ĭ�ϵ���ɫ�����ı�
			hWin = pDrawItemInfo->hWin;
			Id   = WM_GetId(hWin);
			i    = Id - ID_BUTTON;
			if(keypad_dev.padtype==ENGLISH_KEYPAD) pfDraw = _aEngButtonData[i].pfDraw;
			else if(keypad_dev.padtype==NUMBER_KEYPAD) pfDraw = _aNumButtonData[i].pfDraw;
			else if(keypad_dev.padtype==SIGN_KEYPAD) pfDraw = _aSinButtonData[keypad_dev.signpad_flag][i].pfDraw;
       
		
			if (pfDraw) pfDraw(hWin);
			else BUTTON_DrawSkinFlex(pDrawItemInfo);
			break;
		case WIDGET_ITEM_DRAW_BACKGROUND:
			//���ư�ť�ı���
			IsPressed = BUTTON_IsPressed(pDrawItemInfo->hWin); 	//�ж��Ƿ�ť�Ƿ񱻰���
			xPos      = WM_GetWindowOrgX(pDrawItemInfo->hWin);	//��ȡ��ťС���ߵ�X����ԭ��λ��
			yPos      = WM_GetWindowOrgY(pDrawItemInfo->hWin);	//��ȡ��ťС���ߵ�Y����ԭ��λ��
			xSize     = WM_GetWindowSizeX(pDrawItemInfo->hWin);	//��ȡ��ťС���ߵ�X��С
			ySize     = WM_GetWindowSizeY(pDrawItemInfo->hWin);	//��ȡ��ťС���ߵ�Y��С
			if (IsPressed) 										//�����ť������
			{
				GUI_DrawGradientRoundedV(0, 0, xSize - 1, ySize - 1, 4, BUTTON_COLOR2, BUTTON_COLOR3);//���ƴ�ֱ��ɫ�ݶ�����Բ�Ǿ���
			} 
			else 
			{
				GUI_DrawGradientRoundedV(0, 0, xSize - 1, ySize - 1, 4, BUTTON_COLOR0, BUTTON_COLOR1);//���ƴ�ֱ��ɫ�ݶ�����Բ�Ǿ���
			}
			GUI_SetColor(COLOR_BORDER);
			//���ư�ť�������߽�
			GUI_DrawHLine(        0, 4, xSize - 5);
			GUI_DrawHLine(ySize - 1, 4, xSize - 5);
			GUI_DrawVLine(        0, 4, ySize - 5);
			GUI_DrawVLine(xSize - 1, 4, ySize - 5);
			//�������洢�豸������д�뵱ǰѡ���洢�豸���ƶ�λ��	
            if(keypad_dev.padtype==CHINESE_KEYPAD) butheigh = 77;
            else butheigh = 68;
			GUI_MEMDEV_WriteAt(ahMemEdges[0], xPos +  0, yPos +  0);			    //��ť���Ͻ�Բ��
			GUI_MEMDEV_WriteAt(ahMemEdges[1], xPos + xSize - 4, yPos +  0);		    //��ť���Ͻ�Բ��
			GUI_MEMDEV_WriteAt(ahMemEdges[2], xPos + xSize - 4, yPos + butheigh);   //��ť���½�Բ��
			GUI_MEMDEV_WriteAt(ahMemEdges[3], xPos +  0, yPos + butheigh);			//��ť���½�Բ��
			break;
		default:
			return BUTTON_DrawSkinFlex(pDrawItemInfo);//���������ʹ��Ĭ�ϵ�Ƥ��
	}
	return 0;
}



//���º����6����ť
void keypad_hbtn_updc(u8* str)
{
    vu8 i=0;
    if((keypad_dev.dispagecur>=keypad_dev.dispagenum)&&keypad_dev.dispagenum)return;  //��ǰҳ���ڻ��ߴ�����ҳ��
    for(i=0;i<6;i++) keypad_dev.butname[i]=NULL;
    i=0;
    for(i=0;i<6;i++)
    {
        if((i+6*(keypad_dev.dispagecur))>(keypad_dev.hznum-1)) //������Ӧƴ���ĺ�������
        {
            keypad_dev.disbuf[i][0]=' ';
            keypad_dev.disbuf[i][1]=' ';
            keypad_dev.disbuf[i][2]='\0';
            keypad_dev.butname[i]=keypad_dev.disbuf[i];
        }else
        {
            keypad_dev.disbuf[i][0]=*str++;
            keypad_dev.disbuf[i][1]=*str++;
            keypad_dev.disbuf[i][2]='\0';
            keypad_dev.butname[i]=keypad_dev.disbuf[i];
        }
    }
}

//���ƺ����6����ť
void keypad_draw_hbtn(WM_HWIN *handle)
{
    u8 i;
    for(i=0;i<6;i++)
    {
        BUTTON_SetFont(handle[13+i],&GUI_FontHZ24);
        BUTTON_SetText(handle[13+i],(char*)keypad_dev.butname[i]);
    }

}
//keypad���ڵ��õĻص�����
static void _cbKeyPad(WM_MESSAGE * pMsg) 
{
	WM_HWIN    hWin;
	int        Id;
	int        NCode;
	int        xSize;
	int        ySize;
    u16        slen;
	unsigned   i;

	hWin = pMsg->hWin;
	switch (pMsg->MsgId) 
	{
		case WM_CREATE:				//����Ϣ�ڴ��ڴ������������ͣ��Ǵ��ڿ��Գ�ʼ���������κ��Ӵ���
			drawenglish_pad(hWin);	//����Ӣ�ļ���
			break;
		case WM_PAINT:
			//���Ʊ���
			xSize = WM_GetWindowSizeX(hWin);
			ySize = WM_GetWindowSizeY(hWin);
			GUI_SetColor(COLOR_BORDER);
			GUI_DrawRect(0, 0, xSize - 1, ySize - 1);
			GUI_DrawGradientV(1, 1, xSize - 2, ySize - 2, COLOR_KEYPAD0, COLOR_KEYPAD1);
			break;
		case WM_NOTIFY_PARENT:	//���߸����ڣ��Ӵ��ڷ����˸ı�
			//�뵱ǰ���㴰�ڷ��Ͱ�����Ϣ
			Id    = WM_GetId(pMsg->hWinSrc);
			NCode = pMsg->Data.v;
			switch (NCode) 
			{
				case WM_NOTIFICATION_RELEASED:				//��ť���ͷ�
					if(keypad_dev.padtype==ENGLISH_KEYPAD)	//Ӣ�ļ���
					{
						engkeypad_process(_aEngButtonData,Id,pMsg);		//����Ӣ�ļ���
					}
					else if(keypad_dev.padtype==NUMBER_KEYPAD)			//���ּ���
					{
						numkeypad_process(_aNumButtonData,Id,pMsg);		//�������ּ���
					}
					else if(keypad_dev.padtype==SIGN_KEYPAD)			
					{
						signkeypad_process(_aSinButtonData[keypad_dev.signpad_flag],Id,pMsg);
					}
                    
					break;
			}
			break;	
		case CHANGE_ENGLISHPADTO_NUMPAD:			//��Ӣ�ļ����л������ּ���
			for(i=0;i<GUI_COUNTOF(_aEngButtonData);i++) WM_DeleteWindow(keypad_dev.EngButton_Handle[i]);	
            for(i=0;i<GUI_COUNTOF(_aEngButtonData);i++) keypad_dev.EngButton_Handle[i]=0;
			drawnumber_pad(hWin);
			break;
		case CHANGE_NUMPADTO_ENGLISHPAD:			//�����ּ����л���Ӣ�ļ���
			for(i=0;i<GUI_COUNTOF(_aNumButtonData);i++) WM_DeleteWindow(keypad_dev.NumButton_Handle[i]);	
            for(i=0;i<GUI_COUNTOF(_aEngButtonData);i++) keypad_dev.NumButton_Handle[i]=0;
			drawenglish_pad(hWin);					
			break;
		case CHANGE_ENGLISHPADTO_SIGNPAD1:			//��Ӣ�ļ����л������ż���
			for(i=0;i<GUI_COUNTOF(_aEngButtonData);i++) WM_DeleteWindow(keypad_dev.EngButton_Handle[i]);
            for(i=0;i<GUI_COUNTOF(_aEngButtonData);i++) keypad_dev.SignButton_Handle[i]=0;
		    drawsign_pad(hWin);
			break;
		case CHANGE_SIGNPADTO_ENGLISHPAD:			//�ӷ��ż����л���Ӣ�ļ���
			for(i=0;i<GUI_COUNTOF(_aSinButtonData[keypad_dev.signpad_flag]);i++) WM_DeleteWindow(keypad_dev.SignButton_Handle[i]);
            for(i=0;i<GUI_COUNTOF(_aSinButtonData[keypad_dev.signpad_flag]);i++) keypad_dev.SignButton_Handle[i]=0;
			drawenglish_pad(hWin);
			break;
		case CHANGE_SIGNPAD:						//�л����ż���
			for(i=0;i<GUI_COUNTOF(_aSinButtonData[keypad_dev.signpad_flag]);i++) WM_DeleteWindow(keypad_dev.SignButton_Handle[i]);
            for(i=0;i<GUI_COUNTOF(_aSinButtonData[keypad_dev.signpad_flag]);i++) keypad_dev.SignButton_Handle[i]=0;
		    drawsign_pad(hWin);
			break;
        
        
       
	}
}


//�������ڻص�����
static void _cbBk(WM_MESSAGE * pMsg) 
{
	switch (pMsg->MsgId) 
	{
		case WM_PAINT:
			GUI_DrawGradientV(0, 0,lcddev.width,lcddev.height, COLOR_BACK0, COLOR_BACK1);
			break;
	}
}

static void _cbDisplayInput(WM_MESSAGE * pMsg)
{
	WM_HWIN    hWin;
	int        Id;
	int        NCode;
	hWin = pMsg->hWin;
	printf("%s MsgId=%d\r\n",__func__,pMsg->MsgId);
	switch (pMsg->MsgId) 
	{
		case WM_NOTIFY_PARENT:	//���߸����ڣ��Ӵ��ڷ����˸ı�
		{
			//�뵱ǰ���㴰�ڷ��Ͱ�����Ϣ
			Id    = WM_GetId(pMsg->hWinSrc);
			NCode = pMsg->Data.v;
			printf("%s Id=%d NCode=%d\r\n",__func__,Id,NCode);
			switch (NCode) 
			{
			}
		}
		break;
		case WM_DELETE:
		{
			WM_DeleteWindow(pMsg->hWinSrc);
		}
		break;
	}
}
/*
void keypad_demo(WM_HWIN WM_Parent,pDialog_MSG Msg) 
{
	WM_HWIN hFrame;
	WM_HWIN hText;
	WM_HWIN hEdit;

//	WM_SetCallback(WM_HBKWIN, _cbBk);		        //��ָ�������ڻص�����
	
	keypad_dev.xpos=0;
	keypad_dev.ypos=(lcddev.height*6)/10;
	keypad_dev.width=lcddev.width;
	keypad_dev.height=lcddev.height-((lcddev.height*6)/10);
	keypad_dev.padtype=ENGLISH_KEYPAD;				//Ĭ��ΪӢ�ļ���
	keypad_dev.signpad_flag=0;	
	keypad_dev.signpad_num=2;
    keypad_dev.inputlen=0;
    keypad_dev.pynowpage=0;
    keypad_dev.cur_index=0;
    keypad_dev.sta=0;
	
	//����keypad��ʹ�õİ�ť��Ƥ��
	BUTTON_SetDefaultSkin(_DrawSkinFlex_BUTTON); 	//����BUTTON��Ƥ��
	//����keypad
	keypad_dev.hKeypad = WM_CreateWindowAsChild(keypad_dev.xpos,keypad_dev.ypos,keypad_dev.width,keypad_dev.height, WM_Parent, WM_CF_SHOW | WM_CF_STAYONTOP, _cbKeyPad, 0);
	
	//����notepad����
	BUTTON_SetDefaultSkin(BUTTON_SKIN_FLEX);
	FRAMEWIN_SetDefaultSkin(FRAMEWIN_SKIN_FLEX);
	FRAMEWIN_SetDefaultTextAlign(GUI_TA_HCENTER);

	//����FRAME����
	hFrame = FRAMEWIN_CreateEx(Msg->x0, Msg->y0, Msg->xSize, Msg->ySize, WM_Parent, WM_CF_SHOW, 0, 0, (char *) Msg->DialogTiltle, _cbDisplayInput);
	FRAMEWIN_SetTextColor(hFrame, GUI_YELLOW);
	FRAMEWIN_SetFont(hFrame, &GUI_Font20_ASCII);
	FRAMEWIN_AddCloseButton(hFrame,FRAMEWIN_BUTTON_RIGHT,0);
	FRAMEWIN_SetClientColor(hFrame, GUI_LIGHTGRAY);
	
	
	hText=TEXT_CreateEx(Msg->x0+10,Msg->y0+35,Msg->xSize-20,20,hFrame,WM_CF_SHOW,0,GUI_ID_TEXT9,Msg->Editname);
	TEXT_SetFont(hText, &GUI_Font20_ASCII);
	
	
	hEdit=EDIT_CreateEx(Msg->x0+10,Msg->y0+35+20,Msg->xSize-20,30,hFrame,WM_CF_SHOW,0,GUI_ID_EDIT9,(Msg->xSize)/8);
	EDIT_SetFont(hEdit, &GUI_Font20_ASCII);
	EDIT_EnableBlink(hEdit, 500, 1);
	WM_SetFocus(hEdit);

	
//	while(1)
//	{
//		GUI_Delay(100);
//	}
}
*/
WM_HWIN  CreatDispalyDialog(WM_HWIN WM_Parent ,pDialog_MSG Msg)
{
	WM_HWIN hFrame;
	WM_HWIN hText;
	WM_HWIN hEdit;
	uint16_t ys;
		//����FRAME����
	hFrame = FRAMEWIN_CreateEx(Msg->x0, Msg->y0, Msg->xSize, Msg->ySize, WM_Parent, WM_CF_SHOW, 0, 0, (char *) Msg->DialogTiltle, _cbDisplayInput);
	FRAMEWIN_SetTextColor(hFrame, GUI_YELLOW);
	FRAMEWIN_SetFont(hFrame, &GUI_FontHZ16);
	FRAMEWIN_AddCloseButton(hFrame,FRAMEWIN_BUTTON_RIGHT,0);
	FRAMEWIN_SetClientColor(hFrame, GUI_LIGHTGRAY);
	
	ys = (Msg->ySize - 20-30)/2;//������������м�
	//Msg->xSize-40��40��ǰ�����20
	hText=TEXT_CreateEx(20,ys,Msg->xSize-40,20,hFrame,WM_CF_SHOW,0,GUI_ID_TEXT9,Msg->Editname);
	TEXT_SetFont(hText, &GUI_FontHZ16);
	
	
	hEdit=EDIT_CreateEx(20,ys+20,Msg->xSize-40,30,hFrame,WM_CF_SHOW,0,GUI_ID_EDIT9,32);///���32���ַ�
	EDIT_SetFont(hEdit, &GUI_Font20_ASCII);
	EDIT_EnableBlink(hEdit, 500, 1);
	//WM_SetFocus(hEdit);
	return hFrame;
}
void CreatKeypad(WM_HWIN WM_Parent,u16 x0, u16 y0, u16 xSize, u16 ySize)
{


	keypad_dev.xpos=x0;
	keypad_dev.ypos= y0;
	keypad_dev.width=xSize;
	keypad_dev.height=ySize;
	keypad_dev.padtype=ENGLISH_KEYPAD;				//Ĭ��ΪӢ�ļ���
	keypad_dev.signpad_flag=0;	
	keypad_dev.signpad_num=2;
    keypad_dev.inputlen=0;
    keypad_dev.pynowpage=0;
    keypad_dev.cur_index=0;
    keypad_dev.sta=0;
	
	//����keypad��ʹ�õİ�ť��Ƥ��
	BUTTON_SetDefaultSkin(_DrawSkinFlex_BUTTON); 	//����BUTTON��Ƥ��
	//����keypad
	keypad_dev.hKeypad = WM_CreateWindowAsChild(keypad_dev.xpos,keypad_dev.ypos,keypad_dev.width,keypad_dev.height, WM_Parent, WM_CF_SHOW | WM_CF_STAYONTOP, _cbKeyPad, 0);
	
	//����notepad����
	BUTTON_SetDefaultSkin(BUTTON_SKIN_FLEX);
	FRAMEWIN_SetDefaultSkin(FRAMEWIN_SKIN_FLEX);
}
