#include <string.h>
#include <stdlib.h>

#include "app_skyiot_server.h"
#include "datatrans_server_app.h"
#include "datatrans_model.h"
#include "utils_md5.h"
#include "platform_utils.h"
#include "soft_wdt.h"


// #define APP_DBG_PRINTF(fmt, ...)   data_uart_debug(fmt, ##__VA_ARGS__)
#define APP_DBG_PRINTF0  APP_PRINT_WARN0
#define APP_DBG_PRINTF1  APP_PRINT_WARN1
#define APP_DBG_PRINTF2  APP_PRINT_WARN2
#define APP_DBG_PRINTF3  APP_PRINT_WARN3
#define APP_DBG_PRINTF4  APP_PRINT_WARN4
#define APP_DBG_PRINTF5  APP_PRINT_WARN5

MESH_PROVISION_STATE_e mesh_provison_state = MESH_PROVISION_STATE_UNPROV;
/*
 * DEFINES
 ****************************************************************************************
 */
#if MESH_TEST_PRESSURE == 1
uint8_t test_flag = 0;
uint16_t testperid = 0; //10s
uint32_t makepackcnt = 0;
uint32_t sendpackcnt = 0;
uint32_t revackpackcnt = 0;
uint32_t acktimoutcnt = 0;
uint8_t  ackattrsval=0;
#endif

// timer 
#define MAX_QUICK_ONOFF_CNT        (5)
#define DEVICE_POWER_ON_TIMEOUT    (3000)    // ms  快速开关进配网，检查间隔3s
#define PROV_FLASH_LIGHT_INTERVAL  (500)     // ms

#define SKYBLERESET_MAXCNT         (10)    // 重配网延时服务，延时次数，即重配网闪灯次数
#define SKYBLERESET_TIMEOUT        (150)

#define SKYBLEPROVSUCCESS_MAXCNT   (10)    // 配网成功闪
#define SKYBLEPROVSUCCESS_TIMEOUT  (500)

#define SKYIOTSAVEATTR_INTERVAL    (5000)  // 定时保存attr的参数


// FIFO
#define MAIN_MSG_FIFO_DEEP         (10)	

// communication
#define MAC_ADDRESS_LEN           (6)
#define BT_PRODUCT_MODEL_LEN      (16)
#define BT_MESH_UUID_SIZE         (MESH_DEV_UUID_LEN)  
#define MD5_LEN                   (16)  
#define BLEMESH_PROPERTY_NAME_LEN (4)
#define BLEMESH_COMMAND_WAIT_MS   (500) /* WAIT 250MS FOR Report ack */	
#define DEFAULT_WAKEUP_ALIVE_CNT  (3)

#define DEFAULT_SKYIOT_ALIVE_MS   (60000)  // qlj 需要一个定时器来换算
#define DEFAULT_GATEWAY_ALIVE_MS  (90000)
#define DEFAULT_GATEWAY_SENQ_MS   (5000)


#define MAX_BLEMESH_PACKET_LEN    (32)	// BLEMESH 数据包最大长度 


#define TX_OPENCODE_POS     0
#define TX_COMMAND_POS      3
#define TX_ATTR_ID_POS      5
#define TX_ATTR_VAL_POS     7

#define MAX_TX_BUF_LEN 			(MAX_BLEMESH_PACKET_LEN)    // 每个发送缓存最大32字节
#define MAX_RE_TX_CNT 			(3)     // 最大重发次数
#define MAX_ACK_TIMEOUT 		(4000)  // 最大应答超时ms
#define MAX_TX_BUF_DEEP 		(10)    // 最大10个发送缓存
static uint16_t maxacktimout =  MAX_ACK_TIMEOUT;

typedef struct
{
	uint8_t  buf[MAX_TX_BUF_LEN]; // opecode ~ end
	uint8_t  len;
	uint8_t  fullflag; // 缓存占用标志 1:used 0:unused
	uint8_t  txcnt;    // 本包发送次数倒计
	uint32_t txtick;   // 记录最近一次发送时间tick
}MESH_TX_ATTR_STRUCT;
// MESH_TX_ATTR_STRUCT MeshTxAttrStruct[ MAX_TX_BUF_DEEP ];
MESH_TX_ATTR_STRUCT *MeshTxAttrStruct=NULL;


// 接收数据类型
typedef enum {
	EVENT_TYPE_PROVISIONED = 0x1,	/* 设备配置成功 */	
	EVENT_TYPE_KEEPALIVE,
	EVENT_TYPE_UPDATE_PROPERTY,
	EVENT_TYPE_GET_PROPERTY,
	EVENT_TYPE_REPORT_PROPERTY_ACK,
	EVENT_TYPE_GET_PRODUCT_TYPE,
	EVENT_TYPE_GET_PRODUCT_BRAND,
	EVENT_TYPE_GET_PRODUCT_MODEL,
	EVENT_TYPE_GET_PRODUCT_MAC,
}MESH_EVENT_TYPE_e;
	
typedef enum {
	MESH_PACK_STATE_IDLE,        // 当前不在发送
	MESH_PACK_STATE_REPORT_ATTR, // 当前正在发送
}MESH_PACK_STATE_e;

typedef enum  {
	PROPERTY_TYPE_CHAR   = 0x80,     // attr_param[0]
	PROPERTY_TYPE_SHORT  = 0x81,     // attr_param[0]~[1]
	PROPERTY_TYPE_INT    = 0x82,     // attr_param[0]~[3]
	PROPERTY_TYPE_FLOAT  = 0x83,     // attr_param[0]~[3] 按 Prop_float 格式
	PROPERTY_TYPE_STRING = 0x84	
}PROPERTY_DATA_TYPE_e;

/*
 * STRUCT
 ****************************************************************************************
 */
typedef struct {
	uint8_t  event_id;	/*keep alive, getproducttype, getproduct model, getproduct brand, control*/
	uint8_t seqence_num;
	uint16_t prop_ID;
	int32_t  prop_value;
}req_event_t;

typedef struct 
{	
	uint8_t  head;
	uint8_t  tail;
	req_event_t  even_t[MAIN_MSG_FIFO_DEEP];
}main_msg_fifo_t;


typedef struct {
	// SkyBleMesh_IsProvision_Sate()
	// uint8_t network_mode;  /* NETWORK_MODE_e, 设备的网络模式 */  
	
	uint8_t process_state; /* 设备工作状态 */	
	uint8_t mac_address[MAC_ADDRESS_LEN];
	char    product_model[BT_PRODUCT_MODEL_LEN];
	uint32_t  product_type;
	uint32_t  product_brand; 		/* 设备品牌*/
	uint8_t device_uuid[BT_MESH_UUID_SIZE]; /* 设备UUID */
	unsigned char critical_md5[MD5_LEN]; /* md5 for network_mode, mode,bri,ctp hsv */
		
	//协议控制字段
	//uint8_t  mesh_pack_state; /* wait idle */	
	uint8_t  alive_status;	  /* 0: online, 1: offline */
	uint8_t  alive_wakeup_cnt;  /*上电或者睡眠唤醒后发三次心跳包*/
	uint32_t recv_alive_tick;	/* 上次接受到网关心跳的时间 */
	uint32_t send_alive_tick; /* 发送心跳得时间 */

	//uint32_t updpack_recv_tick; /* 控制数据包序列号 */ 	
	//uint16_t updpack_seqence_num; /* 控制数据包序列号，应用层检测 ,网关的数据包是一直增加的，如果断线记得清掉 */
	//uint16_t reppack_seqence_num; /* 上报数据包序列号 */
	//uint32_t reppack_send_tick;	  /* 上报的时间 */	
	uint8_t  reppack_buffer[MAX_BLEMESH_PACKET_LEN];
	uint8_t  reppack_len;
	
	//设备字段
	#if USE_LIGHT_FOR_SKYIOT
		SkyLightManager mLightManager;
	
		#if ((SKY_LIGHT_TYPE==SKY_LIGHT_BELT_TYPE)||(SKY_LIGHT_TYPE==SKY_LIGHT_BULB_TYPE))
		uint8_t status_seqnum; /* 上报属性的序列号 */
		uint8_t bri_seqnum;	 
		uint8_t ctp_seqnum;	
		#elif (SKY_LIGHT_TYPE==SKY_LIGHT_BULB_RGBWY_TYPE)
		uint8_t status_seqnum;  
		uint8_t bri_seqnum;	 
		uint8_t ctp_seqnum;	 
		uint8_t hue_seqnum;	 		
		uint8_t sat_seqnum;	
		#endif
	#endif

	
	 uint32_t report_flag;	 // 按位对应设备的控制属性,置位即发
	// Queue_t	 req_queue;		/* message queue for wifi , uart module */
}SkyBleMeshIotManager;
	

/*
 * VARIABLES
 ****************************************************************************************
 */

// timer
static plt_timer_t quick_onoff_timer = NULL;
static plt_timer_t skyblereset_timer = NULL;
static plt_timer_t skyiot_SaveAttr_timer = NULL;
static plt_timer_t skybleprosuccess_timer = NULL;
static plt_timer_t skyblemainloop_timer = NULL;

// FIFO
static main_msg_fifo_t MainMsg_fifo={
	.head=0,
	.tail=0,
};
	
static SkyBleMeshIotManager mIotManager;
static int8_t   g_aliveTimerCnt = 0;
static uint32_t g_quick_onoff_Cnt = 0;
static bool IsSkyAppInited = false;
static uint8_t g_skybleresetcnt=0;     // 重配网延时服务，延时次数，即重配网闪灯次数
static uint8_t skybleprovsucesscnt=0;     

#if USE_LIGHT_FOR_SKYIOT
// 需要定时保存的属性
	#if ((SKY_LIGHT_TYPE==SKY_LIGHT_BELT_TYPE)||(SKY_LIGHT_TYPE==SKY_LIGHT_BULB_TYPE))
	static int32_t save_oldbri;	/* 日光模式亮度 */		
	static int32_t save_oldctp;	/* 日光模式色温 */

	#elif (SKY_LIGHT_TYPE==SKY_LIGHT_BULB_RGBWY_TYPE)
	static int32_t save_oldbri;	/* 亮度 */		
	static int32_t save_oldctp;	/* 日光模式色温 */
	static int32_t save_oldhue;	/* 色度 */		
	static int32_t save_oldsat;	/* 饱和度 */

	#endif
#endif


static unsigned int SkyBleMesh_GetProductType(void);
static int SkyBleMesh_ReadConfig(void);

/*
 * FUNCTIONS
 ****************************************************************************************
 */
 
#if 1
// qlj len 加长度越界判断
static bool Hal_FlashWrite(FLASH_PARAM_TYPE_e type, uint16_t len, void *pdata)
{
    uint32_t ret = 0;
	
    switch (type) {       
		case FLASH_PARAM_TYPE_APP_CFGDATA:{
            ret = ftl_save(pdata, FTLMAP_APPCFGDATA_OFFSET, len);
			break;
		}
		case FLASH_PARAM_TYPE_QUICK_ONOFF_CNT:{
            ret = ftl_save(pdata, FTLMAP_QUICK_ONOFF_OFFSET, len);
			 break;			   
        }
        
		default:
        break;
    }

    if (0 != ret){
        printe("Hal_FlashWrite: failed, type = %d, cause = %d", type, ret);
    }

    return (0 == ret);
}

