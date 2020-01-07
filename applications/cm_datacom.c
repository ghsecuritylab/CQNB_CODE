#include "cm_datacom.h"
#include <string.h>
#include <stdio.h>


#define VERSION                 0x01
uint32_t cmcycle = 1000 * 60 * 10; // 1h ת����ms
uint32_t cmgpscycle = 1000 * 60 * 5;      //5min     //30min
uint32_t cmmotioncycle = 1000 * 60 * 5;   //20min 
uint16_t cmsample = 30; //��λs  //10s
float surface_angle = 12.0;     //�ر�Ƕ���ֵ float 4byte
float surface_dist = 10.0;      //�ر�������ֵ float 4byte
float wall_angle = 15.0;        //ǽ�ѷ�Ƕ���ֵ float 4byte
float wall_dist = 15.0;         //ǽ�ѷ�������ֵ�޸� float 4byte
float dmqx_angle = 15.0;        //������б�Ƕ���ֵ   float 4byte
float mudthreshold = 10.0;
uint16_t cmrain = 20;    //��λmm ��ֵ

uint8_t cmautos = 1,cmautolang = 0; //���ܱ���������Ĭ������ֵ(�ر����ܱ�������Դ,����������)
extern float monitor_value[2];
extern uint8_t bat_q;
extern void connect_nb_net(void);
extern void disconnect_nb_net(void);
extern uint8_t sel_thread;
uint16_t cm_soft_ver = 0x1000;       //����汾Ĭ��V1.000
uint8_t cm_hard_ver = 0x10;    // Ӳ���汾Ĭ��V1.0
uint8_t otaflag = 0;
uint32_t cm_packet_len = 0;
uint32_t cm_crc32 = 0;
uint32_t cm_rand_crc = 0;
uint32_t cm_packet_num = 0;
uint32_t cm_pack_index = 0;

struct rt_timer otatimer;
//static void cm_otadecodehandle(uint8_t *rev,uint16_t len);
//static void cm_otaencode_handle(cm_header_t head,cm_mainmsg_t msg,uint8_t *userdata);
extern void refreshkvalue();
//extern void nb_send_ota(char* msg, uint16_t len);
extern rt_thread_t tid1;
extern    rt_thread_t tid2;
extern	rt_thread_t tid3;
extern	rt_thread_t tid4;
    
 extern   rt_thread_t tid5;
extern	rt_thread_t tid6;
extern    rt_thread_t tid7;
#if USING_DBLH
struct cm_header cm_default_config = {
    VERSION,                  //version
	CM_EARTH_DEV,            //�ر��ѷ�����
    0x00,                   // spare byte
	cm_msg_up,              //����
    CM_DBLF_ID,            // SN
    0x0017,                 //23���ֽڳ���
	RT_NULL,
};
#endif
#if USING_QLH
struct cm_header cm_default_config = {
    VERSION,                  //version
	CM_WALL_DEV,            //�ر��ѷ�����
    0x00,                   // spare byte
	cm_msg_up,              //����
    CM_QLF_ID,            // SN
    0x0017,                 //23���ֽڳ���
	RT_NULL,
};
#endif
#if USING_DMQX
struct cm_header cm_default_config = {
    VERSION,                  //version
	CM_ANGLE_DEV,            //�ر��ѷ�����
    0x00,                   // spare byte
	cm_msg_up,              //����
    CM_DMQX_ID,            // SN
    0x0017,                 //23���ֽڳ���
	RT_NULL,
};
#endif
#if USING_MUD
struct cm_header cm_default_config = {
    VERSION,                  //version
	CM_MUD_DEV,            //�ر��ѷ�����
    0x00,                   // spare byte
	cm_msg_up,              //����
    CM_MUD_ID,            // SN
    0x0017,                 //23���ֽڳ���
	RT_NULL,
};
#endif
#if USING_RAIN
struct cm_header cm_default_config = {
    VERSION,                  //version
	CM_RAIN_DEV,            //�ر��ѷ�����
    0x00,                   // spare byte
	cm_msg_up,              //����
    CM_RAIN_ID,            // SN
    0x0017,                 //23���ֽڳ���
	RT_NULL,
};
#endif
#if USING_ALARMER
struct cm_header cm_default_config = {
    VERSION,                  //version
	CM_AUTO_DEV,            //�ر��ѷ�����
    0x00,                   // spare byte
	cm_msg_up,              //����
    CM_ANNU_ID,            // SN
    0x0017,                 //23���ֽڳ���
	RT_NULL,
};
#endif
struct cm_mainmsg config_data = {
		
        cm_heartbeat_up,       //�ϱ�������
		"120619",
		"102015",
		0,     //����λ��
		0,
		0,
		0,       //���
		0,
		0,
		0,       //����
		0,            //�����ٷֱ�
	    12,        //��ѹֵ
		153.41,      //gps
	    29.588,
	    133.41,      //����
	    29.588,
		24,          //csq
	    1,          //����
	    1,           //��λ
        0,           //���ܱ�������Դ���� 0���򿪵�Դ
	    0,           //���ܱ������������ز���������
        0,           //ota
        0,           //ota
        0,           //ota
        0,           //ota
	};

extern void nb_send(char* msg,uint16_t len);

static uint16_t cm_surface_encode(cm_header_t head,cm_mainmsg_t msg);
static uint16_t cm_wall_encode(cm_header_t head,cm_mainmsg_t msg);
static uint16_t cm_angle_encode(cm_header_t head,cm_mainmsg_t msg);
static uint16_t cm_rain_encode(cm_header_t head,cm_mainmsg_t msg);
static uint16_t cm_mud_encode(cm_header_t head,cm_mainmsg_t msg);
static uint16_t cm_auto_encode(cm_header_t head,cm_mainmsg_t msg);
static void cm_init(cm_header_t head,uint8_t *dat,uint16_t len);

static void cm_earth_decode(uint8_t *rev,uint16_t len);
static void cm_wall_decode(uint8_t *rev,uint16_t len);
static void cm_angle_decode(uint8_t *rev,uint16_t len);
static void cm_rain_decode(uint8_t *rev,uint16_t len);
static void cm_mud_decode(uint8_t *rev,uint16_t len);
static void cm_auto_decode(uint8_t *rev,uint16_t len);
//static void cm_angle_decode(uint8_t *rev,uint16_t len);
/*���豸���ͷֱ�����ݱ���*/
uint16_t cm_encode(cm_header_t head,cm_mainmsg_t msg)
{
	uint8_t lg=0;
	if(head->msg_mode==0x00 || head->msg_mode==0x01) //�豸������Ϣ
	{
		head->user_data = msg;       
		if(head->dev_type == CM_EARTH_DEV)
		{
			head->dev_type = CM_EARTH_DEV;
			lg = cm_surface_encode(head,msg);
		}
        if(head->dev_type == CM_WALL_DEV)
		{
			head->dev_type = CM_WALL_DEV;
			lg = cm_wall_encode(head,msg);
		}
		else if(head->dev_type == CM_ANGLE_DEV)
		{
			head->dev_type = CM_ANGLE_DEV;
			lg = cm_angle_encode(head,msg);
		}
		else if(head->dev_type == CM_RAIN_DEV)
		{
			head->dev_type = CM_RAIN_DEV;
			lg = cm_rain_encode(head,msg);
		}
		else if(head->dev_type == CM_MUD_DEV)
		{
			head->dev_type = CM_MUD_DEV;
			lg = cm_mud_encode(head,msg);
		}
		else if(head->dev_type == CM_AUTO_DEV)
		{
			head->dev_type = CM_AUTO_DEV;
			lg = cm_auto_encode(head,msg);
		}
		
    }

    return (lg + 14); 
}

