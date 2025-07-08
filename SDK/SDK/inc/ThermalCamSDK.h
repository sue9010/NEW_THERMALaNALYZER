// ThermalCamSDK.h : ThermalCamDll is header file for DLL
// Title : Description of the functions and structure
// 
// Date : 2010.11.24
// Update : 2016.08.05
// 
//	MODIFY DATE : 2016.08.05 csi
//				  ADD CG MODEL
#pragma	once


#ifdef ThermalCamDll_EXPORTS
#define THERMALSDK_API __declspec(dllexport)
#else
#define THERMALSDK_API __declspec(dllimport)
#endif


#include	<windows.h>
#include	<stdint.h>


#pragma region Define definition
//	Define Macro
#define		IRF_PACKET_ID				0x434F5802		//	Packet ID
#define		IRF_FILE_ID					0x499602D2		//	CRD File ID

#define		RAWTAIL_V2					0x434F5802		//	IR Raw Data Tail Version 2
#define		RAWTAIL_V3					0x434F5803		//	IR Raw Data Tail Version 3

#define		IRF_IMAGE_LEVEL				65536			//	IR raw level 16bits
#define		IRF_GRAYSCALE				256				//	Grey Scale Level
#define		IRF_MAX_IR_BUFFER_SIZE		1				//	IR buffer
#define		IRF_HEADER_BUFFER_SIZE		64				//	header memory size (word count) : 128 bytes

#define		CONVERT565(r,g,b)			(((r>>3)<<11) | ((g>>3)<<5) | (b>>3))
#define		CONVERT555(r,g,b)			(((r>>3)<<10) | ((g>>2)<<5) | (b>>3))


//++	MODIFY		NEW CG MODEL
#define		FILE_VERSION				0x20			//	current file version
#define		FILE_VERSION_14				0x14			//	previous file version
#define		FILE_VERSION_11				0x11			//	previous file version

#define		SAVEDATA_VER_OLD			0x11			//	previous save data version (CX640 : 0x11, CX320 : 0x0B)
#define		QVGA_ID_OLD					0x00			//	CX320 Sensor ID
#define		VGA_ID_OLD					0x01			//	CX640 Sensor ID

#define		SAVEDATA_VERSION			0x21			//	current save data version (CG640, CG320 : 0x21)
#define		SAVEDATA_VER_20				0x20			//	previous save data version (CG640, CG320 : 0x20)
#define		QVGA_ID						0x20			//	CG320 Sensor ID
#define		VGA_ID						0x21			//	CG640 Sensor ID
#define		XGA_ID						0x22			//	CGXGA Sensor ID
#define		CVGA_ID						0x23			//	CVGA Sensor ID
//++

#define		CRD_FILE_VER				20				//	CRD File Version
#define		SAVEDATA_CRC				0xea3a242b		//	CRD Data CRC

#define		IRF_BUFFER_SIZE				8192			//	Packet Rx/Tx Buffer Size
#define		DEFAULT_TIMEOUT				100L			//	Timeout


//	Camera Configuration Command Code
#define		CMD_AGC					0x0101		//	AGC On/OFF								( CX, CG )
												//	CX RCODE  => 0 : OFF, 1 : HISTOGRAM, 2 : BRIGHTNESS
												//	CG RCODE  => 0 : MANUAL, 1 : AUTO

#define		CMD_LEVEL				0x0102		//	LEVEL									( CX )
												//	RCODE  => -20 ~ 120

#define		CMD_CG_BRIGHT			0x0102		//	BRIGHTNESS								( CG )
												//	RCODE  => -40 ~ 40

#define		CMD_SPAN				0x0103		//	SPAN									( CX )
												//	RCODE  => 10 ~ 100

#define		CMD_CG_CONTRAST			0x0103		//	CONTRAST								( CG )
												//	RCODE  => -10 ~ 10

#define		CMD_PALETTE				0x0105		//	PALETTE									( CX, CG )
												//	RCODE  => IRF_CAM_PALETTE_TYPE_T

#define		CMD_INVERT				0x0106		//	INVERT									( CX, CG )
												//	CX RCODE  => 0 : OFF, 1 : ON
												//	CG RCODE  => 0 : OFF, 1 : LUMA, 2 : CHROMA, 3 : L + C

#define		CMD_MIRROR				0x0107		//	MIRROR									( CX, CG )
												//	RCODE  => 0 : OFF, 1 : ON

#define		CMD_FLIP				0x0108		//	FLIP									( CX, CG )
												//	RCODE  => 0 : OFF, 1 : ON

#define		CMD_ZOOM				0x0109		//	ZOOM									( CX, CG )
												//	RCODE  => 0 : OFF, 1 : x2, 2 : x4

#define		CMD_NOISE_FILTER		0x010A		//	NOISE FILTER							( CX, CG )
												//	CX RCODE  => 0 : OFF, 1 : SLIGHT, 2 : STRONG, 3 : MEDIAN, 4 : GAUSSIAN
												//	CG RCODE  => 0 : OFF, 1 : NR1, 2 : NR2, 3 : NR1 + NR2

#define		CMD_EDGE_FILTER			0x010B		//	EDGE FILTER								( CG )
												//	RCODE  => 0 : OFF, 1 : ON

#define		CMD_NR1_LEVEL			0x010C		// CG MODEL ONLY							( CG )
												//	RCODE	: NR1 LEVEL	(0 : LOWEST, 1 : LOW, 2 : MIDDLE, 3 : HIGH, 4 : HIGHEST)

#define		CMD_NR2_LEVEL			0x010D		// CG MODEL ONLY							( CG )
												//	RCODE	: NR2 LEVEL	(0 : LOWEST, 1 : LOW, 2 : MIDDLE, 3 : HIGH, 4 : HIGHEST)

#define		CMD_EE_LEVEL			0x010E		// CG MODEL ONLY							( CG )
												//	RCODE	: EE LEVEL	(0 : LOWEST, 1 : LOW, 2 : MIDDLE, 3 : HIGH, 4 : HIGHEST)

#define		CMD_COLORBAR			0x0201		//	DISPLAY COLOR-BAR						( CX, CG )
												//	RCODE  => 0 : OFF, 1 : ON

#define		CMD_TEMP_VIEW			0x0202		//	DISPLAY TEMPERATURE INFORAMTION			( CX, CG )
												//	RCODE  => 0 : OFF, 1 : ON

#define		CMD_TEMP_INDICATOR		0x0203		//	DISPLAY HOT/COLD INDICATOR				( CX, CG )
												//	RCODE  => 0 : OFF, 1 : ON

#define		CMD_TEMP_TYPE			0x0204		//	TEMPERATURE UNIT						( CX, CG )
												//	RCODE  => 0 : CELSIUS, 1 : FAHRENHEIT

#define		CMD_TRANSPARENCY		0x0205		//	OSD MENU ALPHA BLENDING					( CX, CG )
												//	RCODE  => 0 : OFF, 1 : 20%, 2 : 40%, 3 : 60%, 4 : 80%

#define		CMD_CENTER_CROSS		0x0206		//	DISPLAY CENTER-CROSS					( CX, CG )
												//	RCODE  => 0 : OFF, 1 : ON

#define		CMD_SPOT_INFO			0x0207		//	DISPLAY SPOT INFORMATION				( CX )
												//	RCODE  => 0 : OFF, 1 : ON

#define		CMD_CG_ROI_INFO			0x0207		//	DISPLAY ROI INFORMATION					( CG )
												//	RCODE  => 0 : OFF, 1 : EMPTY RECT, 2 : FILLED RECT

#define		CMD_SHOW_CORR			0x0208		//	DISPLAY CORRECTION INFORMATION			( CX, CG )
												//	RCODE  => 0 : OFF, 1 : ON

#define		CMD_SHOW_ICON			0x0209		//	DISPLAY ICON							( CG )
												//	RCODE  => 0 : OFF, 1 : ON

#define		CMD_ETHERNET			0x0301		//	NETWORK CONFIG							( CX, CG )
												//	RCODE  => 0 : DHCP OFF, 1 : DHCP ON
												//	RCODE2 => IP Address
												//	RCODE3 => Subnet Mask
												//	RCODE4 => Gateway

