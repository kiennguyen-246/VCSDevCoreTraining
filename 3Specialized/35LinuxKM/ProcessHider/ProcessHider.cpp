#include <iostream>
#include <cstring>

#include <unistd.h>
#include <sys/socket.h>
#include <linux/socket.h>
#include <linux/netlink.h>

int main(int argc, char* argv[]) {
    unsigned int uiPID = getpid();
    std::cout << "PID: ";
    std::cin >> uiPID;

    int ifdSocket = socket(PF_NETLINK, SOCK_RAW, 30);
    if (ifdSocket < 0) {
        printf("socket failed %d\n", errno);
        return -1;
    }

    sockaddr_nl saSrcAddr, saDesAddr;
    memset(&saSrcAddr, 0, sizeof(saSrcAddr));
    saSrcAddr.nl_family = AF_NETLINK;
    saSrcAddr.nl_pid = getpid();

    if (bind(ifdSocket, (sockaddr*)&saSrcAddr, sizeof(saSrcAddr))) {
        printf("bind failed %d\n", errno);
        close(ifdSocket);
        return -1;
    }
    printf("Connection successful\n");

    memset(&saDesAddr, 0, sizeof(saSrcAddr));
    saDesAddr.nl_family = AF_NETLINK;
    saDesAddr.nl_pid = 0;
    saDesAddr.nl_groups = 0;

    nlmsghdr* pnlh;
    pnlh = (nlmsghdr*)malloc(NLMSG_SPACE(1024));
    memset(pnlh, 0, NLMSG_SPACE(1024));
    pnlh->nlmsg_len = NLMSG_SPACE(1024);
    pnlh->nlmsg_pid = getpid();
    pnlh->nlmsg_flags = 0;

    for (int i = 0; i < 32; ++i) {
        unsigned char ch = ((uiPID >> i) & 1) + '0';
        memcpy(NLMSG_DATA(pnlh) + i, &ch, 1);
    }
    unsigned char ch = 0;
    memcpy(NLMSG_DATA(pnlh) + 33, &ch, 1);

    iovec iov;
    msghdr msg, resp;
    iov.iov_base = (void*)pnlh;
    iov.iov_len = pnlh->nlmsg_len;
    msg.msg_name = (void*)&saDesAddr;
    msg.msg_namelen = sizeof(saDesAddr);
    msg.msg_iov = &iov;
    msg.msg_iovlen = 1;

    // printf("Ready to send message\n");
    int iRet = sendmsg(ifdSocket, &msg, 0);
    if (iRet < 0) {
        printf("sendmsg failed %d\n", errno);
    }
    // printf("Message sent\n");

    nlmsghdr* pnlh2;
    pnlh2 = (nlmsghdr*)malloc(NLMSG_SPACE(1024));
    memset(pnlh2, 0, NLMSG_SPACE(1024));
    pnlh2->nlmsg_len = NLMSG_SPACE(1024);
    pnlh2->nlmsg_pid = getpid();
    pnlh2->nlmsg_flags = 0;

    iov.iov_base = (void*)pnlh2;
    iov.iov_len = pnlh2->nlmsg_len;
    resp.msg_name = (void*)&saDesAddr;
    resp.msg_namelen = sizeof(saDesAddr);
    resp.msg_iov = &iov;
    resp.msg_iovlen = 1;
    
    // printf("Ready to receive message\n");
    iRet = recvmsg(ifdSocket, &resp, 0);
    if (iRet < 0) {
        printf("recvmsg failed %d\n", errno);
    }
    // printf("Receive message successful\n");

    char* pcResp = (char*)NLMSG_DATA(pnlh2);
    if (*pcResp == 'o') {
        printf("Operation succeeded");
    } else {
        printf("Operation failed");
    }
    
    free(pnlh);
    free(pnlh2);
    close(ifdSocket);
    return 0;

}