/*�ر��ѷ���뺯��*/
static uint16_t cm_surface_encode(cm_header_t head,cm_mainmsg_t msg)
{
	uint8_t userdat[51] = {0};//�ֽڳ�����Ҫ���� ���51�ֽ�
	float f;
	uint32_t d;
	uint32_t c;
    
    if(msg->cmd == cm_modulestatus_up)   //�ϱ�ģ��״̬���ر�/ǽ�ѷ����ǣ�
	
	{
			head->data_len = 51;         // 48 + 3 (����1byte type(0xF6)  ,1byte datalen, n byte data 1 byte crc-8 )
            userdat[0] = 0xF6;  //�̶�����ʾΪNB�������
            userdat[1] = 0x2F;  //
            userdat[2] = cm_modulestatus_up;  // 01:�ϱ��ر�/ǽ�ѷ�ģ��״̬
			strcopy(&userdat[3],(uint8_t*)msg->date,7);
			strcopy(&userdat[10],(uint8_t*)msg->time,7);
			getfloat(&msg->dir_x,&userdat[17]);
			getfloat(&msg->dir_y,&userdat[21]);
			getfloat(&msg->dir_z,&userdat[25]);
			getfloat(&msg->angle_x,&userdat[29]);
			getfloat(&msg->angle_y,&userdat[33]);
			getfloat(&msg->angle_z,&userdat[37]);
			getfloat(&msg->dist,&userdat[41]);
			userdat[45] = msg->bat_per;
			getfloat(&msg->bat_vol,&userdat[46]);
		}
		else if(msg->cmd == cm_angle_up)     //�ϱ��Ƕ�ƫ�Ƹ澯��ֵ���ر�/ǽ�ѷ����ǣ�
		{
			f = surface_angle;   //�ر�Ƕ�ƫ���趨��ֵ
			head->data_len = 8;    // 5 + 3 + 14 = 22
            userdat[0] = 0xF6;  //�̶�����ʾΪNB�������
            userdat[1] = 0x05;  
            userdat[2] = cm_angle_up;  //�ϱ��Ƕ�ƫ�Ƹ澯��ֵ���ر�/ǽ�ѷ����ǣ�
			getfloat(&f,&userdat[3]);
		}
		else if(msg->cmd == cm_dist_up)      //�ϱ�����ƫ�Ƹ澯��ֵ���ر�/ǽ�ѷ����ǣ�
		{
			f = surface_dist;     //�ر������趨��ֵ
            head->data_len = 8;    // 5 + 3 + 14 = 22
            userdat[0] = 0xF6;  //�̶�����ʾΪNB�������
            userdat[1] = 0x05;  //
            userdat[2] = cm_dist_up;  //�ϱ�����ƫ�Ƹ澯��ֵ���ر�/ǽ�ѷ����ǣ�
			
			getfloat(&f,&userdat[3]);
		}
		else if(msg->cmd == cm_cycle_up)        //�ϱ��������ڣ��ر�/ǽ�ѷ����ǣ�
		{
			c = cmcycle;
			head->data_len = 16;        //13 + 3 +14 = 30
            userdat[0] = 0xF6;  //�̶�����ʾΪNB�������
            userdat[1] = 0x0D;  //
            userdat[2] = cm_cycle_up;  //�ϱ��������ڣ��ر�/ǽ�ѷ����ǣ�
            userdat[3] = cmgpscycle & 0x000000FF;
            userdat[4] = cmgpscycle>>8 & 0x000000FF;
            userdat[5] = cmgpscycle>>16 & 0x000000FF;
            userdat[6] = cmgpscycle>>24 & 0x000000FF;
            
            userdat[7] = cmmotioncycle & 0x000000FF;;
            userdat[8] = cmmotioncycle>>8 & 0x000000FF;
            userdat[9] = cmmotioncycle>>16 & 0x000000FF;
            userdat[10] = cmmotioncycle>>24 & 0x000000FF;
            
            userdat[11] = cmcycle & 0x000000FF;;
            userdat[12] = cmcycle>>8 & 0x000000FF;
            userdat[13] = cmcycle>>16 & 0x000000FF;
            userdat[14] = cmcycle>>24 & 0x000000FF;
		
		}
		else if(msg->cmd == cm_softversion_up )      //  �ϱ�����汾 ���ر�/ǽ�ѷ����ǣ�
		{   
			d = 0x101000;   //1000��ʾ����汾V1.000 10��ʾӲ���汾V1.0
			head->data_len = 7;      //4 + 3 + 14 = 21
            userdat[0] = 0xF6;  //�̶�����ʾΪNB�������
            userdat[1] = 0x04;  //
            userdat[2] = cm_softversion_up;  //  �ϱ�����汾 ���ر�/ǽ�ѷ����ǣ�
			userdat[3] = (cm_soft_ver>>8) & 0x00FF;
			userdat[4] = cm_soft_ver & 0x00FF;
			userdat[5] = cm_hard_ver;  //Ӳ���汾
		}
		else if(msg->cmd == cm_sample_up)       //  �ϱ�������� ���ر�/ǽ�ѷ����ǣ�
		{
			d = cmsample;   //2400(0x0960)      2400S С��ģʽ 0x00 0x60 0x0960
			head->data_len = 7;  //4 + 3 + 14 = 21
			userdat[0] = 0xF6;
            userdat[1] = 0x04;  //
            userdat[2] = cm_sample_up;  //  �ϱ�������� ���ر�/ǽ�ѷ����ǣ�
			userdat[3] = 0;   //0������
			userdat[4] = d & 0x00ff;
            userdat[5]= d>>8 & 0xff;
		}
		else if(msg->cmd == cm_heartbeat_up)        //  �ϱ������� ���ر�/ǽ�ѷ����ǣ�
		{
			head->data_len = 23;             //20 + 3 + 14 = 37
            userdat[0] = 0xF6;
            userdat[1] = 0x14;  //
            userdat[2] = cm_heartbeat_up;  //  �ϱ������� ���ر�/ǽ�ѷ����ǣ�
			getfloat(&msg->gps_lat,&userdat[3]);
			getfloat(&msg->gps_long,&userdat[7]);
			getfloat(&msg->bd_lat,&userdat[11]);
			getfloat(&msg->bd_long,&userdat[15]);
			userdat[19] = msg->bat_per;
			userdat[20] = msg->signal & 0xff;
			userdat[21] = msg->signal >>8 & 0xff;
			
		}
//        cm_otaencode_handle(head,msg,userdat);  
		cm_init(head,userdat,head->data_len);
        return 	head->data_len;  
}

/*ǽ�ѷ���뺯��*/
static uint16_t cm_wall_encode(cm_header_t head,cm_mainmsg_t msg)
{
	uint8_t userdat[51] = {0};//�ֽڳ�����Ҫ���� ���51�ֽ�
	float f;
	uint32_t d;
	uint32_t c;
    
    if(msg->cmd == cm_modulestatus_up)   //�ϱ�ģ��״̬���ر�/ǽ�ѷ����ǣ�
	
	{
			head->data_len = 51;         // 48 + 3 (����1byte type(0xF6)  ,1byte datalen, n byte data 1 byte crc-8 )
            userdat[0] = 0xF6;  //�̶�����ʾΪNB�������
            userdat[1] = 0x2F;  //
            userdat[2] = cm_modulestatus_up;  // 01:�ϱ��ر�/ǽ�ѷ�ģ��״̬
			strcopy(&userdat[3],(uint8_t*)msg->date,7);
			strcopy(&userdat[10],(uint8_t*)msg->time,7);
			getfloat(&msg->dir_x,&userdat[17]);
			getfloat(&msg->dir_y,&userdat[21]);
			getfloat(&msg->dir_z,&userdat[25]);
			getfloat(&msg->angle_x,&userdat[29]);
			getfloat(&msg->angle_y,&userdat[33]);
			getfloat(&msg->angle_z,&userdat[37]);
			getfloat(&msg->dist,&userdat[41]);
			userdat[45] = msg->bat_per;
			getfloat(&msg->bat_vol,&userdat[46]);
		}
		else if(msg->cmd == cm_angle_up)     //�ϱ��Ƕ�ƫ�Ƹ澯��ֵ��ǽ�ѷ����ǣ�
		{
			f = wall_angle;   //ǽ�Ƕ�ƫ���趨��ֵ
			head->data_len = 8;    // 5 + 3 + 14 = 22
            userdat[0] = 0xF6;  //�̶�����ʾΪNB�������
            userdat[1] = 0x05;  
            userdat[2] = cm_angle_up;  //�ϱ��Ƕ�ƫ�Ƹ澯��ֵ��ǽ�ѷ����ǣ�
			getfloat(&f,&userdat[3]);
		}
		else if(msg->cmd == cm_dist_up)      //�ϱ�����ƫ�Ƹ澯��ֵ��ǽ�ѷ����ǣ�
		{
			f = wall_dist;     //ǽ�����趨��ֵ
            head->data_len = 8;    // 5 + 3 + 14 = 22
            userdat[0] = 0xF6;  //�̶�����ʾΪNB�������
            userdat[1] = 0x05;  //
            userdat[2] = cm_dist_up;  //�ϱ�����ƫ�Ƹ澯��ֵ���ر�/ǽ�ѷ����ǣ�
			
			getfloat(&f,&userdat[3]);
		}
		else if(msg->cmd == cm_cycle_up)        //�ϱ��������ڣ��ر�/ǽ�ѷ����ǣ�
		{
			c = cmcycle;
			head->data_len = 16;        //13 + 3 +14 = 30
            userdat[0] = 0xF6;  //�̶�����ʾΪNB�������
            userdat[1] = 0x0D;  //
            userdat[2] = cm_cycle_up;  //�ϱ��������ڣ��ر�/ǽ�ѷ����ǣ�
            userdat[3] = cmgpscycle & 0x000000FF;
            userdat[4] = cmgpscycle>>8 & 0x000000FF;
            userdat[5] = cmgpscycle>>16 & 0x000000FF;
            userdat[6] = cmgpscycle>>24 & 0x000000FF;
            
            userdat[7] = cmmotioncycle & 0x000000FF;;
            userdat[8] = cmmotioncycle>>8 & 0x000000FF;
            userdat[9] = cmmotioncycle>>16 & 0x000000FF;
            userdat[10] = cmmotioncycle>>24 & 0x000000FF;
            
            userdat[11] = cmcycle & 0x000000FF;;
            userdat[12] = cmcycle>>8 & 0x000000FF;
            userdat[13] = cmcycle>>16 & 0x000000FF;
            userdat[14] = cmcycle>>24 & 0x000000FF;
		
		}
		else if(msg->cmd == cm_softversion_up )      //  �ϱ�����汾 ���ر�/ǽ�ѷ����ǣ�
		{   
			d = 0x101000;   //1000��ʾ����汾V1.000 10��ʾӲ���汾V1.0
			head->data_len = 7;      //4 + 3 + 14 = 21
            userdat[0] = 0xF6;  //�̶�����ʾΪNB�������
            userdat[1] = 0x04;  //
            userdat[2] = cm_softversion_up;  //  �ϱ�����汾 ���ر�/ǽ�ѷ����ǣ�
			userdat[3] = (cm_soft_ver>>8) & 0x00FF;
			userdat[4] = cm_soft_ver & 0x00FF;
			userdat[5] = cm_hard_ver;  //Ӳ���汾
		}
		else if(msg->cmd == cm_sample_up)       //  �ϱ�������� ���ر�/ǽ�ѷ����ǣ�
		{
			d = cmsample;   //2400(0x0960)      2400S С��ģʽ 0x00 0x60 0x0960
			head->data_len = 7;  //4 + 3 + 14 = 21
			userdat[0] = 0xF6;
            userdat[1] = 0x04;  //
            userdat[2] = cm_sample_up;  //  �ϱ�������� ���ر�/ǽ�ѷ����ǣ�
			userdat[3] = 0;   //0������
			userdat[4] = d & 0x00ff;
            userdat[5]= d>>8 & 0xff;
		}
		else if(msg->cmd == cm_heartbeat_up)        //  �ϱ������� ���ر�/ǽ�ѷ����ǣ�
		{
			head->data_len=23;             //20 + 3 + 14 = 37
            userdat[0] = 0xF6;
            userdat[1] = 0x14;  //
            userdat[2] = cm_heartbeat_up;  //  �ϱ������� ���ر�/ǽ�ѷ����ǣ�
			getfloat(&msg->gps_lat,&userdat[3]);
			getfloat(&msg->gps_long,&userdat[7]);
			getfloat(&msg->bd_lat,&userdat[11]);
			getfloat(&msg->bd_long,&userdat[15]);
			userdat[19] = msg->bat_per;
			userdat[20] = msg->signal & 0xff;
			userdat[21] = msg->signal >>8 & 0xff;
			
		}
//        cm_otaencode_handle(head,msg,userdat);  
		cm_init(head,userdat,head->data_len);
        return 	head->data_len;  
}