#define		CMD_PROTOCOL			0x0305		//	SERIAL PROTOCOL							( CG )
												//	RCODE  => 0 : Pelco-D, 1 : COX

#define		CMD_CAM_ID				0x0306		//	RS485 CAMERA ID							( CX, CG )
												//	RCODE  => 1 ~ 255

#define		CMD_BAUDRATE			0x0307		//	SERIAL BAUDRATES						( CX, CG )
												//	CX RCODE  => 0 : 2400, 1 : 4800, 2 : 9600, 3 : 19200, 4 : 38400
												//	CG RCODE  => 0 : 2400, 1 : 4800, 2 : 9600, 3 : 19200, 4 : 38400, 5 : 57600, 6 : 115200

#define		CMD_ALARM1_RMC			0x0401		//	ALARM01 REMOTE CONTROL					( CG )
												//	RCODE  => 0 : OFF, 1 : ON

#define		CMD_ALARM1_FUNC			0x0401		//	ALARM01 FUNCTION						( CX )
												//	RCODE  => 0 : OFF, 1 : CENTER, 2 : MEAN, 3 : MAX, 4 : MIN, 5 : ON

#define		CMD_ALARM1_COND			0x0402		//	ALARM01 CONDITION						( CX )
												//	RCODE  => 0 : ABOVE, 1 : BELOW

#define		CMD_ALARM1_VAL			0x0403		//	ALARM01 VALUE							( CX )
												//	RCODE  => -20 ~ 120

#define		CMD_ALARM2_RMC			0x0404		//	ALARM02 REMOTE CONTROL					( CG )
												//	RCODE  => 0 : OFF, 1 : ON

#define		CMD_ALARM2_FUNC			0x0404		//	ALARM02 FUNCTION						( CX )
												//	RCODE  => 0 : OFF, 1 : CENTER, 2 : MEAN, 3 : MAX, 4 : MIN, 5 : ON

#define		CMD_ALARM2_COND			0x0405		//	ALARM02 CONDITION						( CX )
												//	RCODE  => 0 : ABOVE, 1 : BELOW

#define		CMD_ALARM2_VAL			0x0406		//	ALARM02 VALUE							( CX )
												//	RCODE  => -20 ~ 120

#define		CMD_ALARM1_DUR			0x0407		//	ALARM01 DURATION						( CX )
												//	RCODE  => 0 ~ 99

#define		CMD_ALARM2_DUR			0x0408		//	ALARM02 DURATION						( CX )
												//	RCODE  => 0 ~ 99

#define		CMD_ALARM1_ROI			0x0409		//	ALARM01 ROI								( CX )
#define		CMD_ALARM2_ROI			0x040A		//	ALARM02 ROI								( CX )
												//	RCODE  => [31:2] : Not used
												//			     [1] : 0 Include ROI, 1 Exclude ROI
												//			     [0] : 0 Full Screen, 1 ROI
												//	CX320 RCODE2 => [31:24] : y2 point (half of the real coordinate)
												//					[23:16] : x2 point (half of the real coordinate)
												//					[15: 8] : y1 point (half of the real coordinate)
												//					[ 7: 0] : x1 point (half of the real coordinate)
												//	CX640 RCODE2 => [31:16] : y1 point (the real coordinate)
												//					[15: 0] : x1 point (the real coordinate)
												//	CX640 RCODE3 => [31:16] : y2 point (the real coordinate)
												//					[15: 0] : x2 point (the real coordinate)

#define		CMD_ALARM_CONFIG		0x0411		//	ALARM OUTPUT CONFIG						( CG )
												//	RCODE  => 0 : ALARM01, 1 : ALARM02
												//	RCODE2 => [31:24] ALARM TYPE ( NO = 0,  NC = 1 )
												//	RCODE2 => [23:16] ALARM MODE ( OFF = 0, STABILIZE = 1, ALIVE PWM = 2, TEMPERATURE = 3, TEST ALARM ON = 4, TEST ALARM OFF = 5 )
												//	RCODE2 => [15:8]  ALARM DURATION ( 0~ 99 )
												//	RCODE2 => [7:0]	  ALARM REMOTE CONTROL ( 0 : OFF, 1 : ON )

#define		CMD_ALARM01_TYPE		0x0412		//	ALARM01 TYPE							( CG )
												//	RCODE  => 0 : NO, 1 : NC

#define		CMD_ALARM01_MODE		0x0413		//	ALARM01 MODE							( CG )
												//	RCODE  => 0 : OFF, 1 : STABILIZE, 2 : ALIVE PWM, 3 : TEMPERATURE, 4 : TEST ALARM ON, 5 : TEST ALARM OFF

#define		CMD_ALARM01_DURA		0x0414		//	ALARM01 DURATION						( CG )
												//	RCODE  => 0 ~ 99

#define		CMD_ALARM01_RMCT		0x0415		//	ALARM01 REMOTE CONTROL					( CG )
												//	RCODE  => 0 : OFF, 1 : ON

#define		CMD_ALARM02_TYPE		0x0416		//	ALARM02 TYPE							( CG )
												//	RCODE  => 0 : NO, 1 : NC

#define		CMD_ALARM02_MODE		0x0417		//	ALARM02 MODE							( CG )
												//	RCODE  => 0 : OFF, 1 : STABILIZE, 2 : ALIVE PWM, 3 : TEMPERATURE, 4 : TEST ALARM ON, 5 : TEST ALARM OFF

#define		CMD_ALARM02_DURA		0x0418		//	ALARM02 DURATION						( CG )
												//	RCODE  => 0 ~ 99

#define		CMD_ALARM02_RMCT		0x0419		//	ALARM02 REMOTE CONTROL					( CG )
												//	RCODE  => 0 : OFF, 1 : ON

#define		CMD_ALARM_TEMPCFG		0x0421		//	ALARM TEMPERATURE CONFIG				( CG )
												//	RCODE  => MAX : 0, MIN : 1, AVG : 2, CTR : 3, ROI_01 : 11 ~ ROI_10 : 20
												//	RCODE2 => [31:24] ALARM Enable 0 or 1
												//	RCODE2 => [23:16] ALARM-Output Mask ( ALARM01 : 0x01, ALARM02 : 0x02 )
												//	RCODE2 => [15:8]  ALARM Condition ( 0 : Above, 1 : Below )
												//	RCODE3 => [31:16] ALARM-IN DURATION ( 0 ~ 1800 )
												//	RCODE3 => [15:0]  ALARM Temperature

#define		CMD_ROI_COORCFG			0x0422		//	ROI COORDINATE CONFIG					( CG )
												//	RCODE  => [15:8]  ROI_01 : 11 ~ ROI_10 : 20
												//	RCODE  => [7:0]   ROI Mode ( 0 : OFF, 1 : SPOT, 2 : RECT )
												//	RCODE2 => [31:16] ROI X Position ( QVGA : 0 ~ 383, VGA : 0 ~ 639 )
												//	RCODE2 => [16:0]  ROI Y Position ( QVGA : 0 ~ 287, VGA : 0 ~ 479 )
												//	RCODE3 => [31:16] ROI WIDTH ( QVGA : 16 ~ 384, VGA : 16 ~ 640 )
												//	RCODE3 => [16:0]  ROI HEIGHT ( QVGA : 16 ~ 288, VGA : 16 ~ 480 )

#define		CMD_ROI_TEMPCFG			0x0423		//	ROI TEMPERATURE CONFIG					( CG )
												//	RCODE  => [15:8]  ROI_01 : 11 ~ ROI_10 : 20
												//	RCODE  => [7:0]   ROI Correction Enable 0 or 1
												//	RCODE2 => [31:24] Emissivity ( 100 ~ 1 )
												//	RCODE2 => [23:16] Transmission ( 100 ~ 1 )
												//	RCODE2 => [15:0]  Atmosphere ( -500 ~ 1000 )
												//	RCODE3 => Temperature Type ( 0 : AVG, 1 : MAX, 2 : MIN )

