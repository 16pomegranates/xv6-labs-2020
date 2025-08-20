// user/find.c
#include "kernel/types.h"
#include "kernel/stat.h"
#include "kernel/fs.h"
#include "user/user.h"

static void dofind(const char *path, const char *target) {
  int fd;
  struct stat st;

  if ((fd = open(path, 0)) < 0) {
    fprintf(2, "find: cannot open %s\n", path);
    return;
  }
  if (fstat(fd, &st) < 0) {
    fprintf(2, "find: cannot stat %s\n", path);
    close(fd);
    return;
  }

  if (st.type == T_FILE) {
    // 取 path 的 basename 与目标名比较
    const char *p = path + strlen(path);
    while (p > path && *(p - 1) != '/') p--;
    if (strcmp(p, target) == 0) {
      printf("%s\n", path);
    }
  } else if (st.type == T_DIR) {
    char buf[512], *p;
    struct dirent de;

    if (strlen(path) + 1 + DIRSIZ + 1 > sizeof(buf)) {
      fprintf(2, "find: path too long\n");
      close(fd);
      return;
    }

    strcpy(buf, path);
    p = buf + strlen(buf);
    *p++ = '/';

    while (read(fd, &de, sizeof(de)) == sizeof(de)) {
      if (de.inum == 0) continue;

      // 把目录项名拷到 buf 并补 NUL，避免 DIRSIZ 的未终止问题
      memmove(p, de.name, DIRSIZ);
      p[DIRSIZ] = 0;

      // 跳过 "." 和 ".."
      if (strcmp(p, ".") == 0 || strcmp(p, "..") == 0) continue;

      // 递归进入子路径 buf = "<path>/<name>"
      dofind(buf, target);
    }
  }

  close(fd);
}

int main(int argc, char *argv[]) {
  if (argc != 3) {
    fprintf(2, "usage: find <path> <name>\n");
    exit(1);
  }
  dofind(argv[1], argv[2]);
  exit(0);
}