/*������б��Ǳ��뺯��*/
static uint16_t cm_angle_encode(cm_header_t head,cm_mainmsg_t msg)
{
	uint8_t userdat[49]={0};  //�ֽڳ�����Ҫ����
	float f;
	uint32_t d;
	uint32_t c;
    
	if(msg->cmd == cm_modulestatus_up)        //�ϱ�ģ��״̬��������б����ǣ�
	{
        head->data_len = 35; //info����31 ��cmd ��0xf6��CRC��datalen ��һ�ֽ�
        userdat[0] = 0xF6;  //�̶�����ʾΪNB�������
        userdat[1] = 0x20;  //info��cmd����
        userdat[2] = cm_modulestatus_up;  // 01:�ϱ�������б�����ģ��״̬
        strcopy(&userdat[3],(uint8_t*)msg->date,7);    //data
        strcopy(&userdat[10],(uint8_t*)msg->time,7);   //time
        getfloat(&msg->angle_x,&userdat[17]);
        getfloat(&msg->angle_y,&userdat[21]);
        getfloat(&msg->angle_z,&userdat[25]);
        
        userdat[29]=msg->bat_per;
        getfloat(&msg->bat_vol,&userdat[30]);
    }	
    else if(msg->cmd == cm_angle_up)     //�ϱ��Ƕ�ƫ�Ƹ澯��ֵ��������б����ǣ�
    {
        f = dmqx_angle;            //������,����ƽ̨�趨����ֵ
        head->data_len = 8;
        userdat[0] = 0xF6;  //�̶�����ʾΪNB�������
        userdat[1] = 0x05;  
        userdat[2] = cm_angle_up;  //�ϱ��Ƕ�ƫ�Ƹ澯��ֵ��������б����ǣ�
        getfloat(&f,&userdat[3]);
        
    }
    else if(msg->cmd == cm_cycle_up)       //�ϱ��������ڣ�������б����ǣ�
    {
        c = cmcycle;
        head->data_len = 16;
        userdat[0] = 0xF6;  //�̶�����ʾΪNB�������
        userdat[1] = 0x0D;  //
        userdat[2] = cm_cycle_up;  //�ϱ��������ڣ�������б����ǣ�
        userdat[3] = cmgpscycle & 0x000000FF;
        userdat[4] = cmgpscycle>>8 & 0x000000FF;
        userdat[5] = cmgpscycle>>16 & 0x000000FF;
        userdat[6] = cmgpscycle>>24 & 0x000000FF;
        
        userdat[7] = cmmotioncycle & 0x000000FF;;
        userdat[8] = cmmotioncycle>>8 & 0x000000FF;
        userdat[9] = cmmotioncycle>>16 & 0x000000FF;
        userdat[10] = cmmotioncycle>>24 & 0x000000FF;
        
        userdat[11] = cmcycle & 0x000000FF;;
        userdat[12] = cmcycle>>8 & 0x000000FF;
        userdat[13] = cmcycle>>16 & 0x000000FF;
        userdat[14] = cmcycle>>24 & 0x000000FF;

        
    }
    else if(msg->cmd == cm_softversion_up)      //  �ϱ�����汾 ��������б����ǣ�
    {   
//        d = 0x101000;   //1000��ʾ����汾V1.000 10��ʾӲ���汾V1.0
        head->data_len = 7;
        userdat[0] = 0xF6;  //�̶�����ʾΪNB�������
        userdat[1] = 0x04;  //
        userdat[2] = cm_softversion_up;  //  �ϱ�����汾 ��������б����ǣ�
        userdat[3] = (cm_soft_ver>>8) & 0x00FF;
        userdat[4] = cm_soft_ver & 0x00FF;
        userdat[5]= cm_hard_ver;  //Ӳ���汾
       
    }
    else if(msg->cmd == cm_sample_up)       //  �ϱ����������������б����ǣ�
    {
        d = cmsample;   //2400(0x0960)      2400S С��ģʽ 0x00 0x60 0x0960
        head->data_len = 7;
        userdat[0]=0xF6;
        userdat[1] = 0x04;  //
        userdat[2] = cm_sample_up;  //  �ϱ�������� ���ر�/ǽ�ѷ����ǣ�
        userdat[3]= 0;   //0������
        userdat[4]= d & 0x00ff;
        userdat[5]= d>>8 & 0xff;
        
    }
    else if(msg->cmd == cm_heartbeat_up)        //�ϱ������� ���ر�/ǽ�ѷ����ǣ�
    {
        head->data_len = 23;
        userdat[0] = 0xF6;
        userdat[1] = 0x14;  //
        userdat[2] = cm_heartbeat_up;  //  �ϱ������� ���ر�/ǽ�ѷ����ǣ�
        getfloat(&msg->gps_lat,&userdat[3]);
        getfloat(&msg->gps_long,&userdat[7]);
        getfloat(&msg->bd_lat,&userdat[11]);
        getfloat(&msg->bd_long,&userdat[15]);
        userdat[19]=msg->bat_per;
        userdat[20]=msg->signal & 0xff;
        userdat[21]=msg->signal >>8 & 0xff;
        
        
    }
//		head->cmd |= head->dev_type<<8;
//    cm_otaencode_handle(head,msg,userdat);    
    cm_init(head,userdat,head->data_len);
 	return 	head->data_len;  
}

/*�������뺯��*/
static uint16_t cm_rain_encode(cm_header_t head,cm_mainmsg_t msg)
{
	uint8_t userdat[37]= {0};
	float f;
	uint32_t d;
	uint32_t c;
//    uint32_t gps_up_cycle = 3000;            //GPS��������,4byte   3000ms = 3s = 0.05min
//    uint32_t motion_up_cycle = 0;         //�˶��ϱ�����,4byte
//    uint32_t up_cycle = 5*1000*60 ;                //�ϴ����,4byte    5min
	
    if(msg->cmd == cm_modulestatus_up)      //�����ϱ���Ϣ
    {
        head->data_len = 13;
        userdat[0] = 0xF6;  //�̶�����ʾΪNB�������
        userdat[1] = 0x0A;  //
        userdat[2] = cm_modulestatus_up;  // 01:�ϱ�������ģ��״̬
        getfloat(&msg->rain_n,&userdat[3]);
        userdat[7]=msg->bat_per;
        getfloat(&msg->bat_vol,&userdat[8]);
    }
    else if(msg->cmd == cm_rain_threshold_up)     //�ϱ��澯��ֵ (����) 2���ֽ�
    {
        uint16_t rain_level;
        rain_level = cmrain ;
        rain_level = rain_level * 10;//��������10������
//        rain_level = (uint16_t)f;
        head->data_len = 6;
        userdat[0] = 0xF6;  //�̶�����ʾΪNB�������
        userdat[1] = 0x03;  //
        userdat[2] = cm_rain_threshold_up;  //�澯��ֵ (����)
        userdat[3] = rain_level & 0xff;
        userdat[4] = rain_level>>8 & 0xff;  
//        getfloat(&f,&userdat[1]);
    }
    else if(msg->cmd == cm_rain_cycle_up)       //�ϱ��������ڣ������ƣ�
    {
        c = cmcycle;
        head->data_len = 16;
        userdat[0] = 0xF6;  //�̶�����ʾΪNB�������
        userdat[1] = 0x0D;  //
        userdat[2] = cm_rain_cycle_up;  //�ϱ��������ڣ��ر�/ǽ�ѷ����ǣ�
        userdat[3] = cmgpscycle & 0x000000FF;
        userdat[4] = cmgpscycle>>8 & 0x000000FF;
        userdat[5] = cmgpscycle>>16 & 0x000000FF;
        userdat[6] = cmgpscycle>>24 & 0x000000FF;
        
        userdat[7] = cmmotioncycle & 0x000000FF;;
        userdat[8] = cmmotioncycle>>8 & 0x000000FF;
        userdat[9] = cmmotioncycle>>16 & 0x000000FF;
        userdat[10] = cmmotioncycle>>24 & 0x000000FF;
        
        userdat[11] = cmcycle & 0x000000FF;;
        userdat[12] = cmcycle>>8 & 0x000000FF;
        userdat[13] = cmcycle>>16 & 0x000000FF;
        userdat[14] = cmcycle>>24 & 0x000000FF;
        
        
    }
    else if(msg->cmd == cm_rain_softversion_up)      //�ϱ�����汾 
    {   
//        d = 0x101000;   //1000��ʾ����汾V1.000 10��ʾӲ���汾V1.0
        head->data_len = 7;
        userdat[0] = 0xF6;  //�̶�����ʾΪNB�������
        userdat[1] = 0x04;  //
        userdat[2] = cm_rain_softversion_up;  //  �ϱ�����汾 ���ر�/ǽ�ѷ����ǣ�
        userdat[3] = (cm_soft_ver>>8) & 0x00FF;
        userdat[4]= cm_soft_ver & 0x00FF;
        
        userdat[5]= cm_hard_ver;  //Ӳ���汾
        
    }
    else if(msg->cmd == cm_rain_sample_up)       //����Ƶ��
    {
        d = cmsample;   //2400(0x0960)      2400S С��ģʽ 0x00 0x60 0x0960
        head->data_len = 7;  //4 + 3 + 14 = 21
        userdat[0]=0xF6;
        userdat[1] = 0x04;  //
        userdat[2] = cm_rain_sample_up;  //  �ϱ�������� ���ر�/ǽ�ѷ����ǣ�
        userdat[3]= 0;   //0������
        userdat[4]= d & 0x00ff;
        userdat[5]= d>>8 & 0xff;
        
    }
    else if(msg->cmd == cm_heartbeat_up)        //����
    {
        head->data_len = 23;             //20 + 3 + 14 = 37
        userdat[0] = 0xF6;
        userdat[1] = 0x14;  //
        userdat[2] = cm_heartbeat_up;  //  �ϱ������� 
        getfloat(&msg->gps_lat,&userdat[3]);
        getfloat(&msg->gps_long,&userdat[7]);
        getfloat(&msg->bd_lat,&userdat[11]);
        getfloat(&msg->bd_long,&userdat[15]);
        userdat[19]= msg->bat_per;
        userdat[20]= msg->signal & 0xff;
        userdat[21]= msg->signal >>8 & 0xff;
        
    }
//    cm_otaencode_handle(head,msg,userdat);
//    head->cmd |= head->dev_type<<8; 
    cm_init(head,userdat,head->data_len);
	return 	head->data_len;  
}