#define		CMD_NUC_CFG				0x0431		//	NUC CONFIG								( CG )
												//	RCODE  => NUC MODE ( 0 : OFF, 1 : TIME, 2 : AUTO, 3 : TIME + AUTO, 4 : SHUTTER OPEN, 5 : SHUTTER CLOSE )
												//	RCODE2 => [31:16]	NUC TIME CONFIG ( 0 : 1 MIN, 1 : 5 MIN, 2 : 10 MIN, 3 : 30 MIN, 4 : 60 MIN )
												//	RCODE2 => [15:0]	NUC AUTO THRESHOLD ( 0 : LOWEST, 1 : LOW, 2 : MIDDLE, 3 : HIGH, 4 : HIGHEST )

#define		CMD_NUC_ONETIME			0x0432		//	NUC ONE TIME							( CG )

#define		CMD_CVBS_VDO_SIZE		0x0441		//	CVBS VIDEO SIZE							( CG )
												//	RCODE  => NONE
												//	RCODE2 => [31:16]	Left Margin ( 0 ~ 70 )
												//	RCODE2 => [15:0]	Right Margin ( 0 ~ 70 )
												//	RCODE3 => [31:16]	Top Margin ( 0 ~ 70 )
												//	RCODE3 => [15:0]	Bottom Margin ( 0 ~ 70 )

#define		CMD_HDMI_VDO_SIZE		0x0442		//	HDMI VIDEO SIZE							( CG )
												//	RCODE  => NONE
												//	RCODE2 => [31:16]	Left Margin ( 0 ~ 150 )
												//	RCODE2 => [15:0]	Right Margin ( 0 ~ 150 )
												//	RCODE3 => [31:16]	Top Margin ( 0 ~ 150 )
												//	RCODE3 => [15:0]	Bottom Margin ( 0 ~ 150 )

#define		CMD_VDO_ROT				0x0451		//	VIDEO ROTATION							( CG )
												//	RCODE  => 0 : OFF, 1 : MIRROR, 2 : FLIP, 3 : MIRROR + FLIP

#define		CMD_AGC_MANMAX			0x0461		//	VIDEO AGC MANUAL MAXIMUM				( CG )
												//	RCODE  => 0 : INC, 1 : DEC

#define		CMD_AGC_MANMIN			0x0462		//	VIDEO AGC MANUAL MINIMUM				( CG )
												//	RCODE  => 0 : INC, 1 : DEC

#define		CMD_AGC_MANMAX_LVL		0x0463		//	VIDEO AGC MANUAL MAXIMUM LEVEL			( CG )
												//	RCODE  => 0 ~ 16383

#define		CMD_AGC_MANMIN_LVL		0x0464		//	VIDEO AGC MANUAL MINIMUM LEVEL			( CG )
												//	RCODE  => 0 ~ 16383

#define		CMD_HDMI_MODE			0x0471		//	HDMI MODE								( CG )
												//	RCODE  => HDMI MODE ( 2 : 480P, 3 : 576P, 4 : 720P 50, 5 : 720P 60, 6 : 1080I 50, 7 : 1080I 60, 9 : 1080P 50, 10 : 1080P 60 )

#define		CMD_CGISO_CFG			0x0481		//	CG Isotherm Configuration				( CG )
												//	RCODE  => must be 0
												//	RCODE2 => [15:8] Transparent Mask ( 0x01 : above transparent, 0x02 : below transparent, 0x04 : between transparent )
												//	RCODE2 => [7:0]  ISO Mode Mask ( 0x01 : min above mode, 0x02 : max below mode, 0x04 : min/max between mode ) 
												//	RCODE3 => [15:0]	Max Temperature ( -200 ~ 6500 )
												//	RCODE4 => [15:0]	Min Temperature ( -200 ~ 6500 )

#define		CMD_CGISO_CLR			0x0482		//	CG Isotherm Color Configuration			( CG )
												//	RCODE  => must be 0
												//	RCODE2 => Above RGB Color ( [31:24] : NOT USED, [23:16] : BLUE, [15:8] : GREEN, [7:0] : RED )
												//	RCODE3 => Below RGB Color ( [31:24] : NOT USED, [23:16] : BLUE, [15:8] : GREEN, [7:0] : RED )
												//	RCODE4 => Between RGB Color ( [31:24] : NOT USED, [23:16] : BLUE, [15:8] : GREEN, [7:0] : RED )

#define		CMD_MOTORIZED			0x0491		//	CG Motorized Controller					( CG )
												//	RCODE  => FOCUS OR ZOOM ( 0 : FOCUS, 1 : ZOOM )
												//	RCODE2 => OFF, INC or DEC ( 0 : OFF, 1 : INC, 2 : DEC )

#define		CMD_TV_MODE				0x0501		//	TV MODE									( CG )
												//	RCODE  => 0 : NTSC, 1 : PAL

#define		CMD_NUC					0x0502		//	NUC										( CX )
												//	RCODE  => 0 : 1 MIN, 1 : 5 MIN, 2 : 10 MIN, 3 : 30 MIN, 4 : 60 MIN, 5 : OFF, 7 : MANUAL

#define		CMD_TEMP_MODE			0x0503		//	TEMPERATURE MODE						( CX, CG )
												//	RCODE  => 0 : NORMAL, 1 : HIGH

#define		CMD_NETWORK_FPS			0x0A01		//	RAW DATA CAPTURE FPS					( CX, CG )
												//	RCODE  => SKIP FRAME NUMBER ( 0 : NO SKIP, 1 : 1/2 fps, 2 : 1/3 fps ... )
												//			  0 ~ 255

#define		CMD_DISPLAY_FPS			0x0A02		//	DISPLAY DATA CAPTURE FPS				( CG )
												//	RCODE  => SKIP FRAME NUMBER ( 0 : NO SKIP, 1 : 1/2 fps, 2 : 1/3 fps ... )
												//			  0 ~ 255

#define		CMD_TEMP_CORRECT		0x0B01		//	Entire emissivity correction			( CX, CG )
												//	RCODE  => USED CORRECTION ( 0 : OFF, 1 : ON )
												//	RCODE2 => [31:24] Emissivity ( 100 ~ 1 )
												//	RCODE2 => [23:16] Transmission ( 100 ~ 1 )
												//	RCODE2 => [15:0]  Atmosphere ( -500 ~ 1000 )
												//	RCODE3 => [15:0]  ZERO OFFSET ( -200 ~ 200 )  CG ONLY
												//	RCODE3 => [31:16]  Measure Distance ( It can be different depand on lens. )  CG ONLY

#define		CMD_SPOT_CONF			0x0B02		// Spot configuration						( CX )

#define		CMD_ISOTHERM_CONF		0x0B03		// Isotherm configuration					( CX )

#define		CMD_FACTORYDEF			0x0F01		// Factory Default							( CG )


