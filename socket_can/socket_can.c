/*
MIT License

Copyright (c) 2023 yaofei zheng

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/

#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <linux/can.h>
#include <linux/can/raw.h>
#include <unistd.h>
#include <net/if.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <sys/time.h>

int main(int argc, char *argv[])
{
    uint16_t index;
    struct ifreq ifr;
    struct sockaddr_can addr;
    int sockfd;
    int err;
    struct can_frame frame;
    sockfd = socket(AF_CAN, SOCK_RAW, CAN_RAW);
    if (sockfd < 0)
    {
        printf("socket error\n");
        return -1;
    }

    snprintf(ifr.ifr_name, IFNAMSIZ, "can0");

    err = ioctl(sockfd, SIOCGIFINDEX, &ifr);
    if (0 != err)
    {
        printf("ioctl error\n");
        return -1;
    }

    int loopback = 1;
    err = setsockopt(sockfd, SOL_CAN_RAW, CAN_RAW_LOOPBACK, &loopback, sizeof(int));
    if (0 != err)
    {
        printf("setsockopt error\n");
        return -1;
    }

    struct timeval timeout;
    timeout.tv_sec = 0;
    timeout.tv_usec = 200000;
    if (setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, (char *)&timeout, sizeof(timeout)) < 0)
    {
        printf("setsockopt failed\n");
        return -1;
    }
    addr.can_family = AF_CAN;
    addr.can_ifindex = ifr.ifr_ifindex;
    err = bind(sockfd, (struct sockaddr *)&addr, sizeof(struct sockaddr_can));
    if (0 != err)
    {
        printf("bind error\n");
        return -1;
    }

    frame.can_id = 0x601;
    frame.can_dlc = 8;
    frame.data[0] =0x40;
    index = 0x6064;
    memcpy(&frame.data[1], &index, sizeof(uint16_t));
    frame.data[3] = 0;

    if (0 > send(sockfd, &frame, sizeof(struct can_frame), 0))
    {
        printf("send error\n");
        return -1;
    }

    err = recv(sockfd, &frame, sizeof(struct can_frame), 0);
    if (err < 0)
    {
        printf("recv error\n");
        return -1;
    }

    printf("can id: %X\n", frame.can_id);
    printf("data len: %u\n", frame.can_dlc);
    printf("data: \n");
    for (int i = 0; i < frame.can_dlc; i++)
    {
        printf("%02X", frame.data[i]);
    }
    printf("\n");

    close(sockfd);
    return 0;
}
