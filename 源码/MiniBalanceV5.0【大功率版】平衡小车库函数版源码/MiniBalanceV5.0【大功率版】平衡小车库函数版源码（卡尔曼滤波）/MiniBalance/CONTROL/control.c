#include "control.h"	
#include "filter.h"	
  /**************************************************************************
���ߣ�ƽ��С��֮��
�ҵ��Ա�С�꣺http://shop114407458.taobao.com/
**************************************************************************/
int Balance_Pwm,Velocity_Pwm,Turn_Pwm;
u8 Flag_Target;
u32 Flash_R_Count;
int Voltage_Temp,Voltage_Count,Voltage_All;
/**************************************************************************
�������ܣ����еĿ��ƴ��붼��������
         5ms��ʱ�ж���MPU6050��INT���Ŵ���
         �ϸ�֤���������ݴ����ʱ��ͬ��				 
**************************************************************************/
int EXTI9_5_IRQHandler(void) 
{    
	 if(PBin(5)==0)		
	{   
		  EXTI->PR=1<<5;                                                      //���LINE5�ϵ��жϱ�־λ   
		   Flag_Target=!Flag_Target;
		  if(delay_flag==1)
			 {
				 if(++delay_50==10)	 delay_50=0,delay_flag=0;                     //���������ṩ50ms�ľ�׼��ʱ
			 }
		  if(Flag_Target==1)                                                  //5ms��ȡһ�������Ǻͼ��ٶȼƵ�ֵ�����ߵĲ���Ƶ�ʿ��Ը��ƿ������˲��ͻ����˲���Ч��
			{
				Get_Angle(Way_Angle);                                               //===������̬		
			if(++Flash_R_Count==150&&Angle_Balance>30)Flash_Read();               //=====��ȡFlash��PID����		
      Voltage_Temp=Get_battery_volt();		                                //=====��ȡ��ص�ѹ		
			Voltage_Count++;                                                    //=====ƽ��ֵ������
			Voltage_All+=Voltage_Temp;                                          //=====��β����ۻ�
			if(Voltage_Count==100) Voltage=Voltage_All/100,Voltage_All=0,Voltage_Count=0;//=====��ƽ��ֵ		
			return 0;	                                               
			}                                                                   //10ms����һ�Σ�Ϊ�˱�֤M�����ٵ�ʱ���׼�����ȶ�ȡ����������
			Encoder_Left=-Read_Encoder(2);                                      //===��ȡ��������ֵ����Ϊ�����������ת��180�ȵģ����Զ�����һ��ȡ������֤�������һ��
			Encoder_Right=Read_Encoder(4);                                      //===��ȡ��������ֵ
	  	Get_Angle(Way_Angle);                                               //===������̬	
			Read_Distane();                                                     //===��ȡ��������������ֵ
  		if(Bi_zhang==0)Led_Flash(100);                                      //===LED��˸;����ģʽ 1s�ı�һ��ָʾ�Ƶ�״̬	
			if(Bi_zhang==1)Led_Flash(0);                                        //===LED��˸;����ģʽ ָʾ�Ƴ���	
			Key();                                                              //===ɨ�谴��״̬ ����˫�����Ըı�С������״̬
 			Balance_Pwm=Mycarbl(Angle_Balance,Gyro_Balance);                   //===ƽ��PID����	
		  Velocity_Pwm=Mycarvl(Encoder_Left,Encoder_Right);                  //===�ٶȻ�PID����	 ��ס���ٶȷ�����������������С�����ʱ��Ҫ����������Ҫ���ܿ�һ��
 	    Turn_Pwm    =Mycarturn(Encoder_Left,Encoder_Right,Gyro_Turn);            //===ת��PID����     
 		  Moto1=Balance_Pwm-Velocity_Pwm+Turn_Pwm;                            //===�������ֵ������PWM
 	  	Moto2=Balance_Pwm-Velocity_Pwm-Turn_Pwm;                            //===�������ֵ������PWM
   		Xianfu_Pwm();                                                       //===PWM�޷�
			if(Pick_Up(Acceleration_Z,Angle_Balance,Encoder_Left,Encoder_Right))//===����Ƿ�С��������
			Flag_Stop=1;	                                                      //===���������͹رյ��
			if(Put_Down(Angle_Balance,Encoder_Left,Encoder_Right))              //===����Ƿ�С��������
			Flag_Stop=0;	                                                      //===��������¾��������
      if(Turn_Off(Angle_Balance,Voltage)==0)                              //===����������쳣
 			Set_Pwm(Moto1,Moto2);                                               //===��ֵ��PWM�Ĵ���  
	}       	
	 return 0;	  
} 