//	SDK Interface Return Code
#define		IRF_NO_ERROR						1			//	OK, No error
#define		IRF_HANDLE_ERROR					-1			//	Invalid Handle
#define		IRF_FILE_OPEN_ERROR					-2			//	File Open Error.
#define		IRF_FILE_CLOSE_ERROR				-3			//	File Close Error.
#define		IRF_IR_IMAGE_READ_ERROR				-4			//	CRD File Read Error.
#define		IRF_FILE_BUFFER_ALLOCATION_ERROR	-5			//	File Stream Buffer Allocation Error.
#define		IRF_END_OF_FILE						-6			//	End of CRD File.
#define		IRF_BEGIN_OF_FILE					-7			//	Begin of CRD File.
#define		IRF_IR_IMAGE_WRITE_ERROR			-8			//	CRD File Write Error.
#define		IRF_NOT_FOUND_WINSOCK_DLL			-9			//	Not Found WS2_32.dll
#define		IRF_CAMERA_CONNECTION_ERROR			-10			//	Connection error from a camera.
#define		IRF_CAMERA_DISCONNECTION			-11			//	Disconnected from a camera
#define		IRF_PACKET_ID_ERROR					-12			//	Unknown network packet ID
#define		IRF_MESSAGE_SEND_ERROR				-13			//	Message sending error
#define		IRF_FIRST_FRAME_POS_ERROR			-14			//	First frame position error.
#define		IRF_FILTER_SIZE_ERROR				-15			//	Image filter size error.
#define		IRF_FILE_WRITE_COUNT_OVER			-16			//	Image frame count is bigger than limit.
#define		IRF_PALETTE_FILE_OPEN_ERROR			-17			//	Palette File open error.
#define		IRF_CRD_SINATURE_ERROR				-18			//	CRD file signature error.
#define		IRF_FILTER_UNSUPPORT_ERROR			-19			//	Unsupported Filter type.
//++	MODIFY		NEW CG MODEL
#define		IRF_PACKET_UNSUPPORT_ERROR			-20			//	Unsupported Packet type.
//++
#define		IRF_NAK								-100		//	Received NAK message from a camera.
#define		IRF_BUFFER_ALLOCATION_ERROR			-1000		//	Buffer allocation error.
#define		IRF_INVALID_PARAM					-1001		//	Invalid API Parameter.
#define		IRF_INVALID_MODEL					-1002		//	Invalid Model Type.
#define		IRF_UNSUPPORTED_MODEL				-1003		//	Unsupported Model Type.
#define		IRF_NOT_ENOUGH_MEMORY				-1004		//	Not enough memory
#define		IRF_ERROR_PACKET_SIZE				-1005		//	Error Packet Size

//	SDK Define File Interface
#define		IRF_OF_CREATE						0x0001		//	CRD File Creation Mode
#define		IRF_OF_READ							0x0002		//	CRD File Read Mode
#define		IRF_PLAY_NEXT						0x0003		//	CRD File Play : NEXT
#define		IRF_PLAY_PREVIOUS					0x0004		//	CRD File Play : PREVIOUS
#define		IRF_PLAY_FIRST						0x0005		//	CRD File Play : FIRST
#define		IRF_PLAY_LAST						0x0006		//	CRD File Play : LAST

#pragma endregion Define definition


#pragma region Enumeration Definition
/* Enumeration Temperature Mode */
typedef enum
{
	_IRF_CELSIUS	= 0,				/* Celsius */
	_IRF_FAHRENHEIT,					/* Fahrenheit */
	_IRF_KELVIN							/* Kelvin */
}	IRF_TEMP_MODE_T;


/* Enumeration Automatic Type */
typedef enum
{
	_IRF_AUTO	= 0,					//	AGC Auto
	_IRF_MANUAL							//	AGC Manual
}	IRF_AUTOMATIC_TYPE_T;


/* Enumeration Dynamic range(Low/Middle/High Range) */
typedef enum
{
	_IRF_LOW_RANGE		= 0,			// Normal Temperature
	_IRF_MIDDLE_RANGE,					// High Temperature
	_IRF_MEDICAL_RANGE					// Medical Temperature
}	IRF_DYNAMIC_RANGE_T;


/* Enumeration of Automatic Range Algorithms (Input Range Setting) */
typedef enum
{
	_IRF_MIN_MAX			= 0,		//	MinMax Algorithm
	_IRF_BRIGHTNESS_RATE,				//	Brightness Rate (%)
	_IRF_SD_RATE,						//	Standard Deviation Rate (%)
	_IRF_AUTO_BRIGHT,					//	Auto Brightness
	_IRF_ENHANCE_HIST					//	Enhance Histogram
}	IRF_AUTO_RANGE_INPUT_METHOD_T;


/* Enumeration of output range setting algorithms */
typedef enum
{
	_IRF_LINEAR				= 0,		//	Linear method. (contrast + brightness)
	_IRF_NON_LINEAR,					//	Non-Linear method. (Gamma function)
	_IRF_TPE,							//	Tail-less Plateau Equalization.
	_IRF_APE,							//	Adaptive Plateau Equalization.
	_IRF_SAPE							//	Self-adaptive plateau equalization.
}	IRF_AUTO_RANGE_OUTPUT_METHOD_T;


/* Enumeration of image filters */
typedef enum
{
	_IRF_FILTER_NONE		= 0,
	_IRF_MEDIAN,
	_IRF_SOFTEN_SLIGHTLY,
	_IRF_SOFTEN_STRONG,
	_IRF_SHARPENING_SLIGHTLY,
	_IRF_SHARPENING_STRONG,
	_IRF_BOXBLUR,
	_IRF_FAST_GAUSSIAN,
	_IRF_FAST_STACK_BLUR,
	_IRF_BI_LATERAL
}	IRF_IMAGE_FILTER_T;


/* Enumeration of Message Type */
typedef enum
{
	_IRF_NONE				= -1,			//	No Received Packet
	_IRF_ACK				= 0,			//	Receive Acknowledgement as a result of request
	_IRF_NAK				= 1,			//	Receive Negative Acknowledgement as a result of request
	_IRF_ALIVE				= 2,			//	Send Alive Message
	_IRF_STREAM_ON			= 3,			//	Request to start raw data transfer.
	_IRF_STREAM_OFF			= 4,			//	Request to stop raw data transfer.
	_IRF_STREAM_DATA		= 5,			//	Receive raw data
	_IRF_REQ_CAM_DATA		= 7,			//	Request a camera configuration data.
	_IRF_CAM_DATA			= 8,			//	Receive a camera configuration data as a result of request.
	_IRF_SET_CAM_DATA		= 10,			//	Request camera to save one of various setting CMD_xxxx.
	_IRF_SET_USER_PALETTE	= 11,			//	User color palette update. (pc --> cam)
	_IRF_REQ_SYS_INFO		= 12,			//	Request System Information. (pc --> cam)
	_IRF_SYS_INFO			= 13,			//	Receive System Information as a result of request.	(cam --> pc)
	_IRF_SPOT_STREAM_ON		= 14,			//	Request to start spot streaming transfer.
	_IRF_SPOT_STREAM_OFF	= 15,			//	Request to stop spot streaming transfer.
	_IRF_SPOT_STREAM_DATA	= 16,			//	Receive spot streaming data.
	_IRF_REQ_TEMP_TABLE		= 19,			//	Request temperature and offset table. (pc --> cam)
}	IRF_MESSAGE_TYPE_T;


/* Enumeration of Color Palette Type */
typedef enum
{
	YELLOW_COLOR_MAP		= 0,
	RAINBOW_COLOR_MAP,
	RAIN900_COLOR_MAP,
	RAIN10_COLOR_MAP,	
	MIDGREY_COLOR_MAP,
	MIDGREEN_COLOR_MAP,
	MEDICAL_COLOR_MAP,
	IRON10_COLOR_MAP,
	IRON_COLOR_MAP,
	GREYRED_COLOR_MAP,
	GREY10_COLOR_MAP,
	GREY_COLOR_MAP,
	GLOWBOW_COLOR_MAP
}	IRF_PALETTE_TYPE_T;


/* Enumeration of Color Palette Type Supported By Camera */
typedef enum
{
	GREY		= 0,
	RAINBOW,
	IRON,
	GREYRED,
	GLOWBOW,
	YELLOW,
	MIDGREY,
	MIDGREEN
}	IRF_CAM_PALETTE_TYPE_T;


/* Enumeration of Thermal Camera Model Type */
typedef enum
{
	CX320	= 0,
	CX640,
	CG320,
	CG640,
	CZ1024,
}	IRF_CAM_MODEL_TYPE_T;

#pragma endregion Enumeration Definition


#pragma region Structure Definition
#pragma pack(1)
/* Structure of image data size */
typedef struct
{	
	uint16_t				xSize;					//	horizontal size of raw data
	uint16_t				ySize;					//	vertical size of raw data
}	IRF_IMAGE_INFO_T;


/* Structure of temperature correction parameters */
typedef struct
{
	float					emissivity;				//	0.01 - 1.00
	float					atmTemp;				//	Atmospheric Temperature in Celsius
	float					atmTrans;				//	Atmospheric Transmission in Celsius

	int						zero_offset;			//	Offset for Temperature
}	IRF_TEMP_CORRECTION_PAR_T;


