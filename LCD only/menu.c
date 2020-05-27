#include "UI.h"

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/* BOSS MENU = transfer settings registers */
  //Name			Next			Previous, 		Parent, 	Child   	EnterF 		PlusF 		MinusF   	ID	Text
M_M(ServParamSett0,	NULL_MENU, 		NULL_MENU, 		NULL_MENU, 	NULL_MENU, 	NULL_FUNC,	NULL_FUNC,	NULL_FUNC, 	253,"@<->BOSS Mode0");
M_M(ServParamSett1,	NULL_MENU, 		NULL_MENU, 		NULL_MENU, 	NULL_MENU, 	NULL_FUNC,	NULL_FUNC,	NULL_FUNC, 	254,"@<->BOSS Mode1");
M_M(ServParamSett2,	NULL_MENU, 		NULL_MENU, 		NULL_MENU, 	NULL_MENU, 	NULL_FUNC,	NULL_FUNC,	NULL_FUNC, 	255,"@<->BOSS Mode2");

/* Display screens */
  //Name			Next			Previous, 		Parent, 	Child   	EnterF 		PlusF 		MinusF   	ID	Text
M_M(MAnalogClock, 	MDigitalClock, 	MDAClock, 		NULL_MENU, 	NULL_MENU, 	NULL_FUNC,	NULL_FUNC,	NULL_FUNC, 	0, 	"@->AnalogClock");
M_M(MDigitalClock, 	MDAClock, 		MAnalogClock, 	NULL_MENU, 	NULL_MENU, 	NULL_FUNC, 	NULL_FUNC,	NULL_FUNC, 	32,	"@->DigitalClock");
M_M(MDAClock, 		MAnalogClock, 	MDigitalClock, 	NULL_MENU, 	NULL_MENU, 	NULL_FUNC, 	NULL_FUNC,	NULL_FUNC, 	64,	"@->DAClock");
M_M(MMetronome, 	NULL_MENU, 		NULL_MENU, 		NULL_MENU, 	NULL_MENU, 	MetrEnter, 	MetrUp,		MetrDown,	96,	"@->Metronome");

/* Main menus */
//Name			//Next		//Previous, //Parent, 	//Child 	//EnterF 	//PlusF 	//MinusF   ID	Text
M_M(MMENU, 		NULL_MENU, 	NULL_MENU, 	NULL_MENU, 	SetClock, 	NULL_FUNC,	NULL_FUNC,	NULL_FUNC, 128, "@->Menu");

//Name			//Next		//Previous, //Parent, 	//Child 	//EnterF 	//PlusF 	//MinusF 		//Text
M_M(SetClock, 	SetDate, 	NULL_MENU, 	MMENU, 		Set_Hour, 	NULL_FUNC,	NULL_FUNC,	NULL_FUNC, 144, "@->Menu->Clock");
M_M(SetDate, 	SetVol, 	SetClock, 	MMENU, 		Set_Date, 	NULL_FUNC, 	NULL_FUNC,	NULL_FUNC, 160,	"@->Menu->Date");
M_M(SetVol, 	SetTone, 	SetDate, 	MMENU, 		Set_VClick, NULL_FUNC, 	NULL_FUNC,	NULL_FUNC, 176,	"@->Menu->Volume");
M_M(SetTone, 	SetMetr, 	SetVol, 	MMENU, 		Set_TClick, NULL_FUNC, 	NULL_FUNC,	NULL_FUNC, 192,	"@->Menu->Tone");
M_M(SetMetr, 	SetColors, 	SetTone, 	MMENU, 		Set_4Strk, 	NULL_FUNC, 	NULL_FUNC,	NULL_FUNC, 208,	"@->Menu->Metronome");
M_M(SetColors, 	NULL_MENU, 	SetMetr, 	MMENU, 		Set_ColArr, NULL_FUNC, 	NULL_FUNC,	NULL_FUNC, 224,	"@->Menu->Colors");
//Extra slot with ID = [!245, !246, !247],[241,242,243,244]										 //240

//Name			//Next		//Previous, //Parent, 	//Child 	//EnterF 	//PlusF 	//MinusF   //ID	//Text
M_M(Set_Hour, 	Set_Min, 	NULL_MENU, 	SetClock, 	_Set_Hour, 	NULL_FUNC,	NULL_FUNC,	NULL_FUNC, 145, "@->Menu->Clock->Hour");
M_M(Set_Min, 	Set_Sec, 	Set_Hour, 	SetClock, 	_Set_Min, 	NULL_FUNC, 	NULL_FUNC,	NULL_FUNC, 146,	"@->Menu->Clock->Minute");
M_M(Set_Sec, 	Set_Corr, 	Set_Min, 	SetClock, 	_Set_Sec, 	NULL_FUNC, 	NULL_FUNC,	NULL_FUNC, 147,	"@->Menu->Clock->Seconds");
M_M(Set_Corr, 	NULL_MENU,	Set_Sec, 	SetClock, 	_Set_Corr, 	NULL_FUNC, 	NULL_FUNC,	NULL_FUNC, 148,	"@->Menu->Clock->Correction");//..151

