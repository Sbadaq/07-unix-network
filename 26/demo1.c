/**
 * @file demo1.c
 * @author your name (you@domain.com)
 * @brief 
 *
 * @version 0.1
 * @date 2025-03-20
 * 
 * @copyright Copyright (c) 2025
 * 
 *  
 */

//  为了在C语言中使用pthread库创建多线程并传递多个参数，可以将参数封装在一个结构体中，并将结构体指针传递给线程函数。以下是详细步骤和示例代码：

// 步骤说明
// ​定义结构体：创建一个结构体类型，包含需要传递的所有参数。
// ​动态分配内存：在主线程中为结构体分配堆内存，避免栈溢出。
// ​初始化参数：设置结构体各字段的值。
// ​创建线程：将结构体指针作为参数传递给pthread_create。
// ​线程函数处理：在线程函数中接收结构体指针，解包参数并使用。
// ​释放内存：线程结束后释放结构体占用的内存。
// ​同步线程：使用pthread_join确保主线程等待子线程完成。

 
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

// 定义包含多个参数的结构体
typedef struct {
    int num1;
    int num2;
    char *message;
} ThreadData;

typedef struct {
    int sum;
}OutputRes;

// 线程函数声明
void* thread_function(void *argument);

int main() {
    pthread_t thread_id;
    ThreadData *data = NULL;

    // 动态分配结构体内存
    data = (ThreadData *)malloc(sizeof(ThreadData));
    if (data == NULL) {
        perror("malloc failed");
        exit(EXIT_FAILURE);
    }

    // 初始化参数
    data->num1 = 42;
    data->num2 = 100;
    data->message = "Hello from the thread!";

    // 创建线程，传递结构体指针
    int rc = pthread_create(&thread_id, NULL, thread_function, (void *)data);
    if (rc != 0) {
        perror("pthread_create failed");
        exit(EXIT_FAILURE);
    }

    OutputRes *res;
    // 等待线程结束
    pthread_join(thread_id, (void**)&res);

    printf("Main thread get sum = %d\n",res->sum);
    free(res);
    // （可选）此处可释放数据，但若线程已处理并free，则无需再次释放
    // free(data);

    return 0;
}

// 线程函数实现
void* thread_function(void *argument) {
    ThreadData *params = (ThreadData *)argument;
    OutputRes* res = malloc(sizeof(OutputRes));
    // 使用参数进行计算
    res->sum = params->num1 + params->num2;
    

    printf("Thread received parameters: %d + %d = %d\n", params->num1, params->num2,  res->sum);
    printf("%s\n", params->message);

    // 释放结构体内存
    free(params);
    params = NULL; // 防止悬空指针
   
   
    return res;
}