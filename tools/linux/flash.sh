#!/bin/bash

# 自动烧录bin文件到s5pv210开发板的脚本

# 颜色定义
GREEN="\033[0;32m"
RED="\033[0;31m"
YELLOW="\033[1;33m"
NC="\033[0m" # No Color

echo -e "${GREEN}=== S5PV210 自动烧录工具 ===${NC}"

# 检查tftp是否安装
if ! command -v tftp &> /dev/null; then
    echo -e "${YELLOW}警告: tftp 命令未安装，请先安装 tftp${NC}"
    echo -e "${YELLOW}脚本将继续构建项目，但烧录需要tftp${NC}"
fi

# 检查mobaxterm是否在PATH中（可选）
if ! command -v mobaxterm &> /dev/null; then
    echo -e "${YELLOW}警告: mobaxterm 命令未在PATH中找到${NC}"
    echo -e "${YELLOW}请确保mobaxterm已安装并配置正确${NC}"
fi

# 构建项目
echo -e "${GREEN}正在构建项目...${NC}"
make clean
if make; then
    echo -e "${GREEN}构建成功!${NC}"  
else
    echo -e "${RED}构建失败，请检查错误信息${NC}"
    exit 1
fi

# 检查bin文件是否生成
BIN_FILE="output/scale.bin"
if [ ! -f "$BIN_FILE" ]; then
    echo -e "${RED}错误: bin文件未生成，请检查构建过程${NC}"
    exit 1
fi

echo -e "${GREEN}bin文件生成成功: $BIN_FILE${NC}"

# 启动tftp服务器（假设使用系统tftp服务）
echo -e "${GREEN}启动tftp服务器...${NC}"

# 检查tftp服务是否运行
if systemctl is-active --quiet tftpd-hpa; then
    echo -e "${GREEN}tftp服务已运行${NC}"
else
    echo -e "${YELLOW}tftp服务未运行，尝试启动...${NC}"
    if sudo systemctl start tftpd-hpa; then
        echo -e "${GREEN}tftp服务启动成功${NC}"
    else
        echo -e "${YELLOW}警告: 无法启动tftp服务，请手动启动${NC}"
    fi
fi

# 复制bin文件到tftp根目录
TFTP_ROOT="/srv/tftp"
if [ -d "$TFTP_ROOT" ]; then
    echo -e "${GREEN}复制bin文件到tftp根目录...${NC}"
    if sudo cp "$BIN_FILE" "$TFTP_ROOT/"; then
        echo -e "${GREEN}bin文件复制成功${NC}"
    else
        echo -e "${RED}错误: 无法复制bin文件到tftp根目录${NC}"
        exit 1
    fi
else
    echo -e "${YELLOW}警告: tftp根目录 $TFTP_ROOT 不存在${NC}"
    echo -e "${YELLOW}请确保tftp服务器已正确配置${NC}"
fi

# 显示烧录说明
echo -e "${GREEN}\n=== 烧录步骤 ===${NC}"
echo -e "1. ${YELLOW}确保开发板已连接到电脑并启动到U-Boot模式${NC}"
echo -e "2. ${YELLOW}在MobaXterm串口工具中输入以下命令:${NC}"
echo -e "   ${GREEN}tftp 0x30000000 scale.bin${NC}"
echo -e "   ${GREEN}go 0x30000000${NC}"
echo -e "3. ${YELLOW}等待烧录完成${NC}"

echo -e "\n${GREEN}=== 操作完成 ===${NC}"
echo -e "${GREEN}bin文件已准备就绪，可通过MobaXterm进行烧录${NC}"