static bool Hal_FlashRead(FLASH_PARAM_TYPE_e type, uint16_t len, void *pdata)
{
    uint32_t ret = 0;
	
    switch (type) {       
		case FLASH_PARAM_TYPE_APP_CFGDATA:{
            ret = ftl_load(pdata, FTLMAP_APPCFGDATA_OFFSET, len);
			break;
		}
		case FLASH_PARAM_TYPE_QUICK_ONOFF_CNT:{
            ret = ftl_load(pdata, FTLMAP_QUICK_ONOFF_OFFSET, len);
			 break;			   
        }
        
		default:
        break;
    }

    if (0 != ret){
        printe("Hal_FlashRead: failed, type = %d, cause = %d", type, ret);
    }

    return (0 == ret);
}

#endif

static char HAL_HexToChar(uint8_t bHex)
{
	if((bHex>=0)&&(bHex<=9))
	{
		bHex += 0x30;
	}
	else if((bHex>=10)&&(bHex<=15))//Capital
	{
		bHex += 0x37;
	}
	else 
	{
		bHex = 0xff;
	}
	if((bHex >= 0x41)&&(bHex <= 0x5A))
		bHex += 32;
	
	return (char)bHex;
}
static uint8_t HAL_IntToString(int num, char* str)
{
	int len = 0;
	int temp = num;
	char buf[16];
	int index = 0;

	while(temp)
	{
		buf[len++] = (temp % 10) + '0';  //把temp的每一位上的数存入buf
		temp = temp / 10;
	}
	//change seq
	for (index = 0; index < len; index++){
		str[index] = buf[len-index-1];
	}
	return len;
}

/*
** tick cnt
*/
static uint32_t HAL_GetTickCount(void)
{
	uint32_t tmp32 = 0;
	
	tmp32 = os_sys_time_get();
	// tmp32 = os_sys_tick_get(); // *portTICK_PERIOD_MS
	
	return (tmp32 & 0xFFFFFFFF);
}

static uint32_t HAL_CalculateTickDiff(uint32_t oldtick, uint32_t newtick)
{
	uint32_t ms=0;
	
	if(newtick >= oldtick){
		ms = newtick - oldtick;
	} else {
		ms = (0xFFFFFFFF - oldtick) + newtick;
	}
	
	return ms;
}

/*
** reset
*/
static void HAL_ResetBleDevice(void)
{
	// DisableSoftWdt();
	WDG_SystemReset(RESET_ALL, SW_RESET_APP_START);
	while(1);
}


/*
** timer
*/	


/*
** FIFO
*/
static void Push_Msg_Into_FIFO(req_event_t *event)
{
	if(event){
		memcpy(&(MainMsg_fifo.even_t[MainMsg_fifo.head]), event, sizeof(req_event_t));

		MainMsg_fifo.head = (MainMsg_fifo.head+1)%MAIN_MSG_FIFO_DEEP;
		if(MainMsg_fifo.head == MainMsg_fifo.tail){
			MainMsg_fifo.tail = (MainMsg_fifo.tail+1)%MAIN_MSG_FIFO_DEEP;
		}
	}
}

static bool Pop_Msg_From_FIFO(req_event_t *event)
{
	bool isgetmag = false;

	if(event) {
		if(MainMsg_fifo.head != MainMsg_fifo.tail){
				
//		APP_DBG_PRINTF2("Pop_Msg_From_FIFO %d %d \n", MainMsg_fifo.head , MainMsg_fifo.tail );
		
			memcpy(event, &(MainMsg_fifo.even_t[MainMsg_fifo.tail]), sizeof(req_event_t));

			MainMsg_fifo.tail = (MainMsg_fifo.tail+1)%MAIN_MSG_FIFO_DEEP;

			isgetmag = true;
		}
	}

	return isgetmag;
}

/*
** 获取随机数
*/
static void Regain_Random_UUID(uint8_t *uuid, uint8_t len)
{
	uint32_t val;
	uint8_t slen;
	
	while(len){
		
		// val = rand();
		val = platform_random(0xFFFFFFFF);
		
		slen = len > 4 ? 4: len;
		len -= slen;
		while(slen--){
			*uuid++ = val;
			val >>= 8;
		}
	}	
}

#if 1  // 获取参数
extern bool Hal_Get_Ble_MacAddr(uint8_t* mac)
{
    bool isgetted=false;
	
	if(mac){
		if( GAP_CAUSE_SUCCESS == gap_get_param(GAP_PARAM_BD_ADDR, mac) ){
			if(mac[0]!=0xFF||mac[1]!=0xFF||mac[2]!=0xFF||mac[3]!=0xFF||mac[4]!=0xFF||mac[5]!=0xFF){
				
				isgetted = true;
			}
		}
	}
	
	if(isgetted==false){
		APP_DBG_PRINTF0("Hal_Get_Ble_MacAddr error!!!\n");
	}
	
	return isgetted;
}

extern void SkyBleMesh_Get_UUID(uint8_t *uuid, uint8_t len)
{
	if(uuid && len==MESH_DEV_UUID_LEN){
		memcpy(uuid, mIotManager.device_uuid, MESH_DEV_UUID_LEN);
	}
}

extern void SkyBleMesh_Get_DeviceName(char *name, uint8_t *len)
{
	// 广播需要前面加 len+GAP_AD_TYPE_COMPLETE_NAME
	uint8_t typelen;
	int product_type = SkyBleMesh_GetProductType();

	if(name){	
	
		name[0] = 'S';
		name[1] = 'K';
		name[2] = 'Y';
		name[3] = 'M';
		name[4] = 'E';
		name[5] = 'S';
		name[6] = 'H';
		
		typelen = HAL_IntToString(product_type, &name[7]);
	
		name[7+typelen] = 'M';	
		
		// 后四位mac地址， 注意大小端
		name[8+typelen] = HAL_HexToChar(mIotManager.mac_address[1]>>4);
		name[9+typelen] = HAL_HexToChar(mIotManager.mac_address[1]&0xF);
		name[10+typelen] = HAL_HexToChar(mIotManager.mac_address[0]>>4);
		name[11+typelen] = HAL_HexToChar(mIotManager.mac_address[0]&0xF);	

		*len = 11+typelen+1;
	}
	APP_DBG_PRINTF1("SkyBleMesh_Get_DeviceName :%x\n", name[0]);	
	// APP_DBG_PRINTF1("SkyBleMesh_Get_DeviceName :%s\n", name);	
}

static int SkyBleMesh_SetProductType(unsigned int type)
{
	if (mIotManager.product_type == 0){
		mIotManager.product_type = type;
	}
	return 0;
}

static unsigned int SkyBleMesh_GetProductType(void)
{
	return mIotManager.product_type;
}

static int SkyBleMesh_SetProductModel(char* model)
{
	if (mIotManager.product_model[0] == 0){
		strncpy(mIotManager.product_model, model, BT_PRODUCT_MODEL_LEN-1);
	}	
	return 0;	
}

static int SkyBleMesh_SetProductBrand(unsigned int brand)
{
	if (mIotManager.product_brand == 0) {
		mIotManager.product_brand = brand;
	}
	return 0;
}

static char* SkyBleMesh_GetProductModel()
{
	return mIotManager.product_model;
}

static unsigned int SkyBleMesh_GetProductBrand()
{
	return mIotManager.product_brand;
}
static uint8_t* SkyBleMesh_GetProductMac()
{
	return mIotManager.mac_address;
}


#endif


#if 1  // 通信组包
static void BleMesh_Packet_Clear(void)
{	
	uint8_t i=0;
	
	if(MeshTxAttrStruct==NULL){
		return;
	}
	
	for(i=0; i<MAX_TX_BUF_DEEP; i++){
		
		MeshTxAttrStruct[i].txcnt    = 0 ;
		MeshTxAttrStruct[i].fullflag = 0;  
		MeshTxAttrStruct[i].txtick   = 0;
		MeshTxAttrStruct[i].len      = 0;
		memset(MeshTxAttrStruct[i].buf, 0, MAX_TX_BUF_LEN);
	}
}
static void BleMesh_Packet_Init(void)
{	
	if(MeshTxAttrStruct){
		return;
	}

	MeshTxAttrStruct = os_mem_zalloc(RAM_TYPE_DATA_ON, sizeof(MESH_TX_ATTR_STRUCT)*MAX_TX_BUF_DEEP);
	if(MeshTxAttrStruct==NULL){
		return;
	}
	BleMesh_Packet_Clear();
}
// put
static void BleMesh_Vendor_Make_Packet(uint8_t *buf, uint8_t len, bool needack )
{
	uint8_t i=0, index=0xFF;
	uint8_t opcode=buf[TX_OPENCODE_POS];
	
	if(MeshTxAttrStruct==NULL){
		return;
	}
	
	if(len<3 || len>MAX_TX_BUF_LEN){
		return;
	}
	APP_DBG_PRINTF2("BleMesh_Vendor_Make_Packet opcode %02X  len%d\n", opcode, len);	
	#if 0
	for(int j=0; j<len ; j++){
		APP_DBG_PRINTF1("%02X ", buf[j]);
	}
	#endif
	if( opcode==BLEMESH_SKYWORTH_OPCODE_PROPERTY ){		
		for(i=0; i<MAX_TX_BUF_DEEP; i++){
			if( MeshTxAttrStruct[i].fullflag==1 ){
				
				if((buf[TX_ATTR_ID_POS]==MeshTxAttrStruct[i].buf[TX_ATTR_ID_POS]) 
					&& buf[TX_ATTR_ID_POS+1]==MeshTxAttrStruct[i].buf[TX_ATTR_ID_POS+1]) {
					// 同一属性上报，更新到一个报文中。不管之前报文是否重发，都按当前的needack来。
					index = i;
					APP_DBG_PRINTF3("find same attr index=%d ATTRID:%02X%02X\n", index,buf[TX_ATTR_ID_POS+1],buf[TX_ATTR_ID_POS]);
					break;
				}
			}
		}
	}
	
	if( index==0xFF ){
		// 缓存中没有同一属性报文	
		// 这种缓存方式，在数据量大的时候可能会导致后面index得不到发送，这里暂不存在这个问题。
		for(i=0; i<MAX_TX_BUF_DEEP; i++){
			if( MeshTxAttrStruct[i].fullflag == 0 ){
				index = i;
				break;
			}
		}
	}
	
	// APP_DBG_PRINTF2("BleMesh_Vendor_Make_Packet buff index:%d i:%d\n",i,index);
	if( index < MAX_TX_BUF_DEEP ){
		MeshTxAttrStruct[index].fullflag = 1;
		MeshTxAttrStruct[index].txtick   = 0;
		MeshTxAttrStruct[index].len      = len;		
		memset(MeshTxAttrStruct[i].buf, 0, MAX_TX_BUF_LEN);
		memcpy(MeshTxAttrStruct[index].buf, buf, len);
		if(needack==true){
			MeshTxAttrStruct[index].txcnt = MAX_RE_TX_CNT;
		}else{
			MeshTxAttrStruct[index].txcnt = 1;
		}
	}
}
// pull
#define SEND_MESH_PACKET_PERIOD  (800)   // ms
static void BleMesh_Vendor_Send_Packet(void)
{
	uint8_t  i=0;
	uint32_t sub_timeout_ms=0, tick=HAL_GetTickCount();	
	static uint32_t oldtick=0;
	
	if(MeshTxAttrStruct==NULL){
		return;
	}
	

	if(HAL_CalculateTickDiff(oldtick, tick) < SEND_MESH_PACKET_PERIOD){
		return;
	}else{
		oldtick = tick;
	}
	
	for(i=0; i<MAX_TX_BUF_DEEP; i++){
		if( MeshTxAttrStruct[i].fullflag==1 && MeshTxAttrStruct[i].txcnt>0 ){
			
			if(MeshTxAttrStruct[i].txtick==0){
				sub_timeout_ms=0;
			}else{
				sub_timeout_ms = HAL_CalculateTickDiff(MeshTxAttrStruct[i].txtick, tick);
			}
			
			if( MeshTxAttrStruct[i].txtick==0 || sub_timeout_ms>=maxacktimout){
				if(sub_timeout_ms>=maxacktimout){
					maxacktimout = MAX_ACK_TIMEOUT - 100*(((uint32_t)rand())%11);
				}
				APP_DBG_PRINTF4("BleMesh_Vendor_Send_Packet opcode %02X  len%d time%d %d\n", MeshTxAttrStruct[i].buf[TX_OPENCODE_POS], MeshTxAttrStruct[i].len,sub_timeout_ms,maxacktimout);	
				#if 1
				for(int j=0; j<MeshTxAttrStruct[i].len ; j++){
					APP_DBG_PRINTF1("%02X ", MeshTxAttrStruct[i].buf[j]);
				}
				#endif
				#if MESH_TEST_PRESSURE == 1
				if(MeshTxAttrStruct[i].buf[TX_OPENCODE_POS]==BLEMESH_SKYWORTH_OPCODE_PROPERTY ){
					sendpackcnt++;
				}
				#endif
				
				// MESH_MSG_SEND_CAUSE_SUCCESS = 0
				datatrans_publish(&VendorModel_Server, MeshTxAttrStruct[i].len,  MeshTxAttrStruct[i].buf);

				MeshTxAttrStruct[i].txcnt--;
				MeshTxAttrStruct[i].txtick = tick;
				if( MeshTxAttrStruct[i].txcnt==0 ){
					MeshTxAttrStruct[i].fullflag = 0;  // 最后一包不管应答
					MeshTxAttrStruct[i].txtick   = 0;
				}
				break;
			}
		}
	}
}


