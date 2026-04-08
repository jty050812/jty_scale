#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
自动控制MobaXterm进行S5PV210开发板烧录的脚本
"""

import os
import time
import subprocess
import sys
from pathlib import Path

# 尝试导入pyautogui，如果未安装则提示
pyautogui_available = False
try:
    import pyautogui
    import pyperclip
    pyautogui_available = True
except ImportError:
    print("错误: pyautogui库未安装")
    print("请运行: pip install pyautogui pyperclip")
    sys.exit(1)

def wait_for_window(window_title, timeout=30):
    """等待窗口出现"""
    start_time = time.time()
    while time.time() - start_time < timeout:
        windows = pyautogui.getAllWindows()
        for window in windows:
            if window_title in window.title:
                return window
        time.sleep(1)
    return None

def click_image(image_path, confidence=0.8, timeout=10):
    """点击指定图片"""
    start_time = time.time()
    while time.time() - start_time < timeout:
        try:
            location = pyautogui.locateCenterOnScreen(image_path, confidence=confidence)
            if location:
                pyautogui.click(location)
                return True
        except Exception as e:
            pass
        time.sleep(1)
    return False

def type_text(text, interval=0.05):
    """输入文本"""
    pyautogui.typewrite(text, interval=interval)

def press_enter():
    """按回车键"""
    pyautogui.press('enter')

def main():
    """主函数"""
    print("=== S5PV210自动烧录工具 ===")
    print("正在启动MobaXterm...")
    
    # 启动MobaXterm
    mobaxterm_path = r"C:\Program Files (x86)\Mobatek\MobaXterm\MobaXterm.exe"
    if not os.path.exists(mobaxterm_path):
        # 尝试其他可能的路径
        alternative_paths = [
            r"C:\Program Files\Mobatek\MobaXterm\MobaXterm.exe",
            r"D:\Program Files (x86)\Mobatek\MobaXterm\MobaXterm.exe",
            r"D:\Program Files\Mobatek\MobaXterm\MobaXterm.exe"
        ]
        found = False
        for path in alternative_paths:
            if os.path.exists(path):
                mobaxterm_path = path
                found = True
                break
        if not found:
            print("错误: 找不到MobaXterm可执行文件")
            print("请检查MobaXterm安装路径")
            return
    
    # 启动MobaXterm
    subprocess.Popen([mobaxterm_path])
    time.sleep(3)
    
    # 等待MobaXterm窗口出现
    print("等待MobaXterm窗口...")
    mobaxterm_window = wait_for_window("MobaXterm")
    if not mobaxterm_window:
        print("错误: MobaXterm窗口未出现")
        return
    
    # 激活窗口
    mobaxterm_window.activate()
    time.sleep(1)
    
    # 打开TFTP服务
    print("打开TFTP服务...")
    
    # 点击Servers菜单
    # 注意：这里需要根据实际的MobaXterm界面布局调整坐标
    # 以下坐标是基于默认布局的估计值，可能需要调整
    pyautogui.moveTo(50, 20)
    pyautogui.click()
    time.sleep(1)
    
    # 点击TFTP设置
    pyautogui.moveTo(100, 100)
    pyautogui.click()
    time.sleep(2)
    
    # 设置TFTP根目录为output文件夹
    print("设置TFTP根目录...")
    # 这里需要根据实际的MobaXterm TFTP设置窗口调整
    # 假设TFTP设置窗口中有一个路径输入框
    
    # 打开串口会话
    print("打开串口会话...")
    
    # 点击Session菜单
    pyautogui.moveTo(80, 20)
    pyautogui.click()
    time.sleep(1)
    
    # 点击New Session
    pyautogui.moveTo(120, 60)
    pyautogui.click()
    time.sleep(2)
    
    # 选择Serial
    pyautogui.moveTo(50, 50)
    pyautogui.click()
    time.sleep(1)
    
    # 设置串口参数（假设使用默认参数）
    # 点击OK
    pyautogui.moveTo(400, 300)
    pyautogui.click()
    time.sleep(3)
    
    # 等待用户点击开发板电源键
    print("请点击开发板电源键开机...")
    input("开发板开机后按回车键继续...")
    
    # 发送烧录命令
    print("发送烧录命令...")
    
    # 输入tftp命令
    type_text("tftp 0x30000000 scale.bin")
    press_enter()
    time.sleep(5)  # 等待传输完成
    
    # 输入go命令
    type_text("go 0x30000000")
    press_enter()
    time.sleep(2)
    
    print("=== 烧录完成 ===")
    print("开发板正在运行程序...")

if __name__ == "__main__":
    main()
