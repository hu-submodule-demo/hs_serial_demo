/**
 * @file      main.c
 * @brief     程序入口文件
 * @author    huenrong (sgyhy1028@outlook.com)
 * @date      2026-02-01 15:38:46
 *
 * @copyright Copyright (c) 2026 huenrong
 *
 */

#include <stdio.h>
#include <time.h>
#include <string.h>

#include "hs_serial.h"

/**
 * @brief 获取当前 UTC 时间戳(毫秒)
 *
 * @param[out] timestamp_ms: 自 1970-01-01 00:00:00 UTC 起的毫秒数
 *
 * @return 0 : 成功
 * @return <0: 失败
 */
int hs_time_get_current_timestamp_ms(uint64_t *timestamp_ms)
{
    if (timestamp_ms == NULL)
    {
        return -1;
    }

    struct timespec time_spec = {0};
    if (clock_gettime(CLOCK_REALTIME, &time_spec) == -1)
    {
        return -2;
    }

    *timestamp_ms = time_spec.tv_sec * 1000 + time_spec.tv_nsec / 1000000;

    return 0;
}

/**
 * @brief 发送并接收数据测试
 *
 * @param[in] serial_name: 串口名称
 *
 * @return 0 : 成功
 * @return <0: 失败
 */
int send_and_recv_data_test(const char *serial_name)
{
    hs_serial_t *hs_serial = hs_serial_create();
    if (hs_serial == NULL)
    {
        printf("create serial failed\n");

        return -1;
    }

    int ret = hs_serial_init(hs_serial, serial_name, E_BAUD_RATE_115200, 0, E_DATA_BIT_8, E_PARITY_BIT_N, E_STOP_BIT_1);
    if (ret != 0)
    {
        printf("init %s failed. ret: %d\n", serial_name, ret);

        return -2;
    }

    uint8_t send_data[] = {0xAA, 0x01, 0x03, 0x11, 0x22, 0x33, 0xBB};
    size_t send_data_len = strlen((char *)send_data);

    uint64_t current_timestamp = 0;
    hs_time_get_current_timestamp_ms(&current_timestamp);

    // 打印发送数据
    printf("[%ld] serial %s write data[len = %ld]: ", current_timestamp, serial_name, send_data_len);
    for (size_t i = 0; i < send_data_len; i++)
    {
        printf("0x%02X ", send_data[i]);
    }
    printf("\n");

    // 清空输入, 输出缓存
    ret = hs_serial_flush_both_cache(hs_serial);
    if (ret != 0)
    {
        printf("serial %s flush both data failed. ret: %d\n", serial_name, ret);
        hs_serial_destroy(hs_serial);
        hs_serial = NULL;

        return -3;
    }

    // 串口发送数据
    ssize_t write_len = hs_serial_write_data(hs_serial, send_data, send_data_len);
    if (write_len < 0)
    {
        printf("serial %s write data failed. ret: %d\n", serial_name, ret);
        hs_serial_destroy(hs_serial);
        hs_serial = NULL;

        return -4;
    }

    // 串口接收数据
    uint8_t recv_data[send_data_len];
    memset(recv_data, 0, sizeof(recv_data));
    ssize_t read_len = hs_serial_read_data(hs_serial, recv_data, send_data_len, 1000);
    // 失败
    if (read_len < 0)
    {
        hs_time_get_current_timestamp_ms(&current_timestamp);
        printf("[%ld] serial %s read data failed. ret: %ld\n", current_timestamp, serial_name, read_len);
        hs_serial_destroy(hs_serial);
        hs_serial = NULL;

        return -5;
    }
    // 超时未收到数据
    else if (read_len == 0)
    {
        hs_time_get_current_timestamp_ms(&current_timestamp);
        printf("[%ld] serial %s read data timeout\n", current_timestamp, serial_name);
        hs_serial_destroy(hs_serial);
        hs_serial = NULL;

        return -6;
    }
    else
    {
        hs_time_get_current_timestamp_ms(&current_timestamp);
        if (memcmp(send_data, recv_data, send_data_len) != 0)
        {
            printf("[%ld] serial %s read data failed. ret: %ld\n", current_timestamp, serial_name, read_len);
            hs_serial_destroy(hs_serial);
            hs_serial = NULL;

            printf("[%ld] serial %s read data[len = %ld]: ", current_timestamp, serial_name, read_len);
            for (ssize_t i = 0; i < read_len; i++)
            {
                printf("0x%02X ", recv_data[i]);
            }
            printf("\n");

            return -7;
        }
        else
        {
            hs_time_get_current_timestamp_ms(&current_timestamp);
            printf("[%ld] serial %s read data success\n", current_timestamp, serial_name);
            hs_serial_destroy(hs_serial);
            hs_serial = NULL;

            printf("[%ld] serial %s read data[len = %ld]: ", current_timestamp, serial_name, read_len);
            for (ssize_t i = 0; i < read_len; i++)
            {
                printf("0x%02X ", recv_data[i]);
            }
            printf("\n");

            return 0;
        }
    }
}

/**
 * @brief 程序入口
 *
 * @param[in] argc: 参数个数
 * @param[in] argv: 参数列表
 *
 * @return 成功: 0
 * @return 失败: 其它
 */
int main(int argc, char *argv[])
{
    // 不插入 USB-TTL 设备，可测试接收超时
    const char *serial_name = "/dev/ttyUSB0";
    int ret = send_and_recv_data_test(serial_name);
    if (ret != 0)
    {
        printf("serial %s test failed. ret: %d\n", serial_name, ret);
    }
    else
    {
        printf("serial %s test success\n", serial_name);
    }

    return 0;
}
