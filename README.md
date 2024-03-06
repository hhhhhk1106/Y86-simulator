# Report

## 后端开发<hr>

### 概览

1.  编译、运行方式
2.  cpu设计
3.  yo文件的处理
4.  json相关的处理
5.  扩展功能：显示当前执行的指令

### 编译、运行方式

1.  编译：`g++ cpuRun.cpp trans.cpp Y86.cpp -o cpu`
2.  运行：`./cpu.exe < xxx.yo > ans.json`
    输入当前目录下的xxx.yo文件，并将结果输出到ans.json中。

\*为实现展示上的功能，最终给前端的文件里对`cpuRun.cpp`做了一些修改，并更名为`cpuRun2.cpp`。其编译运行方式不变，但最后输出的json文件内容有所变化（不能通过脚本测试）。

<br>

### cpu设计

#### · 文件结构概览：

1.  `constVal.h`：按照教材命名方式存放了各类常值，如16个寄存器的名称，stat不同状态，不同操作指令、分支指令对应编码等；
    `wire.h`：设计了结构体wire，模拟cpu中所有传输数据的线，依然采取书上的名称，并按不同传输位数选择了对应的变量（64位，8位，bool类）。
2.  `Y86.h`, `Y86.cpp`：包含了基础的头文件，并以宏定义的方式设置了一些参数的最大值（icode，reg，mem的大小等），超过最大值会给出相应报错。同时设计了CPU类，其具体内容与实现会在下一节详细介绍。
3.  `trans.h`, `trans.cpp`：使用了json的库，辅助实现yo文件，cpu状态，json文件直接的相互转换。
4.  `cpuRun.h`, `cpuRun.cpp`：组合不同函数，最后生成可执行文件，模拟出完整可运行的cpu。

#### · CPU类的设计

1.  **类属性**：
    将硬件单元和线路集合（结构体wire）都列为变量，增添STAT变量来更好判断cpu运行状态，并实现测试所需标准输出格式。

    ```c++
    uint8_t STAT = SAOK;
    uint64_t PC = 0;
    uint64_t REG[16] = {0};
    uint8_t MEM[mem_size] = {0};
    bool CC[3] = {1,0,0};
    wire one;
    ```

2.  **类方法**：
    cpu本身功能：按照处理指令的六个阶段划分并实现对应函数，同时不同阶段可能经过的控制逻辑块、对于REG、MEM的读写操作也分别以函数实现，并列在对应阶段之后。

    ```c++
    // 6 processing stages:
    void fetch();
    void decode();
    void execute();
    void memory();
    void writeback();
    void updatePC();
    
    // (fetch)
    bool insValid();    // return 0 for illegal instruction
    
    // (decode)
    void setsrcA();
    void setsrcB();
    void setdstE();
    void setdstM();
    
    // (execute) get aluA, aluB, etc.
    uint64_t getaluA();
    uint64_t getaluB();
    uint64_t aluFun(uint8_t, uint64_t, uint64_t);   // calculate and return answer
    bool cond();    // return Cnd signal
    ```
    ```c++
    // (memory) get addr, data, etc.
    uint64_t getAddr();
    uint64_t getData();
    uint8_t setStat();
    bool readMem(uint64_t addr);    // return 0 if meet illeagal address
    bool writeMem(uint64_t addr, uint64_t data);
    
    // (updatePC)
    uint64_t setPC();
    ```
    
    输入输出：提供了不同接口来初始化cpu状态（通过标准输入，通过经处理的yo，通过json），并设置函数来将每条指令运行完的状态输出到json中。 
    
    ```c++
    void inState_json(const std::string &filePath);    // read in .json
    void inState_yoed(const std::string &filePath);    // read in .yoed (processed .yo)
    void inState();
    Json outState();    // put out .json
    ```
    
    此外还实现了一些工具函数：如进制间的简单转换，划分一个字节的高低4位，从指定地址取得64位数据，对不必要数据的清理，模拟书上HCL语言中in功能的函数等。其中in函数的参数个数不确定，上网查阅后选择使用初始化列表`initializer_list`解决此问题。

<br>

### yo文件的处理

* 将yo文件当做txt处理。观察发现有效的数据格式为`（地址）:（数据）`，且地址的前两位固定为`0x`，于是每次读入一整行，若满足上述格式，则将对应数字读入内存对应地址中。
\* 注意到可能出现地址增幅与数据长度不同，遂不采用顺序存储
* 一开始没能理解“将从`stdio`读取机器码”的含义，于是将yo处理为对应的CPU初始状态（PC=0），并以json或yoed（以二进制保存了内存的初始状态）的形式存储，再用该json或yoed来初始化CPU。得知输入重定向后，对函数做了相应调整，从原来对文件的操作转为对标准输入的处理。

<br>

### json相关的处理

听从助教建议选取了`nlohmann`的json库，添加文件夹到系统路径，并在使用时包含对应的头文件，成功得到测试所需的格式。
之后又添加`nlohmann/fifo_map.hpp`，使得json对象中键值对的顺序按插入顺序排列，而非默认的字典序。
\* 期间发现寄存器、内存中数据有时会发生错误，观察后发现是有无符号整型的问题，遂在输出到json时作类型转换，解决此问题。
<br>

### 扩展功能：显示当前执行的指令

1.  实现：
    函数以`switch`为主，通过不同`icode`、`ifun`以及可能有的寄存器、数据等，搭建出汇编指令令。并将这些指令（字符串）放入json数组中，最后输出整个数组。

    ```c++
    Json outIns();      // put out instrctions in json
    std::string getRegName(uint8_t);
    std::string getJxxName(uint8_t);
    std::string getOPName(uint8_t);
    ```

2.  应前端同学要求，将输出放在cpu状态之后，而非原先分别存放在两个json文件中。于是得到`cpuRun2.cpp`。

<br>