/**************************************************************************
�������ܣ�ֱ��PD����
��ڲ������Ƕȡ����ٶ�
����  ֵ��ֱ������PWM
��    �ߣ�ƽ��С��֮��
**************************************************************************/
int Mycarbl(float angle_in,float Gyro_in)
{  
   float error;
	 int Car_balance;
	 error=angle_in-jxzz;       //===���ƽ��ĽǶ���ֵ �ͻ�е���
	 Car_balance=Balance_Kp*error+Gyro_in*Balance_Kd;   //===����ƽ����Ƶĵ��PWM  PD����   kp��Pϵ�� kd��Dϵ�� 
	 return Car_balance;
}

/**************************************************************************
�������ܣ��ٶ�PI���� �޸�ǰ�������ٶȣ�����Target_Velocity�����磬�ĳ�60�ͱȽ�����
��ڲ��������ֱ����������ֱ�����
����  ֵ���ٶȿ���PWM
��    �ߣ�ƽ��С��֮��
**************************************************************************/
int Mycarvl(int enc_l,int enc_r)
{  
    static float Car_Velocity,Enc_Least,Enc,Car_Movement,Car_bizhang;
	  static float Enc_I,Car_Target_V;
	  //=============ң��ǰ�����˲���=======================// 
	  if(Bi_zhang==1&&Flag_sudu==1)  Car_Target_V=45;                 //����������ģʽ,�Զ��������ģʽ
    else 	                         Car_Target_V=110;                 
		if(1==Flag_Qian)    	Car_Movement=-Car_Target_V/Flag_sudu;	         //===ǰ����־λ��1 
		else if(1==Flag_Hou)	Car_Movement=Car_Target_V/Flag_sudu;         //===���˱�־λ��1
	  else  Car_Movement=0;	
	  if(Bi_zhang==1&&Distance<500&&Flag_Left!=1&&Flag_Right!=1)        //���ϱ�־λ��1�ҷ�ң��ת���ʱ�򣬽������ģʽ
	  Car_Movement=Car_Target_V/Flag_sudu;
   //=============�ٶ�PI������=======================//	
		Enc_Least =(enc_l+enc_r)-0;                    //===��ȡ�����ٶ�ƫ��==�����ٶȣ����ұ�����֮�ͣ�-Ŀ���ٶȣ��˴�Ϊ�㣩 
		Enc *= 0.8;		                                                //===һ�׵�ͨ�˲���       
		Enc += Enc_Least*0.2;	                                    //===һ�׵�ͨ�˲���    
		Enc_I +=Enc;                                       //===���ֳ�λ�� ����ʱ�䣺10ms
		Enc_I=Enc_I-Car_Movement;                       //===����ң�������ݣ�����ǰ������
		if(Enc_I>10000)  	Enc_I=10000;             //===�����޷�
		if(Enc_I<-10000)	Enc_I=-10000;              //===�����޷�	
		Car_Velocity=Enc*Velocity_Kp+Enc_I*Velocity_Ki;                          //===�ٶȿ���	
		if(Turn_Off(Angle_Balance,Voltage)==1||Flag_Stop==1)   Enc_I=0;      //===����رպ��������
	  return Car_Velocity;
}

/**************************************************************************
�������ܣ�ת�����  �޸�ת���ٶȣ����޸�Turn_Amplitude����
��ڲ��������ֱ����������ֱ�������Z��������
����  ֵ��ת�����PWM
��    �ߣ�ƽ��С��֮��
**************************************************************************/
int Mycarturn(int enc_l,int enc_r,float gyro_z_in)//ת�����
{
	 static float Car_Tar_Turn,Car_turn,Enc_tp,Car_turn_convert=0.9,Car_turn_count;
	  float Car_turn_Amp=44/Flag_sudu,Kp=100,Kd=0;     
	  //=============ң��������ת����=======================//
//  	if(1==Flag_Left||1==Flag_Right)                      //��һ������Ҫ�Ǹ�����תǰ���ٶȵ����ٶȵ���ʼ�ٶȣ�����С������Ӧ��
//		{
//			if(++Car_turn_count==1)
//			Enc_tp=myabs(enc_l+enc_r);
//			Car_turn_convert=50/Enc_tp;
//			if(Car_turn_convert<0.6)Car_turn_convert=0.6;
//			if(Car_turn_convert>3)Car_turn_convert=3;
//		}	
//	  else
//		{
//			Car_turn_convert=0.9;
//			Car_turn_count=0;
//			Enc_tp=0;
//		}			
		if(1==Flag_Left)	           Car_Tar_Turn-=Car_turn_convert;
		else if(1==Flag_Right)	     Car_Tar_Turn+=Car_turn_convert; 
		else Car_Tar_Turn=0;
    if(Car_Tar_Turn>Car_turn_Amp)  Car_Tar_Turn=Car_turn_Amp;    //===ת���ٶ��޷�
	  if(Car_Tar_Turn<-Car_turn_Amp) Car_Tar_Turn=-Car_turn_Amp;
		if(Flag_Qian==1||Flag_Hou==1)  Kd=0.5;        
		else Kd=0;   //ת���ʱ��ȡ�������ǵľ��� �е�ģ��PID��˼��
  	//=============ת��PD������=======================//
		Car_turn=-Car_Tar_Turn*Kp -gyro_z_in*Kd;                 //===���Z�������ǽ���PD����
	  return Car_turn;
}

