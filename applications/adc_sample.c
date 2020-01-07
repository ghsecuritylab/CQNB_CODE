#include <rtthread.h>
#include <rtdevice.h>
#define ADC_DEV_NAME        "adc1"       /* ADC �豸���� */
#define ADC_DEV_CHANNEL1     1           /* ADC_CHANNEL_1 */
#define ADC_DEV_CHANNEL2     2           /* ADC_CHANNEL_2 */
#define BATT_PARA            3.3/4096
#define WIRE_PARA 		     50.0/30.0/5
//rt_adc_device_t adc_dev;				/* ADC �豸��� */
rt_uint32_t ADC_BUFF[2];
float monitor_value[2] = {0};
uint8_t bat_q;
int adc_vol_sample(void)
{
		rt_adc_device_t adc_dev;		/* ADC �豸��� */
		
		rt_uint32_t bat_q_mid;
		rt_err_t ret = RT_EOK;

		/* �����豸 */
		adc_dev = (rt_adc_device_t)rt_device_find(ADC_DEV_NAME);
		if (adc_dev == RT_NULL)
		{
			rt_kprintf("adc sample run failed! can't find %s device!\n", ADC_DEV_NAME);
			return RT_ERROR;
		}

		/* ʹ���豸 */
		ret = rt_adc_enable(adc_dev, ADC_DEV_CHANNEL1);	 /* ADC_CHANNEL_1 */	

		/* ��ȡ����ֵ */
		ADC_BUFF[0] = rt_adc_read(adc_dev, ADC_DEV_CHANNEL1);
		monitor_value[0] = (float)ADC_BUFF[0] * BATT_PARA * 10;     //��λV 12V��Դ��ѹ
		bat_q_mid = (monitor_value[0]-10)/2 *100;              //����  
		bat_q = bat_q_mid & 0xFF;
		rt_kprintf("the battery voltage is :%dV \n", monitor_value[0]);

		rt_kprintf("electric quantity = %d%%\r\n",bat_q);
		/* �ر�ͨ�� */
		ret = rt_adc_disable(adc_dev, ADC_DEV_CHANNEL1);
		
		/* ʹ���豸 */
		ret = rt_adc_enable(adc_dev, ADC_DEV_CHANNEL2);	 /* ADC_CHANNEL_2 */	

		/* ��ȡ����ֵ */
		ADC_BUFF[1] = rt_adc_read(adc_dev, ADC_DEV_CHANNEL2);
		monitor_value[1] = (float)ADC_BUFF[1] * BATT_PARA * WIRE_PARA * 4000;    //��λmm ����1m = 1000������4m = 4000  �ⲿ����
		
		rt_kprintf("wire adc voltage = %d V\n",monitor_value[1]);
		/* �ر�ͨ�� */
		ret = rt_adc_disable(adc_dev, ADC_DEV_CHANNEL2);
		return ret;
}
/* ������ msh �����б��� */
//MSH_CMD_EXPORT(adc_vol_sample, adc voltage convert sample);
