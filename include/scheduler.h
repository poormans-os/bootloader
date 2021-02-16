typedef int pid_t;
typedef long long register_t;

typedef struct
{
    register_t rax;
    register_t rbx;
    register_t rcx;
    register_t rdx;
    register_t eflags;
    register_t cs;
    register_t ds;
    register_t ss;
    register_t eip;
} registers_t;

typedef struct proc_t
{
    pid_t pid;
    registers_t regs;
    void *memLower;
    void *memUpper;
    struct proc_t *next;
} proc_t;