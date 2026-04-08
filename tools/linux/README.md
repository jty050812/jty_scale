# S5PV210 自动烧录工具

## 简介

这是一个用于自动构建并烧录bin文件到S5PV210开发板的脚本工具。

## 功能特性

- 自动构建项目生成bin文件
- 检查tftp服务状态
- 自动复制bin文件到tftp根目录
- 提供详细的烧录步骤说明

## 依赖项

在使用本工具前，需要安装以下依赖：

1. **交叉编译工具链**：`arm-none-eabi-gcc`
2. **tftp客户端**：用于传输文件到开发板
3. **tftp服务器**：用于提供文件传输服务
4. **MobaXterm**：用于串口通信（Windows平台）

## 安装依赖

### Ubuntu/Debian系统

```bash
# 安装交叉编译工具链
sudo apt-get install gcc-arm-none-eabi

# 安装tftp客户端和服务器
sudo apt-get install tftp tftpd-hpa

# 配置tftp服务器
sudo nano /etc/default/tftpd-hpa
```

在配置文件中设置：

```
TFTP_USERNAME="tftp"
TFTP_DIRECTORY="/srv/tftp"
TFTP_ADDRESS=":69"
TFTP_OPTIONS="--secure"
```

创建tftp目录并设置权限：

```bash
sudo mkdir -p /srv/tftp
sudo chmod 777 /srv/tftp
```

重启tftp服务：

```bash
sudo systemctl restart tftpd-hpa
```

## 使用方法

1. **运行烧录脚本**：

   ```bash
   ./tools/linux/flash.sh
   ```

2. **脚本执行步骤**：
   - 检查依赖项
   - 构建项目生成bin文件
   - 检查tftp服务状态
   - 复制bin文件到tftp根目录
   - 显示烧录步骤说明

3. **在MobaXterm中执行烧录**：
   - 确保开发板已连接到电脑并启动到U-Boot模式
   - 在MobaXterm串口工具中输入以下命令：
     ```
     tftp 0x30000000 scale.bin
     go 0x30000000
     ```

## 目录结构

```
tools/linux/
├── flash.sh       # 自动烧录脚本
├── mkv210         # S5PV210镜像生成工具
└── README.md      # 本说明文件
```

## 注意事项

1. 确保开发板与电脑在同一网络中
2. 确保tftp服务器已正确配置并运行
3. 确保MobaXterm已正确连接到开发板的串口
4. 烧录前请确保开发板已启动到U-Boot模式

## 故障排除

- **tftp服务未运行**：执行 `sudo systemctl start tftpd-hpa` 启动服务
- **bin文件未生成**：检查交叉编译工具链是否正确安装
- **烧录失败**：检查网络连接和tftp服务器配置
