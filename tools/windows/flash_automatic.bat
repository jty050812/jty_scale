@echo off

rem 自动控制MobaXterm进行S5PV210开发板烧录的批处理文件

echo === S5PV210自动烧录工具 ===
echo 

rem 检查Python是否安装
python --version >nul 2>&1
if %errorlevel% neq 0 (
    echo 错误: Python未安装
    echo 请先安装Python 3.6或更高版本
    pause
    exit /b 1
)

rem 检查pyautogui是否安装
python -c "import pyautogui" >nul 2>&1
if %errorlevel% neq 0 (
    echo 正在安装pyautogui和pyperclip...
    pip install pyautogui pyperclip
    if %errorlevel% neq 0 (
        echo 错误: 安装pyautogui失败
        pause
        exit /b 1
    )
)

rem 执行Python脚本
echo 正在启动自动烧录工具...
echo 
python "%~dp0flash_automatic.py"

pause