static void BleMesh_Vendor_Ack_Packet(uint8_t eventid, uint16_t attrID, uint32_t val)
{
	uint32_t value = 0;
	uint8_t i=0;
	uint16_t tmpid=0;
	
	// APP_DBG_PRINTF3("%s opcode %04X  val%d\n",__func__, attrID, val);	
	if( eventid==EVENT_TYPE_REPORT_PROPERTY_ACK ){		
		for(i=0; i<MAX_TX_BUF_DEEP; i++){
			if( MeshTxAttrStruct[i].fullflag==1 ){
				tmpid = MeshTxAttrStruct[i].buf[TX_ATTR_ID_POS+1] ;
				tmpid = (tmpid<<8) | MeshTxAttrStruct[i].buf[TX_ATTR_ID_POS];
				if( attrID == tmpid ){						
					switch(attrID){
						case ATTR_CLUSTER_ID_SWT:
						case ATTR_CLUSTER_ID_SW1:
						case ATTR_CLUSTER_ID_SW2:
						case ATTR_CLUSTER_ID_SW3:{
							value = MeshTxAttrStruct[i].buf[TX_ATTR_VAL_POS];
							break;
						}
						case ATTR_CLUSTER_ID_BRI:
						case ATTR_CLUSTER_ID_CTP:
						case ATTR_CLUSTER_ID_HUE:
						case ATTR_CLUSTER_ID_SAT:{
							value = MeshTxAttrStruct[i].buf[TX_ATTR_VAL_POS+3];
							value = (value<<8) | MeshTxAttrStruct[i].buf[TX_ATTR_VAL_POS+2];
							value = (value<<8) | MeshTxAttrStruct[i].buf[TX_ATTR_VAL_POS+1];
							value = (value<<8) | MeshTxAttrStruct[i].buf[TX_ATTR_VAL_POS];
							break;
						}					
					}
					if( value == val ){
						MeshTxAttrStruct[i].txcnt    = 0 ;
						MeshTxAttrStruct[i].fullflag = 0;  
						MeshTxAttrStruct[i].txtick   = 0;
					}
					break;
				}
			}
		}
	}
}

//1.SEND PING TO GATEWAY opcode(1B) + company id(2B) + 0x33(1B) 
static void SkyIotSendKeepAlivePacket(void)
{
    uint8_t payload[10];
	
	memset(payload, 0x0, 10);
    payload[0] = BLEMESH_SKYWORTH_OPCODE_HEARTBEAT;
    payload[1] = BLEMESH_VENDOR_COMPANY_ID & 0x00FF;
    payload[2] = (BLEMESH_VENDOR_COMPANY_ID & 0xFF00) >> 8;
    payload[3] = OPCODE_HEARTBEAT_CMD_PING;

	BleMesh_Vendor_Make_Packet(payload, 4, false);
}

//get product type reponse: opcode(1B) + company id(2B) + 0x2(1B) + product_type
static void SkyIotGetProductTypeAckPacket(void)
{
    uint8_t payload[10];
	uint32_t product_type = SkyBleMesh_GetProductType();

	memset(payload, 0x0, 10);
    payload[0] = BLEMESH_SKYWORTH_OPCODE_DEVINFO;
    payload[1] = BLEMESH_VENDOR_COMPANY_ID & 0x00FF;
    payload[2] = (BLEMESH_VENDOR_COMPANY_ID & 0xFF00) >> 8;
    payload[3] = OPCODE_DEVINFO_RSP_PRODUCT_TYPE;
	payload[4] = (product_type & 0x00FF);
	payload[5] = (product_type & 0xFF00) >> 8;
	payload[6] = (product_type & 0xFF0000) >> 16;
	payload[7] = (product_type & 0xFF000000) >> 24;
	
	BleMesh_Vendor_Make_Packet(payload, 8, false);
}

static void SkyIotGetProductBrandAckPacket(void)
{
	uint8_t payload[10];
	uint32_t product_brand = SkyBleMesh_GetProductBrand();	

	memset(payload, 0x0, 10);
	payload[0] = BLEMESH_SKYWORTH_OPCODE_DEVINFO;
	payload[1] = BLEMESH_VENDOR_COMPANY_ID & 0x00FF;
	payload[2] = (BLEMESH_VENDOR_COMPANY_ID & 0xFF00) >> 8;
	payload[3] = OPCODE_DEVINFO_RSP_PRODUCT_BRAND;
	payload[4] = (product_brand & 0x00FF);
	payload[5] = (product_brand & 0xFF00) >> 8;
	payload[6] = (product_brand & 0xFF0000) >> 16;
	payload[7] = (product_brand & 0xFF000000) >> 24;

	BleMesh_Vendor_Make_Packet(payload, 8, false);
}

static void SkyIotGetProductModelAckPacket(void)
{
	uint8_t payload[30];
	char* model = SkyBleMesh_GetProductModel();
	
	memset(payload, 0x0, 30);
	payload[0] = BLEMESH_SKYWORTH_OPCODE_DEVINFO;
	payload[1] = BLEMESH_VENDOR_COMPANY_ID & 0x00FF;
	payload[2] = (BLEMESH_VENDOR_COMPANY_ID & 0xFF00) >> 8;
	payload[3] = OPCODE_DEVINFO_RSP_PRODUCT_MODEL;
	memcpy(&payload[4], model, BT_PRODUCT_MODEL_LEN);
	
	BleMesh_Vendor_Make_Packet(payload, 4+BT_PRODUCT_MODEL_LEN, false);
}
#if 0
//opcode(1B) + company id(2B) + 0x19(1B) + seq(1B)
static void SkyIotUpdPropertyAckPacekt(uint8_t seqence_num)
{
    uint8_t payload[10];
	
	memset(payload, 0x0, 10);
    payload[0] = BLEMESH_SKYWORTH_OPCODE_PROPERTY;
    payload[1] = BLEMESH_VENDOR_COMPANY_ID & 0x00FF;
    payload[2] = (BLEMESH_VENDOR_COMPANY_ID & 0xFF00) >> 8;
    payload[3] = OPCODE_PROPERTY_CMD_UPDATTR_ACK;
	payload[4] = seqence_num;
	
	BleMesh_Vendor_Make_Packet(payload, 5, false);
}
#endif

static void SkyIotGetProductMacAckPacket(void)
{
	uint8_t payload[30];
	uint8_t* macaddr = SkyBleMesh_GetProductMac();
	
	memset(payload, 0x0, 30);
	payload[0] = BLEMESH_SKYWORTH_OPCODE_DEVINFO;
	payload[1] = BLEMESH_VENDOR_COMPANY_ID & 0x00FF;
	payload[2] = (BLEMESH_VENDOR_COMPANY_ID & 0xFF00) >> 8;
	payload[3] = OPCODE_DEVINFO_RSP_PRODUCT_MACADDR;
	
	payload[4] = macaddr[5];  // 应答给网关以小端模式
	payload[5] = macaddr[4];
	payload[6] = macaddr[3];
	payload[7] = macaddr[2];
	payload[8] = macaddr[1];
	payload[9] = macaddr[0];
	
	BleMesh_Vendor_Make_Packet(payload, 4+MAC_ADDRESS_LEN, false);
}