/*��λ���뺯��*/
static uint16_t cm_mud_encode(cm_header_t head,cm_mainmsg_t msg)
{
	uint8_t userdat[37]={0};
	float f;
	uint32_t d;
    if(msg->cmd == cm_modulestatus_up)      //�����ϱ���Ϣ
    {
        head->data_len = 13;
        userdat[0] = 0xF6;  //�̶�����ʾΪNB�������
        userdat[1] = 0x07;  //����Ϊ�׷������ĳ���Ϊ�����
        userdat[2] = cm_modulestatus_up;  // 01:�ϱ�ģ��״̬
        getfloat(&msg->mud_n,&userdat[3]);
        userdat[7]=msg->bat_per;
        getfloat(&msg->bat_vol,&userdat[8]);
    }
    else if(msg->cmd == cm_mud_threshold_up)    // �ϱ���λ�澯��ֵ
    {
        
        f = mudthreshold;
        head->data_len = 8;
        userdat[0] = 0xF6;
        userdat[1] = 0x05;
        userdat[2] = cm_mud_threshold_up;  // 01:�ϱ��ر�/ǽ�ѷ�ģ��״̬
        getfloat(&f,&userdat[3]);
    }
    else if(msg->cmd == cm_cycle_up)       //����  ��λms
    {
//        c = cmcycle;
        head->data_len = 16;
        userdat[0] = 0xF6;  //�̶�����ʾΪNB�������
        userdat[1] = 0x0D;  //
        userdat[2] = cm_cycle_up;  //�ϱ��������ڣ��ر�/ǽ�ѷ����ǣ�
        userdat[3] = cmgpscycle & 0x000000FF;
        userdat[4] = cmgpscycle>>8 & 0x000000FF;
        userdat[5] = cmgpscycle>>16 & 0x000000FF;
        userdat[6] = cmgpscycle>>24 & 0x000000FF;
        
        userdat[7] = cmmotioncycle & 0x000000FF;;
        userdat[8] = cmmotioncycle>>8 & 0x000000FF;
        userdat[9] = cmmotioncycle>>16 & 0x000000FF;
        userdat[10] = cmmotioncycle>>24 & 0x000000FF;
        
        userdat[11] = cmcycle & 0x000000FF;;
        userdat[12] = cmcycle>>8 & 0x000000FF;
        userdat[13] = cmcycle>>16 & 0x000000FF;
        userdat[14] = cmcycle>>24 & 0x000000FF;

        
    }
    else if(msg->cmd == cm_softversion_up)      //�ϱ���λ����汾
    {   
//        d = 0x101000;   //1000��ʾ����汾V1.000 10��ʾӲ���汾V1.0
        head->data_len = 7;
        userdat[0] = 0xF6;  //�̶�����ʾΪNB�������
        userdat[1] = 0x04;  //
        userdat[2] = cm_softversion_up;  //  �ϱ�����汾 ���ر�/ǽ�ѷ����ǣ�
        
        userdat[3] = (cm_soft_ver>>8) & 0x00FF;
        userdat[4]= cm_soft_ver & 0x00FF;
        
        userdat[5]= cm_hard_ver;  //Ӳ���汾 //Ӳ���汾
        
    }
    else if(msg->cmd == cm_sample_up)       //  �ϱ�������� ���ر�/ǽ�ѷ����ǣ�
    {
        d = cmsample;    //2400(0x0960)      2400S С��ģʽ 0x00 0x60 0x0960
        head->data_len = 7;
        userdat[0]=0xF6;
        userdat[1] = 0x04;  //
        userdat[2] = cm_sample_up;  //  �ϱ�������� ���ر�/ǽ�ѷ����ǣ�
        userdat[3]= 0;   //0������
        userdat[4]= d & 0x00ff;
        userdat[5]= d>>8 & 0xff;
    
    }
    else if(msg->cmd == cm_heartbeat_up)        //����
    {
        head->data_len = 23;
        userdat[0] = 0xF6;
        userdat[1] = 0x14;  //
        userdat[2] = cm_heartbeat_up;  //  �ϱ������� ���ر�/ǽ�ѷ����ǣ�
        getfloat(&msg->gps_lat,&userdat[3]);
        getfloat(&msg->gps_long,&userdat[7]);
        getfloat(&msg->bd_lat,&userdat[11]);
        getfloat(&msg->bd_long,&userdat[15]);
        userdat[19]=msg->bat_per;
        userdat[20]=msg->signal & 0xff;
        userdat[21]=msg->signal >>8 & 0xff;
        
    }
//    cm_otaencode_handle(head,msg,userdat);
    cm_init(head,userdat,head->data_len);
	return 	head->data_len;  
}

/*�����豸���뺯��*/
static uint16_t cm_auto_encode(cm_header_t head,cm_mainmsg_t msg)
{
	uint8_t userdat[37]={0};
	uint32_t d;
	uint32_t c;
//    uint32_t gps_up_cycle = 3000;            //GPS��������,4byte   3000ms = 3s = 0.05min
//    uint32_t motion_up_cycle = 0;         //�˶��ϱ�����,4byte
//    uint32_t up_cycle = 5*1000*60 ;                //�ϴ����,4byte    5min
	
    if(msg->cmd == cm_modulestatus_up)      //�����ϱ���Ϣ
    {
        head->data_len = 6;
        userdat[0] = 0xF6;  //�̶�����ʾΪNB�������
        userdat[1] = 0x03;  //
        userdat[2] = cm_modulestatus_up;  // 01:�ϱ�״̬
        userdat[3] = msg->auto_s;
        userdat[4] = msg->auto_lang;
    }
    else if(msg->cmd == cm_annu_up)     //�ϱ����ܱ��������������ܱ�������
    {
        head->data_len = 6;
        userdat[0] = 0xF6;  //�̶�����ʾΪNB�������
        userdat[1] = 0x03;  //
        userdat[2] = cm_annu_up;  // 01:�ϱ�״̬
       
        userdat[3] = msg->auto_s;
        userdat[4] = msg->auto_lang;
    }
    else if(msg->cmd == cm_annu_cycle_up)       //�ϱ��������ڵ�λms
    {
        c = cmcycle;
        head->data_len = 16;        //13 + 3 +14 = 30
        userdat[0] = 0xF6;  //�̶�����ʾΪNB�������
        userdat[1] = 0x0D;  //
        userdat[2] = cm_annu_cycle_up;  //�ϱ��������ڣ��ر�/ǽ�ѷ����ǣ�
        userdat[3] = cmgpscycle & 0x000000FF;
        userdat[4] = cmgpscycle>>8 & 0x000000FF;
        userdat[5] = cmgpscycle>>16 & 0x000000FF;
        userdat[6] = cmgpscycle>>24 & 0x000000FF;
        
        userdat[7] = cmmotioncycle & 0x000000FF;;
        userdat[8] = cmmotioncycle>>8 & 0x000000FF;
        userdat[9] = cmmotioncycle>>16 & 0x000000FF;
        userdat[10] = cmmotioncycle>>24 & 0x000000FF;
        
        userdat[11] = cmcycle & 0x000000FF;;
        userdat[12] = cmcycle>>8 & 0x000000FF;
        userdat[13] = cmcycle>>16 & 0x000000FF;
        userdat[14] = cmcycle>>24 & 0x000000FF;
  
        
    }
    else if(msg->cmd == cm_annu_softversion_up)      //�汾
    {   
//        d = 0x101000;   //1000��ʾ����汾V1.000 10��ʾӲ���汾V1.0
        head->data_len = 7;      //4 + 3 + 14 = 21
        userdat[0] = 0xF6;  //�̶�����ʾΪNB�������
        userdat[1] = 0x04;  //
        userdat[2] = cm_annu_softversion_up;  //  �ϱ�����汾 ���ر�/ǽ�ѷ����ǣ�
        userdat[3] = (cm_soft_ver>>8) & 0x00FF;
        userdat[4]= cm_soft_ver & 0x00FF;
        
        userdat[5]= cm_hard_ver;  //Ӳ���汾
       
    }
    else if(msg->cmd == cm_annu_sample_up)       //�ϱ�������� (���ܱ�����)
    {
        d = cmsample;   //2400(0x0960)      2400S С��ģʽ 0x00 0x60 0x0960
        head->data_len = 7;  //4 + 3 + 14 = 21
        userdat[0] = 0xF6;
        userdat[1] = 0x04;  //
        userdat[2] = cm_annu_sample_up;  //  �ϱ�������� ���ر�/ǽ�ѷ����ǣ�
        userdat[3]= 0;   //0������
        userdat[4]= d & 0x00ff;
        userdat[5]= d>>8 & 0xff;
    }
    else if(msg->cmd == cm_heartbeat_up)        //����
    {
        head->data_len = 23;
        userdat[0] = 0xF6;
        userdat[1] = 0x14;  //
        userdat[2] = cm_heartbeat_up;  //
        
        getfloat(&msg->gps_lat,&userdat[3]);
        getfloat(&msg->gps_long,&userdat[7]);
        getfloat(&msg->bd_lat,&userdat[11]);
        getfloat(&msg->bd_long,&userdat[15]);
        userdat[19] = msg->bat_per;
        userdat[20] = msg->signal & 0xff;
        userdat[21] = msg->signal >>8 & 0xff;
        
    }
    else if(msg->cmd == cm_client_request_otadata)        //�豸�������������������
    {
        head->data_len = 12;
        userdat[0] = 0xF6;
        userdat[1] = 0x09;  //
        userdat[2] = cm_client_request_otadata;  //  0xF2
        
        userdat[3] = msg->pack_index & 0x000000FF;                  
        userdat[4] = msg->pack_index>>8 & 0x000000FF;
        userdat[5] = msg->pack_index>>16 & 0x000000FF;
        userdat[6] = msg->pack_index>>24 & 0x000000FF; //index��䣨msg->pack_index++��
        
        userdat[7] = msg->pack_num & 0x000000FF;             
        userdat[8] = msg->pack_num>>8 & 0x000000FF;
        userdat[9] = msg->pack_num>>16 & 0x000000FF;
        userdat[10] = msg->pack_num>>24 & 0x000000FF;    // ��ǰ����ְ��������ְ�����һֱΪ1,ÿ������һ������
   
    }
    else if(msg->cmd == cm_ota_status_up)        // �豸�ϱ�Զ������״̬
    {
        head->data_len = 9;
        userdat[0] = 0xF6;
        userdat[1] = 0x06;  //
        userdat[2] = cm_ota_status_up;  //  0xF0
        userdat[3] = msg->ota_lab;     // ota_lab ��1���豸������������������ʧ�ܣ�2�������������ݰ�����ɹ� 
        userdat[4] = msg->ota_sum & 0x000000FF;      //ota_sum
        userdat[5] = msg->ota_sum>>8 & 0x000000FF;             
        userdat[6] = msg->ota_sum>>16 & 0x000000FF;
        userdat[7] = msg->ota_sum>>24 & 0x000000FF; //cm_rand_crc:�ְ����ۼӺ�    
    }
//    cm_otaencode_handle(head,msg,userdat);
//    head->cmd |= head->dev_type<<8; 
    cm_init(head,userdat,head->data_len);
	return 	head->data_len;  
}

static void cm_init(cm_header_t head,uint8_t *dat,uint16_t len)       //��װ��Ϣ len = �ڶ�������ݽṹ�ĳ��� dat:�ڶ������ݽṹ
{
	memset(cm_senddata,0,512);
	cm_senddata[cm_head_que] = head->version;
    cm_senddata[cm_count_que] = head->dev_type;
    cm_senddata[cm_spare_byte] = head->spare;
	cm_senddata[cm_data_type_que] = head->msg_mode;
	
	strcopy(&cm_senddata[cm_sn_que],(uint8_t*)head->dev_id,8);
	
	cm_senddata[cm_datalen1_que] = head->data_len & 0x00ff;            //2�ֽڵ����ݳ���
	cm_senddata[cm_datalen1_que + 1] = (head->data_len>>8) & 0xff;

//	strcopy(&cm_senddata[cm_nb_type_que],dat,len);  //ȥ������CRCУ���
	strcopy(&cm_senddata[cm_nb_type_que],dat,(len - 1));  //ȥ������CRCУ���
	cm_senddata[cm_nb_type_que+(len - 1)] = crc8_table(&cm_senddata[cm_nb_type_que],len-1);	 //��0xF6��ʼ 
}