M_M(Set_Date, 	Set_Mon, 	NULL_MENU, 	SetDate, 	_Set_Date, 	NULL_FUNC,	NULL_FUNC,	NULL_FUNC, 161, "@->Menu->Date->Date");
M_M(Set_Mon, 	Set_Year, 	Set_Date, 	SetDate, 	_Set_Mon, 	NULL_FUNC, 	NULL_FUNC,	NULL_FUNC, 162,	"@->Menu->Date->Month");
M_M(Set_Year, 	NULL_MENU,	Set_Mon, 	SetDate, 	_Set_Year, 	NULL_FUNC, 	NULL_FUNC,	NULL_FUNC, 163,	"@->Menu->Date->Year");//..167

M_M(Set_VClick,	Set_VMetr, 	NULL_MENU, 	SetVol, 	_Set_VClick,NULL_FUNC,	NULL_FUNC,	NULL_FUNC, 177, "@->Menu->Vol->Click");
M_M(Set_VMetr, 	Set_VHrStr, Set_VClick,	SetVol, 	_Set_VMetr, NULL_FUNC, 	NULL_FUNC,	NULL_FUNC, 178,	"@->Menu->Vol->Metr");
M_M(Set_VHrStr, NULL_MENU,	Set_VMetr, 	SetVol, 	_Set_VHrStr,NULL_FUNC, 	NULL_FUNC,	NULL_FUNC, 179,	"@->Menu->Vol->HrStrk");//..183

M_M(Set_TClick,	Set_TMtr1, 	NULL_MENU, 	SetTone, 	_Set_TClick,NULL_FUNC,	NULL_FUNC,	NULL_FUNC, 193, "@->Menu->Tone->Click");
M_M(Set_TMtr1, 	Set_TMtr2, 	Set_TClick, SetTone, 	_Set_TMtr1, NULL_FUNC, 	NULL_FUNC,	NULL_FUNC, 194,	"@->Menu->Tone->Metr1");
M_M(Set_TMtr2, 	Set_THrStr, Set_TMtr1, 	SetTone, 	_Set_TMtr2, NULL_FUNC, 	NULL_FUNC,	NULL_FUNC, 195,	"@->Menu->Tone->Metr2");
M_M(Set_THrStr,	NULL_MENU,	Set_TMtr2, 	SetTone, 	_Set_THrStr,NULL_FUNC, 	NULL_FUNC,	NULL_FUNC, 196,	"@->Menu->Tone->HrStrk");//..199

M_M(Set_4Strk, 	NULL_MENU, 	NULL_MENU, 	SetMetr, 	_Set_4Strk,	NULL_FUNC, 	NULL_FUNC,	NULL_FUNC, 209,	"@->Menu->Metronome->4thStike");//..215

M_M(Set_ColArr, Set_ColMrk, NULL_MENU, 	SetColors, 	_Set_ColArr,NULL_FUNC,	NULL_FUNC,	NULL_FUNC, 225, "@->Menu->Colors->Arrow");
M_M(Set_ColMrk, Set_ColTim, Set_ColArr, SetColors, 	_Set_ColMrk,NULL_FUNC, 	NULL_FUNC,	NULL_FUNC, 226,	"@->Menu->Colors->Marks");
M_M(Set_ColTim, Set_ColDat, Set_ColMrk, SetColors, 	_Set_ColTim,NULL_FUNC, 	NULL_FUNC,	NULL_FUNC, 227,	"@->Menu->Colors->Time");
M_M(Set_ColDat, Set_ColPend,Set_ColTim, SetColors, 	_Set_ColDat,NULL_FUNC, 	NULL_FUNC,	NULL_FUNC, 228,	"@->Menu->Colors->Date");
M_M(Set_ColPend,NULL_MENU, 	Set_ColDat, SetColors, 	_Set_ColPend,NULL_FUNC, NULL_FUNC,	NULL_FUNC, 229,	"@->Menu->Colors->Pendulum");//..231


