/*
 * File:   base16_encryption.c
 * Author: liukai
 * 加密解密算法 base16 xor
 * Created on 2017年4月7日, 上午10:54
 */
#include  "TASK_COM.h"


void base16_encryption(char * key, char * data) 
{
    int key_length = strlen((char*)key);
    int data_length = strlen((char*)data);

    int i;
    char temp = 0;
    char temp1 = 0;
    char temp2 = 0;

    for (i = 0; i < data_length; i++)
    {
        temp = data[i]^key[i % key_length];
        temp1 = ((temp & 0xf0) >> 4) < 10 ? ((temp & 0xf0) >> 4) + 0x30 : ((temp & 0xf0) >> 4) % 10 + 0x61;
        temp2 = (temp & 0x0f) < 10 ? (temp & 0x0f) + 0x30 : (temp & 0x0f) % 10 + 0x61;
        wifi_printf("%c%c",temp1,temp2);
    }
}

void base16_decrypt(char * key, char * data)
{
    int key_length = strlen(key);
    int data_length = strlen(data);
    int i;
    char temp = 0, temp1 = 0;

    for (i = 0; i < data_length; i += 2)
    {
        temp = data[i] <= '9' ? data[i] - 0x30 : ((data[i] < 'a' ? data[i] - 0x41 : data[i] - 0x61) + 10);
        temp1 = data[i + 1] <= '9' ? data[i + 1] - 0x30 : ((data[i + 1] < 'a' ? data[i + 1] - 0x41 : data[i + 1] - 0x61) + 10);
        temp = temp << 4;
        temp += temp1;
        data[i / 2] = temp^key[(i / 2) % key_length];
    }
    data[i/2] = 0;
}