//opcode + company id(2B) + 0x23(1B) + seq(1B) + attrID(2B) + attr value (MAX 4B)
static void SkyIotReportPropertyPacket(uint16_t attrID, int value, uint8_t seq_num)
{
	uint8_t uAttrSize = 0;
	
	switch(attrID){
		case ATTR_CLUSTER_ID_SWT:
		case ATTR_CLUSTER_ID_SW1:
		case ATTR_CLUSTER_ID_SW2:
		case ATTR_CLUSTER_ID_SW3:{
			uAttrSize = 1;
			break;
		}
		case ATTR_CLUSTER_ID_BRI:
		case ATTR_CLUSTER_ID_CTP:
		case ATTR_CLUSTER_ID_HUE:
		case ATTR_CLUSTER_ID_SAT:{
			uAttrSize = 4;
			break;
		}	
		default:
			APP_DBG_PRINTF1("SkyIotReportPropertyPacket ERR attrID %04X\n", attrID);
			return;
	}
		
	memset(mIotManager.reppack_buffer, 0x0, MAX_BLEMESH_PACKET_LEN);	
	mIotManager.reppack_len = 7 + uAttrSize; 	
	mIotManager.reppack_buffer[0] = BLEMESH_SKYWORTH_OPCODE_PROPERTY;
    mIotManager.reppack_buffer[1] = BLEMESH_VENDOR_COMPANY_ID & 0x00FF;
    mIotManager.reppack_buffer[2] = (BLEMESH_VENDOR_COMPANY_ID & 0xFF00) >> 8;
    mIotManager.reppack_buffer[3] = OPCODE_PROPERTY_CMD_RPTATTR;
	mIotManager.reppack_buffer[4] = seq_num;
	mIotManager.reppack_buffer[5] = (attrID & 0x00FF);
	mIotManager.reppack_buffer[6] = (attrID & 0xFF00) >> 8;

	if (uAttrSize == 1){
		mIotManager.reppack_buffer[7] = value;
	}else if (uAttrSize == 4){
		mIotManager.reppack_buffer[7] = (value & 0xFF);
		mIotManager.reppack_buffer[8] = (value & 0xFF00) >> 8;
		mIotManager.reppack_buffer[9] = (value & 0xFF0000) >> 16;
		mIotManager.reppack_buffer[10] = (value & 0xFF000000) >> 24;
	}
	
	if(seq_num==0){	// 主动上报，需要重传机制
		BleMesh_Vendor_Make_Packet(mIotManager.reppack_buffer, mIotManager.reppack_len, true);
	}else{
		BleMesh_Vendor_Make_Packet(mIotManager.reppack_buffer, mIotManager.reppack_len, false);
	}
	#if MESH_TEST_PRESSURE == 1
	makepackcnt++;
	#endif
}



#endif


static void SkyBleMesh_handle_vendor_rx_cb(uint8_t opcode, uint8_t len, uint8_t *data)
{
	uint8_t *p_data = data;
	uint8_t cmd_type = 0;
	req_event_t event;
	
	memset(&event, 0x0, sizeof(req_event_t));
	
	cmd_type = p_data[MESH_COMMAND_POS];   // 获得命令码  ，
	
	APP_DBG_PRINTF2("SkyBleMesh_handle_vendor_rx_cb opcode %02X, type %02X\n", opcode, cmd_type);	
	
    switch (opcode)
    {
        case BLEMESH_SKYWORTH_OPCODE_HEARTBEAT:
        {
        	if(cmd_type == OPCODE_HEARTBEAT_CMD_PONG){
				mIotManager.alive_wakeup_cnt   = DEFAULT_WAKEUP_ALIVE_CNT;
				
				event.event_id = EVENT_TYPE_KEEPALIVE;
				Push_Msg_Into_FIFO(&event);				
			}
		break;
        }
		
        case BLEMESH_SKYWORTH_OPCODE_PROPERTY:
        {
        	if(cmd_type == OPCODE_PROPERTY_CMD_GETATTR){
								
				event.event_id    = EVENT_TYPE_GET_PROPERTY;
				event.seqence_num = p_data[MESH_SEQUENCE_POS];


				Push_Msg_Into_FIFO(&event);	
				
			}else if(cmd_type == OPCODE_PROPERTY_CMD_UPDATTR){
				
				int32_t  prop_value = 0;	
				uint16_t prop_ID = (p_data[MESH_ATTR_ID_POS+1]<<8) | (p_data[MESH_ATTR_ID_POS]);
			
				switch(prop_ID){
					case ATTR_CLUSTER_ID_SWT:
					case ATTR_CLUSTER_ID_SW1:
					case ATTR_CLUSTER_ID_SW2:
					case ATTR_CLUSTER_ID_SW3:{
						prop_value = p_data[MESH_ATTR_VAL_POS];
						break;
					}
					case ATTR_CLUSTER_ID_BRI:
					case ATTR_CLUSTER_ID_CTP:
					case ATTR_CLUSTER_ID_HUE:{
						prop_value = ((p_data[MESH_ATTR_VAL_POS+3]<<24) | (p_data[MESH_ATTR_VAL_POS+2]<<16) | 
								 (p_data[MESH_ATTR_VAL_POS+1]<<8) | (p_data[MESH_ATTR_VAL_POS]));
						break;
					}	
					case ATTR_CLUSTER_ID_SAT:{
						prop_value = ((p_data[MESH_ATTR_VAL_POS+3]<<24) | (p_data[MESH_ATTR_VAL_POS+2]<<16) | 
								 (p_data[MESH_ATTR_VAL_POS+1]<<8) | (p_data[MESH_ATTR_VAL_POS]));
						prop_value &= 0xFF;
						break;
					}	
					default:{
						APP_DBG_PRINTF1("SkyBleMesh_handle_vendor_rx_cb ERR proID %04X\n", prop_ID);
						return;
					}
						
				}
				// APP_DBG_PRINTF2("%s prop_value %04X\n",__func__, prop_value);				
				event.event_id = EVENT_TYPE_UPDATE_PROPERTY;
				event.seqence_num = p_data[MESH_SEQUENCE_POS];
				event.prop_ID     = prop_ID;
				event.prop_value  = prop_value;	
				Push_Msg_Into_FIFO(&event);
				
			}else if(cmd_type == OPCODE_PROPERTY_CMD_RPTATTR_ACK){		
				int32_t  prop_value = 0;				
				uint16_t prop_ID = (p_data[MESH_ATTR_ID_POS-1+1]<<8) | (p_data[MESH_ATTR_ID_POS-1]);
				
				switch(prop_ID){
					case ATTR_CLUSTER_ID_SWT:
					case ATTR_CLUSTER_ID_SW1:
					case ATTR_CLUSTER_ID_SW2:
					case ATTR_CLUSTER_ID_SW3:{
						prop_value = p_data[MESH_ATTR_VAL_POS-1];
						break;
					}
					case ATTR_CLUSTER_ID_BRI:
					case ATTR_CLUSTER_ID_CTP:
					case ATTR_CLUSTER_ID_HUE:
					case ATTR_CLUSTER_ID_SAT:{
					prop_value = ((p_data[MESH_ATTR_VAL_POS-1+3]<<24) | (p_data[MESH_ATTR_VAL_POS-1+2]<<16) | 
								 (p_data[MESH_ATTR_VAL_POS-1+1]<<8) | (p_data[MESH_ATTR_VAL_POS-1]));
						break;
					}
					default:{
						APP_DBG_PRINTF1("SkyBleMesh_handle_vendor_rx_cb ERR proID %04X\n", prop_ID);
						return;
					}
					
				}
				#if MESH_TEST_PRESSURE==1
				switch(prop_ID){
					case ATTR_CLUSTER_ID_SWT:{
						ackattrsval &= ~BLEMESH_REPORT_FLAG_SWT;
						break;
					}
					case ATTR_CLUSTER_ID_BRI:{
						ackattrsval &= ~BLEMESH_REPORT_FLAG_BRI;
						break;
					}
					case ATTR_CLUSTER_ID_CTP:{
						ackattrsval &= ~BLEMESH_REPORT_FLAG_CTP;
						break;
					}
					case ATTR_CLUSTER_ID_HUE:{
						ackattrsval &= ~BLEMESH_REPORT_FLAG_HUE;
						break;
					}
					case ATTR_CLUSTER_ID_SAT:{
						ackattrsval &= ~BLEMESH_REPORT_FLAG_SAT;
						break;
					}
					
				}
				revackpackcnt++;
				#endif 
				
				event.event_id = EVENT_TYPE_REPORT_PROPERTY_ACK;
				event.seqence_num = 0;
				event.prop_ID     = prop_ID;
				event.prop_value  = prop_value;	
				Push_Msg_Into_FIFO(&event);
			}
		break;
        }
		
        case BLEMESH_SKYWORTH_OPCODE_DEVINFO:
        {
        	if(cmd_type == OPCODE_DEVINFO_GET_PRODUCT_TYPE){				
				SkyIotGetProductTypeAckPacket();						
			}else if(cmd_type == OPCODE_DEVINFO_GET_PRODUCT_BRAND){				
				SkyIotGetProductBrandAckPacket();				
			}else if(cmd_type == OPCODE_DEVINFO_GET_PRODUCT_MODEL){	
				SkyIotGetProductModelAckPacket();
			}else if(cmd_type == OPCODE_DEVINFO_GET_PRODUCT_MACADDR){	
				SkyIotGetProductMacAckPacket();
			}
			
		break;
        }
      
        default:break;
        
    }

}





#if 1  // 联网配置

static void SkyBleMesh_Prov_Success_Timeout_cb(void *timer)
{
	req_event_t event;
	
	skybleprovsucesscnt++;
	if(skybleprovsucesscnt <= SKYBLEPROVSUCCESS_MAXCNT){
		#if USE_LIGHT_FOR_SKYIOT
		if( skybleprovsucesscnt&0x01 ){			
			HAL_Lighting_ON();
		}else{
			HAL_Lighting_OFF();
		}
		#endif

	}else{
		if(skybleprovsucesscnt == (SKYBLEPROVSUCCESS_MAXCNT+1)){			
			#if USE_LIGHT_FOR_SKYIOT
			mIotManager.mLightManager.status = 1;  // 不保存，默认开
			HAL_Lighting_ON();
			#endif

			// 闪灯结束后延时10s，触发配网成功事件。 
			plt_timer_change_period(skybleprosuccess_timer, SKYBLEPROVSUCCESS_TIMEOUT, 10000);
			
		}else{		
			event.event_id = EVENT_TYPE_PROVISIONED;
			Push_Msg_Into_FIFO(&event);	
	
			if(skybleprosuccess_timer){
				plt_timer_delete(skybleprosuccess_timer, 0);
				skybleprosuccess_timer = NULL;
			}
			skybleprovsucesscnt = 0;
		}
	}

}


/*
** Provisioning state values
*/

extern void SkyBleMesh_Provision_State(MESH_PROVISION_STATE_e sate)
{
	mesh_provison_state = sate;
	
	switch(mesh_provison_state){
		case MESH_PROVISION_STATE_START:{
			break;
		}
		case MESH_PROVISION_STATE_FAILED:{
			break;
		}
		case MESH_PROVISION_STATE_SUCCEED:{
			
			if(skybleprosuccess_timer == NULL){		
				skybleprosuccess_timer = plt_timer_create("prvok", SKYBLEPROVSUCCESS_TIMEOUT, true, 0, SkyBleMesh_Prov_Success_Timeout_cb);
				if (skybleprosuccess_timer != NULL){
					plt_timer_start(skybleprosuccess_timer, 1000);
				}
			}
			break;
		}
		case MESH_PROVISION_STATE_UNPROV:{
			break;
		}
		case MESH_PROVISION_STATE_PROVED:{
			break;
		}
		
		default:
			break;
	}
}
static bool SkyBleMesh_IsProvision_Sate(void)
{
	bool ret=false;
	
	if(mesh_provison_state==MESH_PROVISION_STATE_SUCCEED || mesh_provison_state==MESH_PROVISION_STATE_PROVED){
		ret = true;
	}

	return ret;
}

