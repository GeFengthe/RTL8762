@echo off
echo before_build_common bat

set keil_root=%1

set keil_spic_algo_dst=%keil_root%ARM\Flash\RTL876x_SPI_FLASH.FLM
set keil_spic_algo_src=..\..\..\tool\flash\RTL876x_SPI_FLASH.FLM

set keil_log_algo_dst=%keil_root%ARM\Flash\RTL876x_LOG_TRACE.FLM
set keil_log_algo_src=..\..\..\tool\flash\RTL876x_LOG_TRACE.FLM

if not exist %keil_spic_algo_dst% copy %keil_spic_algo_src% %keil_spic_algo_dst%
if not exist %keil_log_algo_dst% copy %keil_log_algo_src% %keil_log_algo_dst%

::bash.exe ..\..\..\tool\version_generation\GitGenerateVersion.sh bee2-sdk- ..\..\..\tool\version_generation\ ..\..\..\inc\platform\version.h