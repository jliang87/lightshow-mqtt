/**
 *-----------------------------------------------------------------------------
 * @file can_control.cpp
 * @brief 
 * @author your name
 * @version 0.1
 * @date 2021-07-22
 * @note [change history] 
 * 
 * @copyright GEELY AUTOMOBILE RESEARCH INSTITUTE CO.,LTD Copyright (c) 2021
 *-----------------------------------------------------------------------------
 */

#include <stdio.h>
#include <stdlib.h>
#include <thread>
#include <iostream>
#include <mutex>
#include <string.h>
#include <unistd.h>
#include <stdint.h>
#include <chrono>
#include <net/if.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <linux/can.h>
#include <linux/can/raw.h>
#include <termios.h>
#include <linux/types.h>
#include <stdlib.h>
#include <mqtt.h>
#include <cjson/cJSON.h>
#include "templates/posix_sockets.h"

#define set_up_can0 "sudo ip link set can0 type can bitrate 1000000"//将CAN0波特率设置为500K
#define can0_up "sudo ifconfig can0 up"//打开CAN0
#define can0_down "sudo ifconfig can0 down"//关闭CAN0
#define prevent_can0_bus_off "sudo ip link set can0 type can restart-ms 1000"
#define set_can0_buffer_length "sudo ifconfig can0 txqueuelen 1024"

uint8_t POLYNOMIAL    = 0x1D;
uint8_t INITIAL_VALUE = 0x00;
uint8_t XOR_VALUE     = 0x00;

uint8_t byte0_20A = 0xA0;//UB位置1
uint8_t byte4_20A = 0xAA;//UB位置1
std::mutex can_mutex;

bool verbose_log_level = false;
int file_description;

int32_t initialize_can0_network()
{
    //关闭CAN设备，设置波特率后，重新打开CAN设备
    system(can0_down);
    system(prevent_can0_bus_off);
    system(set_can0_buffer_length);
    system(set_up_can0);
    system(can0_up);
    return 0;
}

int32_t can_send(can_frame frame)
{
    int s, nbytes;
    struct sockaddr_can addr;
    struct ifreq ifr;
    //创建套接字
    s = socket(PF_CAN, SOCK_RAW, CAN_RAW);
    strcpy(ifr.ifr_name, "can0" );
    //指定 can0 设备
    ioctl(s, SIOCGIFINDEX, &ifr); 
    addr.can_family = AF_CAN;
    addr.can_ifindex = ifr.ifr_ifindex;
    //将套接字与 can0 绑定
    bind(s, (struct sockaddr *)&addr, sizeof(addr));
    //发送 frame[0]
    nbytes = write(s, &frame, sizeof(frame));

    if(nbytes != sizeof(frame))
    {
        if(verbose_log_level) printf("---Send Error frame[0]!\n");
        initialize_can0_network();
    }

    close(s);
    return 0;
}

int32_t can_receive(struct can_frame * r_frame,unsigned int filter_id)
{
    int s, nbytes = 0;
    struct sockaddr_can addr;
    struct ifreq ifr;
    struct can_frame frame;
    struct can_filter rfilter;

    // Initial fram
    memset(&frame,0,sizeof(can_frame));
    //创建套接字
    s = socket(PF_CAN, SOCK_RAW, CAN_RAW);
    strcpy(ifr.ifr_name, "can0" );
    //指定 can0 设备
    ioctl(s, SIOCGIFINDEX, &ifr); 
    addr.can_family = AF_CAN;
    addr.can_ifindex = ifr.ifr_ifindex;
    //将套接字与 can0 绑定
    bind(s, (struct sockaddr *)&addr, sizeof(addr));
    //设置过滤规则，取消当前注释为禁用过滤规则，即不接收所有报文，
    // 不设置此项（即如当前代码被注释）为接收所有ID的报文。

    if (filter_id != 0)
    {
        rfilter.can_id   = 0x123;
        // CAN_EFF_MASK | CAN_SFF_MASK
        rfilter.can_mask = CAN_SFF_MASK;
        setsockopt(s, SOL_CAN_RAW, CAN_RAW_FILTER, &rfilter, sizeof(rfilter));
    }

    while (nbytes == 0)
    {
        //接收总线上的报文保存在frame中
        nbytes = read(s, &frame, sizeof(frame));
    }
    *r_frame = frame;

#ifdef MSG_DEBUG
    printf("---the nbytes:%d\n", nbytes);
    printf("---length:%d", sizeof(frame));
    printf("---ID=0x%X DLC=%d\n", frame.can_id, frame.can_dlc);
    printf("---data0=0x%02x\n",frame.data[0]);
    printf("---data1=0x%02x\n",frame.data[1]);
    printf("---data2=0x%02x\n",frame.data[2]);
    printf("---data3=0x%02x\n",frame.data[3]);
    printf("---data4=0x%02x\n",frame.data[4]);
    printf("---data5=0x%02x\n",frame.data[5]);
    printf("---data6=0x%02x\n",frame.data[6]);
    printf("---data7=0x%02x\n",frame.data[7]);
#endif
    return 0;
}

