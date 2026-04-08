#!/bin/bash

# 自动烧录bin文件到s5pv210开发板的脚本

# 颜色定义
GREEN="\033[0;32m"
RED="\033[0;31m"
YELLOW="\033[1;33m"
NC="\033[0m" # No Color

echo -e "${GREEN}=== S5PV210 自动构建和烧录工具 ===${NC}"
echo ""

# 构建项目
echo -e "${GREEN}[1/3] 正在构建项目...${NC}"
make clean
if make; then
    echo -e "${GREEN}构建成功!${NC}"  
else
    echo -e "${RED}构建失败，请检查错误信息${NC}"
    exit 1
fi
echo ""

# 检查bin文件是否生成
echo -e "${GREEN}[2/3] 检查编译结果...${NC}"
BIN_FILE="output/scale.bin"
if [ ! -f "$BIN_FILE" ]; then
    echo -e "${RED}错误: bin文件未生成，请检查构建过程${NC}"
    exit 1
fi

BIN_FILE_SIZE=$(ls -lh "$BIN_FILE" | awk '{print $5}')
echo -e "${GREEN}bin文件生成成功: $BIN_FILE (大小: $BIN_FILE_SIZE)${NC}"
echo ""

# 显示详细的烧录步骤
echo -e "${GREEN}[3/3] 烧录步骤说明 ===${NC}"
echo -e "${YELLOW}请按照以下步骤在Windows电脑上操作：${NC}"
echo ""
echo -e "1. ${GREEN}打开MobaXterm串口工具${NC}"
echo -e "2. ${GREEN}点击Servers选项打开TFTP服务${NC}"
echo -e "   - 设置TFTP根目录为项目的output文件夹"
echo -e "   - output文件夹路径: $(pwd)/output"
echo -e "3. ${GREEN}在MobaXterm中打开串口命令行${NC}"
echo -e "4. ${GREEN}点击开发板电源键开机${NC}"
echo -e "5. ${GREEN}在串口命令行中依次输入以下命令：${NC}"
echo -e "   ${YELLOW}tftp 0x30000000 scale.bin${NC}"
echo -e "   ${YELLOW}go 0x30000000${NC}"
echo ""
echo -e "${GREEN}=== 准备就绪 ===${NC}"
echo -e "${GREEN}bin文件已在 $(pwd)/output 目录下，等待您进行烧录操作${NC}"
