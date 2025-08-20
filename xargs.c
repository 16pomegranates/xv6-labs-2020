// user/xargs.c
#include "kernel/types.h"
#include "kernel/param.h"
#include "user/user.h"

static void run_line(char *line, char **base, int m) {
  // 组装 argv：先放固定前缀参数，再把本行按空白切成参数
  char *args[MAXARG];
  int k = 0;
  for (int i = 0; i < m; i++) args[k++] = base[i];

  // 把 line 按空白分词，依次追加到 args
  char *p = line;
  while (*p) {
    while (*p == ' ' || *p == '\t') p++;
    if (*p == 0) break;
    if (k >= MAXARG - 1) break;   // 预留最后一个给 NULL
    args[k++] = p;
    while (*p && *p != ' ' && *p != '\t') p++;
    if (*p) *p++ = 0;             // 断词并前进
  }
  args[k] = 0;

  if (k == 0) return;             // 空行或只有空白，跳过

  int pid = fork();
  if (pid == 0) {
    exec(args[0], args);
    // 只有 exec 失败才会走到这里
    fprintf(2, "xargs: exec %s failed\n", args[0]);
    exit(1);
  } else if (pid > 0) {
    wait(0); // 简化：逐行顺序等待
  } else {
    fprintf(2, "xargs: fork failed\n");
    exit(1);
  }
}

int
main(int argc, char *argv[])
{
  if (argc < 2) {
    fprintf(2, "usage: xargs command [args...]\n");
    exit(1);
  }

  // 固定前缀参数（命令 + 其自带参数）
  char *base[MAXARG];
  int m = 0;
  for (int i = 1; i < argc && m < MAXARG - 1; i++) base[m++] = argv[i];

  // 逐字符读 stdin，按行触发一次执行
  char buf[512];
  int len = 0;
  char c;
  while (1) {
    int n = read(0, &c, 1);
    if (n < 1) {
      // EOF：若缓冲里还有未处理的一行，执行之
      if (len > 0) {
        buf[len] = 0;
        run_line(buf, base, m);
      }
      break;
    }
    if (c == '\n') {
      buf[len] = 0;
      run_line(buf, base, m);
      len = 0;
    } else {
      if (len + 1 < sizeof(buf)) {
        buf[len++] = c;
      }
      // 行太长时静默截断（亦可选择报错）
    }
  }
  exit(0);
}