/*
** unprovision advertising timeout, deinit something.
*/
extern void SkyBleMesh_UnPro_Adv_timeout_cb(void)
{
	APP_DBG_PRINTF0("SkyBleMesh_UnPro_Adv_timeout_cb\n");	
}


// 进入重配网模式
extern void SkyBleMesh_unBind_complete(void)
{	
    APP_DBG_PRINTF0("**********************************************************************\n");
    APP_DBG_PRINTF0("******************* SkyMesh_unBind_complete **************************\n");    
    APP_DBG_PRINTF0("**********************************************************************\n");
	
	#if USE_LIGHT_FOR_SKYIOT
	SkyBleMesh_ReadConfig();
	mIotManager.mLightManager.ctp = DEFAULT_SUNLIGHT_CTP_VALUE;
	mIotManager.mLightManager.bri = DEFAULT_LIGHT_BRI_VALUE;
	HAL_Lighting_Sunlight( mIotManager.mLightManager.ctp, mIotManager.mLightManager.bri );
	SkyBleMesh_WriteConfig();
	#endif
	
	mesh_node_clear(); // 恢复重配网

}

/*
** Check Quick onoff
*/
extern void SkyBleMesh_PowerOn_Save(void)
{
    if(g_quick_onoff_Cnt != 0){
		
		g_quick_onoff_Cnt = 0;
		Hal_FlashWrite(FLASH_PARAM_TYPE_QUICK_ONOFF_CNT, sizeof(g_quick_onoff_Cnt), &g_quick_onoff_Cnt );
    }

}
extern void *skyonoff_sem_handle;
static void SkyBleMesh_PowerOn_Timeout_cb(void *timer)
{
    if(skyonoff_sem_handle){
		os_sem_give(skyonoff_sem_handle);
	}
	
	if(quick_onoff_timer){
		quick_onoff_timer = NULL;
	}
}

static void SkyBleMesh_Check_Quick_onoff_timer(void)
{
	if(quick_onoff_timer == NULL){		
		quick_onoff_timer = plt_timer_create("onff", DEVICE_POWER_ON_TIMEOUT, false, 0, SkyBleMesh_PowerOn_Timeout_cb);
		if (quick_onoff_timer != NULL){
			plt_timer_start(quick_onoff_timer, DEVICE_POWER_ON_TIMEOUT);
		}
	}
}

static bool SkyBleMesh_Check_Quick_onoff(void)
{
	bool ifreset=false, flashret=false;
	
	flashret = Hal_FlashRead(FLASH_PARAM_TYPE_QUICK_ONOFF_CNT, sizeof(g_quick_onoff_Cnt), &g_quick_onoff_Cnt );
    if(flashret == true) {
        g_quick_onoff_Cnt++;
        if(g_quick_onoff_Cnt < MAX_QUICK_ONOFF_CNT) {
            APP_DBG_PRINTF1("---quick_onoff_cnt %d\n", g_quick_onoff_Cnt);
			SkyBleMesh_Check_Quick_onoff_timer();
            Hal_FlashWrite(FLASH_PARAM_TYPE_QUICK_ONOFF_CNT, sizeof(g_quick_onoff_Cnt), &g_quick_onoff_Cnt );

		}  else { 
			APP_DBG_PRINTF1("+++quick_onoff_cnt %d,do factory reset\n", g_quick_onoff_Cnt);
			
			// when reset clear cnt
			g_quick_onoff_Cnt = 0;
			Hal_FlashWrite(FLASH_PARAM_TYPE_QUICK_ONOFF_CNT, sizeof(g_quick_onoff_Cnt), &g_quick_onoff_Cnt );

			ifreset = true;
        }
        
    } else {
		APP_DBG_PRINTF0("get quick_onoff_cnt error !!!\n");
		
        g_quick_onoff_Cnt = 1;
        Hal_FlashWrite(FLASH_PARAM_TYPE_QUICK_ONOFF_CNT, sizeof(g_quick_onoff_Cnt), &g_quick_onoff_Cnt );
        SkyBleMesh_Check_Quick_onoff_timer();
    }

	return ifreset;
}

static void SkyBleMesh_Reset_Timeout_cb(void *timer)
{
	if( ++g_skybleresetcnt > SKYBLERESET_MAXCNT ){		
		if(skyblereset_timer){
			plt_timer_delete(skyblereset_timer, 0);
			skyblereset_timer = NULL;
		}
		HAL_ResetBleDevice(); 
	} else {
		if( g_skybleresetcnt&0x01 ){
			#if USE_LIGHT_FOR_SKYIOT
			HAL_Lighting_ON();
			#endif
		}else{
			#if USE_LIGHT_FOR_SKYIOT
			HAL_Lighting_OFF();
			#endif
		}
	}
}
static void SkyBleMesh_Reset_timer(void)
{
	if(skyblereset_timer == NULL){		
		skyblereset_timer = plt_timer_create("rest", SKYBLERESET_TIMEOUT, true, 0, SkyBleMesh_Reset_Timeout_cb);
		if (skyblereset_timer != NULL){
			plt_timer_start(skyblereset_timer, 0);
		}
	}
}

extern void *skysave_sem_handle;
static void SkyIotSaveAttr_Timeout_cb(void *timer)
{
#if USE_LIGHT_FOR_SKYIOT
	#if ((SKY_LIGHT_TYPE==SKY_LIGHT_BELT_TYPE)||(SKY_LIGHT_TYPE==SKY_LIGHT_BULB_TYPE))
	if(save_oldbri!=mIotManager.mLightManager.bri || save_oldctp!=mIotManager.mLightManager.ctp ){
		save_oldbri = mIotManager.mLightManager.bri;
		save_oldctp = mIotManager.mLightManager.ctp;

		if(skysave_sem_handle){
			os_sem_give(skysave_sem_handle);
		}
	}
	
	#elif (SKY_LIGHT_TYPE==SKY_LIGHT_BULB_RGBWY_TYPE)
	if(save_oldbri!=mIotManager.mLightManager.bri || save_oldctp!=mIotManager.mLightManager.ctp 
		||save_oldhue!=mIotManager.mLightManager.hue || save_oldsat!=mIotManager.mLightManager.sat ){
		save_oldbri = mIotManager.mLightManager.bri;
		save_oldctp = mIotManager.mLightManager.ctp;
		save_oldhue = mIotManager.mLightManager.hue;
		save_oldsat = mIotManager.mLightManager.sat;

		if(skysave_sem_handle){
			os_sem_give(skysave_sem_handle);
		}
		data_uart_debug("SkyIotSaveAttr %d %d %d %d \r\n", save_oldbri,save_oldctp,save_oldhue,save_oldsat);
	}
	
	#endif
#endif
}
static void SkyIotSaveAttr_timer(void)
{
	if(skyiot_SaveAttr_timer == NULL){		
		skyiot_SaveAttr_timer = plt_timer_create("save", SKYIOTSAVEATTR_INTERVAL, true, 0, SkyIotSaveAttr_Timeout_cb);
		if (skyiot_SaveAttr_timer != NULL){
			plt_timer_start(skyiot_SaveAttr_timer, SKYIOTSAVEATTR_INTERVAL);
		}
	}
}


#endif




/*
** resotre/store param config
*/
extern int SkyBleMesh_WriteConfig(void)
{	
	uint8_t offset=0;
	bool flashret=false;
	uint8_t *buffer=NULL; // skyconfig_buffer;
	iot_md5_context *context=NULL;

	context = os_mem_zalloc(RAM_TYPE_DATA_ON, sizeof(iot_md5_context));
	if(context ==NULL){
		return -1;
	}

	buffer = os_mem_zalloc(RAM_TYPE_DATA_ON, FLASH_USERDATA_SAVE_LEN);
	if(buffer ==NULL){
		return -1;
	}
	memset(buffer , 0 , FLASH_USERDATA_SAVE_LEN);
		
	utils_md5_init(context);									   /* init context for 1st pass */
	utils_md5_starts(context); 								   /* setup context for 1st pass */

	memcpy(buffer + offset, mIotManager.mac_address, MAC_ADDRESS_LEN);
	offset += MAC_ADDRESS_LEN;
	utils_md5_update(context, (unsigned char*)mIotManager.mac_address, MAC_ADDRESS_LEN);
	
	memcpy(buffer + offset, mIotManager.device_uuid, BT_MESH_UUID_SIZE);
	offset += BT_MESH_UUID_SIZE;
	utils_md5_update(context, (unsigned char*)mIotManager.device_uuid, BT_MESH_UUID_SIZE);

#if USE_LIGHT_FOR_SKYIOT
	#if ((SKY_LIGHT_TYPE==SKY_LIGHT_BELT_TYPE)||(SKY_LIGHT_TYPE==SKY_LIGHT_BULB_TYPE))
	memcpy(buffer + offset, &mIotManager.mLightManager.bri, 4);
	offset += 4;
	utils_md5_update(context, (unsigned char*)&mIotManager.mLightManager.bri, 4);

	memcpy(buffer + offset, &mIotManager.mLightManager.ctp, 4);
	offset += 4;
	utils_md5_update(context, (unsigned char*)&mIotManager.mLightManager.ctp, 4);
	
	#elif (SKY_LIGHT_TYPE==SKY_LIGHT_BULB_RGBWY_TYPE)
	memcpy(buffer + offset, &mIotManager.mLightManager.bri, 4);
	offset += 4;
	utils_md5_update(context, (unsigned char*)&mIotManager.mLightManager.bri, 4);

	memcpy(buffer + offset, &mIotManager.mLightManager.ctp, 4);
	offset += 4;
	utils_md5_update(context, (unsigned char*)&mIotManager.mLightManager.ctp, 4);

	memcpy(buffer + offset, &mIotManager.mLightManager.hue, 4);
	offset += 4;
	utils_md5_update(context, (unsigned char*)&mIotManager.mLightManager.hue, 4);
	
	memcpy(buffer + offset, &mIotManager.mLightManager.sat, 4);
	offset += 4;
	utils_md5_update(context, (unsigned char*)&mIotManager.mLightManager.sat, 4);
	
	#endif

#endif
	
	utils_md5_finish(context, mIotManager.critical_md5);	
	os_mem_free(context);
	memcpy(buffer + offset, mIotManager.critical_md5, MD5_LEN);

	flashret = Hal_FlashWrite(FLASH_PARAM_TYPE_APP_CFGDATA, FLASH_USERDATA_SAVE_LEN, buffer );
	if(flashret==false){
		os_mem_free(buffer);
		APP_DBG_PRINTF0("SkyBleMesh_WriteConfig failed\n");
		return -1;
	}
	
	os_mem_free(buffer);
	return 0;
}

