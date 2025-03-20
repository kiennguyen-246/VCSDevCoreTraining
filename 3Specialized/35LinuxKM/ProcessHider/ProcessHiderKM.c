#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/skbuff.h>
#include <linux/ftrace.h>
#include <linux/kprobes.h>
#include <linux/linkage.h>
#include <net/sock.h>

static struct kprobe kp = {
    .symbol_name = "kallsym_lookup_name"
};

typedef struct _FtraceHook {
    const char* name;
    void* hookFunc;
    void* targetFunc;
    unsigned long address;
    struct ftrace_ops ftrops;
} FtraceHook, *PFtraceHook;

static int fhResolveHookAddress(PFtraceHook pHook);

int fhInstallHook(PFtraceHook pHook);

void fhRemoveHook(PFtraceHook pHook);

static void fhGetFtraceThunk(unsigned long ip, unsigned long parentIp,
    struct ftrace_ops* pFtrops, struct ftrace_regs* pFtrRegs);

static asmlinkage long (*syscallKill)(const struct pt_regs*);

asmlinkage int hookedKill(const struct pt_regs* pRegs);

static void netlinkInputRoutine(struct sk_buff *skb);

static int netlinkBindRoutine(struct net *net, int group);

// static void netlinkUnbindRoutine(struct net *net, int group);

// static void netlinkReleaseRoutine(struct sock *sk, unsigned long *groups);

static struct netlink_kernel_cfg netlinkCfg = {
    .input = netlinkInputRoutine,
    // .bind = netlinkBindRoutine,
    // .unbind = netlinkUnbindRoutine,
    // .release = netlinkReleaseRoutine,
};

static struct sock* pNetlinkSock = NULL;

static unsigned int uiTargetPID;

int __init init_module() {
    pr_info("[ProcessHiderKM] init_module called");

    pNetlinkSock = netlink_kernel_create(&init_net, 30, &netlinkCfg);
    if (!pNetlinkSock) {
        pr_info("[ProcessHiderKM] netlink_kernel_create failed");
        return -1;
    }

    pr_info("[ProcessHiderKM] Successfully created netlink socket");
    return 0;
}

void __exit cleanup_module() {
    pr_info("[ProcessHiderKM] cleanup_module called");

    netlink_kernel_release(pNetlinkSock);
}

static int fhResolveHookAddress(PFtraceHook pHook) {
    typedef unsigned long (*FpKallsymsLookupName)(const char* name);
    FpKallsymsLookupName fpKallsymsLookupName;
    register_kprobe(&kp);
    fpKallsymsLookupName = (FpKallsymsLookupName) kp.addr;
    unregister_kprobe(&kp);

    *((unsigned long*) pHook->targetFunc) = pHook->address;
    return 0;
}

int fhInstallHook(PFtraceHook pHook) {
    int iErr = 0;
    iErr = fhResolveHookAddress(pHook);
    if (iErr) {
        pr_info("[ProcessHiderKM] fhResolveHookAddress failed");
    }

    pHook->ftrops.func = fhGetFtraceThunk;
    pHook->ftrops.flags = FTRACE_OPS_FL_SAVE_REGS
    | FTRACE_OPS_FL_RECURSION
    | FTRACE_OPS_FL_IPMODIFY;

    iErr = ftrace_set_filter_ip(&pHook->ftrops, pHook->address, 0, 0);
    if (iErr) {
        pr_info("[ProcessHiderKM] ftrace_set_filter_ip failed %d", iErr);
        return iErr;
    }

    iErr = register_ftrace_function(&pHook->ftrops);
    if (iErr) {
        pr_info("[ProcessHiderKM] register_ftrace_function failed %d", iErr);
        return iErr;
    }

    return 0;
}

void fhRemoveHook(PFtraceHook pHook) {
    int iErr;

    iErr = unregister_ftrace_function(&pHook->ftrops);
    if (iErr) {
        pr_info("[ProcessHiderKM] unregister_ftrace_function failed %d", iErr);
        return;
    }

    iErr = ftrace_set_filter_ip(&pHook->ftrops, pHook->address, 1, 0);
    if (iErr) {
        pr_info("[ProcessHiderKM] ftrace_set_filter_ip failed %d", iErr);
        return;
    }
}

static void fhGetFtraceThunk(unsigned long ip, unsigned long parentIp,
    struct ftrace_ops* pFtrops, struct ftrace_regs* pFtrRegs) {
    PFtraceHook pHook = container_of(pFtrops, FtraceHook, ftrops);
    struct pt_regs* pRegs = ftrace_get_regs(pFtrRegs);

    if (!within_module(parentIp, THIS_MODULE)) {
        pRegs->ip = (unsigned long) pHook->hookFunc;
    }
}

asmlinkage int hookedKill(const struct pt_regs* pRegs) {
    pid_t pid = pRegs->di;
    int sig = pRegs->si;

    if (pid == uiTargetPID) {
        pr_info("[ProcessHiderKM] Process PID = %d is about to be killed", uiTargetPID);
    }

    return syscallKill(pRegs);
}

static void netlinkInputRoutine(struct sk_buff *skb) {
    pr_info("[ProcessHiderKM] netlinkInputRoutine called");

    struct nlmsghdr* pnlh;
    struct sk_buff* pskbOutput;
    unsigned int uiPortID, uiRes, uiMsgSize;

    pnlh = (struct nlmsghdr*)skb->data; 
    pr_info("[ProcessHiderKM] Received message from user mode: %s",
        (char*)nlmsg_data(pnlh));  
    
    uiPortID = pnlh->nlmsg_pid;

    for (int i = 0; i < 32; ++i) {
        char* pc = (char*)nlmsg_data(pnlh) + i;
        uiTargetPID += (*pc - '0') << i;
    }

    char acMsg[] = "ok";
    uiMsgSize = strlen(acMsg);

    pskbOutput = nlmsg_new(uiMsgSize, 0);
    if (!pskbOutput) {
        pr_info("[ProcessHiderKM] nlmsg_new failed");
        return;
    }

    pnlh = nlmsg_put(pskbOutput, 0, 0, NLMSG_DONE, uiMsgSize, 0);

    NETLINK_CB(pskbOutput).dst_group = 0;

    strncpy(nlmsg_data(pnlh), acMsg, uiMsgSize);

    uiRes = nlmsg_unicast(pNetlinkSock, pskbOutput, uiPortID);

    if (uiRes < 0) {
        pr_info("[ProcessHiderKM] nlmsg_unicast failed, return value is %d", uiRes);
        return;
    }
}

static int netlinkBindRoutine(struct net *net, int group) {
    pr_info("[ProcessHiderKM] netlinkBindRoutine called");

    return 0;
}

MODULE_LICENSE("GPL");