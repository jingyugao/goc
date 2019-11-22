typedef struct{
    long buffer[8];
}Context;

int SaveContext(Context *ctx){
    asm(
            "popq %%rsi;"
            "xorl %%eax,%%eax;"
            "movq %%rsp,8%0;"
            "pushq %%rsi;"
            "movq %%rbp,16%0;"
            "movq %%r12,24%0;"
            "movq %%r13,32%0;"
            "movq %%r14,40%0;"
            "movq %%r15,48%0;"
            "movq %%rsi,56%0;"
            "popq	%%rbp;"
	        "retq;"
            :
            :"m"(ctx->buffer)
            :"%rax"
            );
    // never reached;
    return 0;
};

int GetContex(Context* ctx){
    asm(
            "movl $1,%%eax;"
            "movq %0,%%rbx;"
            "movq 8%0,%%rsp;"
            "movq 16%0,%%rbp;"
            "movq 24%0,%%r12;"
            "movq 32%0,%%r13;"
            "movq 40%0,%%r14;"
            "movq 48%0,%%r15;"
            "jmp *56%0;"
            :
            :"m"(ctx->buffer)
            :
    );
};