//static signals
struct can_frame frame_52A;
struct can_frame frame_18;
struct can_frame frame_20A;

//left LEDs
struct can_frame frame_321;
struct can_frame frame_322;
struct can_frame frame_323;
struct can_frame frame_324;
struct can_frame frame_325;
struct can_frame frame_326;
struct can_frame frame_327;
struct can_frame frame_328;
struct can_frame frame_329;
struct can_frame frame_32A;
struct can_frame frame_32B;
struct can_frame frame_32C;
struct can_frame frame_32D;
struct can_frame frame_32E;
struct can_frame frame_32F;
struct can_frame frame_330;
struct can_frame frame_331;
struct can_frame frame_332;
struct can_frame frame_333;
struct can_frame frame_334;
struct can_frame frame_335;
struct can_frame frame_336;
struct can_frame frame_337;
struct can_frame frame_338;

//right LEDs
struct can_frame frame_339;
struct can_frame frame_33A;
struct can_frame frame_33B;
struct can_frame frame_33C;
struct can_frame frame_33D;
struct can_frame frame_33E;
struct can_frame frame_33F;
struct can_frame frame_340;
struct can_frame frame_341;
struct can_frame frame_342;
struct can_frame frame_343;
struct can_frame frame_344;
struct can_frame frame_345;
struct can_frame frame_346;
struct can_frame frame_347;
struct can_frame frame_348;
struct can_frame frame_349;
struct can_frame frame_34A;
struct can_frame frame_34B;
struct can_frame frame_34C;
struct can_frame frame_34D;
struct can_frame frame_34E;
struct can_frame frame_34F;
struct can_frame frame_350;

//download activation/deactivation and lightshow signal
struct can_frame frame_1C;

std::array<struct can_frame, 48> all_leds_array;