/* Structure of CRD file header */
typedef struct  
{
	BYTE							ver;				//	file version <=16 : old

	IRF_IMAGE_INFO_T				image_info;			//	raw data size
	IRF_TEMP_CORRECTION_PAR_T		temp_correction;	//	temperature correction
}	IRF_IR_FILE_HEADER_T;


/* Setting of Input and output range method */
typedef struct 
{
	IRF_AUTOMATIC_TYPE_T			autoScale;		//	Automatic scale.
	IRF_AUTO_RANGE_INPUT_METHOD_T	inputMethod;	//	Input range setting method.
	IRF_AUTO_RANGE_OUTPUT_METHOD_T	outputMethod;	//	Output range setting method.

	float					B_Rate;					//	Parameter of input range method. (Brightness method)	(0 <= B_Rate <= 1.0)
	float					SD_Rate;				//	Parameter of input range method. (SD method)	(1.0 <= SD_Rate <= 6.0)
	uint8_t					intercept;				//	Intercept of linear method (0 <= intercept <= 254)
	float					gamma;					//	Gamma of non-linear method.	(0.1 <= gamma <= 25)
	uint32_t				plateau;				//	Plateau value for tail-less plateau equalization.
	float					epsilon;				//	The epsilon that is threshold value is a scalar arbitrary set to a value between zero and one. (Adaptive Plateau Algorithm)
	float					psi;					//	The psi is a scalar arbitrary set to a value between zero and one. (Adaptive Plateau Algorithm)
	float					prevPalteau;			//	previous plateau value for using Adaptive Plateau Algorithm.
}	IRF_AUTO_RANGE_METHOD_T;


/* Structure of numeric information in ROI */
typedef struct
{
	float					min;					//	minimum temperature
	float					max;					//	maximum temperature
	float					avg;					//	average temperature
	float					std;					//	standard deviation
}	IRF_NUMERIC_INFO_T;


/* Message Body (User palette) */
typedef struct {
	BYTE					Info[7];				//	Reserved
	BYTE					Index;					//	Data Index(0:userPalette1, 1:userPalette2)
	BYTE					Data[1024];				//	RGBA (4bytes * 256 level)
	uint32_t				pngLength;				//	PNG File length
	BYTE					pngData[8192];			//	PNG File data;
}	IRF_SET_USER_PALETTE;


/**
 @struct	__alarm_config__
 @brief		Alarm Config ( CG MODEL )
*/
typedef struct  __alarm_config__
{  
	uint8_t					f_enable;				/**< Alarm Enable */
    uint8_t					output_mask;			/**< Alarm Output Mask */
    uint8_t					cond;					/**< Alarm Condition */
    uint8_t					reserved;				/**< Reserved */
    int16_t					temp;					/**< Alarm Temperature */
    uint16_t				delay;					/**< Alarm Delay */
}   ALRMCFG, * ALRMCFG_PTR;