static int SkyBleMesh_ReadConfig(void)
{
	uint8_t offset=0;
	bool flashret=false;
	unsigned char md5[MD5_LEN];
	uint8_t *buffer=NULL; // skyconfig_buffer;
	iot_md5_context *context=NULL;

	context = os_mem_zalloc(RAM_TYPE_DATA_ON, sizeof(iot_md5_context));
	if(context ==NULL){
		return -1;
	}

	buffer = os_mem_zalloc(RAM_TYPE_DATA_ON, FLASH_USERDATA_SAVE_LEN);
	if(buffer ==NULL){
		return -1;
	}
	memset(buffer , 0 , FLASH_USERDATA_SAVE_LEN);
	
	utils_md5_init(context);									   // init context for 1st pass 
	utils_md5_starts(context); 								   // setup context for 1st pass 

	flashret = Hal_FlashRead(FLASH_PARAM_TYPE_APP_CFGDATA, FLASH_USERDATA_SAVE_LEN, buffer );
	if (flashret == false) {
		os_mem_free(buffer);
		APP_DBG_PRINTF0("SkyBleMesh_ReadConfig error \n");
		return -1;
	}
	
	//
	memcpy(mIotManager.mac_address, buffer + offset, MAC_ADDRESS_LEN);	
	offset += MAC_ADDRESS_LEN;
	utils_md5_update(context, (unsigned char*)mIotManager.mac_address, MAC_ADDRESS_LEN);
	//
	memcpy(mIotManager.device_uuid, buffer + offset, BT_MESH_UUID_SIZE);	
	offset += BT_MESH_UUID_SIZE;
	utils_md5_update(context, (unsigned char*)mIotManager.device_uuid, BT_MESH_UUID_SIZE);

	// ...	
		
#if USE_LIGHT_FOR_SKYIOT
	#if ((SKY_LIGHT_TYPE==SKY_LIGHT_BELT_TYPE)||(SKY_LIGHT_TYPE==SKY_LIGHT_BULB_TYPE))
	memcpy(&mIotManager.mLightManager.bri, buffer + offset, 4); 
	offset += 4;
	utils_md5_update(context, (unsigned char*)&mIotManager.mLightManager.bri, 4);	

	memcpy(&mIotManager.mLightManager.ctp, buffer + offset, 4); 
	offset += 4;
	utils_md5_update(context, (unsigned char*)&mIotManager.mLightManager.ctp, 4);	
		
	#elif (SKY_LIGHT_TYPE==SKY_LIGHT_BULB_RGBWY_TYPE)
	memcpy(&mIotManager.mLightManager.bri, buffer + offset, 4); 
	offset += 4;
	utils_md5_update(context, (unsigned char*)&mIotManager.mLightManager.bri, 4);	

	memcpy(&mIotManager.mLightManager.ctp, buffer + offset, 4); 
	offset += 4;
	utils_md5_update(context, (unsigned char*)&mIotManager.mLightManager.ctp, 4);	
	
	memcpy(&mIotManager.mLightManager.hue, buffer + offset, 4); 
	offset += 4;
	utils_md5_update(context, (unsigned char*)&mIotManager.mLightManager.hue, 4);	

	memcpy(&mIotManager.mLightManager.sat, buffer + offset, 4); 
	offset += 4;
	utils_md5_update(context, (unsigned char*)&mIotManager.mLightManager.sat, 4);	
		
	#endif
	
#endif

	utils_md5_finish(context, md5);  
	os_mem_free(context);
	//copy critical md5 to nvram
	memcpy(mIotManager.critical_md5, buffer + offset, MD5_LEN);			
	if (memcmp(mIotManager.critical_md5, md5, MD5_LEN) != 0) { 
		os_mem_free(buffer);
		APP_DBG_PRINTF0("skyiot_mesh_readConfig critical md5 check error \n");
		return -1;
	}
	
	os_mem_free(buffer);
	return 0;
}

/*
** 设备默认配置 并 控制
*/
static void SkyiotManager_Default_Config(void)
{
#if USE_LIGHT_FOR_SKYIOT
	#if ((SKY_LIGHT_TYPE==SKY_LIGHT_BELT_TYPE)||(SKY_LIGHT_TYPE==SKY_LIGHT_BULB_TYPE))
	mIotManager.mLightManager.status = 1;
	mIotManager.mLightManager.bri = DEFAULT_LIGHT_BRI_VALUE;
	mIotManager.mLightManager.ctp = DEFAULT_SUNLIGHT_CTP_VALUE;

	#elif (SKY_LIGHT_TYPE==SKY_LIGHT_BULB_RGBWY_TYPE)
	mIotManager.mLightManager.status = 1;
	mIotManager.mLightManager.bri = DEFAULT_LIGHT_BRI_VALUE;
	mIotManager.mLightManager.ctp = DEFAULT_SUNLIGHT_CTP_VALUE;
	mIotManager.mLightManager.hue = DEFAULT_LIGHT_HUE_VALUE;
	mIotManager.mLightManager.sat = DEFAULT_LIGHT_SAT_VALUE;

	#endif
#endif

}

/*
** main functions
*/
static void Reset_iotmanager_para(void)
{
	mIotManager.alive_status        = 1;//set device to online status
	mIotManager.recv_alive_tick     = HAL_GetTickCount();

#if USE_LIGHT_FOR_SKYIOT
	#if ((SKY_LIGHT_TYPE==SKY_LIGHT_BELT_TYPE)||(SKY_LIGHT_TYPE==SKY_LIGHT_BULB_TYPE))
	mIotManager.report_flag |= BLEMESH_REPORT_FLAG_SWT;
	mIotManager.report_flag |= BLEMESH_REPORT_FLAG_BRI;
	mIotManager.report_flag |= BLEMESH_REPORT_FLAG_CTP;
	mIotManager.status_seqnum = 0; 
	mIotManager.bri_seqnum = 0; 
	mIotManager.ctp_seqnum = 0; 	

	#elif (SKY_LIGHT_TYPE==SKY_LIGHT_BULB_RGBWY_TYPE)
	mIotManager.report_flag |= BLEMESH_REPORT_FLAG_SWT;
	mIotManager.report_flag |= BLEMESH_REPORT_FLAG_BRI;
	mIotManager.report_flag |= BLEMESH_REPORT_FLAG_CTP;
	mIotManager.report_flag |= BLEMESH_REPORT_FLAG_HUE;
	mIotManager.report_flag |= BLEMESH_REPORT_FLAG_SAT;
	mIotManager.status_seqnum = 0; 
	mIotManager.bri_seqnum = 0; 
	mIotManager.ctp_seqnum = 0; 
	mIotManager.hue_seqnum = 0; 
	mIotManager.sat_seqnum = 0; 	

	#endif
#endif

}
static void Main_Event_Handle(void)
{
	static uint8_t delayreport=0; // 恢复心跳后，延时上报。延时期间可能有seq不为0，故只置上报标志
	req_event_t event;
	
	if(delayreport){
		if(--delayreport == 0){			
		#if USE_LIGHT_FOR_SKYIOT
			#if ((SKY_LIGHT_TYPE==SKY_LIGHT_BELT_TYPE)||(SKY_LIGHT_TYPE==SKY_LIGHT_BULB_TYPE))
				mIotManager.report_flag |= BLEMESH_REPORT_FLAG_SWT;
				mIotManager.report_flag |= BLEMESH_REPORT_FLAG_BRI;
				mIotManager.report_flag |= BLEMESH_REPORT_FLAG_CTP;			
			#elif (SKY_LIGHT_TYPE==SKY_LIGHT_BULB_RGBWY_TYPE)
				mIotManager.report_flag |= BLEMESH_REPORT_FLAG_SWT;
				mIotManager.report_flag |= BLEMESH_REPORT_FLAG_BRI;
				mIotManager.report_flag |= BLEMESH_REPORT_FLAG_CTP;
				mIotManager.report_flag |= BLEMESH_REPORT_FLAG_HUE;
				mIotManager.report_flag |= BLEMESH_REPORT_FLAG_SAT;			
			#endif
		#endif
		}
	}
			
	if (Pop_Msg_From_FIFO(&event)){
		APP_DBG_PRINTF4("Main_Event_Handle %X %d %X %04X \n",
			event.event_id , event.seqence_num , event.prop_value, event.prop_ID );

		switch(event.event_id)
		{
			case EVENT_TYPE_PROVISIONED:{				
				Reset_iotmanager_para();
				mIotManager.report_flag = 0; // 配网成功后不主动上报，等待网关请求。
			break;
			}
			case EVENT_TYPE_KEEPALIVE:{
				if (mIotManager.alive_status == 0){
					Reset_iotmanager_para();
					
					// used in delay report
					mIotManager.report_flag = 0;
					delayreport = (uint8_t)(rand()%80) + 1; // 50~4000ms
					
					APP_DBG_PRINTF0("Main_Event_Handle device online!\n");
				}else {
					mIotManager.recv_alive_tick = HAL_GetTickCount();	
					APP_DBG_PRINTF1("Main_Event_Handle recv keepalive tick: %d!\n", mIotManager.recv_alive_tick);
				}			
			break;
			}
			case EVENT_TYPE_UPDATE_PROPERTY:{
				if(mIotManager.alive_status == 0){
					Reset_iotmanager_para();
				}
				if (mIotManager.alive_status == 1){
						// 根据参数，对设备控制  
					#if USE_LIGHT_FOR_SKYIOT
						#if ((SKY_LIGHT_TYPE==SKY_LIGHT_BELT_TYPE)||(SKY_LIGHT_TYPE==SKY_LIGHT_BULB_TYPE))
						if ( event.prop_ID == ATTR_CLUSTER_ID_SWT ){
							mIotManager.mLightManager.status = event.prop_value;							
							mIotManager.report_flag |= BLEMESH_REPORT_FLAG_SWT;
							mIotManager.status_seqnum = event.seqence_num;
						}else if ( event.prop_ID == ATTR_CLUSTER_ID_BRI){
							mIotManager.mLightManager.bri = event.prop_value;
							mIotManager.report_flag |= BLEMESH_REPORT_FLAG_BRI;
							mIotManager.bri_seqnum = event.seqence_num;
						}else if ( event.prop_ID == ATTR_CLUSTER_ID_CTP){
							mIotManager.mLightManager.ctp = event.prop_value;
							mIotManager.report_flag |= BLEMESH_REPORT_FLAG_CTP;	
							mIotManager.ctp_seqnum = event.seqence_num;
						}
						#elif (SKY_LIGHT_TYPE==SKY_LIGHT_BULB_RGBWY_TYPE)
						if ( event.prop_ID == ATTR_CLUSTER_ID_SWT){
							mIotManager.mLightManager.status = event.prop_value;							
							mIotManager.report_flag |= BLEMESH_REPORT_FLAG_SWT;
							mIotManager.status_seqnum = event.seqence_num;
						}else if ( event.prop_ID == ATTR_CLUSTER_ID_BRI){
							mIotManager.mLightManager.bri = event.prop_value;
							mIotManager.report_flag |= BLEMESH_REPORT_FLAG_BRI;
							mIotManager.bri_seqnum = event.seqence_num;
						}else if ( event.prop_ID == ATTR_CLUSTER_ID_CTP){
							mIotManager.mLightManager.ctp = event.prop_value;
							mIotManager.report_flag |= BLEMESH_REPORT_FLAG_CTP;	
							mIotManager.ctp_seqnum = event.seqence_num;
						}else if ( event.prop_ID == ATTR_CLUSTER_ID_HUE){							
							mIotManager.mLightManager.ctp = 0;
							mIotManager.report_flag |= BLEMESH_REPORT_FLAG_CTP;	
							mIotManager.mLightManager.hue = event.prop_value;
							mIotManager.report_flag |= BLEMESH_REPORT_FLAG_HUE;
							mIotManager.hue_seqnum = event.seqence_num;
						}else if ( event.prop_ID == ATTR_CLUSTER_ID_SAT){				
							mIotManager.mLightManager.ctp = 0;
							mIotManager.report_flag |= BLEMESH_REPORT_FLAG_CTP;	
							mIotManager.mLightManager.sat = event.prop_value;
							mIotManager.report_flag |= BLEMESH_REPORT_FLAG_SAT;	
							mIotManager.sat_seqnum = event.seqence_num;
						}
						#endif
						
						if (mIotManager.mLightManager.status == 0){
							HAL_Lighting_OFF();
							
						}else {
							// APP_DBG_PRINTF2("ctp: %d, bri: %d \n", mIotManager.mLightManager.ctp, mIotManager.mLightManager.bri);
														
						#if USE_LIGHT_FOR_SKYIOT
							#if ((SKY_LIGHT_TYPE==SKY_LIGHT_BELT_TYPE)||(SKY_LIGHT_TYPE==SKY_LIGHT_BULB_TYPE))
							HAL_Lighting_Sunlight(mIotManager.mLightManager.ctp, mIotManager.mLightManager.bri);

							#elif (SKY_LIGHT_TYPE==SKY_LIGHT_BULB_RGBWY_TYPE)
							if(mIotManager.mLightManager.ctp){
								HAL_Lighting_Sunlight(mIotManager.mLightManager.ctp, mIotManager.mLightManager.bri);
							}else{
								HAL_Lighting_Color( mIotManager.mLightManager.hue, mIotManager.mLightManager.sat, mIotManager.mLightManager.bri );	
							}

							#endif
						#endif
						}
					#endif

						
						// SkyBleMesh_WriteConfig();
				}	
			break;
			}
			case EVENT_TYPE_GET_PROPERTY:{				
				if(mIotManager.alive_status == 0){
					Reset_iotmanager_para();
				}
				if(mIotManager.alive_status == 1){			
				#if USE_LIGHT_FOR_SKYIOT
					#if ((SKY_LIGHT_TYPE==SKY_LIGHT_BELT_TYPE)||(SKY_LIGHT_TYPE==SKY_LIGHT_BULB_TYPE))
						mIotManager.report_flag |= BLEMESH_REPORT_FLAG_SWT;
						mIotManager.report_flag |= BLEMESH_REPORT_FLAG_BRI;
						mIotManager.report_flag |= BLEMESH_REPORT_FLAG_CTP;			
					#elif (SKY_LIGHT_TYPE==SKY_LIGHT_BULB_RGBWY_TYPE)
						mIotManager.report_flag |= BLEMESH_REPORT_FLAG_SWT;
						mIotManager.report_flag |= BLEMESH_REPORT_FLAG_BRI;
						mIotManager.report_flag |= BLEMESH_REPORT_FLAG_CTP;
						mIotManager.report_flag |= BLEMESH_REPORT_FLAG_HUE;
						mIotManager.report_flag |= BLEMESH_REPORT_FLAG_SAT;			
					#endif
				#endif
				}
					
			break;
			}
			case EVENT_TYPE_REPORT_PROPERTY_ACK:{
				BleMesh_Vendor_Ack_Packet(event.event_id, event.prop_ID, event.prop_value);
			break;
			}
		}	
		
	}
	
}