uint8_t crc;
void cm_decode(uint16_t len,uint8_t *rev)
{
	

    if(rev[cm_data_type_que] == 0x03)//03:����
	{   
			crc = crc8_table(&rev[cm_nb_type_que],(len-cm_nb_type_que)-1);     //CRCУ���0xF6��ʼ
			if(crc == rev[len-1])          //crcУ��
			{
				if(strncmp((char*)&rev[cm_sn_que],(char*)&CM_DBLF_ID,8)==0)     //�ر��ѷ�
				{
					cm_earth_decode(rev,len);
				}
                else if(strncmp((char*)&rev[cm_sn_que],(char*)&CM_QLF_ID,8)==0) //ǽ�ѷ�
				{
					cm_wall_decode(rev,len);
				}
				else if(strncmp((char*)&rev[cm_sn_que],(char*)&CM_DMQX_ID,8)==0)
				{
					cm_angle_decode(rev,len);
				}
				else if(strncmp((char*)&rev[cm_sn_que],(char*)&CM_RAIN_ID,8)==0)
				{
					cm_rain_decode(rev,len);
				}
				else if(strncmp((char*)&rev[cm_sn_que],(char*)&CM_MUD_ID,8)==0)                 
				{
					cm_mud_decode(rev,len);
				}
				else if(strncmp((char*)&rev[cm_sn_que],(char*)&CM_ANNU_ID,8)==0)  
				{
					cm_auto_decode(rev,len);
				}
                
			}
			else
			{
				rt_kprintf("CRC error\n");
			}
	    }
		else
		{
			rt_kprintf("ID error\n");
		}
}


/*�ر��ѷ���뺯��*/
static void cm_earth_decode(uint8_t *rev,uint16_t len)
{
	float *p;
	uint32_t *i;
	uint16_t *d;
	uint8_t *j;
    uint8_t angle[4] = {0};
    uint8_t dist[4] = {0};
    rt_thread_t ctd;
    ctd = rt_thread_find("BC35-G");
	if(rev[cm_data_type_que] == 0x03)    //����
	{
		if(rev[cm_cmd_que] == cm_angle_set)    //���ýǶ�ƫ�Ƹ澯��ֵ���ر�/ǽ�ѷ����ǣ�
		{
			p = (float *)&rev[cm_info_que];     //��Ӷ�Ӧ����ֵ��ȫ�ֱ������߶�ȡȫ�ֱ���ֵ
            strcopy(angle,&rev[cm_info_que],4);    //Ŀ����Ϊ���������4�ֽڶ�������
            p = (float*)angle;
            surface_angle  = *p;   //��ƽֵ̨�´���ģ�顣
            rev[cm_cmd_que] = cm_angle_up;  //��Ϊ�ϱ��Ƕ�ƫ�Ƹ澯��ֵ���ر�/ǽ�ѷ����ǣ�
            refreshkvalue();
		}
        
		else if(rev[cm_cmd_que] == cm_dist_set) //��������ƫ�Ƹ澯��ֵ���ر�/ǽ�ѷ����ǣ�
		{
			p = (float *)&rev[cm_info_que];
            strcopy(dist,&rev[cm_info_que],4);   //Ŀ����Ϊ���������4�ֽڶ�������
            p = (float*)dist;
            surface_dist  = *p;                 //��ƽֵ̨�´���ģ�顣
            rev[cm_cmd_que] = cm_dist_up;      //�ϱ�����ƫ�Ƹ澯��ֵ���ر�/ǽ�ѷ����ǣ�
            refreshkvalue();
		}
		else if(rev[cm_cmd_que] == cm_cycle_set)   //�����������ڣ��ر�/ǽ�ѷ����ǣ�
		{
			i = (uint32_t *)&rev[cm_info_que];
            cmgpscycle = *i;
			rt_kprintf("GPS�ɼ�����=%d ms\n",*i);
            i = (uint32_t *)&rev[cm_info_que + 4];
            cmmotioncycle = *i;
            rt_kprintf("�˶��ϱ�����=%d ms\n",*i);
			i = (uint32_t *)&rev[cm_info_que + 8];
            cmcycle = *i;
			rt_kprintf("�ϴ����=%d ms\n",*i);
            rev[cm_cmd_que] = cm_cycle_up;    //�ϱ��������ڣ��ر�/ǽ�ѷ����ǣ�
            refreshkvalue();
            rt_thread_resume(ctd);
		}
		else if(rev[cm_cmd_que] == cm_sample_set) //���ò������ ���ر�/ǽ�ѷ����ǣ�
		{ 
			j = (uint8_t *)&rev[cm_info_que];
            if(*j == 0)
            {
                rt_kprintf("����ʱ�䵥λΪ��");    
            }
			d = (uint16_t *)&rev[cm_info_que+1];
            cmsample = *d;
			rt_kprintf("�������=%d\n",*d);
            rev[cm_cmd_que] = cm_sample_up;
            refreshkvalue();
		}
	}
//	cm_otadecodehandle(rev,len);   //OTA	
	rev[cm_data_type_que] = 0x00;   //��Ϊ����
	rev[len-1]= crc8_table(&rev[cm_nb_type_que],(len-cm_nb_type_que)-1);
	if((rev[cm_cmd_que] != 0xF0) && (rev[cm_cmd_que] != 0xF1) && (rev[cm_cmd_que] != 0xF2) && (rev[cm_cmd_que] != 0xF3))
    {
        nb_send((char*)rev,len);  //�ϱ�����(��OTAʱ�������յ����ݰ��ϱ���ƽ̨)    
    }
}

/*ǽ���ѷ���뺯��*/
static void cm_wall_decode(uint8_t *rev,uint16_t len)
{
	float *p;
	uint32_t *i;
	uint16_t *d;
	uint8_t *j;
    uint8_t angle[4] = {0};
    uint8_t dist[4] = {0};
	if(rev[cm_data_type_que] == 0x03)    //����
	{
		if(rev[cm_cmd_que] == cm_angle_set)    //���ýǶ�ƫ�Ƹ澯��ֵ���ر�/ǽ�ѷ����ǣ�
		{
			p = (float *)&rev[cm_info_que];     //��Ӷ�Ӧ����ֵ��ȫ�ֱ������߶�ȡȫ�ֱ���ֵ
            strcopy(angle,&rev[cm_info_que],4);    //Ŀ����Ϊ���������4�ֽڶ�������
            p = (float*)angle;
            wall_angle  = *p;   //��ƽֵ̨�´���ģ�顣
            rev[cm_cmd_que] = cm_angle_up;  //��Ϊ�ϱ��Ƕ�ƫ�Ƹ澯��ֵ���ر�/ǽ�ѷ����ǣ�
            refreshkvalue();
		}
        
		else if(rev[cm_cmd_que] == cm_dist_set) //��������ƫ�Ƹ澯��ֵ���ر�/ǽ�ѷ����ǣ�
		{
			p = (float *)&rev[cm_info_que];
            strcopy(dist,&rev[cm_info_que],4);   //Ŀ����Ϊ���������4�ֽڶ�������
            p = (float*)dist;
            wall_dist  = *p;                 //��ƽֵ̨�´���ģ�顣
            rev[cm_cmd_que] = cm_dist_up;      //�ϱ�����ƫ�Ƹ澯��ֵ���ر�/ǽ�ѷ����ǣ�
            refreshkvalue();
		}
		else if(rev[cm_cmd_que] == cm_cycle_set)   //�����������ڣ��ر�/ǽ�ѷ����ǣ�
		{
			i = (uint32_t *)&rev[cm_info_que];
            cmgpscycle = *i;
			rt_kprintf("GPS�ɼ�����=%d ms\n",*i);
            i = (uint32_t *)&rev[cm_info_que + 4];
            cmmotioncycle = *i;
            rt_kprintf("�˶��ϱ�����=%d ms\n",*i);
			i = (uint32_t *)&rev[cm_info_que + 8];
            cmcycle = *i;
			rt_kprintf("�ϴ����=%d ms\n",*i);
            rev[cm_cmd_que] = cm_cycle_up;    //�ϱ��������ڣ��ر�/ǽ�ѷ����ǣ�
            refreshkvalue();
		}
		else if(rev[cm_cmd_que] == cm_sample_set) //���ò������ ���ر�/ǽ�ѷ����ǣ�
		{ 
			j = (uint8_t *)&rev[cm_info_que];
            if(*j == 0)
            {
                rt_kprintf("����ʱ�䵥λΪ��");    
            }
			d = (uint16_t *)&rev[cm_info_que+1];
            cmsample = *d;
			rt_kprintf("�������=%d\n",*d);
            rev[cm_cmd_que] = cm_sample_up;
            refreshkvalue();
		}
	}
//	cm_otadecodehandle(rev,len);   //OTA	
	rev[cm_data_type_que] = 0x00;   //��Ϊ����
	rev[len-1]= crc8_table(&rev[cm_nb_type_que],(len-cm_nb_type_que)-1);
	if((rev[cm_cmd_que] != 0xF0) && (rev[cm_cmd_que] != 0xF1) && (rev[cm_cmd_que] != 0xF2) && (rev[cm_cmd_que] != 0xF3))
    {
        nb_send((char*)rev,len);  //�ϱ�����(��OTAʱ�������յ����ݰ��ϱ���ƽ̨)    
    }
}
/*��ǽ��뺯��*/
static void cm_angle_decode(uint8_t *rev,uint16_t len)
{
	float *p;
	uint32_t *i;
	uint16_t *d;
	uint8_t *j;
    uint8_t angle[4];
	if(rev[cm_data_type_que] == 0x03)    //����
	{
//		rev[cm_msg_que]=cm_msg_ask;
		if(rev[cm_cmd_que] == cm_angle_set)    //���ýǶ�ƫ�Ƹ澯��ֵ��������б����ǣ�
		{
			p = (float *)&rev[cm_info_que];//��Ӷ�Ӧ����ֵ��ȫ�ֱ������߶�ȡȫ�ֱ���ֵ
            strcopy(angle,&rev[cm_info_que],4);    //Ŀ����Ϊ���������4�ֽڶ�������
            p = (float*)angle;
            dmqx_angle  = *p;   //��ƽֵ̨�´���ģ�顣
            
            rev[cm_cmd_que] = cm_angle_up;  //��Ϊ�ϱ��Ƕ�ƫ�Ƹ澯��ֵ��������б����ǣ�
            refreshkvalue();
		}
		else if(rev[cm_cmd_que] == cm_cycle_set)  //  �����������ڣ�������б����ǣ�
		{
			i = (uint32_t *)&rev[cm_info_que];
            cmgpscycle = *i;
			rt_kprintf("GPS�ɼ�����=%d ms\n",*i);
			i = (uint32_t *)&rev[cm_info_que + 4];
            cmmotioncycle = *i;
            rt_kprintf("�˶��ϱ�����=%d ms\n",*i);
            i = (uint32_t *)&rev[cm_info_que + 8];
            cmcycle = *i;
			rt_kprintf("�ϴ����=%d ms\n",*i);
            rev[cm_cmd_que] = cm_cycle_up;    //�ϱ��������ڣ�������б����ǣ�
            refreshkvalue();
		}
		else if(rev[cm_cmd_que] == cm_sample_set)   //���ò������ ��������б����ǣ�
		{ 
			j = (uint8_t *)&rev[cm_info_que];
            if(*j == 0)
            {
                rt_kprintf("����ʱ�䵥λΪ��");    
            }
			d = (uint16_t *)&rev[cm_info_que + 1];
            cmsample = *d;
			rt_kprintf("�������=%d\n",*d);
            rev[cm_cmd_que] = cm_sample_up;    //�ϱ����������������б����ǣ�
            refreshkvalue();
		}
	}
//	cm_otadecodehandle(rev,len);   //OTA	
    rev[cm_data_type_que] = 0x00;   //��Ϊ����
	rev[len-1]= crc8_table(&rev[cm_nb_type_que],(len-cm_nb_type_que)-1);
    if((rev[cm_cmd_que] != 0xF0) && (rev[cm_cmd_que] != 0xF1) && (rev[cm_cmd_que] != 0xF2) && (rev[cm_cmd_que] != 0xF3))
    {
        nb_send((char*)rev,len);  //�ϱ�����(��OTAʱ�������յ����ݰ��ϱ���ƽ̨)    
    }
	
}