/* Camera information structure (CX, CG MODEL) */
typedef struct strSAVEDATA
{
	union {
		struct
		{
			uint32_t	crc;				//	CRC Data
			uint8_t		ver;				//	Setup Data Version	( CG Model : 0x20 )
			uint8_t		sensor;				//	Sensor Type ( 0x00 : CX320, 0x01 : CX640, 0x20 : CG QVGA, 0x21 : CG VGA )
			uint8_t		tv;					//	Video Output Format ( 0 : NTSC, 1 : PAL )
			uint8_t		temp_mode;			//	Temperature Measurement Mode ( 0 : Normal, 1 : High, 2 : Medical )

			uint8_t		id;					//	RS485 ID
			uint8_t		baudrate;			//	RS485 Baudrates
			int16_t		level;				//	CX Model Only
			uint16_t	span;				//	CX Model Only
			uint8_t		agc;				//	AGC MODE ( 1 : AUTO, 0 : MANUAL )
			uint8_t		invert;				//	Video Color Invert ( 0 : OFF, 1 : LUMA, 2 : CHROMA, 3 : L + C )
			uint8_t		mirror;				//	Video Mirror ( 0 : OFF, 1 : ON )
			uint8_t		flip;				//	Video Flip ( 0 : OFF, 1 : ON )
			uint8_t		colorbar;			//	Color-Bar ( 0 : OFF, 1 : ON )
			uint8_t		showinfo;			//	Show Temperature Information ( 0 : OFF, 1 : ON )
			uint8_t		indicator;			//	High/Low Indicator ( 0 : OFF, 1 : ON )
			uint8_t		unit;				//	Temperature Unit ( 0 : Celcius, 1 : Fahrenheit )
			uint8_t		dhcp;				//	DHCP Enable ( 0 : OFF, 1 : ON )
			uint8_t		color;				//	Color Pallete ( reference IRF_CAM_PALETTE_TYPE_T )
													//	( 0 : GREY, 1 : RAINBOW, 2 : IRON, 3 : GREYRED, 4 : GLOWBOW, 5 : YELLOW, 6 : MIDGREY, 7 : MIDGREEN )
			uint8_t		alpha;				//	Alpha Blending ( 0 ; OFF, 1 : 20%, 2 : 40%, 3 : 60%, 4 : 80% )
			uint8_t		zoom;				//	Video Zoom ( 0 : OFF, 1 : x2, 2 : x4 )
			uint8_t		sharp;				//	CX Model Only
			uint8_t		noise;				//	Noise Filter ( 0 : OFF, 1 : NR1, 2 : NR2, 3 : NR1 + NR2 )
			uint16_t	nuc;				//	CX Model Only
			uint8_t		econt;				//	CX Model Only
			uint32_t	ipaddr;				//	IP Address ( ex : 192.168.0.100 => 0xC0A80064 )
			uint32_t	netmask;			//	Subnet Mask ( ex : 255.255.255.0 => 0xFFFFFF00 )
			uint32_t	gateway;			//	Gateway ( ex : 192.168.0.1 => 0xC0A80001 )
			uint32_t	dns;				//	Not Used

			uint8_t		alarm1_func;		//	CX Model Only
			uint8_t 	alarm1_cond;		//	CX Model Only
			int16_t		alarm1_value;		//	CX Model Only
			uint8_t		alarm2_func;		//	CX Model Only
			uint8_t		alarm2_cond;		//	CX Model Only
			int16_t		alarm2_value;		//	CX Model Only
			uint8_t		down_filter;		//	CX Model Only
			uint8_t		show_center;		//	Show Center Cross ( 0 : OFF, 1 : ON )
			uint8_t		show_spot;			//	Show Spot For CX Model ( 0 : OFF, 1 : ON ) 
													//	Show ROI For CG Model ( 0 : OFF, 1 : ON ) 
			uint8_t		show_correction;	//	Show Correction Parameters ( 0 : OFF, 1 : ON ) 
			uint8_t		show_isotherm;		//	CX Model Only
			uint8_t		alarm1_duration;	//	CX Model Only
			uint8_t		alarm2_duration;	//	CX Model Only

			struct {
				uint8_t flag;					//	ROI Function Mask ( 0x01 : Enable, 0x02 : Exclude )
				uint16_t x1;					//	Position (x2)
				uint16_t y1;					//	Position (x2)
				uint16_t x2;					//	Position (x2)
				uint16_t y2;					//	Position (x2)
			}	roi[2];								//	CX Model Only

			//-------------------------------------------------------------------------------------
			//	parameters below this line is for CG Model
			uint8_t     f_disp_icon;				//	Show Display Icon Flag ( 0 : OFF, 1 : ON )
            
            uint8_t     brightness;					//	Video Brightness Value ( -40 ~ 40 )
            uint8_t     contrast;					//	Video Contrast Value ( -10 ~ 10 )
            
            uint8_t     f_edge_enhance;				//	Video Edge Enhancement Flag ( 0 : OFF, 1 : ON )
            
            uint8_t     nuc_mode;					//	Core NUC Mode Value ( 0 : OFF, 1 : TIME, 2 : AUTO, 3 : TIME + AUTO )
            uint8_t     nuc_time;					//	Core NUC Time Value ( 0 : 1 MIN, 1 : 5 MIN, 2 : 10 MIN, 3 : 30 MIN, 4 : 60 MIN )
            uint16_t    nuc_thres;					//	Core NUC Auto Threshold Value ( 0 : lowest, 1 : low, 2 : middle, 3 : high, 4 : highest )
            
            uint16_t    agc_man_max;				//	Core AGC Manual Maximum Value ( 0 ~ 16383 )
            uint16_t    agc_man_min;				//	Core AGC Manual Minimum Value ( 0 ~ 16383 )
            
            uint8_t     srl_protocol;				//	Serial Protocol Value ( 0 : Pelco-D, 1 : COX )
            
            uint16_t    scn0_l_margin;				//  Video left margin ( 0 ~ 70 )
            uint16_t    scn0_r_margin;				//  Video right margin ( 0 ~ 70 )
            uint16_t    scn0_t_margin;				//  Video top margin ( 0 ~ 70 )
            uint16_t    scn0_b_margin;				//  Video bottom margin ( 0 ~ 70 )
            
            uint16_t    scn1_l_margin;				//	HDMI left margin ( 0 ~ 150 )
            uint16_t    scn1_r_margin;				//  HDMI right margin ( 0 ~ 150 )
            uint16_t    scn1_t_margin;				//  HDMI top margin ( 0 ~ 150 )
            uint16_t    scn1_b_margin;				//  HDMI bottom margin ( 0 ~ 150 )
            
            uint8_t     hdmi_mode;					//  HDMI Mode ( 2 : 480P, 3 : 576P, 4 : 720P 50, 5 : 720P 60, 6 : 1080I 50, 7 : 1080I 60, 9 : 1080P 50, 10 : 1080P 60 )
            
            uint8_t     alarm1_type;				//  Alarm1 type ( 0 : NO, 1 : NC )
            uint8_t     alarm1_mode;				//  Alarm1 operation mode ( 0 : OFF, 1 : STABILIZE, 2 : ALIVE PWM, 3 : TEMPERATURE, 4 : TEST ALARM ON, 5 : TEST ALARM OFF )
            uint8_t     alarm1_dura;				//  Alarm1 duration ( 0 ~ 99 )
			uint8_t     alarm1_remote_ctrl;			//  Alarm1 remote control ( 0 : OFF, 1 : ON ) 
            
            uint8_t     alarm2_type;				//  Alarm2 type ( 0 : NO, 1 : NC )
            uint8_t     alarm2_mode;				//  Alarm2 operation mode ( 0 : OFF, 1 : STABILIZE, 2 : ALIVE PWM, 3 : TEMPERATURE, 4 : TEST ALARM ON, 5 : TEST ALARM OFF )
            uint8_t     alarm2_dura;				//  Alarm2 duration ( 0 ~ 99 )
			uint8_t     alarm2_remote_ctrl;			//  Alarm2 remote control ( 0 : OFF, 1 : ON ) 

			int16_t		zero_offset;				//	Temperature Zero Offset
			uint16_t	measure_distance;			//	Measure Distamce

			uint8_t		nr1_strength;				//	Noise Reduction 1 Strength
			uint8_t		nr2_strength;				//	Noise Reduction 2 Strength
			uint8_t		ee_strength;				//	Edge Enhancement Strength
			//-------------------------------------------------------------------------------------
		};

		uint8_t		reserved1[128];
	};
	
	union {
		struct
		{
			uint8_t		limit9;				//	9Hz Mode
			uint8_t		enable_high;		//	CX Model Only
			uint8_t		correction;			//	Correction Parameters ( 0 : Not Used, 1 : Used )
			uint8_t		emissivity;			//	Emissivity ( 1 ~ 100 )
			uint8_t		transmission;		//	Transmission ( 1 ~ 100 )
			int16_t		atmosphere;			//	Atmosphere (-500 ~ 1000 )
		
			union {
				struct {
					uint8_t		enable;
					uint16_t	x;
					uint16_t	y;
					uint8_t		local;
					uint8_t		em;
					uint8_t		tr;
					int16_t		at;
					uint8_t		reserved[6];
				} spot[10];							//	CX Model Only

				struct  {
					uint8_t		mode;				//	ROI Mode
					uint8_t     temp_type;			//	Temperature Type ( Select MAX, MIN, AVG )
					uint16_t	x;					//	x position
					uint16_t	y;					//	y position
					uint16_t	w;					//	ROI Width
					uint16_t	h;					//	ROI Height
					uint8_t		f_corr;				//	Enable Correction
					uint8_t		em;					//	Emissivity
					uint8_t		tr;					//	Transmission
					int16_t		at;					//	Atmosphere					
					uint8_t     reserved;
				}   cg_roi[10];						//	CG Model Only
			};

			union {
				struct {
					uint8_t		enable;
					uint32_t	seed_color;
					int16_t		top;
					int16_t		bottom;
					uint8_t		reserved[3];
				} iso[3];							//	CX Model Only

				struct {
                    uint8_t		mode_mask;			//	ISO Mode Mask
                    uint8_t		tran_mask;			//	ISO Transparent Mask
                    
                    int16_t     max_temp;			//	Max Temperature
                    int16_t     min_temp;			//	Min Temperature
                    
                    uint32_t	above_color;		//	Above Seed Color
                    uint32_t	below_color;		//	Below Seed Color
                    uint32_t	between_color;		//	Between Seed Color
                } cg_iso[2];						//	CG Model Only
			};

													//	Start CG Model Only

			uint8_t         hdmi_list[8];			//	Supported HDMI Output Resolution List
			uint16_t		support_temp_mode;		//	Supported Temperature Mode

			uint8_t			f_support_motor_focus;	//	Support Motorized Focus
			uint8_t			f_support_motor_zoom;	//	Support Motorized Zoom

			uint16_t		meas_dist_min;			//	Supported Measurment Distance Minimum
			uint16_t		meas_dist_max;			//	Supported Measurment Distance Maximum

			int16_t			meas_temp[2][2];		//	Measurement Temperature
			uint16_t		meas_level[2][2];		//	Measurement Level

			uint8_t			meas_name[2][10];		/**<  20  363  Measurment Name */

													//	End CG Model Only
		};

		uint8_t		reserved2[256];	
	};
		
	union {	// 3rd section (128) = 512
        struct  {
            ALRMCFG         max_temp;				//	MAX Temperature 
            ALRMCFG         min_temp;				//	MIN Temperature 
            ALRMCFG         avg_temp;				//	AVG Temperature 
            ALRMCFG         ctr_temp;				//	CTR Temperature 
            ALRMCFG         roi_temp[10];			//	ROI Temperature 
        }   temp_alarm;								//	CG Model Only
        
		uint8_t     reserved3[128];
	};	// 3rd section (128) = 512
}	IRF_SAVEDATA_T;


/* Structure of IR data header */
typedef struct  
{
	BYTE					dynamic_range;			// IRF_DYNAMIC_RANGE_T 

	int						year;					// File creation year 
	int						month;					// File creation month 
	int						day;					// File creation day 
	int						hour;					// File creation hour 
	int						minute;					// File creation minute 
	int						second;					// File creation second 
	int						total_frame;			// Total frame number 

	IRF_SAVEDATA_T			save_data;		// Cam data in CAM_DATA

	BYTE					reserved[460];
}	IRF_IR_DATA_HEADER_T;


/* Structure of CRD file header */
typedef struct
{
	BYTE					fhSignature[3];			// ASC CODE of "CRD" 
	uint16_t				fhSize;					// Size of header 
	BYTE					fhReserved1;			// Reserved for later use 
	BYTE					fhVer;					// File version of CRD format 

	IRF_IMAGE_INFO_T				imageInfo;		// Width and height of image 
	IRF_TEMP_CORRECTION_PAR_T		tempCorrection;	// Correction Information 

	BYTE					dynamicRange;			// IRF_DYNAMIC_RANGE_T 
	int						fcYear;					// File creation year 
	int						fcMon;					// File creation month 
	int						fcDay;					// File creation day 
	int						fcHour;					// File creation hour 
	int						fcMin;					// File creation minute 
	int						fcSec;					// File creation second 
	uint32_t				totalFrame;				// Total frame number 
	int						fps;					// Frame per second 

	BYTE					reserved[968];			// Reserved for later use 
}	IRF_CRD_FILE_HEADER_T;


