void ACD_init_Fun(void)
{
	PCD_SI522A_TypeA_Init();	//Reader模式的初始化
	
	PCD_ACD_AutoCalc(); //自动获取阈值
	
	PCD_ACD_Start();   //初始化ACD配置寄存器，并且进入ACD模式
}





void PCD_ACD_AutoCalc(void)
{
	unsigned char temp; 
	unsigned char temp_Compare=0; 
	unsigned char VCON_TR[8]={ 0x00,0x01,0x02,0x03,0x04,0x05,0x06,0x07};//acd灵敏度调节 V1.3版本修改VCON值，
	unsigned char TR_Compare[4]={ 0x00, 0x00, 0x00, 0x00};
	ACDConfigRegC_Val = 0x7f;
	unsigned char	ACDConfigRegK_RealVal = 0;
	I_SI522A_IO_Write(TxControlReg, 0x83);	//打开天线
	I_SI522A_SetBitMask(CommandReg, 0x06);	//开启ADC_EXCUTE
	delay_us(200);
	
	
	
	for(int i=7; i>0; i--)
	{	
		I_SI522A_IO_Write(ACDConfigSelReg, (ACDConfigK << 2) | 0x40);		
		I_SI522A_IO_Write(ACDConfigReg, VCON_TR[i]);
		
		I_SI522A_IO_Write(ACDConfigSelReg, (ACDConfigG << 2) | 0x40);
		temp_Compare = I_SI522A_IO_Read(ACDConfigReg);
		for(int m=0;m<100;m++)
		{
			I_SI522A_IO_Write(ACDConfigSelReg, (ACDConfigG << 2) | 0x40);		
			temp = I_SI522A_IO_Read(ACDConfigReg);
			
			if(	temp	==	0) 	break;          //处在接近的VCON值附近值，如果偶合出现0值，均有概率误触发，应舍弃该值。
				
			temp_Compare=(temp_Compare+temp)/2;		
			delay_us(100);
		}		
		
		if(temp_Compare == 0 || temp_Compare == 0x7f) //比较当前值和所存值
		{

		}
		else
		{
			if(temp_Compare < ACDConfigRegC_Val)
			{
				ACDConfigRegC_Val = temp_Compare;
				ACDConfigRegK_Val = VCON_TR[i];
			}
		}
	}
	ACDConfigRegK_RealVal	=	ACDConfigRegK_Val;     //取得最接近的参考电压VCON
	if((ACDConfigRegK_Val==0x0e)|(ACDConfigRegK_Val==0x0f))
	{
			while(1)
			{
				LED_RED=0;
				printf("\r\nAttention!!! The RX partial resistance ratio is too small,please turn down 5.1K resistance and try again!!");
				delay_ms(2000);
			}
	}
	
	for(int j=0; j<4; j++)
	{
		I_SI522A_IO_Write(ACDConfigSelReg, (ACDConfigK << 2) | 0x40);		
		I_SI522A_IO_Write(ACDConfigReg, j*32+ACDConfigRegK_Val);
		
		I_SI522A_IO_Write(ACDConfigSelReg, (ACDConfigG << 2) | 0x40);
		temp_Compare = I_SI522A_IO_Read(ACDConfigReg);
		for(int n=0;n<100;n++)
		{
			I_SI522A_IO_Write(ACDConfigSelReg, (ACDConfigG << 2) | 0x40);		
			temp = I_SI522A_IO_Read(ACDConfigReg);
			temp_Compare=(temp_Compare+temp)/2;		
			delay_us(100);
		}		
		TR_Compare[j] = temp_Compare;
	}//再调TR的档位，将采集值填入TR_Compare[]

	for(int z=0; z<3; z++)                   //TR有四档可调，但是最大档的时候，电源有抖动，可能会导致ADC的值抖动较大，造成误触发
	{
		if(TR_Compare[z] ==0x7F)         
		{
			
		}
		else
		{
			ACDConfigRegC_Val = TR_Compare[z];//最终选择的配置
			ACDConfigRegK_Val = ACDConfigRegK_RealVal + z*32+16;  // V1.3版本将斜率修改，bit[4:3]修改为10
		}
	}//再选出一个非7f大值
	
	
	I_SI522A_IO_Write(ACDConfigSelReg, (ACDConfigK << 2) | 0x40);
	printf("\r\n ACDConfigRegK_Val:%02x ",ACDConfigRegK_Val);	
	
	I_SI522A_SetBitMask(CommandReg, 0x06);		//关闭ADC_EXCUTE
}