/**************************************************************************
�������ܣ���ֵ��PWM�Ĵ���
��ڲ���������PWM������PWM
����  ֵ����
**************************************************************************/
void Set_Pwm(int moto1,int moto2)
{
    	if(moto1<0)			AIN2=1,			AIN1=0;
			else 	          AIN2=0,			AIN1=1;
			PWMA=myabs(moto1);
		  if(moto2<0)	BIN1=0,			BIN2=1;
			else        BIN1=1,			BIN2=0;
			PWMB=myabs(moto2);	
}

/**************************************************************************
�������ܣ�����PWM��ֵ 
��ڲ�������
����  ֵ����
**************************************************************************/
void Xianfu_Pwm(void)
{	
	  int Amplitude=6900;    //===PWM������7200 ������6900
	  if(Flag_Qian==1)  Moto1-=DIFFERENCE;  //DIFFERENCE��һ������ƽ��С������ͻ�е��װ�����һ��������ֱ���������������С�����и��õ�һ���ԡ�
	  if(Flag_Hou==1)   Moto2+=DIFFERENCE;
    if(Moto1<-Amplitude) Moto1=-Amplitude;	
		if(Moto1>Amplitude)  Moto1=Amplitude;	
	  if(Moto2<-Amplitude) Moto2=-Amplitude;	
		if(Moto2>Amplitude)  Moto2=Amplitude;		
	
}
/**************************************************************************
�������ܣ������޸�С������״̬ 
��ڲ�������
����  ֵ����
**************************************************************************/
void Key(void)
{	
	u8 tmp,tmp2;
	tmp=click_N_Double(50); 
	if(tmp==1)Flag_Stop=!Flag_Stop;//��������С������ͣ
	if(tmp==2)Flag_Show=!Flag_Show;//˫������С������ʾ״̬
	tmp2=Long_Press();                   
  if(tmp2==1) Bi_zhang=!Bi_zhang;		//��������С���Ƿ���볬��������ģʽ 
}

/**************************************************************************
�������ܣ��쳣�رյ��
��ڲ�������Ǻ͵�ѹ
����  ֵ��1���쳣  0������
**************************************************************************/
u8 Turn_Off(float angle, int voltage)
{
	    u8 temp;
			if(angle<-40||angle>40||1==Flag_Stop||voltage<1110)//��ص�ѹ����11.1V�رյ��
			{	                                                 //===��Ǵ���40�ȹرյ��
      temp=1;                                            //===Flag_Stop��1�رյ��
			AIN1=0;                                            
			AIN2=0;
			BIN1=0;
			BIN2=0;
      }
			else
      temp=0;
      return temp;			
}
	
