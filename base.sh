#!/bin/bash

SCRIPT_NAME="$(basename "$0")"
# 定义颜色代码
RED='\033[0;31m'
NC='\033[0m' # No Color，颜色重置
GREEN='\033[0;32m'
BLUE='\033[0;34m'
YELLOW='\033[1;33m' # 带有亮度的黄色，更醒目
GRAY='\033[1;30m'

# 定义日志函数
echo_err() {
    local lineno=${BASH_LINENO[0]}
    echo -e "${RED} $(date +"%Y-%m-%d %H:%M:%S") [$SCRIPT_NAME:${lineno}] ERROR: ${1}${NC}"
}

echo_warn() {
    local lineno=${BASH_LINENO[0]}
    echo -e "${YELLOW} $(date +"%Y-%m-%d %H:%M:%S") [$SCRIPT_NAME:${lineno}] WARN: ${1}${NC}"
}

echo_info() {
    local lineno=${BASH_LINENO[0]}
    echo -e "${GREEN} $(date +"%Y-%m-%d %H:%M:%S") [$SCRIPT_NAME:${lineno}] INFO: ${1}${NC}"
}

echo_debug() {
    if [ "${shell_debug}" = "1" ]; then
    local lineno=${BASH_LINENO[0]}
    echo -e "${BLUE} $(date +"%Y-%m-%d %H:%M:%S") [$SCRIPT_NAME:${lineno}] DEBUG: ${1}${NC}"
    fi
}

echo_trace() {
    if [ "${shell_debug}" = "1" ]; then
    local lineno=${BASH_LINENO[0]}
    echo -e "${GRAY} $(date +"%Y-%m-%d %H:%M:%S") [$SCRIPT_NAME:${lineno}] INFO: ${1}${NC}"
    fi
}