void initialize_can_signals()
{
    memset(&frame_52A, 0, sizeof(can_frame));
    frame_52A.can_id = 0x52A;
    frame_52A.can_dlc = 8;
    frame_52A.data[0] = 0x2A;
    frame_52A.data[1] = 0x50;
    frame_52A.data[2] = 0xFF;
    frame_52A.data[3] = 0xFF;
    frame_52A.data[4] = 0xFF;
    frame_52A.data[5] = 0xFF;
    frame_52A.data[6] = 0xFF;
    frame_52A.data[7] = 0xFF;

    memset(&frame_18, 0, sizeof(can_frame));
    frame_18.can_id = 0x18;
    frame_18.can_dlc = 8;
    frame_18.data[0] = 0x09;
    frame_18.data[1] = 0x00;
    frame_18.data[2] = 0x00;
    frame_18.data[3] = 0xC8;
    frame_18.data[4] = 0x00;
    frame_18.data[5] = 0x00;
    frame_18.data[6] = 0xC8;
    frame_18.data[7] = 0xC8;

    memset(&frame_20A, 0, sizeof(can_frame));
    frame_20A.can_id = 0x20A;
    frame_20A.can_dlc = 8;
    frame_20A.data[0] = 0xA3;
    frame_20A.data[1] = 0xD8;
    frame_20A.data[2] = 0x80;
    frame_20A.data[3] = 0x00;
    frame_20A.data[4] = 0xAA;
    frame_20A.data[5] = 0xA0;
    frame_20A.data[6] = 0xB3;
    frame_20A.data[7] = 0x0A;

    memset(&frame_321, 0, sizeof(can_frame));
    frame_321.can_id = 0x321;
    frame_321.can_dlc = 8;

    memset(&frame_322, 0, sizeof(can_frame));
    frame_322.can_id = 0x322;
    frame_322.can_dlc = 8;

    memset(&frame_323, 0, sizeof(can_frame));
    frame_323.can_id = 0x323;
    frame_323.can_dlc = 8;

    memset(&frame_324, 0, sizeof(can_frame));
    frame_324.can_id = 0x324;
    frame_324.can_dlc = 8;

    memset(&frame_325, 0, sizeof(can_frame));
    frame_325.can_id = 0x325;
    frame_325.can_dlc = 8;

    memset(&frame_326, 0, sizeof(can_frame));
    frame_326.can_id = 0x326;
    frame_326.can_dlc = 8;

    memset(&frame_327, 0, sizeof(can_frame));
    frame_327.can_id = 0x327;
    frame_327.can_dlc = 8;

    memset(&frame_328, 0, sizeof(can_frame));
    frame_328.can_id = 0x328;
    frame_328.can_dlc = 8;

    memset(&frame_329, 0, sizeof(can_frame));
    frame_329.can_id = 0x329;
    frame_329.can_dlc = 8;

    memset(&frame_32A, 0, sizeof(can_frame));
    frame_32A.can_id = 0x32A;
    frame_32A.can_dlc = 8;

    memset(&frame_32B, 0, sizeof(can_frame));
    frame_32B.can_id = 0x32B;
    frame_32B.can_dlc = 8;

    memset(&frame_32C, 0, sizeof(can_frame));
    frame_32C.can_id = 0x32C;
    frame_32C.can_dlc = 8;

    memset(&frame_32D, 0, sizeof(can_frame));
    frame_32D.can_id = 0x32D;
    frame_32D.can_dlc = 8;

    memset(&frame_32E, 0, sizeof(can_frame));
    frame_32E.can_id = 0x32E;
    frame_32E.can_dlc = 8;

    memset(&frame_32F, 0, sizeof(can_frame));
    frame_32F.can_id = 0x32F;
    frame_32F.can_dlc = 8;

    memset(&frame_330, 0, sizeof(can_frame));
    frame_330.can_id = 0x330;
    frame_330.can_dlc = 8;

    memset(&frame_331, 0, sizeof(can_frame));
    frame_331.can_id = 0x331;
    frame_331.can_dlc = 8;

    memset(&frame_332, 0, sizeof(can_frame));
    frame_332.can_id = 0x332;
    frame_332.can_dlc = 8;

    memset(&frame_333, 0, sizeof(can_frame));
    frame_333.can_id = 0x333;
    frame_333.can_dlc = 8;

    memset(&frame_334, 0, sizeof(can_frame));
    frame_334.can_id = 0x334;
    frame_334.can_dlc = 8;

    memset(&frame_335, 0, sizeof(can_frame));
    frame_335.can_id = 0x335;
    frame_335.can_dlc = 8;

    memset(&frame_336, 0, sizeof(can_frame));
    frame_336.can_id = 0x336;
    frame_336.can_dlc = 8;

    memset(&frame_337, 0, sizeof(can_frame));
    frame_337.can_id = 0x337;
    frame_337.can_dlc = 8;

    memset(&frame_338, 0, sizeof(can_frame));
    frame_338.can_id = 0x338;
    frame_338.can_dlc = 8;

    memset(&frame_339, 0, sizeof(can_frame));
    frame_339.can_id = 0x339;
    frame_339.can_dlc = 8;

    memset(&frame_33A, 0, sizeof(can_frame));
    frame_33A.can_id = 0x33A;
    frame_33A.can_dlc = 8;

    memset(&frame_33B, 0, sizeof(can_frame));
    frame_33B.can_id = 0x33B;
    frame_33B.can_dlc = 8;

    memset(&frame_33C, 0, sizeof(can_frame));
    frame_33C.can_id = 0x33C;
    frame_33C.can_dlc = 8;

    memset(&frame_33D, 0, sizeof(can_frame));
    frame_33D.can_id = 0x33D;
    frame_33D.can_dlc = 8;

    memset(&frame_33E, 0, sizeof(can_frame));
    frame_33E.can_id = 0x33E;
    frame_33E.can_dlc = 8;

    memset(&frame_33F, 0, sizeof(can_frame));
    frame_33F.can_id = 0x33F;
    frame_33F.can_dlc = 8;

    memset(&frame_340, 0, sizeof(can_frame));
    frame_340.can_id = 0x340;
    frame_340.can_dlc = 8;

    memset(&frame_341, 0, sizeof(can_frame));
    frame_341.can_id = 0x341;
    frame_341.can_dlc = 8;

    memset(&frame_342, 0, sizeof(can_frame));
    frame_342.can_id = 0x342;
    frame_342.can_dlc = 8;

    memset(&frame_343, 0, sizeof(can_frame));
    frame_343.can_id = 0x343;
    frame_343.can_dlc = 8;

    memset(&frame_344, 0, sizeof(can_frame));
    frame_344.can_id = 0x344;
    frame_344.can_dlc = 8;

    memset(&frame_345, 0, sizeof(can_frame));
    frame_345.can_id = 0x345;
    frame_345.can_dlc = 8;

    memset(&frame_346, 0, sizeof(can_frame));
    frame_346.can_id = 0x346;
    frame_346.can_dlc = 8;

    memset(&frame_347, 0, sizeof(can_frame));
    frame_347.can_id = 0x347;
    frame_347.can_dlc = 8;

    memset(&frame_348, 0, sizeof(can_frame));
    frame_348.can_id = 0x348;
    frame_348.can_dlc = 8;

    memset(&frame_349, 0, sizeof(can_frame));
    frame_349.can_id = 0x349;
    frame_349.can_dlc = 8;

    memset(&frame_34A, 0, sizeof(can_frame));
    frame_34A.can_id = 0x34A;
    frame_34A.can_dlc = 8;

    memset(&frame_34B, 0, sizeof(can_frame));
    frame_34B.can_id = 0x34B;
    frame_34B.can_dlc = 8;

    memset(&frame_34C, 0, sizeof(can_frame));
    frame_34C.can_id = 0x34C;
    frame_34C.can_dlc = 8;

    memset(&frame_34D, 0, sizeof(can_frame));
    frame_34D.can_id = 0x34D;
    frame_34D.can_dlc = 8;

    memset(&frame_34E, 0, sizeof(can_frame));
    frame_34E.can_id = 0x34E;
    frame_34E.can_dlc = 8;

    memset(&frame_34F, 0, sizeof(can_frame));
    frame_34F.can_id = 0x34F;
    frame_34F.can_dlc = 8;

    memset(&frame_350, 0, sizeof(can_frame));
    frame_350.can_id = 0x350;
    frame_350.can_dlc = 8;

    memset(&frame_1C, 0, sizeof(can_frame));
    frame_1C.can_id = 0x1C;
    frame_1C.can_dlc = 8;
    frame_1C.data[0] = 0x00;
    frame_1C.data[1] = 0x00;
    frame_1C.data[2] = 0x00;
    frame_1C.data[3] = 0x00;
    frame_1C.data[4] = 0x00;
    frame_1C.data[6] = 0x00;
    frame_1C.data[7] = 0x00;

    all_leds_array = { frame_321, frame_322, frame_323, frame_324, frame_325, frame_326, frame_327, frame_328, 
                       frame_329, frame_32A, frame_32B, frame_32C, frame_32D, frame_32E, frame_32F, frame_330, 
                       frame_331, frame_332, frame_333, frame_334, frame_335, frame_336, frame_337, frame_338, 
                       frame_339, frame_33A, frame_33B, frame_33C, frame_33D, frame_33E, frame_33F, frame_340, 
                       frame_341, frame_342, frame_343, frame_344, frame_345, frame_346, frame_347, frame_348, 
                       frame_349, frame_34A, frame_34B, frame_34C, frame_34D, frame_34E, frame_34F, frame_350 
                     };
}