/*�������뺯��*/
static void cm_rain_decode(uint8_t *rev,uint16_t len)
{
	float *p;
	uint32_t *i;
	uint16_t *d;
	uint8_t *j;
    
    uint8_t rain[4];
	if(rev[cm_data_type_que] == 0x03)    //����
	{
//		rev[cm_msg_que]=cm_msg_ask;
		if(rev[cm_cmd_que] == cm_rain_threshold_set)    //���ø澯��ֵ (����) ��������10������  2���ֽ�
		{
            cmrain = rev[cm_info_que] | (rev[cm_info_que + 1] << 8);  //��λ��ǰ����λ�ں�
//			p = (float *)&rev[cm_info_que];//��Ӷ�Ӧ����ֵ��ȫ�ֱ������߶�ȡȫ�ֱ���ֵ
//            strcopy(rain,&rev[cm_info_que],4);    //Ŀ����Ϊ���������4�ֽڶ�������
//            p = (float*)rain;
//            cmrain = *p;
            cmrain = cmrain/10 ; //��С10��(��Ϊ�ϱ�ʱҪ�Ŵ�10������һ���ϱ���ֵҲ��Ŵ�10��)
            rev[cm_cmd_que] = cm_rain_threshold_up;  //��Ϊ�ϱ��澯��ֵ����������������10������
            refreshkvalue();
           
            
		}
		else if(rev[cm_cmd_que] == cm_rain_cycle_set)
		{
			i = (uint32_t *)&rev[cm_info_que];
            cmgpscycle = *i;
			rt_kprintf("GPS�ɼ�����=%d ms\n",*i);
            i = (uint32_t *)&rev[cm_info_que + 4];
            cmmotioncycle = *i;
            rt_kprintf("�˶��ϴ����=%d ms\n",*i);
			i = (uint32_t *)&rev[cm_info_que + 8];
            cmcycle = *i;
			rt_kprintf("�ϴ����=%d ms\n",*i);
            rev[cm_cmd_que] = cm_rain_cycle_up;    //�ϱ��������ڣ��ر�/ǽ�ѷ����ǣ�
            refreshkvalue();
		}
		else if(rev[cm_cmd_que] == cm_rain_sample_set)   //���ò��������������
		{ 
			j = (uint8_t *)&rev[cm_info_que];
            if(*j == 0)
            {
                rt_kprintf("����ʱ�䵥λΪ��");    
            }
			d = (uint16_t *)&rev[cm_info_que + 1];
            cmsample = *d;
			rt_kprintf("����ʱ��=%d\n",*d);
            rev[cm_cmd_que] = cm_rain_sample_up;    //�ϱ����������������
            refreshkvalue();
		}
	}
//    cm_otadecodehandle(rev,len);   //OTA	
	rev[cm_data_type_que] = 0x00;   //��Ϊ����
	rev[len-1]= crc8_table(&rev[cm_nb_type_que],(len-cm_nb_type_que)-1);
    
	if((rev[cm_cmd_que] != 0xF0) && (rev[cm_cmd_que] != 0xF1) && (rev[cm_cmd_que] != 0xF2) && (rev[cm_cmd_que] != 0xF3))
    {
        nb_send((char*)rev,len);  //�ϱ�����(��OTAʱ�������յ����ݰ��ϱ���ƽ̨)    
    }    
}

/*��λ���뺯��*/
static void cm_mud_decode(uint8_t *rev,uint16_t len)
{
	float *p;
	uint32_t *i;
	uint16_t *d;
	uint8_t *j;
    uint8_t mud[4];
	if(rev[cm_data_type_que] == 0x03)    //����
	{
//		rev[cm_msg_que]=cm_msg_ask;
		if(rev[cm_cmd_que] == cm_mud_threshold_set)    //������λ�澯��ֵ1���ֽ�
		{
			p = (float *)&rev[cm_info_que];//��Ӷ�Ӧ����ֵ��ȫ�ֱ������߶�ȡȫ�ֱ���ֵ
            strcopy(mud,&rev[cm_info_que],2);    //Ŀ����Ϊ���������4�ֽڶ�������
            p = (float*)mud;
            mudthreshold  = *p;   //��ƽֵ̨�´���ģ�顣
            rev[cm_cmd_que] = cm_mud_threshold_up;  //��Ϊ�ϱ���λ�澯��ֵ
             refreshkvalue();
		}
		else if(rev[cm_cmd_que] == cm_cycle_set) //�����������ڣ���λ��
		{
			i = (uint32_t *)&rev[cm_info_que];
            cmgpscycle = *i;
			rt_kprintf("GPS�ɼ�����=%d ms\n",*i);
            i = (uint32_t *)&rev[cm_info_que + 4];
            cmmotioncycle = *i;
            rt_kprintf("�˶��ϱ�����=%d ms\n",*i);
			i = (uint32_t *)&rev[cm_info_que + 8];
            cmcycle = *i;
			rt_kprintf("�ϴ����=%d ms\n",*i);
            rev[cm_cmd_que] = cm_cycle_up;    //�ϱ��������ڣ���λ��
            refreshkvalue();
		}
		else if(rev[cm_cmd_que] == cm_sample_set) //���ò������ ����λ�� 
		{ 
			j = (uint8_t *)&rev[cm_info_que];
             if(*j == 0)
            {
                rt_kprintf("����ʱ�䵥λΪ��");    
            }
//			rt_kprintf("����ʱ�䵥λ=%x\n",*j);
			d = (uint16_t *)&rev[cm_info_que + 1];
            cmsample = *d;
			rt_kprintf("�������=%d\n",*d);
            rev[cm_cmd_que] = cm_sample_up;    //�ϱ������������λ��
            refreshkvalue();
		}
	}
//	cm_otadecodehandle(rev,len);   //OTA	
	rev[cm_data_type_que] = 0x00;   //��Ϊ����
	rev[len-1]= crc8_table(&rev[cm_nb_type_que],(len-cm_nb_type_que)-1);
	if((rev[cm_cmd_que] != 0xF0) && (rev[cm_cmd_que] != 0xF1) && (rev[cm_cmd_que] != 0xF2) && (rev[cm_cmd_que] != 0xF3))
    {
        nb_send((char*)rev,len);  //�ϱ�����(��OTAʱ�������յ����ݰ��ϱ���ƽ̨)    
    } 
}

/*�����豸���뺯��*/
extern void read_data(uint8_t *data,uint8_t size);
uint8_t vlum[8] = {0x7E,0xFF,0x06,0x05,0x00,0x00,0x00,0xEF};    //������
uint8_t music[8] = {0x7E,0xFF,0x06,0x08,0x00,0x00,0x01,0xEF};
uint8_t close_music[8] = {0x7E,0xFF,0x06,0x16,0x00,0x00,0x00,0xEF};  //ֹͣ����
uint8_t close_shark[8] = {0x7E,0xFF,0x06,0x3A,0x00,0x00,0x01,0xEF};   //���ƹ�
static void cm_auto_decode(uint8_t *rev,uint16_t len)
{
	uint32_t *i;
	uint16_t *d;
	uint8_t *power_switch;
    uint8_t *voice;
    uint8_t *j;
	if(rev[cm_data_type_que] == 0x03)    //����
	{
//		rev[cm_msg_que]=cm_msg_ask;
		if(rev[cm_cmd_que] == cm_annu_set)    //�������ܱ��������������ܱ�������
		{
			j = (uint8_t *)&rev[cm_info_que];//��Ӷ�Ӧ����ֵ��ȫ�ֱ������߶�ȡȫ�ֱ���ֵ
            cmautos = *j;
			j = (uint8_t *)&rev[cm_info_que + 1];
            cmautolang = *j;
            if(cmautos == 0 && cmautolang == 0)     //�򿪵�Դ,�ر�����
			{
				read_data(close_music,8); //�ر�����
				rt_thread_delay(100);     
				read_data(close_shark,8); //�ر������
			}
			else if(cmautos == 0 && cmautolang != 0)
			{
				music[6] = cmautolang;
				read_data(music,8);  //��������
			}
            rev[cm_cmd_que] = cm_annu_up;  //�ϱ����ܱ��������������ܱ�������
            refreshkvalue();
		}
		else if(rev[cm_cmd_que] == cm_annu_cycle_set) //������������ (���ܱ�����)
		{
			i = (uint32_t *)&rev[cm_info_que];
            cmgpscycle = *i;
			rt_kprintf("GPS�ɼ�����=%d ms\n",*i);
            i = (uint32_t *)&rev[cm_info_que + 4];
            cmmotioncycle = *i;
            rt_kprintf("�˶��ϱ�����=%d ms\n",*i);
			i = (uint32_t *)&rev[cm_info_que + 8];
            cmcycle = *i;
			rt_kprintf("�ϴ����=%d ms\n",*i);
            rev[cm_cmd_que] = cm_annu_cycle_up;    //�ϱ��������� (���ܱ�����)
            refreshkvalue();
		}
		else if(rev[cm_cmd_que] == cm_annu_sample_set)   //  ���ò������ (���ܱ�����)
		{ 
			j = (uint8_t *)&rev[cm_info_que];
            if(*j == 0)
            {
                rt_kprintf("����ʱ�䵥λΪ��");    
            }
//			rt_kprintf("����ʱ�䵥λ=%x\n",*j);
			d = (uint16_t *)&rev[cm_info_que + 1];
            cmsample = *d;
			rt_kprintf("�������=%d\n",*d);
            rev[cm_cmd_que] = cm_annu_sample_up;    //�ϱ�������������ܱ�������
            refreshkvalue();
		}
	}
//    cm_otadecodehandle(rev,len);   //OTA
	rev[cm_data_type_que] = 0x00;   //��Ϊ����
	rev[len-1]= crc8_table(&rev[cm_nb_type_que],(len-cm_nb_type_que)-1);
    if((rev[cm_cmd_que] != 0xF0) && (rev[cm_cmd_que] != 0xF1) && (rev[cm_cmd_que] != 0xF2) && (rev[cm_cmd_que] != 0xF3))
    {
        nb_send((char*)rev,len);  //�ϱ�����(��OTAʱ�������յ����ݰ��ϱ���ƽ̨)    
    } 
}

