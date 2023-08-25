#include <stdio.h>
#include <Windows.h>
#include <stdint.h>
#include <time.h>
// 返回格式化好的时间字符串
char *get_formatted_time()
{
    static char time_string[80];

    time_t current_time;
    struct tm *time_info;

    time(&current_time);
    time_info = localtime(&current_time);

    strftime(time_string, sizeof(time_string), "%Y-%m-%d %H:%M:%S", time_info);

    return time_string;
}
// 计算 Modbus CRC-16
void calculate_modbus_crc16(const uint8_t *data,
                            size_t length,
                            uint8_t *crc_low,
                            uint8_t *crc_high)
{
    uint16_t crc = 0xFFFF;

    for (size_t i = 0; i < length; i++)
    {
        crc ^= data[i];
        for (int j = 0; j < 8; j++)
        {
            if (crc & 0x0001)
            {
                crc >>= 1;
                crc ^= 0xA001;
            }
            else
            {
                crc >>= 1;
            }
        }
    }

    *crc_low = crc & 0xFF;
    *crc_high = (crc >> 8) & 0xFF;
}
// ./main com2 9600
int main(int argc, const char **argv)
{
    if (argc != 3)
    {
        printf("Must Have Serial port and baudrate, such as: scanner COM2 9600\n");
        exit(0);
    }
    char port[10];
    if (argc == 3)
    {
        strcpy(port, argv[1]);
    }

    int baudrate = atoi(argv[2]);
    int bauds[] = {4800, 9600, 115200};
    BOOL valid = FALSE;
    for (size_t i = 0; i < 3; i++)
    {
        if (bauds[i] == baudrate)
        {
            valid = TRUE;
            break;
        }
    }

    if (!valid)
    {
        printf("Unsupported baudrate:%d\n", baudrate);
        exit(0);
    }
    char *Time1 = get_formatted_time();
    printf("[%s] Start scanner with config: %s ,%d\n", Time1, port, baudrate);
    HANDLE hSerial;

    hSerial = CreateFile(port, GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_EXISTING, 0, NULL);

    if (hSerial == INVALID_HANDLE_VALUE)
    {
        printf("Error opening serial port:%s\n", port);
        exit(1);
    }

    DCB dcbSerialParams = {0};
    dcbSerialParams.DCBlength = sizeof(dcbSerialParams);

    if (!GetCommState(hSerial, &dcbSerialParams))
    {
        printf("Error getting serial port state\n");
        CloseHandle(hSerial);
        exit(1);
    }
    if (baudrate == 4800)
    {
        dcbSerialParams.BaudRate = CBR_4800;
    }
    if (baudrate == 9600)
    {
        dcbSerialParams.BaudRate = CBR_9600;
    }
    if (baudrate == 115200)
    {
        dcbSerialParams.BaudRate = CBR_115200;
    }
    dcbSerialParams.ByteSize = 8;          // 数据位
    dcbSerialParams.StopBits = ONESTOPBIT; // 停止位
    dcbSerialParams.Parity = NOPARITY;     // 校验位

    if (!SetCommState(hSerial, &dcbSerialParams))
    {

        fprintf(stderr, "Error setting serial port state\n");
        CloseHandle(hSerial);
        return 1;
    }
    // 设置超时时间
    COMMTIMEOUTS timeouts = {0};
    timeouts.ReadIntervalTimeout = 500;       // 读取超时时间
    timeouts.ReadTotalTimeoutConstant = 500;  // 读取超时时间
    timeouts.WriteTotalTimeoutConstant = 500; // 写入超时时间

    if (!SetCommTimeouts(hSerial, &timeouts))
    {
        fprintf(stderr, "Error setting serial port timeouts\n");
        CloseHandle(hSerial);
        return 1;
    }
    for (size_t i = 1; i <= 255; i++)
    {
        // 写入探针报文
        uint8_t test_data[] = {i, 0x03, 0x00, 0x00, 0x00, 0x01, 0, 0};
        uint8_t crc_low, crc_high;
        calculate_modbus_crc16(test_data, 6, &crc_low, &crc_high);
        test_data[6] = crc_low;
        test_data[7] = crc_high;
        DWORD bytes_written;
        char *Time1 = get_formatted_time();
        printf("[%s] Scan Modbus Addr: [%d]\n", Time1, i);
        WriteFile(hSerial, test_data, sizeof(test_data), &bytes_written, NULL);
        // 大部分传感器都会返回一个6字节数据
        char received_buffer[6];
        DWORD bytes_read;
        ReadFile(hSerial, received_buffer, 6, &bytes_read, NULL);
        if (bytes_read > 0)
        {
            char *Time2 = get_formatted_time();
            printf("[%s] Read modbus packet: |", Time2);
            for (size_t i = 0; i < bytes_read; i++)
            {
                printf("0x%02X|", received_buffer[i]);
            }
            if (bytes_read == 6)
            {
                printf(", May be a valid addr: [%d]", i);
            }
        }
        else
        {
            printf("[.....] Modbus Addr:[%d] no response with timeout.\n", i);
        }
        printf("\n");
    }

    CloseHandle(hSerial);
    return 0;
}
