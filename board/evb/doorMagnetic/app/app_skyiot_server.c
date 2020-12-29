#include <string.h>
#include <stdlib.h>

#include "platform_utils.h"
#include "datatrans_server_app.h"
#include "datatrans_model.h"
#include "utils_md5.h"
#include "app_task.h"
#include "app_skyiot_server.h"
#include "app_skyiot_dlps.h"
#include "soft_wdt.h"
#include "skyadc.h"
#define APP_DBG_PRINTF0  DBG_DIRECT // APP_PRINT_WARN0
#define APP_DBG_PRINTF1  DBG_DIRECT // APP_PRINT_WARN1
#define APP_DBG_PRINTF2  DBG_DIRECT // APP_PRINT_WARN2
#define APP_DBG_PRINTF3  DBG_DIRECT // APP_PRINT_WARN3
#define APP_DBG_PRINTF4  DBG_DIRECT // APP_PRINT_WARN4
#define APP_DBG_PRINTF5  DBG_DIRECT // APP_PRINT_WARN5

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

#define SKYBLERESET_MAXCNT          (45)    // 重配网延时服务，延时的时间用作信号灯的闪烁
#define SKYBLERESET_MINCNT          (10)    // 配网过程中按键退出配网
#define SKYBLERESET_TIMEOUT         (150*45)   // 对开关，仅仅延时而已，给闪灯时间

#define SKYBLEPROVSUCCESS_MAXCNT    (16)    // 配网成功闪
#define SKYBLEPROVSUCCESS_TIMEOUT   (500)



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

#define DEFAULT_SKYIOT_ALIVE_MS   (60000*5)  // qlj 需要一个定时器来换算
#define DEFAULT_GATEWAY_ALIVE_MS  (60000*6)
#define DEFAULT_GATEWAY_SENQ_MS   (5000)


#define MAX_BLEMESH_PACKET_LEN    (32)	// BLEMESH 数据包最大长度 


#define TX_OPENCODE_POS     0
#define TX_COMMAND_POS      3
#define TX_ATTR_ID_POS      5
#define TX_ATTR_VAL_POS     7

#define MAX_TX_BUF_LEN 			(MAX_BLEMESH_PACKET_LEN)    // 每个发送缓存最大32字节
#define MAX_RE_TX_CNT 			(2) // (3)     // 最大重发次数          // 待网关作应答了再改到重发3次
#define MAX_ACK_TIMEOUT 		(4000)  // 最大应答超时ms
#define MAX_TX_BUF_DEEP 		(20)    // 最大10个发送缓存
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
static req_event_t  checksameeven={0,0,0,0};

typedef struct 
{	
	uint8_t  head;
	uint8_t  tail;
	req_event_t  even_t[MAIN_MSG_FIFO_DEEP];
}main_msg_fifo_t;


typedef struct {	
	uint8_t  process_state; 					/* 设备工作状态: FF-正常; 55-配网状态 */ 	
	uint8_t  mac_address[MAC_ADDRESS_LEN];
	char     product_model[BT_PRODUCT_MODEL_LEN];
	uint32_t product_type;
	uint32_t product_brand; 					/* 设备品牌*/
	uint8_t  device_uuid[BT_MESH_UUID_SIZE]; 	/* 设备UUID */
	uint8_t  critical_md5[MD5_LEN]; 			/* md5 for network_mode, mode,bri,ctp hsv */
		
	//协议控制字段	
	uint8_t  alive_status;	  					/* 0: online, 1: offline */
	uint8_t  alive_wakeup_cnt;  				/*上电或者睡眠唤醒后发三次心跳包*/
	uint32_t recv_alive_tick;					/* 上次接受到网关心跳的时间 */
	uint32_t send_alive_tick; 					/* 发送心跳得时间 */
	
	uint8_t  reppack_buffer[MAX_BLEMESH_PACKET_LEN];
	uint8_t  reppack_len;
	
	//设备字段
	#if USE_DOOR_FOR_SKYIOT
		uint8_t sw1_seq;
		uint8_t saf_seq;
		uint8_t bat_seq;
		SkySwitchManager mSwitchManager;
		SkyLightManager  mLightManager;

		uint8_t release_flag; // see SKYIOT_RELEASE_e
        uint8_t batt_rank;
		uint8_t sigadc_flag; // 1:需要一次ADC采样       0:无效

	#endif
	 uint32_t report_flag;	 // 按位对应设备的控制属性,置位即发
}SkyBleMeshIotManager;
	
typedef struct {

	#if USE_DOOR_FOR_SKYIOT
    uint8_t alm;
    uint8_t stu;
    uint8_t bat;
	#endif

	uint8_t unuse;
}SkyBleMeshSaveParams;
static SkyBleMeshSaveParams mIotSaveParams;

typedef struct skyOldstate{
    uint8_t alm;
    uint8_t stu;
}skyoldstate;

skyoldstate oldstate;       //门磁与防拆开关旧状态

/*
 * VARIABLES
 ****************************************************************************************
 */

// timer
static plt_timer_t skyblereset_timer = NULL;
static plt_timer_t skybleprosuccess_timer = NULL;
static plt_timer_t skyblemainloop_timer = NULL;
static plt_timer_t skyble_unprov_timer = NULL;
static plt_timer_t skyble_changescan_timer = NULL;

// FIFO
static main_msg_fifo_t MainMsg_fifo={
	.head=0,
	.tail=0,
};
	
static SkyBleMeshIotManager mIotManager;
static bool IsSkyAppInited = false;
static uint8_t g_skybleresetcnt=0;     // 重配网延时服务，延时次数，即重配网闪灯次数
static uint8_t skybleprovsucesscnt=0;     
static unsigned int SkyBleMesh_GetProductType(void);
static int SkyBleMesh_ReadConfig(void);


/*
 * FUNCTIONS
 ****************************************************************************************
 */
 