static void Main_WithoutNet_Handle(void)
{	
	
}

static bool Main_Check_Online(void)
{	
	if (++g_aliveTimerCnt >= 30){  // 30*50ms
		uint32_t tick = HAL_GetTickCount();	
		if (mIotManager.alive_wakeup_cnt < DEFAULT_WAKEUP_ALIVE_CNT){
			if (mIotManager.alive_status == 0){
				APP_DBG_PRINTF1("Main_Check_Online wakeupcnt %d\n", mIotManager.alive_wakeup_cnt);
				SkyIotSendKeepAlivePacket();
				mIotManager.send_alive_tick = tick;
			}else{
				int sub_timeout_ms;
				sub_timeout_ms = HAL_CalculateTickDiff(mIotManager.recv_alive_tick, tick);
				// APP_DBG_PRINTF2("Main_Check_Online wakeupcnt %d %d\n", mIotManager.alive_wakeup_cnt, sub_timeout_ms);
				if (sub_timeout_ms >= DEFAULT_SKYIOT_ALIVE_MS){
					SkyIotSendKeepAlivePacket();
					mIotManager.send_alive_tick = tick;
				}
			}
			mIotManager.alive_wakeup_cnt++;
		}else {
			int sub_timeout_ms;
			sub_timeout_ms = HAL_CalculateTickDiff(mIotManager.send_alive_tick, tick);
			if (sub_timeout_ms >= DEFAULT_SKYIOT_ALIVE_MS){
				APP_DBG_PRINTF1("Main_Check_Online sub_timeout_ms %d\n", sub_timeout_ms);
				//SEND ALIVE PACKET
				SkyIotSendKeepAlivePacket();
				mIotManager.send_alive_tick = tick;
				
				mIotManager.alive_wakeup_cnt   = 1;	// 默认都会去发3次，有心跳应答会把计数清掉 			
			}
		}

		//判断路由器是否在线
		if (mIotManager.alive_status == 1){
			int sub_timeout_ms;
			sub_timeout_ms = HAL_CalculateTickDiff(mIotManager.recv_alive_tick, tick);
			if (sub_timeout_ms >= DEFAULT_GATEWAY_ALIVE_MS){
				mIotManager.alive_status        = 0;		
				mIotManager.report_flag         = 0;
				mIotManager.send_alive_tick     = 0;
				mIotManager.recv_alive_tick     = 0;
				memset(mIotManager.reppack_buffer, 0x0, MAX_BLEMESH_PACKET_LEN);
				mIotManager.reppack_len = 0;
				
				#if USE_LIGHT_FOR_SKYIOT
					#if ((SKY_LIGHT_TYPE==SKY_LIGHT_BELT_TYPE)||(SKY_LIGHT_TYPE==SKY_LIGHT_BULB_TYPE))
					mIotManager.status_seqnum = 0; 
					mIotManager.bri_seqnum = 0; 
					mIotManager.ctp_seqnum = 0; 	
					#elif (SKY_LIGHT_TYPE==SKY_LIGHT_BULB_RGBWY_TYPE)
					mIotManager.status_seqnum = 0; 
					mIotManager.bri_seqnum = 0; 
					mIotManager.ctp_seqnum = 0; 
					mIotManager.hue_seqnum = 0; 
					mIotManager.sat_seqnum = 0; 	
					#endif
				#endif
			}
		}
		g_aliveTimerCnt = 0;
		
	//	return false;   
	}

	return true;   
}

static void Main_Upload_State(void)
{
	if (mIotManager.alive_status == 1) {	
		
	#if USE_LIGHT_FOR_SKYIOT
		#if ((SKY_LIGHT_TYPE==SKY_LIGHT_BELT_TYPE)||(SKY_LIGHT_TYPE==SKY_LIGHT_BULB_TYPE))
		if (mIotManager.report_flag & BLEMESH_REPORT_FLAG_SWT){
			mIotManager.report_flag &= ~BLEMESH_REPORT_FLAG_SWT;
			SkyIotReportPropertyPacket(ATTR_CLUSTER_ID_SWT, mIotManager.mLightManager.status, mIotManager.status_seqnum);
			mIotManager.status_seqnum = 0;

		}else if (mIotManager.report_flag & BLEMESH_REPORT_FLAG_BRI){	
			mIotManager.report_flag &= ~BLEMESH_REPORT_FLAG_BRI;
			SkyIotReportPropertyPacket(ATTR_CLUSTER_ID_BRI, mIotManager.mLightManager.bri, mIotManager.bri_seqnum);
			mIotManager.bri_seqnum = 0;
			
		}else if (mIotManager.report_flag & BLEMESH_REPORT_FLAG_CTP){
			mIotManager.report_flag &= ~BLEMESH_REPORT_FLAG_CTP;			
			SkyIotReportPropertyPacket(ATTR_CLUSTER_ID_CTP, mIotManager.mLightManager.ctp, mIotManager.ctp_seqnum);
			mIotManager.ctp_seqnum = 0;
			
		}
		
		#elif (SKY_LIGHT_TYPE==SKY_LIGHT_BULB_RGBWY_TYPE)
		if (mIotManager.report_flag & BLEMESH_REPORT_FLAG_SWT){
			mIotManager.report_flag &= ~BLEMESH_REPORT_FLAG_SWT;
			SkyIotReportPropertyPacket(ATTR_CLUSTER_ID_SWT, mIotManager.mLightManager.status, mIotManager.status_seqnum);
			mIotManager.status_seqnum = 0;

		}else if (mIotManager.report_flag & BLEMESH_REPORT_FLAG_BRI){	
			mIotManager.report_flag &= ~BLEMESH_REPORT_FLAG_BRI;
			SkyIotReportPropertyPacket(ATTR_CLUSTER_ID_BRI, mIotManager.mLightManager.bri, mIotManager.bri_seqnum);
			mIotManager.bri_seqnum = 0;
			
		}else if (mIotManager.report_flag & BLEMESH_REPORT_FLAG_CTP){
			mIotManager.report_flag &= ~BLEMESH_REPORT_FLAG_CTP;			
			SkyIotReportPropertyPacket(ATTR_CLUSTER_ID_CTP, mIotManager.mLightManager.ctp, mIotManager.ctp_seqnum);
			mIotManager.ctp_seqnum = 0;
			
		}else if (mIotManager.report_flag & BLEMESH_REPORT_FLAG_HUE){		
			mIotManager.report_flag &= ~BLEMESH_REPORT_FLAG_HUE;
			SkyIotReportPropertyPacket(ATTR_CLUSTER_ID_HUE, mIotManager.mLightManager.hue, mIotManager.hue_seqnum);
			mIotManager.hue_seqnum = 0;
						
		}else if (mIotManager.report_flag & BLEMESH_REPORT_FLAG_SAT){
			mIotManager.report_flag &= ~BLEMESH_REPORT_FLAG_SAT;			
			SkyIotReportPropertyPacket(ATTR_CLUSTER_ID_SAT, mIotManager.mLightManager.sat, mIotManager.sat_seqnum);
			mIotManager.sat_seqnum = 0;
			
		}
		
		#endif
	#endif

	}
	
}