void send_periodic_static_can_signals()
{
    while(1)
    {
        can_mutex.lock();
        can_send(frame_52A);
        can_mutex.unlock();
        if (verbose_log_level) std::cout<<"Send 52A!"<<std::endl;

        can_mutex.lock();
        can_send(frame_18);
        can_mutex.unlock();
        if (verbose_log_level) std::cout<<"Send 18!"<<std::endl;

        can_mutex.lock();
        can_send(frame_20A);
        can_mutex.unlock();
        if (verbose_log_level) std::cout<<"Send 20A!"<<std::endl;

        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
}

void stop_default_lightshow()
{
    printf("voice controlled: stop lightshow\n");
    frame_18.data[0] = 0x09;

    frame_20A.data[0] = 0xA3;
    frame_20A.data[1] = 0xD8;
    frame_20A.data[2] = 0x80;
    frame_20A.data[3] = 0x00;
    frame_20A.data[4] = 0xAA;
    frame_20A.data[5] = 0xA0;
    frame_20A.data[6] = 0xB3;
    frame_20A.data[7] = 0x0A;
}

void bruno()
{
    printf("voice controlled: start lightshow\n");
    frame_18.data[0] = 0x09;

    frame_20A.data[0] = 0xAA;
    frame_20A.data[1] = 0xCD;
    frame_20A.data[2] = 0x80;
    frame_20A.data[3] = 0x00;
    frame_20A.data[4] = 0xAA;
    frame_20A.data[5] = 0xA0;
    frame_20A.data[6] = 0xB3;
    frame_20A.data[7] = 0xDE;
}

void bruno2() 
{
    printf("voice controlled: start welcome 2\n");
 
    frame_18.data[0] = 0x18;

    frame_20A.data[0] = 0xAA;
    frame_20A.data[1] = 0xCD;
    frame_20A.data[2] = 0x80;
    frame_20A.data[3] = 0x00;
    frame_20A.data[4] = 0xAA;
    frame_20A.data[5] = 0xA0;
    frame_20A.data[6] = 0xB3;
    frame_20A.data[7] = 0xDE;
}

void bruno3()
{
    printf("voice controlled: start welcome 3\n");

    frame_18.data[0] = 0x29;

    frame_20A.data[0] = 0xAA;
    frame_20A.data[1] = 0xCD;
    frame_20A.data[2] = 0x80;
    frame_20A.data[3] = 0x00;
    frame_20A.data[4] = 0xAA;
    frame_20A.data[5] = 0xA0;
    frame_20A.data[6] = 0xB3;
    frame_20A.data[7] = 0xDE;
}

void send_led_can_signals(const cJSON *led_can_signals_array, can_frame led)
{  
    for (int index = 0 ; index < cJSON_GetArraySize(led_can_signals_array) ; index++)
    {   
        int can_hex_signal = (int)strtol(cJSON_GetArrayItem(led_can_signals_array, index)->valuestring, NULL, 16);
        printf("---Signal: %d\n", can_hex_signal);  

        led.data[index] = can_hex_signal;
    }
    can_send(led);
    printf("---Sent: %d\n", led.can_id);
    std::this_thread::sleep_for(std::chrono::milliseconds(1));
}

void activate_leds_signals_transmission() 
{
    frame_1C.data[5] = 0x0E;
    can_send(frame_1C);
    printf("---Sent: %d\n", frame_1C.can_id);
}

void start_leds_signals_transmission(cJSON *received_json) 
{
    const cJSON *leds_can_signals_array = NULL;
    leds_can_signals_array = cJSON_GetObjectItemCaseSensitive(received_json, "leds_signals");

    for (int index = 0 ; index < cJSON_GetArraySize(leds_can_signals_array) ; index++)
    {   
        can_frame led = all_leds_array[index];
        const cJSON *led_can_signals_array = cJSON_GetArrayItem(leds_can_signals_array, index);

        send_led_can_signals(led_can_signals_array, led);
    }
}

void deactivate_leds_signals_transmission()
{
   for(can_frame &led : all_leds_array) {
        led.data[0] = 0xff;
        led.data[1] = 0xff;
        led.data[2] = 0xff;
        led.data[3] = 0xff;
        led.data[4] = 0xff;
        led.data[5] = 0xff;
        led.data[6] = 0xff;
        led.data[7] = 0xff;
        can_send(led);
        printf("---Sent: %d\n", led.can_id);
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }

    std::this_thread::sleep_for(std::chrono::milliseconds(15000));
    frame_1C.data[5] = 0x0A;
    can_send(frame_1C);
    printf("---Sent: %d\n", frame_1C.can_id);
}

void start_lightshow()
{
    frame_1C.data[5] = 0x1A;
    can_send(frame_1C);
    printf("---Sent: %d\n", frame_1C.can_id);
}

void stop_lightshow()
{
    frame_1C.data[5] = 0x0A;
    can_send(frame_1C);
    printf("---Sent: %d\n", frame_1C.can_id);
}

void mqtt_callback(void** unused, struct mqtt_response_publish *published)
{
    /* note that published->topic_name is NOT null-terminated (here we'll change it to a c-string) */
    char* topic_name = (char*) malloc(published->topic_name_size + 1);
    memcpy(topic_name, published->topic_name, published->topic_name_size);
    topic_name[published->topic_name_size] = '\0';
    const cJSON *operation_type = NULL;

    printf("---Received publish('%s'): %s\n", topic_name, (const char*) published->application_message);

    cJSON *received_json = cJSON_Parse((const char*) published->application_message);
    if (received_json == NULL)
    {
        const char *error_ptr = cJSON_GetErrorPtr();
        if (error_ptr != NULL)
        {
            fprintf(stderr, "Error before: %s\n", error_ptr);
        }
        goto end;
    }

    operation_type = cJSON_GetObjectItemCaseSensitive(received_json, "operation");
    if (cJSON_IsString(operation_type) && (operation_type->valuestring != NULL))
    {
        if(strcmp(operation_type->valuestring, "activation") == 0) 
        {
             activate_leds_signals_transmission();
        } 
        else if (strcmp(operation_type->valuestring, "transmission") == 0) 
        {
            start_leds_signals_transmission(received_json);
        } 
        else if (strcmp(operation_type->valuestring, "deactivation") == 0) 
        {
            deactivate_leds_signals_transmission();
        }
        else if (strcmp(operation_type->valuestring, "start_lightshow") == 0)
        {
            start_lightshow();
        }
        else if (strcmp(operation_type->valuestring, "stop_lightshow") == 0)
        {
            stop_lightshow();
        }
        else if (strcmp(operation_type->valuestring, "bruno") == 0)
        {
            usleep(500000U);
            bruno1();
        }
        else if (strcmp(operation_type->valuestring, "stop_default_lightshow") == 0)
        {
            stop_default_lightshow();
        }
    }

    end:
    free(topic_name);
    cJSON_Delete(received_json);
}

void disconnect_from_mqtt_broker(int status, int sockfd, pthread_t *client_daemon)
{
    if (sockfd != -1) close(sockfd);
    if (client_daemon != NULL) pthread_cancel(*client_daemon);
    exit(status);
}

void* refresh_mqtt_client(void* client)
{
    while(1)
    {
        mqtt_sync((struct mqtt_client*) client);
        usleep(100000U);
    }
    return NULL;
}

void connect_to_mqtt_broker()
{
    const char* addr;
    const char* port;
    const char* topic;

    /* get address (argv[1] if present) */
    addr = "121.37.165.94";

    /* get port number (argv[2] if present) */
    port = "1883";

    /* get the topic name to publish */
    topic = "example/topic";

    /* open the non-blocking TCP socket (connecting to the broker) */
    int sockfd = open_nb_socket(addr, port);

    if (sockfd == -1) {
        perror("Failed to open socket: ");
        disconnect_from_mqtt_broker(EXIT_FAILURE, sockfd, NULL);
    }

    /* setup a client */
    struct mqtt_client client;
    uint8_t sendbuf[65536]; /* sendbuf should be large enough to hold multiple whole mqtt messages */
    uint8_t recvbuf[65536]; /* recvbuf should be large enough any whole mqtt message expected to be received */
    mqtt_init(&client, sockfd, sendbuf, sizeof(sendbuf), recvbuf, sizeof(recvbuf), mqtt_callback);
    /* Create an anonymous session */
    const char* client_id = NULL;
    /* Ensure we have a clean session */
    uint8_t connect_flags = MQTT_CONNECT_CLEAN_SESSION;
    /* Send connection request to the broker. */
    mqtt_connect(&client, client_id, NULL, NULL, 0, "geely", "geely", connect_flags, 400);

    /* check that we don't have any errors */
    if (client.error != MQTT_OK) {
        fprintf(stderr, "error: %s\n", mqtt_error_str(client.error));
        disconnect_from_mqtt_broker(EXIT_FAILURE, sockfd, NULL);
    }

    /* start a thread to refresh the client (handle egress and ingree client traffic) */
    pthread_t client_daemon;
    if(pthread_create(&client_daemon, NULL, refresh_mqtt_client, &client)) {
        fprintf(stderr, "Failed to start client daemon.\n");
        disconnect_from_mqtt_broker(EXIT_FAILURE, sockfd, NULL);

    }

    /* subscribe */
    mqtt_subscribe(&client, topic, 0);

    /* start publishing the time */
    printf("---listening for '%s' messages.\n", topic);
    printf("---Press CTRL-D to exit.\n\n");

    /* block */
    while(fgetc(stdin) != EOF);

    /* disconnect */
    printf("---\n disconnecting from %s\n", addr);
    sleep(1);
    close(file_description);

    /* exit */
    disconnect_from_mqtt_broker(EXIT_SUCCESS, sockfd, &client_daemon);
}

void  voice_lightshow()
{
	int file_length;
	struct termios opt;

	char data[128];

	file_description = open("/dev/ttyUSB0", O_RDWR); //默认为阻塞读方式
    if(file_description == -1)
    {
        perror("open serial 0\n");
        exit(0);
    }
	fcntl(file_description,F_SETFL,0);

    tcgetattr(file_description, &opt);
    cfsetispeed(&opt, B9600);
    cfsetospeed(&opt, B9600);

    if(tcsetattr(file_description, TCSANOW, &opt) != 0 )
    {
        perror("tcsetattr error");
        close(file_description);
        exit(-1);
    }

    opt.c_cflag &= ~CSIZE;
    opt.c_cflag |= CS8;
    opt.c_cflag &= ~CSTOPB;
    opt.c_cflag &= ~PARENB;
    opt.c_cflag &= ~INPCK;
    opt.c_cflag |= (CLOCAL | CREAD);

    opt.c_lflag &= ~(ICANON | ECHO | ECHOE | ISIG);

    opt.c_oflag &= ~OPOST;
    opt.c_oflag &= ~(ONLCR | OCRNL);    //添加的

    opt.c_iflag &= ~(ICRNL | INLCR);
    opt.c_iflag &= ~(IXON | IXOFF | IXANY);    //添加的

    opt.c_cc[VTIME] = 0;
    opt.c_cc[VMIN] = 0;

    tcflush(file_description, TCIOFLUSH);

    printf("configure complete\n");

    if(tcsetattr(file_description, TCSANOW, &opt) != 0)
    {
        perror("serial error");
        close(file_description);
        exit(-1);
    }
    printf("start send and receive data\n");

	while(1)
	{
		bzero(data, sizeof(data)); //类似于memset
        file_length = read(file_description, data, sizeof(data));

        if(file_length > 0)
        {
            stop_default_lightshow();

            if(data[0] == 1)
            {
                usleep(100000U);
                //turn on lightshow
                bruno3();
                sleep(34);
            }
            else if(data[0] == 126)
            {
                bruno2();
                sleep(2);
            }

            stop_default_lightshow();
        }
        usleep(100000U);   
	}
}

int main()
{
    initialize_can0_network();
    initialize_can_signals();
    std::thread t_send(send_periodic_static_can_signals);
    std::thread t_ctrl(connect_to_mqtt_broker);
    std::thread t_voice(voice_lightshow);

    t_send.join();
    t_ctrl.join();
    t_voice.join();

    return 0;
}