/* Structure of CRD data header */
typedef struct
{
	BYTE					dhSeparator[2];			// Start string of DATA_HEADER 
	BYTE					dhSize;					// Size of DATA_HEADER 
	BYTE					dhType;					// Type of Data ( 0x00 : Image, 0x01 : Unkown ) 
	uint32_t				curFrame;				// Current frame 
	__int64					savedTime;				// Saved time 
	uint32_t				dataSize;				// Data size 

	BYTE					reserved[240];			// Reserved for later use 	
}	IRF_CRD_DATA_HEADER_T;


/* Structure of received data processing from TCP/IP */
typedef struct 
{
	uint16_t*				ir_image;				//	16bits raw image data * 10
	DWORD					image_buffer_size;		//	Raw image size.
	LPBYTE					lpNextData;				//	This variable is remainder data after make a raw image from communication buffer, it used for make next raw image.
	DWORD					dwSize;					//	This variable is size of remainder data.
	DWORD					dwPosition;				//	This variable is current position in the remainder data.

	IRF_MESSAGE_TYPE_T		msg_type;
	IRF_SAVEDATA_T			save_data;				//	cam data in CAM_DATA

	uint32_t				fw_ver;					//	Firmware version in SYS_INFO

	//++ csi 20170728 add core info
	uint32_t				core_type;				//	core type ( 0 : QVGA, 1 : VGA )
	uint16_t				h_res;					//	horizontal resolution
	uint16_t				v_res;					//	vertical resolution
	//++

	uint16_t				PMSGTYPE;				//	Primary Message Type Code
	uint16_t				RCODE;					//	Response Code

	DWORD					reserved[10];			//	Reserved ( Write may cause problem )
}	IRF_IR_CAM_DATA_T;

/* Structure of system info */
typedef struct {
	BYTE					MAC[6];					// MAC address
	uint32_t				fw_ver;					// Firmware version (Package)
	uint32_t				cpu_ver;				// Firmware version (CPU)
	uint32_t				fpga_ver;				// Firmware version (FPGA)
	uint32_t				engine_ver;				// Firmware version (Engine)

	BYTE					reserved[42];			// Reserved
}	IRF_SYS_INFO_T;

/* Structure of system information of camera */
typedef struct {
	IRF_SYS_INFO_T			sys_info;				// System info
	IRF_MESSAGE_TYPE_T		msg_type;

	uint16_t				PMSGTYPE;				// Primary Message Type Code
	uint16_t				RCODE;					// Response Code
}	IRF_RECEIVED_CAM_SYS_T;


/* Structure of temperature table for temperature correction */
typedef struct {
	int16_t					normal_min;				// Minimum temperature in the normal temperature table
	int16_t					normal_max;				// Maximum temperature in the normal temperature table
	int16_t					high_min;				// Minimum temperature in the high temperature table
	int16_t					high_max;				// Maximum temperature in the high temperature table

	uint32_t				normal_table[256];		// Normal temperature table ( normal_max - normal_min + 1 ) * 4bytes(level)
	uint32_t				high_table[256];		// High temperature table ( ( high_max - high_min ) / 5 + 1 ) * 4bytes(level)
}	IRF_TEMP_TABLE_T;


/* Structure of temperature offset table for temperature correction */
typedef struct {
	int16_t					normal_min;				// Minimum temperature in the normal temperature table
	int16_t					normal_max;				// Maximum temperature in the normal temperature table
	int16_t					high_min;				// Minimum temperature in the high temperature table
	int16_t					high_max;				// Maximum temperature in the high temperature table

	int						normal_table[256];		// Normal temperature table ( normal_max - normal_min + 1 ) * 4bytes( temperature * 10 )
	int						high_table[256];		// High temperature table ( ( high_max - high_min ) / 5 + 1 ) * 4bytes( temperature * 10 )
}	IRF_TEMP_OFFSET_TABLE_T;


/* Structure of temperature table and temperature offset table */
typedef struct {
	IRF_TEMP_TABLE_T				temp_table;				// Temperature table structure
	IRF_TEMP_OFFSET_TABLE_T			temp_offset_table;		// Temperature offset table structure
}	IRF_MIX_TEMP_TABLE_T;


/* Structure of receive temperature and offset table from a camera */
typedef struct {	
	IRF_MESSAGE_TYPE_T				msg_type;		// Message type
	IRF_MIX_TEMP_TABLE_T			sTemp_table;	// Temperature table structure	

	uint16_t			PMSGTYPE;				// Primary Message Type Code
	uint16_t			RCODE;					// Response Code
}	IRF_RECEIVED_TEMP_TABLE_T;
#pragma pack()
#pragma endregion Structure Definition


