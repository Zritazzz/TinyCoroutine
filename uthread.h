/**
* @file  uthread.h
* @author Zrita
* @version 0.1
* @brief   :A asymmetric coroutine library for C++ 
*      1. Date: 2021-1-12 
*          Author: Zrita
*          Modification: this file was created 
*/

#ifndef MY_UTHREAD_H
#define MY_UTHREAD_H

#include <ucontext.h>

#define DEFAULT_STACK_SZIE (1024 * 128) //128kb
#define MAX_UTHREAD_SIZE 128

enum ThreadState //协程的四个状态，闲置，可用，正在运行，挂起
{
    FREE,
    RUNNABLE,
    SUSPEND
};

typedef void (*Fun)(void *arg);

//协程类
struct uthread_t
{
    ucontext_t ctx;                 //协程上下文
    Fun func;                       //协程所用函数
    void *arg;                      //携程调度器
    enum ThreadState state;         //协程状态
    char stack[DEFAULT_STACK_SZIE]; //协程运行的栈大小
};

//协程调度器
struct schedule_t
{
    ucontext_t main;    //主函数的上下文
    int running_thread; //正在运行的协程id
    uthread_t *threads; //协程数组的首地址指针
    int max_index;      //曾经使用到的最大的index + 1

    schedule_t() : running_thread(-1), max_index(0) //初始化
    {
        threads = new uthread_t[MAX_UTHREAD_SIZE]; //预先分配1024个协程空间，协程池?
        for (int i = 0; i < MAX_UTHREAD_SIZE; i++)
        {
            threads[i].state = FREE; //默认协程状态为FREE
        }
    }

    ~schedule_t()
    {
        delete[] threads;
    }
};

//恢复cpu控制权
void uthread_resume(schedule_t &schedule, int id)
{
    if (id < 0 || id >= schedule.max_index)
        return;

    uthread_t *t = &(schedule.threads[id]);

    if (t->state == SUSPEND)
    {
        swapcontext(&(schedule.main), &(t->ctx)); //t->ctx存储的是函数进行了一半时的上下文
    }
}

//交出cpu控制权
void uthread_yield(schedule_t &schedule)
{
    if (schedule.running_thread != -1)
    {
        uthread_t *t = &(schedule.threads[schedule.running_thread]);
        t->state = SUSPEND;
        schedule.running_thread = -1;

        swapcontext(&(t->ctx), &(schedule.main));
    }
}

//初次创建协程时执行的函数入口
static void uthread_body(schedule_t *ps)
{
    int id = ps->running_thread;

    if (id != -1)
    {
        uthread_t *t = &(ps->threads[id]);

        t->func(t->arg); //调用函数

        t->state = FREE; //函数执行完之后，协程变为FREE状态

        ps->running_thread = -1;
    }
}

//创建协程，分配相关信息和调用的栈空间
int uthread_create(schedule_t &schedule, Fun func, void *arg)
{
    int id = 0;

    for (id = 0; id < schedule.max_index; ++id)
    {
        if (schedule.threads[id].state == FREE)
        {
            break;
        }
    }

    if (id == schedule.max_index)
    {
        schedule.max_index++;
    }

    uthread_t *t = &(schedule.threads[id]);

    t->state = RUNNABLE;
    t->func = func;
    t->arg = arg;

    getcontext(&(t->ctx));  //保存当前上下文到ctx中

    t->ctx.uc_stack.ss_sp = t->stack;
    t->ctx.uc_stack.ss_size = DEFAULT_STACK_SZIE;
    t->ctx.uc_stack.ss_flags = 0;
    t->ctx.uc_link = &(schedule.main);
    schedule.running_thread = id; //将当前id变为正在调度的协程

    makecontext(&(t->ctx), (void (*)(void))(uthread_body), 1, &schedule); //使得ctx指向指向uthread_body函数
    swapcontext(&(schedule.main), &(t->ctx));                             //保存当前上下文到main上下文中，调用ctx执行body函数

    return id;
}

//检查有无正在运行的协程
int schedule_finished(const schedule_t &schedule)
{
    if (schedule.running_thread != -1)
    {
        return 0;
    }
    else
    {
        for (int i = 0; i < schedule.max_index; ++i)
        {
            if (schedule.threads[i].state != FREE)
            {
                return 0;
            }
        }
    }

    return 1;
}

#endif