/* MENU ASSIGNMENT */
//Name			//Next		//Previous, //Parent, 	//Child 	//EnterF 	//PlusF //MinusF 	  //ID	//Text
M_M(_Set_Hour, 	NULL_MENU, 	NULL_MENU, 	Set_Hour, 	NULL_MENU, 	NULL_FUNC,	SetUp,	SetDown, 145|(1<<3),"@->Menu->Clock->Hour #");
M_M(_Set_Min, 	NULL_MENU, 	NULL_MENU, 	Set_Min, 	NULL_MENU, 	NULL_FUNC, 	SetUp,	SetDown, 146|(1<<3),"@->Menu->Clock->Minute #");
M_M(_Set_Sec, 	NULL_MENU, 	NULL_MENU, 	Set_Sec, 	NULL_MENU, 	NULL_FUNC, 	SetUp,	SetDown, 147|(1<<3),"@->Menu->Clock->Seconds #");
M_M(_Set_Corr, 	NULL_MENU, 	NULL_MENU, 	Set_Corr, 	NULL_MENU, 	NULL_FUNC, 	SetUp,	SetDown, 148|(1<<3),"@->Menu->Clock->Correction #");

M_M(_Set_Date, 	NULL_MENU, 	NULL_MENU, 	Set_Date, 	NULL_MENU, 	NULL_FUNC,	SetUp,	SetDown, 161|(1<<3),"@->Menu->Date->Date #");
M_M(_Set_Mon, 	NULL_MENU, 	NULL_MENU, 	Set_Mon, 	NULL_MENU, 	NULL_FUNC, 	SetUp,	SetDown, 162|(1<<3),"@->Menu->Date->Month #");
M_M(_Set_Year, 	NULL_MENU, 	NULL_MENU, 	Set_Year, 	NULL_MENU, 	NULL_FUNC, 	SetUp,	SetDown, 163|(1<<3),"@->Menu->Date->Year #");

M_M(_Set_VClick,NULL_MENU, 	NULL_MENU, 	Set_VClick, NULL_MENU, 	NULL_FUNC,	SetUp,	SetDown, 177|(1<<3),"@->Menu->Vol->Click #");
M_M(_Set_VMetr, NULL_MENU,  NULL_MENU,	Set_VMetr, 	NULL_MENU, 	NULL_FUNC, 	SetUp,	SetDown, 178|(1<<3),"@->Menu->Vol->Metr #");
M_M(_Set_VHrStr,NULL_MENU,	NULL_MENU, 	Set_VHrStr, NULL_MENU, 	NULL_FUNC, 	SetUp,	SetDown, 179|(1<<3),"@->Menu->Vol->HrStrk #");

M_M(_Set_TClick,NULL_MENU, 	NULL_MENU, 	Set_TClick, NULL_MENU, 	NULL_FUNC,	SetUp,	SetDown, 193|(1<<3),"@->Menu->Tone->Click #");
M_M(_Set_TMtr1, NULL_MENU, 	NULL_MENU,  Set_TMtr1, 	NULL_MENU, 	NULL_FUNC, 	SetUp,	SetDown, 194|(1<<3),"@->Menu->Tone->Metr1 #");
M_M(_Set_TMtr2, NULL_MENU, 	NULL_MENU, 	Set_TMtr2, 	NULL_MENU, 	NULL_FUNC, 	SetUp,	SetDown, 195|(1<<3),"@->Menu->Tone->Metr2 #");
M_M(_Set_THrStr,NULL_MENU,	NULL_MENU, 	Set_THrStr, NULL_MENU, 	NULL_FUNC, 	SetUp,	SetDown, 196|(1<<3),"@->Menu->Tone->HrStrk #");

M_M(_Set_4Strk, NULL_MENU, 	NULL_MENU, 	Set_4Strk, 	NULL_MENU, 	NULL_FUNC, 	SetUp,	SetDown, 209|(1<<3),"@->Menu->Metronome->4thStike #");

M_M(_Set_ColArr,NULL_MENU, 	NULL_MENU, 	Set_ColArr, NULL_MENU, 	NULL_FUNC,	SetUp,	SetDown, 225|(1<<3),"@->Menu->Colors->Arrow #");
M_M(_Set_ColMrk,NULL_MENU, 	NULL_MENU, 	Set_ColMrk, NULL_MENU, 	NULL_FUNC, 	SetUp,	SetDown, 226|(1<<3),"@->Menu->Colors->Marks #");
M_M(_Set_ColTim,NULL_MENU, 	NULL_MENU, 	Set_ColTim, NULL_MENU, 	NULL_FUNC, 	SetUp,	SetDown, 227|(1<<3),"@->Menu->Colors->Time #");
M_M(_Set_ColDat,NULL_MENU, 	NULL_MENU, 	Set_ColDat, NULL_MENU, 	NULL_FUNC, 	SetUp,	SetDown, 228|(1<<3),"@->Menu->Colors->Date #");
M_M(_Set_ColPend,NULL_MENU, NULL_MENU, 	Set_ColPend,NULL_MENU, 	NULL_FUNC, 	SetUp,	SetDown, 229|(1<<3),"@->Menu->Colors->Pendulum #");