// #define MY_TEST_TIMER
#ifdef MY_TEST_TIMER
static plt_timer_t skymesh_test_timer = NULL;
static void SkyBleMesh_Test_Timeout_cb(void *timer)
{	
	static uint8_t testcnt=0,reconflag=0;
//	 APP_DBG_PRINTF2("%s prostate %d \n",__func__, SkyBleMesh_IsProvision_Sate() );

//uint32_t time = os_sys_time_get();
//uint32_t tick = os_sys_tick_get();
//	APP_PRINT_TRACE2("SkyBleMesh_Test_Timeout_cb1 = %ld %ld\n", time, tick);
//	APP_PRINT_INFO2("SkyBleMesh_Test_Timeout_cb2 = %ld %ld\n", time, tick);
//	APP_PRINT_WARN2("SkyBleMesh_Test_Timeout_cb3 = %ld %ld\n", time, tick);
//	APP_PRINT_ERROR2("SkyBleMesh_Test_Timeout_cb4 = %ld %ld\n", time, tick);

	if(testperid){
		if (mIotManager.alive_status == 1){
			if(++testcnt >= testperid){
				if(reconflag==0){
					test_update_attr();
				}else{
					reconflag = 0;
				}
				testcnt = 0;
			}			
		}else{
			testcnt = 0;
			reconflag = 1;
		}
	}else{
		testcnt = 0;
	}
}
static void SkyBleMesh_Test_timer(void)
{	
	if(skymesh_test_timer == NULL){		
		skymesh_test_timer = plt_timer_create("test", 1000, true, 0, SkyBleMesh_Test_Timeout_cb);
		if (skymesh_test_timer != NULL){
			plt_timer_start(skymesh_test_timer, 0);
		}
	}
}
#endif

#if MESH_TEST_PRESSURE == 1
extern void test_update_attr(void)
{
	#if USE_LIGHT_FOR_SKYIOT
		#if ((SKY_LIGHT_TYPE==SKY_LIGHT_BELT_TYPE)||(SKY_LIGHT_TYPE==SKY_LIGHT_BULB_TYPE))
		mIotManager.report_flag |= BLEMESH_REPORT_FLAG_SWT;
		mIotManager.report_flag |= BLEMESH_REPORT_FLAG_BRI;
		mIotManager.report_flag |= BLEMESH_REPORT_FLAG_CTP;

		#elif (SKY_LIGHT_TYPE==SKY_LIGHT_BULB_RGBWY_TYPE)
		mIotManager.report_flag |= BLEMESH_REPORT_FLAG_SWT;
		mIotManager.report_flag |= BLEMESH_REPORT_FLAG_BRI;
		mIotManager.report_flag |= BLEMESH_REPORT_FLAG_CTP;
		mIotManager.report_flag |= BLEMESH_REPORT_FLAG_HUE;
		mIotManager.report_flag |= BLEMESH_REPORT_FLAG_SAT;

		#endif
	#endif

	if(ackattrsval){
		acktimoutcnt++;
	}
	ackattrsval = mIotManager.report_flag;
		
}
#endif

extern void SkyBleMesh_MainLoop(void)
{	

					
	if(IsSkyAppInited==false){
		return;
	}
	
	bool isprov = SkyBleMesh_IsProvision_Sate();

	if(isprov){
		
		//recv message	
		Main_Event_Handle();
		
		//判断设备是否离线
		if(Main_Check_Online() == false){
			return;
		}

		//状态上报 
		 Main_Upload_State();
		
		// 轮询发送缓存
		BleMesh_Vendor_Send_Packet();
		
	}else{
		// 没有配网也要执行控制，不给上报标志
		Main_WithoutNet_Handle();

	}
	
}

extern void *skymain_sem_handle;   //!< skyiot main sem handle
static void SkyBleMesh_MainLoop_Timeout_cb(void *time)
{		
	#if USE_SOFT_WATCHDOG
	SoftWdtFed(SKYMESH_THREAD_SWDT_ID); 
	#endif
	
	if(skymain_sem_handle){
		os_sem_give(skymain_sem_handle);
	}
}

extern void SkyBleMesh_MainLoop_timer(void)
{	
	if(skyblemainloop_timer == NULL){		
		skyblemainloop_timer = plt_timer_create("main", 50, true, 0, SkyBleMesh_MainLoop_Timeout_cb);
		if (skyblemainloop_timer != NULL){
			plt_timer_start(skyblemainloop_timer, 0);
						
			#if USE_SOFT_WATCHDOG
			SoftWdtInit(SKYMESH_THREAD_SWDT_ID, 1000);    // config softwdt as 1000ms 
			#endif
		}
	}
}

extern uint8_t SkyBleMesh_App_Init(void)
{
    uint32_t retcfg;
	bool retgetmac=false;
	uint8_t macaddr[MAC_ADDRESS_LEN]={0,0,0,0,0,0};
    
    APP_DBG_PRINTF0("SkyBleMesh_App_Init\n");	
		
	SkyBleMesh_SetProductType(PRODUCT_TYPE);
	SkyBleMesh_SetProductBrand(PRODUCT_BRAND);
	SkyBleMesh_SetProductModel(PRODUCT_MODEL);
	
#if USE_LIGHT_FOR_SKYIOT
	HAL_Lighting_Init(&mIotManager.mLightManager);
#endif
	if( SkyBleMesh_Check_Quick_onoff() ){   
				
		SkyBleMesh_unBind_complete();      // 进入重配网模式

		g_skybleresetcnt = 0; 
		SkyBleMesh_Reset_timer();
		
		return 1; // do factory reset  
	}	
		
    retcfg = SkyBleMesh_ReadConfig();		  
 	if( Hal_Get_Ble_MacAddr(macaddr) ){
		if( memcmp(macaddr, mIotManager.mac_address, MAC_ADDRESS_LEN) == 0){
			retgetmac = true;
		} else { 
			memcpy(mIotManager.mac_address, macaddr, MAC_ADDRESS_LEN); 
		}
	}
    if(retcfg!=0 || retgetmac==false){ 		
        APP_DBG_PRINTF2("SkyBleMesh_ReadConfig read failed %d %d\n",retcfg, retgetmac);
		
		// 本地烧录不会擦除flash。所以通过MACaddr判断擦除flash。并清配网
		mesh_node_clear(); // 恢复重配网
		
		/*创维设备mesh UUID:     MAC地址(6Byte) + PRODUCT_TYPE(4Byte) + 随机数(6Byte) */
		Regain_Random_UUID(mIotManager.device_uuid, MESH_DEV_UUID_LEN);	
		memcpy(mIotManager.device_uuid, mIotManager.mac_address, MAC_ADDRESS_LEN);	
		mIotManager.device_uuid[MAC_ADDRESS_LEN]   = PRODUCT_TYPE & 0xFF;
		mIotManager.device_uuid[MAC_ADDRESS_LEN+1] = (PRODUCT_TYPE>>8) & 0xFF;
		mIotManager.device_uuid[MAC_ADDRESS_LEN+2] = (PRODUCT_TYPE>>16) & 0xFF;
		mIotManager.device_uuid[MAC_ADDRESS_LEN+3] = (PRODUCT_TYPE>>24) & 0xFF;		
		#if MESH_TEST_PRESSURE == 1
		mIotManager.device_uuid[10]   = PRODUCT_TYPE & 0xFF;	// used in test
		mIotManager.device_uuid[11]   = PRODUCT_TYPE & 0xFF;	
		mIotManager.device_uuid[12]   = PRODUCT_TYPE & 0xFF;	
		mIotManager.device_uuid[13]   = PRODUCT_TYPE & 0xFF;	
		mIotManager.device_uuid[14]   = PRODUCT_TYPE & 0xFF;	
		mIotManager.device_uuid[15]   = PRODUCT_TYPE & 0xFF;
		#endif			
			
		SkyiotManager_Default_Config();  // 设备默认配置 并 控制
		SkyBleMesh_WriteConfig();        // 保存参数
				
    } else {
        APP_DBG_PRINTF0("SkyBleMesh_ReadConfig read succ\n");
		// 按保存的参数，恢复对设备的控制
    }	
	
	
#if USE_LIGHT_FOR_SKYIOT
	mIotManager.mLightManager.status = 1;  // 不保存，默认开
	HAL_Lighting_ON();
#endif
	
    for(int i =0 ;i < 16;i++){
        APP_DBG_PRINTF2("mIotManager.device_uuid[%d] = 0x%02x\n",i,mIotManager.device_uuid[i]);
    }
	
	mIotManager.alive_wakeup_cnt    = 0;
	mIotManager.alive_status        = 0;		
	mIotManager.report_flag         = 0;
	mIotManager.send_alive_tick     = 0;
	mIotManager.recv_alive_tick     = 0;
	memset(mIotManager.reppack_buffer, 0x0, MAX_BLEMESH_PACKET_LEN);
	mIotManager.reppack_len = 0;
	
	#if USE_LIGHT_FOR_SKYIOT
		#if ((SKY_LIGHT_TYPE==SKY_LIGHT_BELT_TYPE)||(SKY_LIGHT_TYPE==SKY_LIGHT_BULB_TYPE))
		mIotManager.status_seqnum = 0; 
		mIotManager.bri_seqnum = 0; 
		mIotManager.ctp_seqnum = 0; 	
		#elif (SKY_LIGHT_TYPE==SKY_LIGHT_BULB_RGBWY_TYPE)
		mIotManager.status_seqnum = 0; 
		mIotManager.bri_seqnum = 0; 
		mIotManager.ctp_seqnum = 0; 
		mIotManager.hue_seqnum = 0; 
		mIotManager.sat_seqnum = 0; 	
		#endif
	#endif
	BleMesh_Packet_Init();
	
	IsSkyAppInited = true;
	 
	#if USE_LIGHT_FOR_SKYIOT
	save_oldbri = mIotManager.mLightManager.bri;
	save_oldctp = mIotManager.mLightManager.ctp;
	#endif
	
	SkyIotSaveAttr_timer();
	// SkyBleMesh_MainLoop_timer();
	
#ifdef MY_TEST_TIMER
	SkyBleMesh_Test_timer();
#endif
	
	return 0; 
}


/*
** Creat vendors model
*/
extern void SkyBleMesh_Vendormodel_init(uint8_t elmt_idx)
{
	// qlj 考虑从 datatrans_server_model_init 移植过来
	Vendor_Model_Init(SkyBleMesh_handle_vendor_rx_cb);
}



