// user/primes.c
#include "kernel/types.h"
#include "user/user.h"

static void sieve(int left_read_fd) {
  int p;

  // 读到第一个数作为当前进程的素数 p
  if (read(left_read_fd, &p, sizeof(p)) != sizeof(p)) {
    // 管道已无数据，结束
    close(left_read_fd);
    exit(0);
  }

  // 打印当前素数
  printf("prime %d\n", p);

  // 为右侧下一个筛子进程准备一根新管道
  int right[2];
  pipe(right);

  int pid = fork();
  if (pid < 0) {
    // fork 失败
    close(left_read_fd);
    close(right[0]);
    close(right[1]);
    exit(1);
  }

  if (pid == 0) {
    // 子进程：作为下一个筛子，只需要读端
    close(right[1]);       // 子进程不用写端
    close(left_read_fd);   // 子进程不再用左边这根管道
    sieve(right[0]);       // 递归进入下一层筛子
    // 不会返回
    exit(0);
  } else {
    // 父进程：过滤 left_read_fd 中的剩余数字，写给 right[1]
    close(right[0]); // 父进程不用读端
    int n;
    while (read(left_read_fd, &n, sizeof(n)) == sizeof(n)) {
      if (n % p != 0) {
        write(right[1], &n, sizeof(n));
      }
    }
    // 所有数据写完后，务必关闭写端，通知子进程 EOF
    close(right[1]);
    close(left_read_fd);

    // 等待子进程（其又会等待它的子进程……），确保整条流水线都结束
    wait(0);
    exit(0);
  }
}

int
main(int argc, char *argv[]) {
  int p[2];
  pipe(p);

  int pid = fork();
  if (pid < 0) {
    close(p[0]);
    close(p[1]);
    exit(1);
  }

  if (pid == 0) {
    // 子进程：作为第一层筛子，从 p[0] 读
    close(p[1]);  // 子进程不用写端
    sieve(p[0]);  // 进入筛子逻辑
    // 不会返回
    exit(0);
  } else {
    // 父进程：喂入 2..35
    close(p[0]);  // 父进程不用读端
    for (int i = 2; i <= 35; i++) {
      write(p[1], &i, sizeof(i));
    }
    close(p[1]);  // 写完必须关闭，触发下游 read==0
    // 等最顶层子进程（它会层层等待）结束
    wait(0);
    exit(0);
  }
}