//static void SkyMesh_Product_PCB_Check_State(void);


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
	static uint32_t oldtick = 0;	

	if(event) {
		if(MainMsg_fifo.head != MainMsg_fifo.tail){
				
//		APP_DBG_PRINTF2("Pop_Msg_From_FIFO %d %d \r\n", MainMsg_fifo.head , MainMsg_fifo.tail );

			if(MainMsg_fifo.even_t[MainMsg_fifo.tail].event_id == EVENT_TYPE_KEEPALIVE
				|| MainMsg_fifo.even_t[MainMsg_fifo.tail].event_id == EVENT_TYPE_UPDATE_PROPERTY
				|| MainMsg_fifo.even_t[MainMsg_fifo.tail].event_id == EVENT_TYPE_GET_PROPERTY
				|| MainMsg_fifo.even_t[MainMsg_fifo.tail].event_id == EVENT_TYPE_REPORT_PROPERTY_ACK){
					
				if ( checksameeven.event_id      != MainMsg_fifo.even_t[MainMsg_fifo.tail].event_id
					|| checksameeven.seqence_num != MainMsg_fifo.even_t[MainMsg_fifo.tail].seqence_num
					|| checksameeven.prop_ID     != MainMsg_fifo.even_t[MainMsg_fifo.tail].prop_ID
					|| checksameeven.prop_value  != MainMsg_fifo.even_t[MainMsg_fifo.tail].prop_value ){
					
					checksameeven.event_id    = MainMsg_fifo.even_t[MainMsg_fifo.tail].event_id;
					checksameeven.seqence_num = MainMsg_fifo.even_t[MainMsg_fifo.tail].seqence_num;
					checksameeven.prop_ID     = MainMsg_fifo.even_t[MainMsg_fifo.tail].prop_ID;
					checksameeven.prop_value  = MainMsg_fifo.even_t[MainMsg_fifo.tail].prop_value;
					oldtick = HAL_GetTickCount();

					memcpy(event, &(MainMsg_fifo.even_t[MainMsg_fifo.tail]), sizeof(req_event_t));
					isgetmag = true;
				}
			}else {			
				memcpy(event, &(MainMsg_fifo.even_t[MainMsg_fifo.tail]), sizeof(req_event_t));
				isgetmag = true;
			}
				
			MainMsg_fifo.tail = (MainMsg_fifo.tail+1)%MAIN_MSG_FIFO_DEEP;
		}
	}
	
	if( checksameeven.event_id		 != 0
		|| checksameeven.seqence_num != 0
		|| checksameeven.prop_ID	 != 0
		|| checksameeven.prop_value  != 0){					

		if( oldtick!=0 && HAL_CalculateTickDiff(oldtick, HAL_GetTickCount()) >= 1000){
			// 1s内丢弃重复接收报文 
			checksameeven.event_id    = 0;
			checksameeven.seqence_num = 0;
			checksameeven.prop_ID     = 0;
			checksameeven.prop_value  = 0;

			oldtick = 0;
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
						case ATTR_CLUSTER_ID_ALM:
//						case ATTR_CLUSTER_ID_SW2:
						case ATTR_CLUSTER_ID_BAT:
                        case ATTR_CLUSTER_ID_STU:
							value = MeshTxAttrStruct[i].buf[TX_ATTR_VAL_POS];
							break;				
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
		case ATTR_CLUSTER_ID_ALM:
		case ATTR_CLUSTER_ID_BAT:
        case ATTR_CLUSTER_ID_STU:
			uAttrSize = 1;
			break;
		
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
}



#endif

static void SkyBleMesh_handle_vendor_rx_cb(uint8_t opcode, uint8_t len, uint8_t *data)
{
	uint8_t *p_data = data;
	uint8_t cmd_type = 0;
	req_event_t event;
	
	memset(&event, 0x0, sizeof(req_event_t));
	
	cmd_type = p_data[MESH_COMMAND_POS];   // 获得命令码
	
	APP_DBG_PRINTF2("SkyBleMesh_handle_vendor_rx_cb opcode %02X, type %02X\n", opcode, cmd_type);	
	
    switch (opcode){
        case BLEMESH_SKYWORTH_OPCODE_HEARTBEAT:
        	if(cmd_type == OPCODE_HEARTBEAT_CMD_PONG){
				mIotManager.alive_wakeup_cnt   = DEFAULT_WAKEUP_ALIVE_CNT;
				
				event.event_id = EVENT_TYPE_KEEPALIVE;
				Push_Msg_Into_FIFO(&event);				
			}
			break;
		
        case BLEMESH_SKYWORTH_OPCODE_PROPERTY:
        	if(cmd_type == OPCODE_PROPERTY_CMD_GETATTR){
								
				event.event_id    = EVENT_TYPE_GET_PROPERTY;
				event.seqence_num = p_data[MESH_SEQUENCE_POS];


				Push_Msg_Into_FIFO(&event);	
				
			}else if(cmd_type == OPCODE_PROPERTY_CMD_UPDATTR){
				
				int32_t  prop_value = 0;	
				uint16_t prop_ID = (p_data[MESH_ATTR_ID_POS+1]<<8) | (p_data[MESH_ATTR_ID_POS]);
			
				switch(prop_ID){
					case ATTR_CLUSTER_ID_ALM:
					case ATTR_CLUSTER_ID_SW2:
					case ATTR_CLUSTER_ID_INF:
					case ATTR_CLUSTER_ID_AMB:
					case ATTR_CLUSTER_ID_MOD:	
						prop_value = p_data[MESH_ATTR_VAL_POS];
						break;
			
					default:
						APP_DBG_PRINTF1("SkyBleMesh_handle_vendor_rx_cb ERR proID %04X\n", prop_ID);
						return;
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
					case ATTR_CLUSTER_ID_ALM:
					case ATTR_CLUSTER_ID_SW2:
					case ATTR_CLUSTER_ID_INF:
					case ATTR_CLUSTER_ID_AMB:
					case ATTR_CLUSTER_ID_MOD:
					case ATTR_CLUSTER_ID_BAT:
                    case ATTR_CLUSTER_ID_STU:
						prop_value = p_data[MESH_ATTR_VAL_POS-1];
						break;
					
//					case ATTR_CLUSTER_ID_TIM:
//						prop_value = ((p_data[MESH_ATTR_VAL_POS-1+3]<<24) | (p_data[MESH_ATTR_VAL_POS-1+2]<<16) | 
//									 (p_data[MESH_ATTR_VAL_POS-1+1]<<8) | (p_data[MESH_ATTR_VAL_POS-1]));
//						break;
					
					default:
						APP_DBG_PRINTF1("SkyBleMesh_handle_vendor_rx_cb ERR proID %04X\n", prop_ID);
						return;
				}

				event.event_id = EVENT_TYPE_REPORT_PROPERTY_ACK;
				event.seqence_num = 0;
				event.prop_ID     = prop_ID;
				event.prop_value  = prop_value;	
				Push_Msg_Into_FIFO(&event);
			}
			break;
		
        case BLEMESH_SKYWORTH_OPCODE_DEVINFO:
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
      
        default:break;
        
    }

}

/*
** Creat vendors model
*/
extern void SkyBleMesh_Vendormodel_init(uint8_t elmt_idx)
{
	// qlj 考虑从 datatrans_server_model_init 移植过来
	Vendor_Model_Init(SkyBleMesh_handle_vendor_rx_cb);
}


#if 1  // 联网配置

static void SkyBleMesh_Prov_Success_Timeout_cb(void *timer)
{
	req_event_t event;
	
	skybleprovsucesscnt++;
	if(skybleprovsucesscnt <= SKYBLEPROVSUCCESS_MAXCNT){
		#if USE_NLIGHT_FOR_SKYIOT
		#endif
	}else{
		if(skybleprovsucesscnt == (SKYBLEPROVSUCCESS_MAXCNT+1)){			
			#if USE_DOOR_FOR_SKYIOT		
            SkyLed_Ctrl(LED_MODE_FAST_BLINK,10);      // 入网成功，闪烁5次（5*2）。
			#endif	
            DBG_DIRECT("-------PROV_SUCCESS--------\r\n");
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
** Provisioning state values        配网状态变量
*/

extern void SkyBleMesh_Provision_State(MESH_PROVISION_STATE_e sate)
{
	mesh_provison_state = sate;
	DBG_DIRECT("----------mesh_proviso_state=%d-------\r\n",sate);
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

extern bool SkyBleMesh_IsProvision_Sate(void)
{
	bool ret=false;
//	DBG_DIRECT("----mesh_pro_state=%d,pro_succeed=%d-----\r\n",mesh_provison_state,MESH_PROVISION_STATE_SUCCEED);
	if(mesh_provison_state==MESH_PROVISION_STATE_SUCCEED || mesh_provison_state==MESH_PROVISION_STATE_PROVED){
		ret = true;
	}

	return ret;
}

/*
** unprovision advertising timeout, deinit something.
*/
static void SkyBleMesh_Unprov_Timeout_cb(void *ptimer)
{
	if(skyble_unprov_timer){
		plt_timer_delete(skyble_unprov_timer, 0);
		skyble_unprov_timer = NULL;
	}
	
    T_IO_MSG unprov_timeout_msg;
    unprov_timeout_msg.type     = IO_MSG_TYPE_TIMER;
    unprov_timeout_msg.subtype  = UNPROV_TIMEOUT;
    app_send_msg_to_apptask(&unprov_timeout_msg);
}

void SkyBleMesh_Unprov_timer(void)
{	
	uint32_t timeout = MESH_UNPROV_INTODLPS_TIME_OUT;
	
	if(skyble_unprov_timer == NULL){ 	
		if(mIotManager.process_state == 0x55){
			timeout = MESH_UNPROV_NORMAL_TIME_OUT;
		}
		skyble_unprov_timer = plt_timer_create("unprov calc", timeout, false, 0, SkyBleMesh_Unprov_Timeout_cb);
		if (skyble_unprov_timer != NULL){
			plt_timer_start(skyble_unprov_timer, 0);
			blemesh_unprov_ctrl_dlps(false);
		}
	}
}

void SkyBleMesh_Unprov_timer_delet(void)
{
    if (skyble_unprov_timer) {
        plt_timer_delete(skyble_unprov_timer, 0);
//        blemesh_unprov_ctrl_dlps(true);
    } else {
        APP_PRINT_INFO0("switch_swtimer->unprov_timer_stop failure!");
    }
}

static void SkyBleMesh_ChangeScan_Timeout_cb(void *ptimer)
{
	if(skyble_changescan_timer){
		plt_timer_delete(skyble_changescan_timer, 0);
        DBG_DIRECT("----skyble_chang_delete-\r\n");
		skyble_changescan_timer = NULL;
	}
	
    T_IO_MSG unprov_timeout_msg;
    unprov_timeout_msg.type     = IO_MSG_TYPE_TIMER;
    unprov_timeout_msg.subtype  = PROV_SUCCESS_TIMEOUT;
    app_send_msg_to_apptask(&unprov_timeout_msg);
}

static uint8_t Hal_Check_Influence(uint32_t newtick)
{
	uint8_t infstatu = 0xff;
	uint8_t retinf = 0xff; // invalid
	static uint8_t infcnt = 0;  
	static uint8_t  calledcnt2 = 0;  
	static uint32_t oldtick=0; 

	if(HAL_ReadInf_Power() == 1){		
		infstatu = HAL_ReadInf_Statu();		
		if(infstatu == SKYIOT_INF_HAVE_BODY){
			infcnt++;
		}

		if( HAL_CalculateTickDiff(oldtick, newtick) >= 1000){
			if(infcnt >= 2){           // 1s内2个信号
				retinf = SKYIOT_INF_HAVE_BODY;
				infcnt = 0;
				calledcnt2 = 0;
				APP_DBG_PRINTF3("%s SKYIOT_INF_HAVE_BODY", __func__);
			}else if(infcnt == 0){
				calledcnt2++;
				if(calledcnt2 >= 5){ // 5s内0个信号
					retinf = SKYIOT_INF_NO_BODY;
					calledcnt2 = 0;
				APP_DBG_PRINTF3("%s SKYIOT_INF_NO_BODY", __func__);
				}
			}else{
				infcnt = 0;
				calledcnt2 = 0;
			}
			
			oldtick = newtick;
		}
		
	}else{
		infcnt = 0;
		oldtick = newtick;
		calledcnt2 = 0;

	}
	if(infwakeupflag  == true){
		infcnt = 0;
		calledcnt2 = 0;
		oldtick = newtick;
		
		retinf = SKYIOT_INF_HAVE_BODY;

		infwakeupflag = false;
		APP_DBG_PRINTF3("%s infwakeupflag %d", __func__, infwakeupflag);
	}
	
	
	return retinf;
}

 

void SkyBleMesh_ChangeScan_timer(uint8_t multi)
{
	if(multi==0 || multi>200){
		multi = 1;
	}
	if(skyble_changescan_timer == NULL){ 	
		skyble_changescan_timer = plt_timer_create("change scan", CHANGE_SCAN_PARAM_TIME_OUT*multi, false, 0, SkyBleMesh_ChangeScan_Timeout_cb);
		if (skyble_changescan_timer != NULL){
			plt_timer_start(skyble_changescan_timer, 0);
		}
	}
}

void SkyBleMesh_Handle_SwTmr_msg(T_IO_MSG *io_msg)
{
    switch (io_msg->subtype)
    {
		case MAINLOOP_TIMEOUT:{
             SkyBleMesh_MainLoop();
			 #if USE_DOOR_FOR_SKYIOT
			 HAL_Switch_HandleTimer(NULL);
             Reenter_tmr_ctrl_dlps(false);
			 #endif
            break;
        }
		case ENTER_DLPS_TIMEOUT:{
             SkyBleMesh_EnterDlps_TmrCnt_Handle();
            break;
        }
		case UNPROV_TIMEOUT:{                       //未入网定时器
            beacon_stop();
            gap_sched_scan(false);
            blemesh_unprov_ctrl_dlps(true);
						
			if(mIotManager.process_state == 0x55){
				mIotManager.process_state = 0xff;
				SkyBleMesh_WriteConfig();	
			}
            break;
        }
		case PROV_SUCCESS_TIMEOUT:{			        //入网成功定时器
			if(mIotManager.process_state == 0x55){
				mIotManager.process_state = 0xff;
				SkyBleMesh_WriteConfig();	
			}
            blemesh_unprov_ctrl_dlps(true); 
            break;
        }
        case test_light_TIMEOUT:                    //灯控定时器
            SkyLed_Timeout_cb_handel(NULL);
            break;
		
		default:
            break;
    }
}


// 进入重配网模式
extern void SkyBleMesh_unBind_complete(void)
{	
    APP_DBG_PRINTF0("******************* SkyMesh_unBind_complete **************************\r\n");    

	mesh_node_clear(); // 恢复重配网
	SkyBleMesh_Provision_State(MESH_PROVISION_STATE_UNPROV);
	
	beacon_stop();  	
	gap_sched_scan(false);   
}


static void SkyBleMesh_Reset_Timeout_cb(void *timer)
{
    if(mIotManager.process_state == 0x55){
//        if( ++g_skybleresetcnt > SKYBLERESET_MAXCNT ){		
            if(skyblereset_timer){
                plt_timer_delete(skyblereset_timer, 0);
                skyblereset_timer = NULL;
            }		
            #if USE_DOOR_FOR_SKYIOT
//            HAL_BlinkProLed_Disable();
            #endif
            HAL_ResetBleDevice(); 
//        } 
    }else{
        if( ++g_skybleresetcnt > SKYBLERESET_MINCNT ){		
            if(skyblereset_timer){
                plt_timer_delete(skyblereset_timer, 0);
                skyblereset_timer = NULL;
            }		
            #if USE_DOOR_FOR_SKYIOT
//            HAL_BlinkProLed_Disable();
            #endif
            HAL_ResetBleDevice(); 
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

extern bool SkyBleMesh_Device_Active_Sate(void)
{
	bool ret=false;
	
	if(mIotManager.release_flag == SKYIOT_ACTIVE_RELEASE || mIotManager.release_flag == SKYIOT_FIRST_RELEASE){
		ret = true;
	}

	return ret;
}


#endif

static void SkyBleMesh_Save_Params(uint32_t newtick)
{
	static uint32_t oldtick=0;
	if(HAL_CalculateTickDiff(oldtick, newtick) >= WRITE_DEFAULT_TIME){
		if(	mIotSaveParams.alm != mIotManager.mLightManager.alm
			|| mIotSaveParams.stu != mIotManager.mLightManager.stu
			|| mIotSaveParams.bat != mIotManager.mLightManager.bat){

			SkyBleMesh_WriteConfig();
			
			APP_DBG_PRINTF0("SkyBleMesh_Save_Params \n"); 

			mIotSaveParams.alm = mIotManager.mLightManager.alm;
			mIotSaveParams.stu = mIotManager.mLightManager.stu;
			mIotSaveParams.bat = mIotManager.mLightManager.bat;
		}
		
        oldtick = newtick;
    }
}


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

	memcpy(buffer + offset, &(mIotManager.release_flag), 1);
	offset += 1;
	utils_md5_update(context, (unsigned char*)&(mIotManager.release_flag), 1);
	
	memcpy(buffer + offset, &(mIotManager.process_state), 1);
	offset += 1;
	utils_md5_update(context, (unsigned char*)&(mIotManager.process_state), 1);
	
	memcpy(buffer + offset, mIotManager.mac_address, MAC_ADDRESS_LEN);
	offset += MAC_ADDRESS_LEN;
	utils_md5_update(context, (unsigned char*)mIotManager.mac_address, MAC_ADDRESS_LEN);
	
	memcpy(buffer + offset, mIotManager.device_uuid, BT_MESH_UUID_SIZE);
	offset += BT_MESH_UUID_SIZE;
	utils_md5_update(context, (unsigned char*)mIotManager.device_uuid, BT_MESH_UUID_SIZE);
		
	memcpy(buffer + offset, &(mIotManager.mLightManager.alm), 1);
	offset += 1;
	utils_md5_update(context, (unsigned char*)&(mIotManager.mLightManager.alm), 1);
	
	memcpy(buffer + offset, &(mIotManager.mLightManager.stu), 1);
	offset += 1;
	utils_md5_update(context, (unsigned char*)&(mIotManager.mLightManager.stu), 1);
	
	memcpy(buffer + offset, &(mIotManager.mLightManager.bat), 1);
	offset += 1;
	utils_md5_update(context, (uint8_t *)&mIotManager.mLightManager.bat, 1);

	
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
	
	utils_md5_init(context);     // init context for 1st pass 
	utils_md5_starts(context);   // setup context for 1st pass 

	flashret = Hal_FlashRead(FLASH_PARAM_TYPE_APP_CFGDATA, FLASH_USERDATA_SAVE_LEN, buffer );
	if (flashret == false) {
		os_mem_free(buffer);
		APP_DBG_PRINTF0("SkyBleMesh_ReadConfig error \n");
		return -1;
	}
	
	// 出厂值
	memcpy(&(mIotManager.release_flag), buffer + offset, 1);	
	offset += 1;
	utils_md5_update(context, (unsigned char*)&(mIotManager.release_flag), 1);
	// 进配网标志
	memcpy(&(mIotManager.process_state), buffer + offset, 1);	
	offset += 1;
	utils_md5_update(context, (unsigned char*)&(mIotManager.process_state), 1);
	//
	memcpy(mIotManager.mac_address, buffer + offset, MAC_ADDRESS_LEN);	
	offset += MAC_ADDRESS_LEN;
	utils_md5_update(context, (unsigned char*)mIotManager.mac_address, MAC_ADDRESS_LEN);
	//
	memcpy(mIotManager.device_uuid, buffer + offset, BT_MESH_UUID_SIZE);	
	offset += BT_MESH_UUID_SIZE;
	utils_md5_update(context, (unsigned char*)mIotManager.device_uuid, BT_MESH_UUID_SIZE);
	
	//
	memcpy(&(mIotManager.mLightManager.alm), buffer + offset, 1);	
	offset += 1;
	utils_md5_update(context, (unsigned char*)&(mIotManager.mLightManager.alm), 1);
	//
	memcpy(&(mIotManager.mLightManager.stu), buffer + offset, 1);	
	offset += 1;
	utils_md5_update(context, (unsigned char*)&(mIotManager.mLightManager.stu), 1);
	//
	memcpy(&(mIotManager.mLightManager.bat), buffer + offset, 1);	
	offset += 1;
	utils_md5_update(context, (unsigned char*)&(mIotManager.mLightManager.bat), 1);

	// ...	
		
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
static void SkyiotManager_Default_Config(bool sychsaveparam)
{
    mIotManager.mLightManager.alm = 0;
    mIotManager.mLightManager.stu = 0;
    oldstate.alm = 0;
    oldstate.stu = 0;
//	
	if(sychsaveparam == true){		
        mIotSaveParams.alm = mIotManager.mLightManager.alm;
        mIotSaveParams.stu = mIotManager.mLightManager.stu;
	}
}

static void SkySwitch_Handle(uint8_t key_mode, bool isprov)
{
	if(key_mode > KEY_LONGPRESS_MODE){
		return;
	}
	
	if(key_mode != KEY_SHORTPRESS_MODE && mIotManager.release_flag == SKYIOT_FIRST_RELEASE){
		return;
	}
	
    if(mIotManager.batt_rank != BATT_NORMAL){               // 电压低于正常值，不允许配网
        APP_DBG_PRINTF0("low batt");
        return;
    }

   	if(key_mode == KEY_SINGLE_MODE || key_mode == KEY_SHORTPRESS_MODE ){
		
        if(mIotManager.process_state == 0x55){
            mIotManager.process_state = 0xff;
            SkyBleMesh_WriteConfig();	
            HAL_ResetBleDevice();
			return;
        }	
   	}
	if(key_mode == KEY_SINGLE_MODE){
            DBG_DIRECT("------KEY_SINGLE_MODE--------\r\n");

	}else if(key_mode == KEY_SHORTPRESS_MODE){
           DBG_DIRECT("------KEY_SHORTPRESS_MODE--------\r\n");
	}else if(key_mode == KEY_LONGPRESS_MODE){  
		SkyiotManager_Default_Config(true);	
		SkyBleMesh_unBind_complete();
        SkyLed_Ctrl(LED_MODE_SLOW_BLINK,10);
        DBG_DIRECT("-------ENTER IS unBind---------\r\n");
		if(mIotManager.process_state != 0x55){
			mIotManager.process_state = 0x55;
			SkyBleMesh_WriteConfig();
		}
		SkyBleMesh_Reset_timer();
		APP_DBG_PRINTF0("vvvvvvvvvvvvvvvv %X\n",mIotManager.process_state);
	}		
}


/*
** main functions
*/
static void Reset_iotmanager_para(void)
{
	mIotManager.alive_status        = 1;//set device to online status
	mIotManager.recv_alive_tick     = HAL_GetTickCount();

#if USE_DOOR_FOR_SKYIOT
	// 上报设置项，做同步
	mIotManager.report_flag |= BLEMESH_REPORT_FLAG_ALM;
	mIotManager.report_flag |= BLEMESH_REPORT_FLAG_STU;
	mIotManager.report_flag |= BLEMESH_REPORT_FLAG_BAT;
	mIotManager.sw1_seq = 0;
    mIotManager.saf_seq = 0;
    mIotManager.bat_seq = 0;
#endif
}

static void Sky_listendoorstate(void)
{
     uint8_t skystatu = ReadStatus();
//    DBG_DIRECT("----------skystatu=%d--------\r\n",skystatu);
    switch (skystatu)
    {
        case 0:
            mIotManager.mLightManager.alm = 0;
            mIotManager.mLightManager.stu = 0;
            break;
        case 1:
            mIotManager.mLightManager.alm = 1;
            mIotManager.mLightManager.stu = 0;
            break;
        case 2:
            mIotManager.mLightManager.alm = 0;
            mIotManager.mLightManager.stu = 1;
            break;
        case 3:
            mIotManager.mLightManager.alm = 1;
            mIotManager.mLightManager.stu = 1;
            break;
            
    }
    if(oldstate.alm !=mIotManager.mLightManager.alm)
    {
        mIotManager.report_flag |= BLEMESH_REPORT_FLAG_ALM;
        oldstate.alm = mIotManager.mLightManager.alm;
        HAL_Lighting_ON();
        SkyLed_Ctrl(LED_MODE_NORMAL_BRIGHT,0);
        DBG_DIRECT("----------skystatu=%d--------\r\n",skystatu);
    }
    if(oldstate.stu != mIotManager.mLightManager.stu)
    {
        mIotManager.report_flag |= BLEMESH_REPORT_FLAG_STU;
        oldstate.stu = mIotManager.mLightManager.stu;
        HAL_Lighting_ON();
        SkyLed_Ctrl(LED_MODE_NORMAL_BRIGHT,0);
        DBG_DIRECT("----------skystatu=%d--------\r\n",skystatu);
    }
    door_flag = 0;
}
static void Main_Event_Handle(void)
{
	// 恢复心跳后，延时上报。延时期间可能有seq不为0，故只置上报标志
	static uint32_t oldtick=0, delayreport=0; 
	uint32_t tick=0;
	req_event_t event;
	
	if(delayreport){

		tick = HAL_GetTickCount();
		if(delayreport <= HAL_CalculateTickDiff(oldtick, tick)){
			delayreport = 0;
		#if USE_DOOR_FOR_SKYIOT
			mIotManager.report_flag |= BLEMESH_REPORT_FLAG_ALM;
			mIotManager.report_flag |= BLEMESH_REPORT_FLAG_STU;
			mIotManager.report_flag |= BLEMESH_REPORT_FLAG_BAT;


		#endif
		}
	}
			
	if(Pop_Msg_From_FIFO(&event)){
		APP_DBG_PRINTF4("Main_Event_Handle %X %d %X %04X \n",
			event.event_id , event.seqence_num , event.prop_value, event.prop_ID );

		switch(event.event_id){
			case EVENT_TYPE_PROVISIONED:			
				Reset_iotmanager_para();
				mIotManager.report_flag = 0; // 配网成功后不主动上报，等待网关请求。
			break;

			case EVENT_TYPE_KEEPALIVE:
				if (mIotManager.alive_status == 0){
					Reset_iotmanager_para();
					
					// used in delay report
					mIotManager.report_flag = 0;
					delayreport = (uint32_t)(rand()%80)*50 + 50; // 50~4000ms
					oldtick = HAL_GetTickCount();	
					
					APP_DBG_PRINTF0("Main_Event_Handle device online!\n");
				}else {
					mIotManager.recv_alive_tick = HAL_GetTickCount();	
					APP_DBG_PRINTF1("Main_Event_Handle recv keepalive tick: %d!\n", mIotManager.recv_alive_tick);
				}			
				break;
				
			case EVENT_TYPE_UPDATE_PROPERTY:
				if(mIotManager.alive_status == 0){
					Reset_iotmanager_para();
				}
				if (mIotManager.alive_status == 1){
					// 根据参数，对设备控制
                }
				break;
			case EVENT_TYPE_GET_PROPERTY:				
				if(mIotManager.alive_status == 0){
					Reset_iotmanager_para();
				}
				if(mIotManager.alive_status == 1){		
#if USE_DOOR_FOR_SKYIOT
					// 网关请求属性上传
					mIotManager.report_flag |= BLEMESH_REPORT_FLAG_ALM;
					mIotManager.report_flag |= BLEMESH_REPORT_FLAG_STU;
					mIotManager.report_flag |= BLEMESH_REPORT_FLAG_BAT;
#endif
				}		
				break;

			case EVENT_TYPE_REPORT_PROPERTY_ACK:
				BleMesh_Vendor_Ack_Packet(event.event_id, event.prop_ID, event.prop_value);
				break;
		}	
	}
#if USE_DOOR_FOR_SKYIOT
//     
	if(mIotManager.mSwitchManager.keymode && mIotManager.mSwitchManager.keyval){
		if(mIotManager.mSwitchManager.keyval == 0x01){ // user switch	
			SkySwitch_Handle(mIotManager.mSwitchManager.keymode, false);
		}
		mIotManager.mSwitchManager.keymode = 0;
		mIotManager.mSwitchManager.keyval = 0;
	}		
#endif
    
	
}



static void Main_WithoutNet_Handle(void)
{	
	
#if USE_DOOR_FOR_SKYIOT
	if(mIotManager.mSwitchManager.keymode && mIotManager.mSwitchManager.keyval){
		if(mIotManager.mSwitchManager.keyval == 0x01){ // user switch	
			SkySwitch_Handle(mIotManager.mSwitchManager.keymode, false);
		}
		mIotManager.mSwitchManager.keymode = 0;
		mIotManager.mSwitchManager.keyval = 0;
	}		
#endif

}

static bool Main_Check_Online(void)
{	
	static bool  firstgettick=true;
	static uint32_t alivetick=0;
	uint32_t alivetimecnt=0,sub_timeout_ms=0;
	uint32_t tick = HAL_GetTickCount();	

	if(firstgettick && alivetick==0){
		firstgettick = false;
		alivetick = HAL_GetTickCount();	
		return true; 
	}

	alivetimecnt = HAL_CalculateTickDiff(alivetick, tick);
	if (alivetimecnt >= 150){  // 30*50ms
		if (mIotManager.alive_wakeup_cnt < DEFAULT_WAKEUP_ALIVE_CNT){
			if (mIotManager.alive_status == 0){
//				APP_DBG_PRINTF1("Main_Check_Online wakeupcnt %d\n", mIotManager.alive_wakeup_cnt);
				SkyIotSendKeepAlivePacket();
				mIotManager.send_alive_tick = tick;
			}else{
				sub_timeout_ms = HAL_CalculateTickDiff(mIotManager.recv_alive_tick, tick);
				// APP_DBG_PRINTF2("Main_Check_Online wakeupcnt %d %d\n", mIotManager.alive_wakeup_cnt, sub_timeout_ms);
				if (sub_timeout_ms >= DEFAULT_SKYIOT_ALIVE_MS){
					SkyIotSendKeepAlivePacket();
					mIotManager.send_alive_tick = tick;
				}
			}
			mIotManager.alive_wakeup_cnt++;
		}else {
			sub_timeout_ms = HAL_CalculateTickDiff(mIotManager.send_alive_tick, tick);
			if (sub_timeout_ms >= DEFAULT_SKYIOT_ALIVE_MS){
//				APP_DBG_PRINTF1("Main_Check_Online sub_timeout_ms %d\n", sub_timeout_ms);
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
				
			#if USE_DOOR_FOR_SKYIOT
//				mIotManager.swt1_seqnum = 0;
			#endif
			}
            
		}
		alivetick = HAL_GetTickCount();
		  
	}

	return true;   
}

static void Main_Upload_State(void)
{
	if (mIotManager.alive_status == 1){	
#if USE_DOOR_FOR_SKYIOT

		if(mIotManager.report_flag & BLEMESH_REPORT_FLAG_ALM){							// 门磁状态发生改变
			mIotManager.report_flag &= ~ BLEMESH_REPORT_FLAG_ALM;
			SkyIotReportPropertyPacket(ATTR_CLUSTER_ID_ALM, mIotManager.mLightManager.alm, mIotManager.sw1_seq);	
			mIotManager.sw1_seq = 0;
		}
		
		if(mIotManager.report_flag & BLEMESH_REPORT_FLAG_STU){							// 防拆开关状态
			mIotManager.report_flag &= ~ BLEMESH_REPORT_FLAG_STU;
			SkyIotReportPropertyPacket(ATTR_CLUSTER_ID_STU, mIotManager.mLightManager.stu,mIotManager.saf_seq);	
			mIotManager.saf_seq = 0;
		}
		
		if(mIotManager.report_flag & BLEMESH_REPORT_FLAG_BAT){							// 入网主动上传，或阶段性上传
			mIotManager.report_flag &= ~ BLEMESH_REPORT_FLAG_BAT;
			SkyIotReportPropertyPacket(ATTR_CLUSTER_ID_BAT, mIotManager.mLightManager.bat, mIotManager.bat_seq);	
			mIotManager.bat_seq = 0;
		}
		#endif
	}
}
// DLPS相关
bool SkyBleMesh_Is_No_ReportMsg(void)
{
	bool ret = false;
	uint8_t i=0;

	if (mIotManager.alive_status == 1) {	
		for(i=0; i<MAX_TX_BUF_DEEP; i++){
			if( MeshTxAttrStruct[i].fullflag==1 ){
				break;
			}
		}
		if(i==MAX_TX_BUF_DEEP && mIotManager.report_flag==0){
			ret = true;
		}

	} else{
		ret = true; // qlj 后面考虑下
	}

	return ret;
}

void SkyBleMesh_DlpsLight_Handle(bool isenter)
{
#if USE_DOOR_FOR_SKYIOT
	HAL_Light_Dlps_Control(isenter);		
#endif
}


#if 0 //MESH_TEST_PRESSURE == 1
extern void test_update_attr(void)
{
	bool ret = false;
	uint8_t i=0;

	
	DBG_DIRECT("mIotManager.alive_status %d %d\n", mIotManager.alive_status, mIotManager.report_flag);
	
	for(i=0; i<MAX_TX_BUF_DEEP; i++){
		if( MeshTxAttrStruct[i].fullflag==1 ){
			DBG_DIRECT("buff index %d , code %02X\n", i, MeshTxAttrStruct[i].buf[0]);
		}
	}
		
}
#endif


extern void SkyBleMesh_MainLoop(void)
{
  
	if(IsSkyAppInited==false){
		return;
	}

	static uint32_t battadctick=0;
	uint32_t newtick = HAL_GetTickCount();
//	SkyBleMesh_Save_Params(newtick);
	
	#if SKYMESH_FACTORYTEST_EABLE==1
	// 工厂测试
//	if(SkyMesh_Product_Factory_Checking() == true){ 		
//		return;
//	}
	#endif
	
	bool isprov = SkyBleMesh_IsProvision_Sate();     // 联网状态
    Sky_listendoorstate();              
	if(isprov==true && mIotManager.batt_rank!=BATT_WARING){
		
		//recv message	
		Main_Event_Handle();
//		SkyFunction_Handle(newtick);
		
		//判断设备是否离线
		if(Main_Check_Online() == true){
			//状态上报 
			Main_Upload_State();
			
			// 轮询发送缓存
			BleMesh_Vendor_Send_Packet();
		}
		
	}else{
		// 没有配网也要执行控制，不给上报标志
        if(mIotManager.alive_status !=0 || mIotManager.recv_alive_tick != 0){
            mIotManager.alive_status        = 0;		
            mIotManager.report_flag         = 0;
            mIotManager.send_alive_tick     = 0;
            mIotManager.recv_alive_tick     = 0;
            memset(mIotManager.reppack_buffer, 0x0, MAX_BLEMESH_PACKET_LEN);
            mIotManager.reppack_len = 0;
        }
        if(mIotManager.batt_rank == BATT_WARING){   
//			if(HAL_CalculateTickDiff(battadctick, newtick) >= 30000 ){// 30000 ms 
//	            SkyBleMesh_Batterval_Lightsense(true);		
//				battadctick = newtick;
//			}
        }else{
			Main_WithoutNet_Handle();
//			SkyFunction_Handle(newtick);		
		}
        
        
	}
	
}



static void SkyBleMesh_MainLoop_Timeout_cb(void *time)
{		
	#if USE_SOFT_WATCHDOG
	SoftWdtFed(SKYMESH_THREAD_SWDT_ID); 
	#endif
	
	// 给APPtask发消息,subtype区分哪个tmr
    T_IO_MSG msg;
    msg.type = IO_MSG_TYPE_TIMER;
    msg.subtype = MAINLOOP_TIMEOUT;
    app_send_msg_to_apptask(&msg);
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
extern void SkyBleMesh_StopMainLoop_tmr(void)
{	
	if(skyblemainloop_timer){		
		plt_timer_stop(skyblemainloop_timer, 0);
	}
}
extern void SkyBleMesh_StartMainLoop_tmr(void)
{	
	if(skyblemainloop_timer){		
		plt_timer_start(skyblemainloop_timer, 0);
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
		
		/*创维设备mesh UUID:     CIP(2BYTE)+ PRODUCT_TYPE(4Byte)+ MAC地址(6Byte)+ 机数(4Byte) */
		Regain_Random_UUID(mIotManager.device_uuid, MESH_DEV_UUID_LEN);			
		mIotManager.device_uuid[0] = BLEMESH_VENDOR_COMPANY_ID & 0xFF;
		mIotManager.device_uuid[1] = (BLEMESH_VENDOR_COMPANY_ID>>8) & 0xFF;
		mIotManager.device_uuid[2] = PRODUCT_TYPE & 0xFF;
		mIotManager.device_uuid[3] = (PRODUCT_TYPE>>8) & 0xFF;
		mIotManager.device_uuid[4] = (PRODUCT_TYPE>>16) & 0xFF;
		mIotManager.device_uuid[5] = (PRODUCT_TYPE>>24) & 0xFF;			
		memcpy(&(mIotManager.device_uuid[6]), mIotManager.mac_address, MAC_ADDRESS_LEN);
		#if MESH_TEST_PRESSURE == 1
		mIotManager.device_uuid[12]   = PRODUCT_TYPE & 0xFF;	// used in test
		mIotManager.device_uuid[13]   = PRODUCT_TYPE & 0xFF;	
		mIotManager.device_uuid[14]   = PRODUCT_TYPE & 0xFF;	
		mIotManager.device_uuid[15]   = PRODUCT_TYPE & 0xFF;
		#endif			
			
		mIotManager.process_state = 0xFF;
		if(retgetmac==false){
		// 	SkyBleMesh_unBind_complete();
			mIotManager.release_flag = SKYIOT_INVALID_RELEASE;
		 }
		
		SkyiotManager_Default_Config(true);  // 设备默认配置 并 控制 (暂时不配置)
		SkyBleMesh_WriteConfig();        // 保存参数
		
		// if(retgetmac==false){
		// 	APP_DBG_PRINTF1("Write First Release Flag");
		// 	SkyBleMesh_Reset_timer();
		// }
				
    } else {
        APP_DBG_PRINTF0("SkyBleMesh_ReadConfig read succ\n");
    }
	
    for(int i =0 ;i < 16;i++){
        APP_DBG_PRINTF2("mIotManager.device_uuid[%d] = 0x%02x\n",i,mIotManager.device_uuid[i]);
    }
	
	APP_DBG_PRINTF0("Device Work State: release_flag=%02X\n", mIotManager.release_flag);

	mIotManager.alive_wakeup_cnt    = 0;
	mIotManager.alive_status        = 0;		
	mIotManager.report_flag         = 0;
	mIotManager.send_alive_tick     = 0;
	mIotManager.recv_alive_tick     = 0;
	memset(mIotManager.reppack_buffer, 0x0, MAX_BLEMESH_PACKET_LEN);
	mIotManager.reppack_len = 0;
		
	BleMesh_Packet_Init();
	
	#if USE_DOOR_FOR_SKYIOT 
	mIotManager.sw1_seq = 0;
    mIotManager.saf_seq = 0;
    mIotManager.bat_seq = 0;
    HAL_Light_Init();
    HAL_Switch_Init(&mIotManager.mSwitchManager);
    Sky_ADC_POWER_Init();

	// use save
//	mIotSaveParams.statu[SKY_LED1_STATUS] = mIotManager.mLightManager.statu[SKY_LED1_STATUS];
//	mIotSaveParams.statu[SKY_LED2_STATUS] = mIotManager.mLightManager.statu[SKY_LED2_STATUS];
//	mIotSaveParams.amb      = mIotManager.mLightManager.amb;
//	mIotSaveParams.bri_time = mIotManager.mLightManager.bri_time;
//	mIotSaveParams.mode     = mIotManager.mLightManager.mode;	
	
	#endif

	// SkyBleMesh_MainLoop_timer(); // called in app_task

	mIotManager.release_flag = SKYIOT_ACTIVE_RELEASE ;        // 自测用，为了不产测，也可以进低功耗
	IsSkyAppInited = true;
	blemesh_sysinit_ctrl_dlps(true);
	if(SkyBleMesh_Device_Active_Sate() == true){
		blemesh_factory_ctrl_dlps(true);
	}else{
		blemesh_factory_ctrl_dlps(false);
	}

	
	return 0; 
}
	
extern void SkyBleMesh_Batterval_Lightsense(bool onlybatt)
{
	uint16_t batt_val=0;
				
		HAL_SkyAdc_Sample(&batt_val);	
		if(mIotManager.batt_rank == BATT_NORMAL){	
			if(batt_val <= BATT_WARIN_RANK){
				mIotManager.batt_rank = BATT_WARING;				
				beacon_stop();  	
				gap_sched_scan(false);
				APP_DBG_PRINTF0("[BATT WARING] battery voltage is below 3.1V \n");
			}
		}else{
			if(batt_val > (BATT_WARIN_RANK+150)){ // 电压大于预警值150mv，解除预警
				mIotManager.batt_rank = BATT_NORMAL;
                gap_sched_scan(true); 
                beacon_start();
			}
		}
		
		APP_DBG_PRINTF3("batt_val:%d, lp_dat:%d\r\n", batt_val);
	// }

}


extern uint8_t SkyBleMesh_Batt_Station(void)
{
    return mIotManager.batt_rank;
}

//=======================================================================================//
#if SKYMESH_FACTORYTEST_EABLE==1
extern void HAL_Lighting_Nightlight(uint8_t lightindex, uint16_t val);

// 工厂测试
#define DONGLEADV_LEN_INDEX       3
#define DONGLEADV_CMD_INDEX       5
#define DONGLEADV_SCR_INDEX       6
#define DONGLEADV_DEST_INDEX      12

enum m_FACTORYTESTSTATE
{
    M_FACTORYTEST_INIT_STATE = 0x00,
    M_FACTORYTEST_START_STATE,
    M_FACTORYTEST_FINISH_STATE,
    M_FACTORYTEST_END_STATE
};
enum m_FACTORYTESTSTATE g_factorytest_state=M_FACTORYTEST_INIT_STATE;
const uint8_t DongleAdvAddr[6]={0x53, 0x4B, 0x59, 0x44, 0x55, 0x54};

enum m_PCBA_TESTSTATE
{
    M_PCBA_TEST_INIT_STATE = 0x00,
    M_PCBA_TEST_READY_STATE,
    M_PCBA_TEST_START_STATE,
    M_PCBA_TEST_END_STATE
};
enum m_PCBA_TESTSTATE g_PCBtest_state=M_PCBA_TEST_INIT_STATE;

static bool ReverseComparisonBuff(uint8_t *src, uint8_t *dst, uint8_t len)
{
	uint8_t i = 0;
	bool ret = false;
	if(len){
		for(i=0;i<len;i++){
			if(src[i] != dst[len-i-1]){
				break;
			}
		}
		if( i == len ){
			ret = true;
		}
	}
	
	return ret;
}
void SkyMesh_ProductFactoryCheck_cb(T_GAP_ADV_EVT_TYPE adv_type, uint8_t* bd_addr, uint8_t* data, uint8_t data_len)
{
	// APP_DBG_PRINTF1("%s %02X %02X %02X %02X %02X %02X ",__func__, bd_addr[0], bd_addr[1], bd_addr[2], bd_addr[3], bd_addr[4], bd_addr[5]);
       
	// 未配网状态下，只进一次产测。
	// 020106(沿用GATT头3B) + len(1B) + FF(1B) + 81(cmd) + scraddr(6B) + "dest_addr(6B)" + sum(1B)
	if(g_factorytest_state==M_FACTORYTEST_INIT_STATE && (SkyBleMesh_IsProvision_Sate()!=true) && (adv_type==GAP_ADV_EVT_TYPE_NON_CONNECTABLE)){
		if( ReverseComparisonBuff(bd_addr, (uint8_t*)DongleAdvAddr, 6) == true){
			#if 0
			uint8_t ii;
			APP_DBG_PRINTF1("%s %02X %02X ",__func__, data[DONGLEADV_LEN_INDEX]+4 , data_len);
			for(ii=0;ii< data_len ;ii++){
				APP_DBG_PRINTF1("%02X ",data[ii]);
			}			
			#endif
			if(data[DONGLEADV_LEN_INDEX]+4 ==  data_len){
				uint8_t i=0, sum=0;
				for(i=0;i< data_len-1 ;i++){
					sum += data[i];
				}				
				// APP_DBG_PRINTF1("sum %02X ",sum);
				if(data[data_len-1]==sum && data[DONGLEADV_CMD_INDEX]==0x81){
					if(memcmp(DongleAdvAddr, &(data[DONGLEADV_SCR_INDEX]), 6) == 0){
						if(data_len == (DONGLEADV_DEST_INDEX+1)){
							// 不指定
							APP_DBG_PRINTF1("UNspecify startfactory isintofactory=%d \n", g_factorytest_state );
						}else if(data_len == (DONGLEADV_DEST_INDEX+7)){
							// 指定
							if(ReverseComparisonBuff(SkyBleMesh_GetProductMac(), (uint8_t*)(&(data[DONGLEADV_DEST_INDEX])), 6) == true){								
								APP_DBG_PRINTF1("Specify startfactory isintofactory=%d \n", g_factorytest_state );
								g_factorytest_state = M_FACTORYTEST_START_STATE;
							}
						}
					}
				}
			}
		
		} 
	}
	
}


//static void SkyMesh_Product_PCB_Check_State(void)
//{
//	if(g_PCBtest_state==M_PCBA_TEST_INIT_STATE){
//		g_PCBtest_state = M_PCBA_TEST_READY_STATE;
//	}	
//}
//void SkyMesh_Product_PCB_Check_Start(void)
//{
//	if(g_PCBtest_state==M_PCBA_TEST_READY_STATE){
//		g_PCBtest_state = M_PCBA_TEST_START_STATE;
//	}	
//}
//static void SkyMesh_Product_PCB_Check(void)
//{
//	// 50ms called
//	static uint8_t pcbstep = 0;
//	static uint8_t tmpcnt = 0;
//	
//	if(g_PCBtest_state==M_PCBA_TEST_INIT_STATE){
//		pcbstep = 0;
//		tmpcnt = 0;
//		return;
//	}
//	
//	switch(pcbstep){
//		case 0:{
//			if(g_PCBtest_state==M_PCBA_TEST_READY_STATE){
//				blemesh_factory_ctrl_dlps(false);  // 调禁止休眠
//				
//				if(tmpcnt%8 == 0){ 
//					HAL_Lighting_Nightlight(FRONT_LED_PWM, 0);
//					HAL_Lighting_Nightlight(REAR_LED_PWM,  0);
//				}else if(tmpcnt%8 == 4){
//					HAL_Lighting_Nightlight(FRONT_LED_PWM, 0);
//					HAL_Lighting_Nightlight(REAR_LED_PWM,  100);

//				}
//				// 200ms*2 = 50ms*8
//				if(++tmpcnt >= 8){
//					tmpcnt = 0;
//				}
//			
//			}else if(g_PCBtest_state==M_PCBA_TEST_START_STATE){				
//				HAL_OpenInf_Power(true); // 提前打开电源准备
//				pcbstep = 1;
//				tmpcnt = 0;
//			}
//		break;
//		}
//		case 1:{
//			HAL_OpenInf_Power(true); // 提前打开电源准备
//			
//			HAL_Lighting_Nightlight(FRONT_LED_PWM, 100);
//			HAL_Lighting_Nightlight(REAR_LED_PWM,  0);
//		
//			uint32_t newtick = HAL_GetTickCount();
//			uint8_t infstatu = Hal_Check_Influence(newtick);			
//			if(infstatu == SKYIOT_INF_HAVE_BODY){ // 人感
//				pcbstep = 2;
//			}
//		break;
//		}
//		case 2:{
//			if(tmpcnt%16 == 0){ 
//				HAL_Lighting_Nightlight(FRONT_LED_PWM, 100);
//				HAL_Lighting_Nightlight(REAR_LED_PWM,  0);
//			}else if(tmpcnt%16 == 4){
//				HAL_Lighting_Nightlight(FRONT_LED_PWM, 0);
//				HAL_Lighting_Nightlight(REAR_LED_PWM,  0);

//			}else if(tmpcnt%16 == 8){
//				HAL_Lighting_Nightlight(FRONT_LED_PWM, 0);
//				HAL_Lighting_Nightlight(REAR_LED_PWM,  100);

//			}else if(tmpcnt%16 == 12){
//				HAL_Lighting_Nightlight(FRONT_LED_PWM, 0);
//				HAL_Lighting_Nightlight(REAR_LED_PWM,  0);

//			}
//			// 200ms*4 = 50ms*16
//			if(++tmpcnt >= 16){
//				tmpcnt = 0;
//			}
//		break;
//		}
//				
//	}
//	
//}


void SkyMesh_Product_device_Check_State(void)
{
	if(g_factorytest_state==M_FACTORYTEST_FINISH_STATE){
		g_factorytest_state = M_FACTORYTEST_END_STATE;
	}
}

//static void SkyMesh_Product_device_Check(void)
//{
//	// 50ms called
//	static uint8_t devicestep = 0;
//	static uint8_t tmpcnt = 0;
//	
//	if(g_factorytest_state==M_FACTORYTEST_INIT_STATE){
//		devicestep = 0;
//		tmpcnt = 0;
//		return;
//	}
//	
//	switch(devicestep){
//		case 0:{
//			HAL_Set_Ambient_Power(1); // 提前打开电源准备ADC采集
//			HAL_OpenInf_Power(true); // 提前打开电源准备
//			
//			if(g_factorytest_state == M_FACTORYTEST_START_STATE){
//				blemesh_factory_ctrl_dlps(false);  // 调禁止休眠
//				HAL_Lighting_ON();
//			
//				uint32_t newtick = HAL_GetTickCount();
//				uint8_t infstatu = Hal_Check_Influence(newtick);			
//				if(infstatu == SKYIOT_INF_HAVE_BODY){ // 人感
//					devicestep = 1;
//				}
//			}
//		break;
//		}
//		case 1:{
//			HAL_Lighting_OFF();
//			if(tmpcnt%10 == 0){
//				HAL_Set_Ambient_Power(1); // 一直打开电源准备ADC采集
////				mIotManager.mLightManager.ambstatu = SKYIOT_AMBIENT_BRIGHT; // 强制给下
//			}else if(tmpcnt%10 == 9){
//				SkyBleMesh_Batterval_Lightsense(false);
//			}
//			if(++tmpcnt >= 10){
//				tmpcnt = 0;
//			}
//		
////			if(mIotManager.mLightManager.ambstatu == SKYIOT_AMBIENT_DARK){ // 光感
//				devicestep = 2;
//				tmpcnt = 0;
//			}
//		break;
//		}
//		case 2:{
//			if(tmpcnt%16 == 0){ 
//				HAL_Lighting_Nightlight(FRONT_LED_PWM, 100);
//				HAL_Lighting_Nightlight(REAR_LED_PWM,  0);
//			}else if(tmpcnt%16 == 4){
//				HAL_Lighting_Nightlight(FRONT_LED_PWM, 0);
//				HAL_Lighting_Nightlight(REAR_LED_PWM,  0);

//			}else if(tmpcnt%16 == 8){
//				HAL_Lighting_Nightlight(FRONT_LED_PWM, 0);
//				HAL_Lighting_Nightlight(REAR_LED_PWM,  100);

//			}else if(tmpcnt%16 == 12){
//				HAL_Lighting_Nightlight(FRONT_LED_PWM, 0);
//				HAL_Lighting_Nightlight(REAR_LED_PWM,  0);

//			}
//			// 200ms*4 = 50ms*16
//			if(++tmpcnt >= 16){
//				tmpcnt = 0;
//				
//				if(g_factorytest_state == M_FACTORYTEST_START_STATE){
//					g_factorytest_state = M_FACTORYTEST_FINISH_STATE;
//				}
//			}
//			
//			if(g_factorytest_state == M_FACTORYTEST_END_STATE){
//				// blemesh_factory_ctrl_dlps(true);  // 恢复休眠 有重启不需要
//				// 写首次出厂标志				
//		        SkyBleMesh_unBind_complete();
//		        mIotManager.process_state = 0xFF;
//		        mIotManager.release_flag = SKYIOT_FIRST_RELEASE;
//		        SkyBleMesh_WriteConfig();
//				SkyBleMesh_Reset_timer();
//		        APP_DBG_PRINTF1("Write First Release Flag");
//						
//				devicestep = 0;
//			}
//		break;
//		}
//			
//	
//	}
//}



//bool SkyMesh_Product_Factory_Checking(void)
//{
//	bool ret = false;
//	

//	if(g_PCBtest_state!=M_PCBA_TEST_INIT_STATE){
//		SkyMesh_Product_PCB_Check();
//		g_factorytest_state = M_FACTORYTEST_INIT_STATE; // 优先PCB测试
//	}
//	
//	if(g_factorytest_state!=M_FACTORYTEST_INIT_STATE){
//		SkyMesh_Product_device_Check();
//	}

//	
//	if(g_PCBtest_state!=M_PCBA_TEST_INIT_STATE || g_factorytest_state!=M_FACTORYTEST_INIT_STATE){
//		
//		#if 1
//		if(mIotManager.mSwitchManager.keymode && mIotManager.mSwitchManager.keyval){
//			if(mIotManager.mSwitchManager.keyval == 0x01){ // user switch
//				if(mIotManager.mSwitchManager.keymode == KEY_SHORTPRESS_MODE){ // 在此检测即可
//					SkyMesh_Product_device_Check_State(); // 触发整机厂测  
//				}else if(mIotManager.mSwitchManager.keymode == KEY_LONGPRESS_MODE){ // 在此检测即可
//					SkyBleMesh_Reset_timer(); // 防错重启  
//				}else if(mIotManager.mSwitchManager.keymode == KEY_SINGLE_MODE){ 
//					SkyMesh_Product_PCB_Check_Start();    // PCB开始检测
//				}
//			}else if(mIotManager.mSwitchManager.keyval == 0x02){ // PCB check switch
//				if(mIotManager.mSwitchManager.keymode == KEY_SINGLE_MODE){  // 这个首次需要在外面检测
//					SkyMesh_Product_PCB_Check_State(); // 触发PCB厂测
//				}
//			}
//			mIotManager.mSwitchManager.keymode = 0;
//			mIotManager.mSwitchManager.keyval = 0;
//		}		
//		#endif
//		
//		ret = true;
//	}

//	return ret;
//}
#endif