void PCD_ACD_Start(void)
{
	I_SI522A_IO_Write(DivIrqReg, 0x60);	//清中断，该处不清中断，进入ACD模式后会异常产生有卡中断。
	I_SI522A_IO_Write(ACDConfigSelReg, (ACDConfigJ << 2) | 0x40);		
	I_SI522A_IO_Write(ACDConfigReg, 0x55);	//Clear ACC_IRQ
	
	I_SI522A_IO_Write(ACDConfigSelReg, (ACDConfigA << 2) | 0x40);          //设置轮询时间
	I_SI522A_IO_Write(ACDConfigReg, ACDConfigRegA_Val );
	I_SI522A_IO_Write(ACDConfigSelReg, (ACDConfigB << 2) | 0x40);					//设置相对模式或者绝对模式
	I_SI522A_IO_Write(ACDConfigReg, ACDConfigRegB_Val );
	I_SI522A_IO_Write(ACDConfigSelReg, (ACDConfigC << 2) | 0x40);					//设置无卡场强值
	I_SI522A_IO_Write(ACDConfigReg, ACDConfigRegC_Val );
	I_SI522A_IO_Write(ACDConfigSelReg, (ACDConfigD << 2) | 0x40);					//设置灵敏度，一般建议为4，在调试时，可以适当降低验证该值，验证ACD功能
	I_SI522A_IO_Write(ACDConfigReg, ACDConfigRegD_Val );
	I_SI522A_IO_Write(ACDConfigSelReg, (ACDConfigH << 2) | 0x40);					//设置看门狗定时器时间
	I_SI522A_IO_Write(ACDConfigReg, ACDConfigRegH_Val );
	I_SI522A_IO_Write(ACDConfigSelReg, (ACDConfigI << 2) | 0x40);         //设置ARI功能，在天线场强打开前1us产生ARI电平控制触摸芯片Si12T的硬件屏蔽引脚SCT
	I_SI522A_IO_Write(ACDConfigReg, ACDConfigRegI_Val );	
	I_SI522A_IO_Write(ACDConfigSelReg, (ACDConfigK << 2) | 0x40);					//设置ADC的基准电压和放大增益
	I_SI522A_IO_Write(ACDConfigReg, ACDConfigRegK_Val );
	I_SI522A_IO_Write(ACDConfigSelReg, (ACDConfigM << 2) | 0x40);					//设置监测ACD功能是否产生场强，意外产生可能导致读卡芯片复位或者寄存器丢失
	I_SI522A_IO_Write(ACDConfigReg, ACDConfigRegM_Val );
	I_SI522A_IO_Write(ACDConfigSelReg, (ACDConfigO << 2) | 0x40);					//设置ACD模式下相关功能的标志位传导到IRQ引脚
	I_SI522A_IO_Write(ACDConfigReg, ACDConfigRegO_Val );
	
	I_SI522A_IO_Write(ComIEnReg, ComIEnReg_Val);												//ComIEnReg，DivIEnReg   设置IRQ选择上升沿或者下降沿
	I_SI522A_IO_Write(DivIEnReg, DivIEnReg_Val);												
	
	I_SI522A_IO_Write(ACDConfigSelReg, (ACDConfigJ << 2) | 0x40);				//设置监测ACD功能下的重要寄存器的配置值，寄存器丢失后会立即产生中断
	I_SI522A_IO_Write(ACDConfigReg, ACDConfigRegJ_Val );								// 写非0x55的值即开启功能，写0x55清除使能停止功能。
	I_SI522A_IO_Write(CommandReg, 0xb0);	//进入ACD
}