/**************************************************************************
�������ܣ���ȡ�Ƕ� �����㷨�������ǵĵ�У�����ǳ����� 
��ڲ�������ȡ�Ƕȵ��㷨 1��DMP  2�������� 3�������˲�
����  ֵ����
**************************************************************************/
void Get_Angle(u8 way)
{ 
	    float Accel_Y,Accel_X,Accel_Z,Gyro_Y,Gyro_Z;
	   	Temperature=Read_Temperature();      //===��ȡMPU6050�����¶ȴ��������ݣ����Ʊ�ʾ�����¶ȡ�
	    if(way==1)                           //===DMP�Ķ�ȡ�����ݲɼ��ж����ѵ�ʱ���ϸ���ѭʱ��Ҫ��
			{	
					Read_DMP();                      //===��ȡ���ٶȡ����ٶȡ����
					Angle_Balance=Pitch;             //===����ƽ�����
					Gyro_Balance=gyro[1];            //===����ƽ����ٶ�
					Gyro_Turn=gyro[2];               //===����ת����ٶ�
				  Acceleration_Z=accel[2];         //===����Z����ٶȼ�
			}			
      else
      {
			Gyro_Y=(I2C_ReadOneByte(devAddr,MPU6050_RA_GYRO_YOUT_H)<<8)+I2C_ReadOneByte(devAddr,MPU6050_RA_GYRO_YOUT_L);    //��ȡY��������
			Gyro_Z=(I2C_ReadOneByte(devAddr,MPU6050_RA_GYRO_ZOUT_H)<<8)+I2C_ReadOneByte(devAddr,MPU6050_RA_GYRO_ZOUT_L);    //��ȡZ��������
		  Accel_X=(I2C_ReadOneByte(devAddr,MPU6050_RA_ACCEL_XOUT_H)<<8)+I2C_ReadOneByte(devAddr,MPU6050_RA_ACCEL_XOUT_L); //��ȡX����ٶȼ�
	  	Accel_Z=(I2C_ReadOneByte(devAddr,MPU6050_RA_ACCEL_ZOUT_H)<<8)+I2C_ReadOneByte(devAddr,MPU6050_RA_ACCEL_ZOUT_L); //��ȡZ����ٶȼ�
		  if(Gyro_Y>32768)  Gyro_Y-=65536;                       //��������ת��  Ҳ��ͨ��shortǿ������ת��
			if(Gyro_Z>32768)  Gyro_Z-=65536;                       //��������ת��
	  	if(Accel_X>32768) Accel_X-=65536;                      //��������ת��
		  if(Accel_Z>32768) Accel_Z-=65536;                      //��������ת��
			Gyro_Balance=-Gyro_Y;                                  //����ƽ����ٶ�
	   	Accel_Y=atan2(Accel_X,Accel_Z)*180/PI;                 //�������	
		  Gyro_Y=Gyro_Y/16.4;                                    //����������ת��	
      if(Way_Angle==2)		  	Kalman_Filter(Accel_Y,-Gyro_Y);//�������˲�	
			else if(Way_Angle==3)   Yijielvbo(Accel_Y,-Gyro_Y);    //�����˲�
	    Angle_Balance=angle;                                   //����ƽ�����
			Gyro_Turn=Gyro_Z;                                      //����ת����ٶ�
			Acceleration_Z=Accel_Z;                                //===����Z����ٶȼ�	
		}
}
/**************************************************************************
�������ܣ�����ֵ����
��ڲ�����int
����  ֵ��unsigned int
**************************************************************************/
int myabs(int a)
{ 		   
	  int temp;
		if(a<0)  temp=-a;  
	  else temp=a;
	  return temp;
}
/**************************************************************************
�������ܣ����С���Ƿ�����
��ڲ�����int
����  ֵ��unsigned int
**************************************************************************/
int Pick_Up(float Acceleration,float Angle,int encoder_left,int encoder_right)
{ 		   
	 static u16 flag,count0,count1,count2;
	if(flag==0)                                                                   //��һ��
	 {
	      if(myabs(encoder_left)+myabs(encoder_right)<30)                         //����1��С���ӽ���ֹ
				count0++;
        else 
        count0=0;		
        if(count0>10)				
		    flag=1,count0=0; 
	 } 
	 if(flag==1)                                                                  //����ڶ���
	 {
		    if(++count1>200)       count1=0,flag=0;                                 //��ʱ���ٵȴ�2000ms
	      if(Acceleration>26000&&(Angle>(-20+jxzz))&&(Angle<(20+jxzz)))   //����2��С������0�ȸ���������
		    flag=2; 
	 } 
	 if(flag==2)                                                                  //������
	 {
		  if(++count2>100)       count2=0,flag=0;                                   //��ʱ���ٵȴ�1000ms
	    if(myabs(encoder_left+encoder_right)>135)                                 //����3��С������̥��Ϊ�������ﵽ����ת��   
      {
				flag=0;                                                                                     
				return 1;                                                               //��⵽С��������
			}
	 }
	return 0;
}
/**************************************************************************
�������ܣ����С���Ƿ񱻷���
��ڲ�����int
����  ֵ��unsigned int
**************************************************************************/
int Put_Down(float Angle,int encoder_left,int encoder_right)
{ 		   
	 static u16 flag,count;	 
	 if(Flag_Stop==0)                           //��ֹ���      
   return 0;	                 
	 if(flag==0)                                               
	 {
	      if(Angle>(-10+jxzz)&&Angle<(10+jxzz)&&encoder_left==0&&encoder_right==0)         //����1��С������0�ȸ�����
		    flag=1; 
	 } 
	 if(flag==1)                                               
	 {
		  if(++count>50)                                          //��ʱ���ٵȴ� 500ms
		  {
				count=0;flag=0;
		  }
	    if(encoder_left>3&&encoder_right>3&&encoder_left<60&&encoder_right<60)                //����2��С������̥��δ�ϵ��ʱ����Ϊת��  
      {
				flag=0;
				flag=0;
				return 1;                                             //��⵽С��������
			}
	 }
	return 0;
}


