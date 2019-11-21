

typedef struct{
    long buffer[8];
}Context;

int SaveContext(Context *ctx){
    asm(
            "popq %%rsi;"
            "xorl %%eax, %%eax;"
            "movq %%rbx,%0;"
            "movq %%rsp,8%0;"
            "pushq %%rsi;"
            "movq %%rbp,16%0;"
            "movq %%r12,24%0;"
            "movq %%r13,32%0;"
            "movq %%r14,40%0;"
            "movq %%r15,48%0;"
            "movq %%rsi,56%0;"
            :
            :"m"(ctx->buffer)
            :"%rax"
            );
};
