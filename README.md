# wch-support-cherryusb

基于 [CherryUSB](https://github.com/cherry-embedded/CherryUSB) 协议栈的 WCH 系列 MCU USB 开发工程，使用 [RT-Thread nano](https://github.com/RT-Thread/rtthread-nano) (v4.1.1) 作为 RTOS。

## 支持的芯片

| 系列 | 芯片 | IP | 工具链 |
| --- | --- | --- | --- |
| CH32H417 | ch32h415reu6 / ch32h416rdu6 / ch32h417meu6 / ch32h417qeu6 / ch32h417weu6 | usbfs / usbhs / usbss | RISC-V Embedded GCC15 |
| CH32L103 | ch32l103c8t6 / ch32l103f8p6 / ch32l103f8u6 / ch32l103g8r6 / ch32l103k8u6 / ch32l103k8u7 / ch32m103g8r6 | usbfs | RISC-V Embedded GCC15 |
| CH32V4x7 | ch32v407ret6 / ch32v407vet6 / ch32v407weu6 / ch32v467ret6 / ch32v467vet6 / ch32v467weu6 | usbhs | RISC-V Embedded GCC15 |
| CH32V20x | ch32v208wbu6 / ch32v208rbt6 / ch32v208cbu6 / ch32v208gbu6 / ch32v203rbt6 / ch32v203c8u6 / ch32v203c8t6 / ch32v203k8u7 / ch32v203k8t6 / ch32v203g8r6 / ch32v203f8u6 / ch32v203f8p6 / ch32v203c6u6 / ch32v203c6t6 / ch32v203g6u6 / ch32v203f6p6 | usbfs | RISC-V Embedded GCC |
| CH32V30x | ch32v303cbt6 / ch32v303rbt6 / ch32v303rct6 / ch32v303rct7 / ch32v303vct6 / ch32v305cct6 / ch32v305fbp6 / ch32v305gbu6 / ch32v305rbt6 / ch32v307rct6 / ch32v307vct6 / ch32v307wcu6 / ch32v317vct6 / ch32v317wcu6 | usbfs | RISC-V Embedded GCC |
| CH32V205 | ch32v203cct6 / ch32v205cct6 / ch32v205rct6 / ch32v205vct6 | usbfs / usbhs | RISC-V Embedded GCC12 |
| CH32X3x5 | ch32x305rct6 / ch32x315ccu6 / ch32x315mcu6 / ch32x315wcu6 | usbhs / usbss | RISC-V Embedded GCC15 |

## 目录结构

```
├── CherryUSB/                  # CherryUSB 协议栈 (git submodule)
├── chips/                      # 芯片支持包，按系列划分
│   └── <family>/
│       ├── sdk/                # WCH 官方外设库、启动文件、链接脚本
│       ├── libcpu/             # 上下文切换与中断入口
│       ├── board/              # 板级初始化、系统时钟、中断向量
│       ├── <chip>.mk           # 芯片相关编译配置 (每个型号一个)
│       └── family.mk           # 系列相关编译配置 (工具链、源文件、编译选项)
├── rtos/
│   └── rtthread-nano/          # RT-Thread nano (git submodule)
├── src/                        # 用户代码
│   ├── main.c                  # 应用入口
│   ├── usb_config.h            # CherryUSB 协议栈配置
│   └── rtconfig.h              # RT-Thread 内核配置
├── build/                      # 编译输出
└── makefile                    # 顶层 Makefile
```

## 配置环境

1. 找到 MRS 安装目录（以默认安装路径为例，请按实际安装位置调整）：

   ```
   C:\MounRiver\MounRiver_Studio2\resources\app\resources\win32\others\Build_Tools\Make\bin
   C:\MounRiver\MounRiver_Studio2\resources\app\resources\win32\components\WCH\Toolchain\RISC-V Embedded GCC\bin
   C:\MounRiver\MounRiver_Studio2\resources\app\resources\win32\components\WCH\Toolchain\RISC-V Embedded GCC12\bin
   C:\MounRiver\MounRiver_Studio2\resources\app\resources\win32\components\WCH\Toolchain\RISC-V Embedded GCC15\bin
   ```

2. 打开 `设置 → 系统 → 关于 → 高级系统设置 → 环境变量`，在 "系统变量" 或 "用户变量" 中找到 `Path`，点击“编辑 → 新建”，分别将上面路径添加进去，然后一路确定保存。

3. **打开终端** 使环境变量生效，验证配置:

   ```bash
   riscv-none-embed-gcc -v
   riscv-wch-elf-gcc -v
   riscv32-wch-elf-gcc -v
   make -v
   ```

   能正常输出版本信息即配置成功。

## 编译

克隆时初始化子模块:

```bash
git submodule update --init
```

编译指定芯片:

```bash
make all -j8 CHIP=ch32v205rct6
```

芯片同时支持多个 USB 控制器时，可指定 `IP` ：

```bash
make all -j8 CHIP=ch32v205rct6 IP=usbfs
```

清理:

```bash
make clear
```

输出文件位于 `build/<chip>/output/`，包含 `.elf`、`.bin`、`.hex`、`.lst` 及 `.map`。

## 更换示例程序

修改项目根目录的makefie文件 `# --- Add Demo and Class Source Files ---` 这一段即可更换参与编译的DEMO程序：

```makefile
# --- Add Demo and Class Source Files ---
SRCS += \
	$(wildcard $(CURDIR)/CherryUSB/demo/cdc_acm_template.c) \
	$(wildcard $(CURDIR)/CherryUSB/class/cdc/usbd_cdc_acm.c) \
```

更换示例分三步：

1. 把 demo 换成要跑的模板，例如 `CherryUSB/demo/hid_keyboard_template.c`；
2. 补上该 demo 依赖的 class 驱动源码，组合类示例需要把用到的驱动都列上；
3. 同步修改 `src/main.c`，调用新 demo 的入口函数。
