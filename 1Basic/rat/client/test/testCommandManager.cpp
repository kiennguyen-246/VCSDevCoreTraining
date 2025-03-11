#include <CUnit/Basic.h>
#include <CUnit/CUnit.h>

#include "CommandManager.hpp"

int randInt(unsigned int uiUpperBound) {
  int u = rand();
  int v = rand();
  int w = rand();
  return 1ll * (u + v) * w % uiUpperBound;
}

int randInt(int iLowerBound, int iUpperBound) {
  return iLowerBound + randInt((unsigned)(iUpperBound - iLowerBound));
}

char randFileChar() {
  std::string sCharset =
      "1234567890qwertyuiop[]]asdfghjkl=zxcvbnm,.!$%^_+QWERTYUIOP{ASDFGHJKL:"
      "ZXCVBNM";
  return sCharset[randInt(sCharset.size())];
}

void testSingleton() {
  auto pcmi1 = CommandManager::getInstance();
  auto pcmi2 = CommandManager::getInstance();
  CU_ASSERT(pcmi1 == pcmi2);
}

void testStartupDir() {
  auto pcmi = CommandManager::getInstance();
  char pcCwd[4096];
  getcwd(pcCwd, sizeof(pcCwd));
  CU_ASSERT(std::string(pcCwd) == "/");
}

void testPwd() {
  auto pcmi = CommandManager::getInstance();
  chdir("/usr/include");

  std::string sCwd;
  bool bIsTruncated;
  pcmi->pwd(&sCwd, &bIsTruncated);

  char pcCwd[4096];
  getcwd(pcCwd, sizeof(pcCwd));

  CU_ASSERT(sCwd == std::string(pcCwd));
}

void testPwdVeryShortDirectory() {
  auto pcmi = CommandManager::getInstance();
  chdir("/");

  std::string sCwd;
  bool bIsTruncated;
  pcmi->pwd(&sCwd, &bIsTruncated);

  char pcCwd[4096];
  getcwd(pcCwd, sizeof(pcCwd));

  CU_ASSERT(std::string(pcCwd) == "/");
}

void testPwdVeryLongDirectory() {
  auto pcmi = CommandManager::getInstance();

  std::string sLongFolderName = "/home/kiennd19/Temp/x";
  for (int i = 0; i < 16; i++) {
    for (int j = 0; j <= 254; j++) sLongFolderName.push_back(randFileChar());
    sLongFolderName.push_back('/');
    while (sLongFolderName.size() > 4095) sLongFolderName.pop_back();
    system(std::format("mkdir {}", sLongFolderName).c_str());
  }
  chdir(sLongFolderName.c_str());

  std::string sCwd;
  bool bIsTruncated;
  pcmi->pwd(&sCwd, &bIsTruncated);

  char pcCwd[4096];
  getcwd(pcCwd, sizeof(pcCwd));

  system("rm -rf /home/kiennd19/Temp/x");
  CU_ASSERT(sCwd == std::string(pcCwd));
}

void testCd() {
  auto pcmi = CommandManager::getInstance();

  std::string sCwd;
  bool bIsTruncated;
  pcmi->cd("/usr/lib", &sCwd, &bIsTruncated);

  char pcCwd[4096];
  getcwd(pcCwd, sizeof(pcCwd));

  CU_ASSERT(std::string(pcCwd) == "/usr/lib");
}

void testCdVeryShortDirectory() {
  auto pcmi = CommandManager::getInstance();

  std::string sCwd;
  bool bIsTruncated;
  pcmi->cd("/", &sCwd, &bIsTruncated);

  char pcCwd[4096];
  getcwd(pcCwd, sizeof(pcCwd));

  CU_ASSERT(std::string(pcCwd) == "/");
}

void testCdVeryLongDirectory() {
  auto pcmi = CommandManager::getInstance();

  std::string sLongFolderName = "/home/kiennd19/Temp/x";
  for (int i = 0; i < 16; i++) {
    for (int j = 0; j <= 254; j++) sLongFolderName.push_back('A' + i);
    sLongFolderName.push_back('/');
    while (sLongFolderName.size() > 4095) sLongFolderName.pop_back();
    system(std::format("mkdir {}", sLongFolderName).c_str());
  }

  std::string sCwd;
  bool bIsTruncated;
  pcmi->cd(sLongFolderName, &sCwd, &bIsTruncated);

  char pcCwd[4096];
  getcwd(pcCwd, sizeof(pcCwd));

  system("rm -rf /home/kiennd19/Temp/x");
  CU_ASSERT(sCwd == sLongFolderName);
}

int main() {
  CU_initialize_registry();
  CU_pSuite suiteOverall = CU_add_suite("Overall test", NULL, NULL);
  CU_add_test(suiteOverall, "Singleton test", testSingleton);
  CU_add_test(suiteOverall, "Application Startup Directory Test", testStartupDir);

  CU_pSuite suitePwd = CU_add_suite("Working Directory test", NULL, NULL);
  CU_add_test(suitePwd, "Working Directory Norm Test", testPwd);
  CU_add_test(suitePwd, "Working Directory Low test", testPwdVeryShortDirectory);
  CU_add_test(suitePwd, "Working Directory High test", testPwdVeryLongDirectory);

  CU_pSuite suiteCd = CU_add_suite("Change Directory test", NULL, NULL);
  CU_add_test(suiteCd, "Change Directory Norm Test", testPwd);
  CU_add_test(suiteCd, "Change Directory Low test", testPwdVeryShortDirectory);
  CU_add_test(suiteCd, "Change Directory High test", testPwdVeryLongDirectory);

  CU_basic_set_mode(CU_BRM_VERBOSE);
  CU_basic_run_tests();
  CU_cleanup_registry();

  return 0;
}