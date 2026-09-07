# wch-support-cherryusb

基于 [CherryUSB](https://github.com/cherry-embedded/CherryUSB) 协议栈的 WCH 系列 MCU USB 开发工程。

## 支持的芯片

| 系列 | 芯片 |
| --- | --- |
| CH32V205 | CH32V205RC |
| CH32V30x | CH32V303RC / CH32V305RB / CH32V307VC |
| CH32V4x7 | CH32V407VE |

## 目录结构

```
├── CherryUSB/                  # CherryUSB 协议栈 (git submodule)
├── hw/                         # 芯片支持包
│   └── ch32xxxx/
│       ├── sdk/                # WCH 官方外设库
│       ├── system/             # 系统文件
│       ├── chips/<chip>/
│       │   ├── board/          # 板级初始化、usb_config.h
│       │   ├── linker_script/  # 链接脚本
│       │   └── chip.mk         # 芯片相关编译配置
│       └── family.mk           # 系列相关编译配置
├── src/                        # 用户代码
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

3. **重新打开终端** 使环境变量生效，验证配置:

   ```bash
   riscv32-wch-elf-gcc -v
   make -v
   ```

   两条命令均能正常输出版本信息即配置成功。

## 编译

克隆时初始化子模块:

```bash
git submodule update --init
```

编译指定芯片:

```bash
make all -j8 CHIP=ch32v307vc
```

清理:

```bash
make clear
```

输出文件位于 `build/<chip>/output/`，包含 `.elf`、`.bin`、`.hex`、`.lst` 及 `.map`。
