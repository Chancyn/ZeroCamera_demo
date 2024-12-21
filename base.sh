#!/bin/bash
 
# 定义颜色代码
RED='\033[0;31m'
NC='\033[0m' # No Color，颜色重置
GREEN='\033[0;32m'
BLUE='\033[0;34m'
YELLOW='\033[1;33m' # 带有亮度的黄色，更醒目
 
# 定义日志函数
echo_err() {
    echo -e "${RED}DEBUG: ${1}${NC}"
}

echo_warn() {
    echo -e "${YELLOW}WARNING: ${1}${NC}"
}

echo_debug() {
    echo -e "${BLUE}DEBUG: ${1}${NC}"
}

echo_info() {
    echo -e "${GREEN}INFO: ${1}${NC}"
}