# S5PV210 自动构建和烧录工具

## 简介

这是一个用于自动构建项目并准备烧录bin文件到S5PV210开发板的脚本工具。

## 功能特性

- 自动清理并构建项目
- 生成scale.bin文件到output目录
- 提供详细的Windows平台烧录步骤说明
- 显示bin文件大小信息

## 依赖项

在Linux系统上使用本工具前，需要安装以下依赖：

1. **交叉编译工具链**：`arm-none-eabi-gcc`

## 安装依赖

### Ubuntu/Debian系统

```bash
# 安装交叉编译工具链
sudo apt-get install gcc-arm-none-eabi
```

## 使用方法

### 第一步：在Linux系统上构建项目

1. **运行构建脚本**：

   ```bash
   ./tools/linux/flash.sh
   ```

2. **脚本执行步骤**：
   - 清理旧的构建文件
   - 构建项目生成bin文件
   - 检查bin文件是否成功生成
   - 显示详细的Windows平台烧录步骤

### 第二步：在Windows系统上烧录到开发板

按照脚本显示的步骤操作：

1. **打开MobaXterm串口工具**
2. **点击Servers选项打开TFTP服务**
   - 设置TFTP根目录为项目的output文件夹
   - output文件夹路径：项目根目录下的output文件夹
3. **在MobaXterm中打开串口命令行**
4. **点击开发板电源键开机**
5. **在串口命令行中依次输入以下命令**：
   ```
   tftp 0x30000000 scale.bin
   go 0x30000000
   ```

## 目录结构

```
tools/linux/
├── flash.sh       # 自动构建和烧录准备脚本
├── mkv210         # S5PV210镜像生成工具
└── README.md      # 本说明文件
```

## 注意事项

1. 确保开发板已通过串口连接到Windows电脑
2. 确保MobaXterm已正确配置串口参数
3. 烧录前确保开发板电源已关闭
4. TFTP服务的根目录必须正确设置为output文件夹

## 故障排除

- **bin文件未生成**：检查交叉编译工具链是否正确安装
- **构建失败**：查看控制台输出的错误信息
- **TFTP传输失败**：检查MobaXterm的TFTP服务是否正常运行，根目录设置是否正确