void cm_device_init(cm_header_t ptr,uint16_t length)
{
    #if USING_DBLH
    cm_default_config.dev_type = CM_EARTH_DEV;     //�豸���͸�Ϊ�ر��ѷ�����
    cm_default_config.dev_id = CM_DBLF_ID;
    config_data.cmd = cm_softversion_up;
    length = cm_encode(ptr,&config_data);
    nb_send((char*)cm_senddata,length);  //�ϱ�����汾
    rt_thread_delay(1000*5);
    config_data.cmd = cm_modulestatus_up;  
    length = cm_encode(ptr,&config_data);
    nb_send((char*)cm_senddata,length);  //�ϱ�ģ��״̬(�ر��ѷ�����)
    rt_thread_delay(1000*5);
    config_data.cmd = cm_cycle_up;      
    length = cm_encode(ptr,&config_data);
    nb_send((char*)cm_senddata,length);  //�ϱ���������
    rt_thread_delay(1000*5);
    config_data.cmd = cm_angle_up;      
    length = cm_encode(ptr,&config_data);
    nb_send((char*)cm_senddata,length);  //�ϱ��Ƕ�ƫ�Ƹ澯��ֵ(�ر��ѷ�����)
    rt_thread_delay(1000*5);
    config_data.cmd = cm_dist_up;      
    length = cm_encode(ptr,&config_data);
    nb_send((char*)cm_senddata,length);  //�ϱ�����ƫ�Ƹ澯��ֵ(�ر��ѷ�����)
    rt_thread_delay(1000*5);             
    config_data.cmd = cm_sample_up;     
    length = cm_encode(ptr,&config_data);
    nb_send((char*)cm_senddata,length);  //�ϱ��������
    rt_thread_delay(1000*5);
    #endif

    #if USING_QLH
    cm_default_config.dev_type = CM_WALL_DEV;     //�豸���͸�Ϊǽ�ѷ�����
    cm_default_config.dev_id = CM_QLF_ID;
    config_data.cmd = cm_softversion_up;
    length = cm_encode(ptr,&config_data);
    nb_send((char*)cm_senddata,length);  //�ϱ�����汾
    rt_thread_delay(1000*5);
    config_data.cmd = cm_modulestatus_up;  
    length = cm_encode(ptr,&config_data);
    nb_send((char*)cm_senddata,length);  //�ϱ�ģ��״̬(ǽ�ѷ�����)
    rt_thread_delay(1000*5);
    config_data.cmd = cm_cycle_up;      
    length = cm_encode(ptr,&config_data);
    nb_send((char*)cm_senddata,length);  //�ϱ���������
    rt_thread_delay(1000*5);
    config_data.cmd = cm_angle_up;      
    length = cm_encode(ptr,&config_data);
    nb_send((char*)cm_senddata,length);  //�ϱ��Ƕ�ƫ�Ƹ澯��ֵ��ǽ�ѷ����ǣ�
    rt_thread_delay(1000*5);
    config_data.cmd = cm_dist_up;      
    length = cm_encode(ptr,&config_data);
    nb_send((char*)cm_senddata,length);  //�ϱ�����ƫ�Ƹ澯��ֵ(�ر��ѷ�����)
    rt_thread_delay(1000*5);
    config_data.cmd = cm_sample_up;
    length = cm_encode(ptr,&config_data);
    nb_send((char*)cm_senddata,length);  //�ϱ��������
    rt_thread_delay(1000*5);
    #endif

    #if USING_DMQX

    cm_default_config.dev_type = CM_ANGLE_DEV;     //�豸���͸�Ϊ������б�����
    cm_default_config.dev_id = CM_DMQX_ID;         //�豸ID��Ϊ������б�����
    config_data.cmd = cm_softversion_up;
    length = cm_encode(ptr,&config_data);
    nb_send((char*)cm_senddata,length);  //�ϱ�����汾
    rt_thread_delay(1000*5);
    config_data.cmd = cm_modulestatus_up;  
    length = cm_encode(ptr,&config_data);
    nb_send((char*)cm_senddata,length);  //�ϱ�ģ��״̬(������б�����)
    rt_thread_delay(1000*5);
    config_data.cmd = cm_cycle_up;      
    length = cm_encode(ptr,&config_data);
    nb_send((char*)cm_senddata,length);   // �ϱ���������
    rt_thread_delay(1000*5);
    config_data.cmd = cm_angle_up;      
    length = cm_encode(ptr,&config_data);
    nb_send((char*)cm_senddata,length);  //�ϱ��Ƕ���ֵ(������б�����)
    rt_thread_delay(1000*5);
    config_data.cmd = cm_sample_up;
    length = cm_encode(ptr,&config_data);
    nb_send((char*)cm_senddata,length);  //�ϱ��������
    rt_thread_delay(1000*5);
    #endif

    #if USING_MUD
    cm_default_config.dev_type = CM_MUD_DEV;     //�豸���͸�Ϊ��λ��
    cm_default_config.dev_id = CM_MUD_ID;         //�豸ID��Ϊ��λ��
    config_data.cmd = cm_softversion_up;
    length = cm_encode(ptr,&config_data);
    nb_send((char*)cm_senddata,length);  //�ϱ�����汾
    rt_thread_delay(1000*5);
    config_data.cmd = cm_modulestatus_up;  
    length = cm_encode(ptr,&config_data);
    nb_send((char*)cm_senddata,length);  //�ϱ�ģ��״̬(��λ��)
    rt_thread_delay(1000*5);
    config_data.cmd = cm_cycle_up;      
    length = cm_encode(ptr,&config_data);
    nb_send((char*)cm_senddata,length);  //�ϱ���������
    rt_thread_delay(1000*5);
    config_data.cmd = cm_mud_threshold_up;      
    length = cm_encode(ptr,&config_data);
    nb_send((char*)cm_senddata,length);  //�ϱ���λ�澯��ֵ
    rt_thread_delay(1000*5);
    config_data.cmd = cm_sample_up;
    length = cm_encode(ptr,&config_data);
    nb_send((char*)cm_senddata,length);  //�ϱ��������
    rt_thread_delay(1000*5);
    #endif

    #if USING_RAIN
    cm_default_config.dev_type = CM_RAIN_DEV;     //�豸����������
    cm_default_config.dev_id = CM_RAIN_ID;
    config_data.cmd = cm_rain_softversion_up;
    length = cm_encode(ptr,&config_data);
    nb_send((char*)cm_senddata,length);  //�ϱ�����汾�������ƣ�
    rt_thread_delay(1000*5);
    config_data.cmd = cm_modulestatus_up;  
    length = cm_encode(ptr,&config_data);
    nb_send((char*)cm_senddata,length);  //�ϱ�ģ��״̬(������)
    rt_thread_delay(1000*5);
    config_data.cmd = cm_rain_cycle_up;      
    length = cm_encode(ptr,&config_data);
    nb_send((char*)cm_senddata,length);      // �ϱ���������(������)
    rt_thread_delay(1000*5);
    config_data.cmd = cm_rain_threshold_up;     
    length = cm_encode(ptr,&config_data);    
    nb_send((char*)cm_senddata,length);   //�ϱ��澯��ֵ (����)
    rt_thread_delay(1000*5);
    config_data.cmd = cm_rain_sample_up;
    length = cm_encode(ptr,&config_data);
    nb_send((char*)cm_senddata,length);  //�ϱ��������(����)
    rt_thread_delay(1000*5);
    #endif

    #if USING_ALARMER
    cm_default_config.dev_type = CM_AUTO_DEV;     //�豸���͸�Ϊ���ܱ�����
    cm_default_config.dev_id = CM_ANNU_ID; 
    config_data.cmd = cm_annu_softversion_up;
    length = cm_encode(ptr,&config_data);
    nb_send((char*)cm_senddata,length);  //�ϱ�����汾
    rt_thread_delay(1000*5);
    config_data.cmd = cm_modulestatus_up;  
    length = cm_encode(ptr,&config_data);
    nb_send((char*)cm_senddata,length);  //�ϱ�ģ��״̬(���ܱ�����)
    rt_thread_delay(1000*5);
    config_data.cmd = cm_annu_cycle_up;     
    length = cm_encode(ptr,&config_data);
    nb_send((char*)cm_senddata,length);  //�ϱ���������
    rt_thread_delay(1000*5);
    config_data.cmd = cm_annu_up;      
    length = cm_encode(ptr,&config_data);
    nb_send((char*)cm_senddata,length);  //�ϱ����ܱ���������
    rt_thread_delay(1000*5);
    config_data.cmd = cm_annu_sample_up;
    length = cm_encode(ptr,&config_data);
    nb_send((char*)cm_senddata,length);  //�ϱ����ܱ������������
    rt_thread_delay(1000*5);
    #endif
}

