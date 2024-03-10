变量和常量在 RAM 中使用了 63344 字节，总共有 80192 字节可用。这表示已使用的 RAM 空间占比为 78%。RAM 用于存储程序运行时的变量和数据。
    DATA 段使用了 1496 字节，用于存储初始化的变量。
    RODATA 段使用了 2168 字节，用于存储常量。
    BSS 段使用了 59680 字节，用于存储被初始化为零的变量。

IRAM（指令 RAM）中的代码使用了 60915 字节，总共有 65536 字节可用。这表示已使用的 IRAM 空间占比为 92%。IRAM 用于存储程序的指令，通常速度更快。
    ICACHE 段使用了 32768 字节，用于保留 Flash 指令缓存的空间。
    IRAM 段使用了 28147 字节，用于存储代码。

Flash 中的代码使用了 513512 字节，总共有 1048576 字节可用。这表示已使用的 Flash 空间占比为 48%。Flash 存储器用于存储程序的代码。
80K采样率 意味着 0.000125s 125us采样一次
主频160MHz 所以计数周期为 N*T=125 N*1/160*1000*1000=125us N=20000

void setup() {

  timer1_isr_init();

  timer1_enable(TIM_DIV1, TIM_EDGE, TIM_LOOP);  //此处选择div模式，timer触发方式（不要改），timer_loop循环，SINGLE单次

  timer1_write(25000);    //修改计数比较器，这里为25000个间隔触发中断，变量用uint32_t

  timer1_attachInterrupt(userFunc);

}

ICACHE_RAM_ATTR void userFunc() {

//触发中断执行的内容

}

void loop() {


} 作者：亚棍亚棍亚 https://www.bilibili.com/read/cv15922238/ 出处：bilibili
https://dl.espressif.com/dl/package_esp32_index.json