#pragma region Function definition
#ifdef __cplusplus
extern "C" {
#endif

//	Common Interface
THERMALSDK_API int16_t	PASCAL	OpenConnect(HANDLE *pHandle, HANDLE *pTimerID, LPCSTR strDestination, LPCSTR strServiceName, int nProtocol, int nType);
THERMALSDK_API int16_t	PASCAL	CloseConnect(HANDLE *handle, HANDLE timerID);

THERMALSDK_API int16_t	PASCAL	GetIRHeader(HANDLE handle, IRF_IR_FILE_HEADER_T* header, long *curPos);
THERMALSDK_API int16_t	PASCAL	GetIRHeaders(HANDLE handle, IRF_IR_FILE_HEADER_T* header, IRF_IR_DATA_HEADER_T* addedInfo, long *curPos);

THERMALSDK_API int16_t	PASCAL	SendCameraMessage(HANDLE handle, HANDLE *pTimerID, IRF_MESSAGE_TYPE_T type, uint16_t PMSGTYPE, uint16_t RCODE);
THERMALSDK_API int16_t	PASCAL	SendMessageToCamera(HANDLE handle, HANDLE *pTimerID, IRF_MESSAGE_TYPE_T type, uint16_t PMSGTYPE, uint16_t RCODE, DWORD RCODE2, DWORD RCODE3, DWORD RCODE4);

THERMALSDK_API int16_t	PASCAL	GetIRImages(HANDLE handle, HANDLE *pTimerID, IRF_IR_CAM_DATA_T* cam_data);

THERMALSDK_API int16_t	PASCAL	GetImageLUT(uint8_t *palette, IRF_PALETTE_TYPE_T paletteType, bool bInvert);
THERMALSDK_API int16_t	PASCAL	GetGrayToPaletteImage(uint8_t *from_image, void* to_image, uint16_t width, uint16_t height, uint8_t *palette, int BitsPixel, BOOL bMirror, BOOL bFlip);
THERMALSDK_API int16_t	PASCAL	GetIRHistogram(uint32_t *hist, uint16_t *ir_image, long image_size);


//	CX320 Only Interface
THERMALSDK_API float	PASCAL	GetCorrectedTemp(float *tempLUT, IRF_TEMP_CORRECTION_PAR_T corrPara, uint16_t engineOut);
THERMALSDK_API int16_t	PASCAL	GetCamTempTable(HANDLE handle, HANDLE *pTimerID, IRF_RECEIVED_TEMP_TABLE_T* temp_table);
THERMALSDK_API int16_t	PASCAL	GetTempRangeValue(IRF_DYNAMIC_RANGE_T tempMode, int16_t *min, int16_t *max);

//	CX320, CX640 Only Interface
THERMALSDK_API int16_t	PASCAL	GetTempMapTable(float* tempLUT, IRF_DYNAMIC_RANGE_T tempMode);
THERMALSDK_API int16_t	PASCAL	GetImage(uint8_t *image, HANDLE ir_image, long image_size, float *tempLUT, float *level, float *span, IRF_AUTO_RANGE_METHOD_T *method);
THERMALSDK_API int16_t	PASCAL	GetCorrectedImage(uint8_t *image, HANDLE ir_image, long image_size, float* tempLUT, IRF_TEMP_CORRECTION_PAR_T corrPara, float *level, float *span, IRF_AUTO_RANGE_METHOD_T *method);
THERMALSDK_API float	PASCAL	GetPointTemp(HANDLE ir_image, IRF_IMAGE_INFO_T image_info, float *tempLUT, IRF_TEMP_CORRECTION_PAR_T corrPara, POINT pt);
THERMALSDK_API int16_t	PASCAL	GetROITemp(HANDLE ir_image, IRF_IMAGE_INFO_T image_info, float *tempLUT, IRF_TEMP_CORRECTION_PAR_T corrPara, RECT roi, IRF_NUMERIC_INFO_T *numInfo, POINT *min_pt, POINT *max_pt);
THERMALSDK_API int16_t	PASCAL	GetRawToTemp(HANDLE ir_image, IRF_IMAGE_INFO_T image_info, float *tempLUT, IRF_TEMP_CORRECTION_PAR_T corrPara, float* tempImage);
THERMALSDK_API int16_t	PASCAL	SendPaletteMessage(HANDLE handle, HANDLE *pTimerID, IRF_SET_USER_PALETTE struct_palette);
THERMALSDK_API int16_t	PASCAL	GetCameraTempRangeValue( IRF_DYNAMIC_RANGE_T tempMode, IRF_CAM_MODEL_TYPE_T camType, int16_t *min, int16_t *max );

//	CG320, CG640 Only Interface
THERMALSDK_API int16_t	PASCAL	GetImageCG( uint8_t *image, HANDLE ir_image, long image_size, float *level, float *span, IRF_AUTO_RANGE_METHOD_T *method );
THERMALSDK_API float	PASCAL	GetPointTempCG( HANDLE ir_image, IRF_IMAGE_INFO_T image_info, IRF_TEMP_CORRECTION_PAR_T corrPara, POINT pt );
THERMALSDK_API int16_t	PASCAL	GetROITempCG( HANDLE ir_image, IRF_IMAGE_INFO_T image_info, IRF_TEMP_CORRECTION_PAR_T corrPara, RECT roi, IRF_NUMERIC_INFO_T *numInfo, POINT *min_pt, POINT *max_pt );
THERMALSDK_API int16_t	PASCAL	GetRawToTempCG( HANDLE ir_image, IRF_IMAGE_INFO_T image_info, IRF_TEMP_CORRECTION_PAR_T corrPara, float* tempImage );
THERMALSDK_API float	PASCAL	ConvertRawToTempCG( HANDLE _ir_image, IRF_TEMP_CORRECTION_PAR_T _corr, uint16_t _raw );
THERMALSDK_API uint16_t PASCAL	ConvertTempToRawCG( HANDLE _ir_image, IRF_TEMP_CORRECTION_PAR_T _corr, float _temp );
THERMALSDK_API int16_t	PASCAL	GetTempRangeValueCG( HANDLE _ir_image, IRF_DYNAMIC_RANGE_T tempMode, int16_t *min, int16_t *max );

//	Image Filter Interface
THERMALSDK_API int16_t	PASCAL	ApplyImageFilter(uint8_t *image, uint16_t width, uint16_t height, IRF_IMAGE_FILTER_T filter);
THERMALSDK_API int16_t	PASCAL	ApplyColorImageFilter(void* image, uint16_t width, uint16_t height, IRF_IMAGE_FILTER_T filter, int bitPixel);
THERMALSDK_API void		PASCAL	BilateralFilter(uint8_t *image, uint16_t width, uint16_t height, float sigD, float sigR, int w);
THERMALSDK_API void		PASCAL	GetGaussianKernel(int *kernel, int *mult, int sz);
THERMALSDK_API int16_t	PASCAL	FastGaussianBlur(BYTE *img, int iw, int ih, int *Gkernel, int *Gmult, int radius);
THERMALSDK_API int16_t	PASCAL	FastStackBlur(BYTE* img, int w, int h, int radius);
THERMALSDK_API int16_t	PASCAL	BoxBlur(BYTE *src, int src_w, int src_h, int sz);

//	CRD File Interface
THERMALSDK_API int16_t	PASCAL	CreateIRCamDataCG( IRF_IR_CAM_DATA_T* _p_cam_data );
THERMALSDK_API int16_t	PASCAL	CloseIRCamDataCG( IRF_IR_CAM_DATA_T* _p_cam_data );
THERMALSDK_API int16_t	PASCAL	LoadIRCamDataCG( IRF_IR_CAM_DATA_T* _p_cam_data, uint16_t* _p_ir_buf, long _image_size );
THERMALSDK_API int16_t	PASCAL	LoadIRImage(HANDLE *handle, char *FileName, long *totSize);
THERMALSDK_API int16_t	PASCAL	GetIRImageFromStream(HANDLE handle, uint16_t* ir_image, long image_size, unsigned long totStreamSize, unsigned long *curPos, int* gap_time, bool bLoop);
THERMALSDK_API int16_t	PASCAL	GetIRImageFromStream_n(HANDLE handle, uint16_t* ir_image, long image_size, unsigned long totStreamSize, unsigned long *curPos, int* gap_time, bool bLoop, bool new_ver);	// version < 20
THERMALSDK_API int16_t	PASCAL	GetIRImageFromStream_v2(HANDLE handle, uint16_t* ir_image, long image_size, unsigned long totStreamSize, unsigned long *curPos, int* gap_time, __int64* curTime, bool bLoop, uint8_t ver);	// version >= 20
THERMALSDK_API int16_t	PASCAL	GetRevIRImageFromStream(HANDLE handle, uint16_t* ir_image, long image_size, unsigned long *curPos, int* gap_time);
THERMALSDK_API int16_t	PASCAL	GetRevIRImageFromStream_n(HANDLE handle, uint16_t* ir_image, long image_size, unsigned long *curPos, int* gap_time, bool new_ver);
THERMALSDK_API int16_t	PASCAL	GetRevIRImageFromStream_v2(HANDLE handle, uint16_t* ir_image, long image_size, unsigned long *curPos, int* gap_time, __int64* curTime, uint8_t ver);
THERMALSDK_API int16_t	PASCAL	SaveIRImage(HANDLE *handle, char* filename, IRF_IR_FILE_HEADER_T *pHeader);
THERMALSDK_API int16_t	PASCAL	SaveIRHeader(HANDLE *handle, char* filename, IRF_IR_FILE_HEADER_T *pHeader, IRF_IR_DATA_HEADER_T *pAddedData);
THERMALSDK_API int16_t	PASCAL	SetIRImageToStream(HANDLE handle, uint16_t* ir_image, long image_size, int millisecond, int16_t *frameCnt);	// file version < 20
THERMALSDK_API int16_t	PASCAL	SetIRImageToStream_v2(HANDLE handle, uint16_t* ir_image, long image_size, int millisecond, int16_t *frameCnt, uint8_t ver);
THERMALSDK_API int16_t	PASCAL	CloseIRStream(HANDLE handle);
THERMALSDK_API void		PASCAL	DrawColorBar(HWND hWnd, HDC hDC, uint8_t* palette, float level, float span, IRF_TEMP_MODE_T tempUnit, bool bUpdateOnlyTickArea);
THERMALSDK_API void		PASCAL	DrawMinMaxPos(HDC hDC, POINT minP, POINT maxP, int size);


//	Temperature Converter
THERMALSDK_API float	PASCAL	ConvertFahToCels(float temp);
THERMALSDK_API float	PASCAL	ConvertCelsToFah(float temp);
THERMALSDK_API float	PASCAL	ConvertKelvToCels(float temp);
THERMALSDK_API float	PASCAL	ConvertKelvToFah(float temp);
THERMALSDK_API float	PASCAL	ConvertCelToKel(float temp);
THERMALSDK_API float	PASCAL	ConvertFahToKel(float temp);
//-----------------------------------------------------------------------------
#ifdef __cplusplus
}
#endif


#pragma endregion Function definition