//static void cm_otadecodehandle(uint8_t *rev,uint16_t len)
//{   uint32_t delayms = 0;
//	uint32_t *i;
//	uint16_t *d;
//	uint16_t n;
//	uint32_t indx = 0;
//    uint8_t retry = 4;
//    if(rev[cm_cmd_que] == cm_server_request_ota)      //���������豸����������������
//    {
//        d = (uint16_t *)&rev[cm_info_que];    //info������ʼ�ֽ�   
//        cm_soft_ver = *d;                   //��������汾2byte
//        rt_kprintf("���������������汾 = %4x\n",cm_soft_ver);
//        i = (uint32_t *)&rev[cm_info_que + 2];       //�������ܳ��� 4byte
//        rt_kprintf("�������ܳ��� = %d\n",*i);
//        cm_packet_len = *i;                          //�������ܳ��� 4byte
//        i = (uint32_t *)&rev[cm_info_que + 6];      //CRC 4byte
//        rt_kprintf("�����ļ�CRC32У�� = %8x\n",*i);
//        cm_crc32 = *i;                               //�����ļ�CRCУ��    
//        i = (uint32_t *)&rev[cm_info_que + 10];    //�ְ��ܸ���
//        rt_kprintf("�ְ����� = %d\n",*i);
//        cm_packet_num = *i;                        //�ְ��ܸ���
//        cm_rand_crc = 0;                          //�ְ����ۼӺ�
//        config_data.pack_num = 1;                   //�豸��ÿ������һ����
//        cm_pack_index = 0;
//        config_data.pack_index = cm_pack_index;             //�����Ͱ�������0(��ʼֵ)
//        cm_default_config.msg_mode = cm_msg_up;     //����    
//        config_data.cmd = cm_client_request_otadata;            //�豸�������������������    cm_client_request_otadata    ����
//        cm_default_config.user_data = &config_data;             //config_data �ڶ�������,cm_default_config :header
//        n = cm_encode(&cm_default_config,&config_data);
//        nb_send_ota((char*)cm_senddata,n);                         //�ն�������������͵�һ���������ͣ�
//        for(delayms = 0;delayms < 8000000;delayms++);     //100ms
//        rt_kprintf("�ն�������������͵�һ����\n");
//        cm_default_config.msg_mode = cm_msg_up; //��Ϊ����
//    }
//        
//    if(rev[cm_cmd_que] == cm_server_send_otadata)               //���������豸����������������
//    {
////        rt_timer_start(&otatimer); //������ʱ��      �ش�����25S
//        do{
//            i = (uint32_t *)&rev[cm_info_que];                       //��ǰ�·����������� 4byte
//            indx = *i;
//            rt_kprintf("��ǰ�·����������� = %d\n",indx);
//            i = (uint32_t *)&rev[cm_info_que + 4];                  //��ǰ�·����������� 4byte
//            rt_kprintf("��ǰ�·����������� = %d\n",*i);
//            i = (uint32_t *)&rev[cm_info_que + 8];                //��ǰ������CRC32У��  4byte
//            rt_kprintf("��ǰ������CRC32У�� = %8x\n",*i);
//            d = (uint16_t*)&rev[cm_info_que + 12];               // �ְ��룬�轫���յ��ķְ�������ϴ���2byte 
//            rt_kprintf("����������� = %4x\n",*d);
//            if(cm_pack_index == indx)
//            {
//                rt_timer_stop(&otatimer); //������ʱ��
//                break;                      //����ѭ�� 
//            }    
//                                                      //����ѭ��
//           for(delayms = 0;delayms < 80000000;delayms++);     //1s
//           for(delayms = 0;delayms < 80000000;delayms++);     //1s
//           for(delayms = 0;delayms < 80000000;delayms++);     //1s
//        }while(retry--);                                       //�ƶ����ش����ƣ��ش�����,������ӳ�ʱ���� ����ʱ�Ƴ�
//        
//        if(cm_pack_index == indx)   //���������������ͽ��յ���һ��,�ҷְ�����С��1
//        {           
//            cm_packet_num--;
//            cm_pack_index++;
//            config_data.pack_num = 1;
//            config_data.pack_index = cm_pack_index;
//            rt_kprintf("ʣ���ܰ��� = %d,��һ���������� = %d\n",cm_packet_num,config_data.pack_index);
//            cm_rand_crc = (uint32_t)(cm_rand_crc + (*d));
//            rt_kprintf("������������ۼ� = %4x\n",cm_rand_crc);
//            if(cm_packet_num != 0)
//            {
//                for(delayms = 0;delayms < 80000000;delayms++); 
//                cm_default_config.msg_mode = cm_msg_up;      //��Ϊ����
//                config_data.cmd = cm_client_request_otadata; //�豸�������������������
//                config_data.pack_num = 1;                   //�豸��ÿ������һ����
//                config_data.pack_index = cm_pack_index;
//                cm_default_config.user_data = &config_data;
//                n = cm_encode(&cm_default_config,&config_data);
// 
//                nb_send_ota((char*)cm_senddata,n);     //�豸���������������Ϊcm_pack_index��1���������ݣ������¸����ݰ�
//                rt_timer_start(&otatimer);
//            }
//        }
//        else
//        {
//            cm_default_config.msg_mode = cm_msg_up;      //��Ϊ����
//            config_data.cmd = cm_client_request_otadata; //�豸�������������������
//            config_data.pack_num = 1;                   //�豸��ÿ������һ����
//            config_data.pack_index = cm_pack_index;
//            cm_default_config.user_data = &config_data;
//            n = cm_encode(&cm_default_config,&config_data);
//            nb_send_ota((char*)cm_senddata,n);     //�豸���������������Ϊcm_pack_index��1���������ݡ�
//            rt_timer_start(&otatimer);
//            
////            cm_default_config.msg_mode = cm_msg_up; //����
////            config_data.cmd = cm_ota_status_up;          //�豸�ϱ�Զ������״̬
////            config_data.ota_sum = cm_rand_crc;      //�ְ����ۼӺ�
////            config_data.ota_lab = 1;                //1���ݴ���ʧ�� ��������������   
////            cm_default_config.user_data = &config_data;
////            n = cm_encode(&cm_default_config,&config_data);
////            nb_send_ota((char*)cm_senddata,n);
//        }
//        if(cm_packet_num == 0)
//        {
//            rt_kprintf("������������� %d\n",cm_pack_index);
//            cm_default_config.msg_mode = cm_msg_up; //����
//            config_data.cmd = cm_ota_status_up;          //�豸�ϱ�Զ������״̬
//            config_data.ota_sum = cm_rand_crc;      //�ְ����ۼӺ�
//            config_data.ota_lab = 2;                //2���ݴ���ɹ�    
//            cm_default_config.user_data = &config_data;
//            n = cm_encode(&cm_default_config,&config_data);
//            nb_send_ota((char*)cm_senddata,n);
//            
//            cm_default_config.msg_mode = cm_msg_up; //����
//            #if (USING_DBLH | USING_DMQX | USING_QLH | USING_MUD)
//                config_data.cmd = cm_softversion_up;     
//            #endif
//            #if USING_ALARMER
//                config_data.cmd = cm_annu_softversion_up;    
//            #endif
//            #if USING_RAIN
//                config_data.cmd = cm_rain_softversion_up;      
//            #endif
////                    config_data.cmd = cm_rain_softversion_up;       //cm_annu_softversion_up = 0x10,�ϱ�����汾 (���ܱ�����)
//            cm_default_config.user_data = &config_data;
//            n = cm_encode(&cm_default_config,&config_data);
//            nb_send((char*)cm_senddata,n);
//            refreshkvalue();
//        }

//    }
//}   

///*common ota up hander*/
//static void cm_otaencode_handle(cm_header_t head,cm_mainmsg_t msg,uint8_t *userdata)
//{
//    if(msg->cmd == cm_client_request_otadata)       //�豸�������������������
//    {
//        head->data_len = 12;
//        userdata[0] = 0xF6;
//        userdata[1] = 0x09;  //
//        userdata[2] = cm_client_request_otadata;  //  0xF2
//        
//        userdata[3] = msg->pack_index & 0x000000FF;  //�ְ�����                
//        userdata[4] = msg->pack_index >>8 & 0x000000FF;
//        userdata[5] = msg->pack_index >>16 & 0x000000FF;
//        userdata[6] = msg->pack_index >>24 & 0x000000FF; //index��䣨msg->pack_index ++��
//        
//        userdata[7] = msg->pack_num & 0x000000FF;             
//        userdata[8] = msg->pack_num>>8 & 0x000000FF;
//        userdata[9] = msg->pack_num>>16 & 0x000000FF;
//        userdata[10] = msg->pack_num>>24 & 0x000000FF;    // ��ǰ����ְ��������ְ�����һֱΪ1,ÿ������һ������
//   
//    }
//    else if(msg->cmd == cm_ota_status_up) // �豸�ϱ�Զ������״̬
//    {
//        head->data_len = 9;
//        userdata[0] = 0xF6;
//        userdata[1] = 0x06;  //
//        userdata[2] = cm_ota_status_up;  //  0xF0
//        userdata[3] = msg->ota_lab;     // ota_lab :1���豸������������������ʧ�ܣ�2�������������ݰ�����ɹ� 
//        userdata[4] = msg->ota_sum & 0x000000FF;      //ota_sum
//        userdata[5] = msg->ota_sum >>8 & 0x000000FF;             
//        userdata[6] = msg->ota_sum >>16 & 0x000000FF;
//        userdata[7] = msg->ota_sum >>24 & 0x000000FF; //cm_rand_crc:�ְ����ۼӺ�    
//    }    
//}    

//extern struct rt_event nb_event; 
//void otatimer_callback(void* parameter)
//{
//    rt_err_t err;
//	rt_kprintf("\\*****************OTA ��ʱ����ʱ*****************\\\r\n");
//    otaflag = 1;
//    otaflag ++;
//    err = rt_event_send(&nb_event,0x100);
//    if(err !=RT_EOK)
//    {
//        rt_kprintf("ERROR!!!!!!!!\r\n");
//    }
//}

//void ota_thread_entry(void *args)
//{    
////    uint32_t reque_count = 3;
//    uint16_t n = 0;
//    while(1)
//    {
//        rt_event_recv(&nb_event,1<<8,RT_EVENT_FLAG_OR | RT_EVENT_FLAG_CLEAR,RT_WAITING_FOREVER, RT_NULL);
//        rt_kprintf("\\********1243*********OTA ��ʱ����ʱ*****************\\\r\n");
//        if(otaflag <= 3)
//        {
//            cm_default_config.msg_mode = cm_msg_up;      //��Ϊ����
//            config_data.cmd = cm_client_request_otadata; //�豸�������������������
//            config_data.pack_num = 1;                   //�豸��ÿ������һ����
//            config_data.pack_index = cm_pack_index;
//            cm_default_config.user_data = &config_data;
//            n = cm_encode(&cm_default_config,&config_data);
//            nb_send_ota((char*)cm_senddata,n);     //�豸���������������Ϊcm_pack_index��1���������ݡ�
//            rt_timer_start(&otatimer);
//        }
//        if(otaflag > 3)
//        {
//            otaflag = 0;        //����
//            uint8_t n;
//            cm_default_config.msg_mode = cm_msg_up; //����
//            config_data.cmd = cm_ota_status_up;          //�豸�ϱ�Զ������״̬
//            config_data.ota_sum = cm_rand_crc;      //�ְ����ۼӺ�
//            config_data.ota_lab = 1;                //1���ݴ���ʧ��    
//            cm_default_config.user_data = &config_data;
//            n = cm_encode(&cm_default_config,&config_data);
//            nb_send_ota((char*)cm_senddata,n);
////            for(uint32_t delayms = 0;delayms < 80000000;delayms++);
//            rt_thread_delay(1000 * 1);
//            rt_kprintf("��ʱ����25S,������������\r\n");
//            rt_timer_stop(&otatimer);
//        }
//    }
//}