#include "main.h"
#include "User_LED.h"
#include "User_UART1.h"
#include "User_System_Clock_Config.h"


void ReadUID(uint8_t *uid) {
  uint32_t *uid_addr = (uint32_t *)0x1FFF0000;  // UID 起始地址
  for (int i = 0; i < 4; i++) {                // 读取 4 个 32 位字（共 16 字节）
    uid[i * 4] = (uid_addr[i] >> 0) & 0xFF;    // 低字节
    uid[i * 4 + 1] = (uid_addr[i] >> 8) & 0xFF;
    uid[i * 4 + 2] = (uid_addr[i] >> 16) & 0xFF;
    uid[i * 4 + 3] = (uid_addr[i] >> 24) & 0xFF;  // 高字节
  }
}

int main(void)
{
  HAL_Init();//HAL库初始化
  User_System_Clock_Init();//系统时钟配置

  /*******************************************/
	
	User_LED_Init();
  User_USART1_Init();//串口初始化
//  printf("hello\r\n");//串口测试

  uint8_t uid[16];  // 存储 UID 的数组
  ReadUID(uid);     // 读取 UID
  while (1)
  {
		
			printf("Hello ");
		    // 打印 UID（需实现串口打印函数）
    for (int i = 0; i < 16; i++) {
      printf("%02X ", uid[i]);
    }
    HAL_Delay(1000);
  }
}

