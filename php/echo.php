<?php
$buff=$_REQUEST["content"];

$serverIp="192.168.1.117";//服务端ip地址，如果你的客户端与服务端不在同一台电脑，请修改该ip地址
$serverPort=9981;//通信端口号
//设置超时时间
set_time_limit(0);
//创建套接字
 $sock= socket_create(AF_INET, SOCK_STREAM, SOL_TCP);
if(!$sock)
{
    echo "creat sock failed";
    exit();//创建套接字失败，结束程序
}
//发送数据到udp的服务端（C语言写的）
$connection = socket_connect($sock, $serverIp, $serverPort);
socket_write($sock, $buff);
$buff="";//清空缓冲区
socket_recv($sock, $buff, 65536, 0);
echo trim($buff)."\n";//去掉接受到的字符串的首尾空格，返回给post请求的data
//关闭套接字
socket_close($sock);
?>
