//
//  main.cpp
//  操作系统实验五
//
//  Created by 何忆源 on 2020/5/10.
//  Copyright © 2020 何忆源. All rights reserved.
//

#include <time.h>
#include <iostream>
using namespace std;

#define H 10  // 缓存的大小
#define MAX 10 // 最大可以输入的字符
typedef struct Pcb {
    char name[10];      // 进程名
    char state[10];    // 运行状态
    int breakp;         // 断点保护
    Pcb *next;   // 阻塞时的顺序
}*link;

int producer_s, consumer_s; // 信号量
char str[MAX]; // 输入的字符串
char buffer[10]; // 缓冲池
int len; // 输入字符长度
int sp = 0; //string 的指针
int count1; // 字符计数器
int con_cnt; // 消费计数器
link producer;// 生产者进程
link consumer;// 消费者进程
link ready; // 就绪队列
link producerBlock; //s1 阻塞队列
link consumerBlock; //s2 阻塞队列
void block(int s);// 阻塞函数
void wakeup(int s);// 唤醒函数
int PC; // 指令计数器
void init() { // 初始化
    producer_s = 10;
    consumer_s = 0;
    // 创建生产者进程
    producer = (link)malloc(sizeof(Pcb));
    strcpy(producer->name, "Producer");
    strcpy(producer->state, "Ready");
    producer->breakp = 0;
    producer->next = NULL;
    //创建消费者进程
    consumer = (link)malloc(sizeof(Pcb));
    strcpy(consumer->name, "Consumer");
    strcpy(consumer->state, "Ready");
    consumer->breakp = 0;
    consumer->next = NULL;
    //打开就绪队列
    ready = producer;
    ready->next = consumer;
    consumer->next = NULL;
    producerBlock = NULL;
    consumerBlock = NULL;// 阻塞进程为 NULL
    PC = 0;
    con_cnt = 0; // 消费计数器
}
void p(int s) {
    if (s == 1) {
        producer_s--;
        if (producer_s < 0)//信号量小于0阻塞
            block(1);
        else {
            //printf("\t 生产者进行P操作\n");
            ready->breakp = PC;
        }
    }
    else {
        consumer_s--;
        if (consumer_s < 0)
            block(2);
        else {
           
            //printf("\t 消费者进行P操作\n");
            ready->breakp = PC;
        }
    }
}
void v(int s) {
    if (s == 1) {
        producer_s++;
        if (producer_s <= 0)
            wakeup(1);
        ready->breakp = PC;
    }
    else {
        consumer_s++;
        if (consumer_s <= 0)
            wakeup(2);
        ready->breakp = PC;
    }
}
//阻塞函数
void block(int s) {
    link p;
    if (s == 1) {// 生产进程
        strcpy(producer->state, "Block");
        p = producerBlock;
        while (p) {
            p = p->next;//p 的值为 NULL,表示队尾
        }
        if (!producerBlock)//如果阻塞队列为空
            producerBlock = producer;
        else
            p = producer;//将生成者进程置于阻塞队列末尾
        producer->next = NULL;
        printf("\t 生产进程阻塞了 !\n");
        ready->breakp = PC;
        ready = ready->next;
    }
    else {//消费进程
        strcpy(consumer->state, "Block");
        p = consumerBlock;
        while (p) {

            p = p->next;
        }
        if (!consumerBlock)
            consumerBlock = consumer;
        else
        p = consumer;
        ready->breakp = PC;
        ready = ready->next;
        consumer->next = NULL;
        printf("\t 消费进程阻塞了 !\n");
    }
}
//唤醒函数
void wakeup(int s) {
    link p;
    link q = ready;
    if (s == 1) {
        p = producerBlock;
        producerBlock = producerBlock->next;// 阻塞指针指向下一个阻塞进程
        strcpy(p->state, "Ready");
        //将生产者进程插入就绪队列队尾
        while (q)
        q = q->next;
        q = p;
        p->next = NULL;
        printf("\t 生产进程唤醒了 !\n");
    }
    else {
        p = consumerBlock;
        consumerBlock = consumerBlock->next;
        strcpy(p->state, "Ready");
        // 将消费者进程插入就绪队列队尾
        while (q->next)
        q = q->next;
        q->next = p;
        p->next = NULL;
        printf("\t 消费进程唤醒了 !\n");
    }
}
void control() // 处理器调度程序
{
    int r;
    int num = 0;
    link p = ready;
    if (ready == NULL) // 若无就绪进程 , 结束
        return;
    while (p) // 统计就绪进程个数
    {
        num++;
        p = p->next;// 最终 p 变为 NULL
    }
    time_t t;
    srand((unsigned)time(&t));
    r = rand() % num;// 随机函数产生随机数
    if (r == 1) {
        p = ready;
        ready = ready->next;
        ready->next = p;
        p->next = NULL;
        strcpy(ready->state, "Run");
        strcpy(ready->next->state, "Ready");
    }
    else
        strcpy(ready->state, "Run");
    PC = ready->breakp;
}
char producerRecord[MAX];// 生产记录
int prod = 0;// 生产记录指针
char consumerRecord[MAX];// 消费记录
int cons = 0;// 消费记录指针
int producer_p = 0; // 生产者指针
int consumer_p = 0; // 消费者指针
char temp;
void processor() { // 模拟处理器指令执行
    if (strcmp(ready->name, "Producer") == 0) // 当前进程为生产者
        switch (PC)
        {
        case 0://produce
            printf("\t 生产者生产了字符 %c\n", str[sp]);
            producerRecord[prod] = str[sp];// 添加到生产记录
            sp = (sp + 1) % len;
            PC++;
            ready->breakp = PC; // 保存断点
            break;
        case 1:
            PC++;
            printf("\t 生产者进行P操作\n");
            p(1);
            break;
        case 2: //put
            buffer[producer_p] = producerRecord[prod]; // 放到缓冲区
            printf("\t %c 字符进入缓存池 \n", buffer[producer_p]);
            prod++;
            producer_p = (producer_p + 1) % H;
            PC++;
            ready->breakp = PC; // 保存断点
            break;
        case 3: //v(s2)
            PC++;
            printf("\t 生产者进行V操作\n");
            v(2);
            break;
        case 4://goto
            printf("\t 生产进程 goto 0 操作\n");
            PC = 0;
            count1--; // 剩余字符个数减 1
            ready->breakp = PC; // 保存断点
            if (count1 <= 0) {
                printf("\t 生产者结束生产 !\n");
                strcpy(producer->state, "Stop");
                ready->breakp = -1;
                ready = ready->next;
            }
        }
    else  // 当前进程为消费者
        switch (PC)
        {
        case 0:
            printf("\t消费者进行P操作！\n");
            PC++;
            p(2);
            break;
        case 1: //get
           // printf("\t 消费者进行V操作");
            printf("\t 消费者取字符 !\n");
            temp = buffer[consumer_p];
            consumer_p = (consumer_p + 1) % H;
            PC++;
            ready->breakp = PC; // 保存断点
            break;
        case 2: //v(s1)
            PC++;
            printf("\t 消费者进行V操作\n");
            v(1);
            break;
        case 3: //consume
            printf("\t 消费了字符 %c\n", temp);
            consumerRecord[cons] = temp;
            cons++;
            con_cnt++;
            if (con_cnt >= len) {
                strcpy(consumer->state, "Stop");
                consumer->breakp = -1;
                return;
            }
            PC++;
            ready->breakp = PC; // 保存断点
            break;
        case 4: //goto0
            printf("\t 消费进程 goto 0 操作\n");
            PC = 0;
            ready->breakp = PC; // 保存断点
        }
}
int main() {
    printf(" 请输入要生产的字符串字符串 :\n");
    scanf("%s", str ,10 );    //string 数组存放将要产生的字符
    len = strlen(str);
    count1 = len;    // 输入字符的个数
    init();
    while (con_cnt < len) // 消费完所有的字符为结束
    {
        control();
        processor();
        int i;
        printf("\t1(继续) 0(退出)\n");
        scanf("%d", &i);
        if (i == 0) {
            exit(0);
        }
    }
    printf("\n 程序结束 !\n");
}
