//c写一个http服务器
#include <unistd.h>
#include <fcntl.h>
#include <arpa/inet.h>
#include <sys/socket.h>

//c头文件
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

//监听套接字创建
//搞清除主要代码和次要代码
//次要次要代码是为主要代码服务的
//主要代码有socket(),bind(),listen()
int create_listenfd(void)
{
	//创建tcp连接
	int fd = socket(AF_INET, SOCK_STREAM, 0);
	
//用于解决bind: Address already in use问题
int n = 1;
setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &n, 4);
	
	//绑定地址
	struct sockaddr_in sin;
	bzero(&sin, sizeof(sin));//清空
	sin.sin_family = AF_INET;
	sin.sin_port = htons(80);//80是http默认端口号，计算机网络，网络上是大端序，用htons
	sin.sin_addr.s_addr = INADDR_ANY;//监听任意网卡
	
	int res = bind(fd, (struct sockaddr *)&sin, sizeof(sin));
	
	if(res == -1)
	{
		perror("bind"); //将错误信息打印出来
	}
	
	listen(fd, 100);//监听
	return fd;
}

//处理客户端发来的请求
void handle_request(int fd)
{
	//客户端首先会发一个字符串（请求）过来，读出来
	char buff[1024*1024] = {0};
	int nread = read(fd, buff, sizeof(buff)); 
	printf("读到的请求是%s\n", buff);
	
	//从请求里解析出文件名
	char filename[10] = {0};
	sscanf(buff, "GET /%s", filename);//字符串解析函数,不要的东西Get 放在前面
	printf("解析的文件名是:%s\n", filename);
	
	//根据文件名获得mime类型，放入响应头中，告诉浏览器发给你的是什么类型的文件
	//如果是文本文件，浏览器直接打开
	//如果jpeg等其他文件不可以直接打开
	//这里假设有文本和图片两种类型
	char* mime = NULL;
	if(strstr(filename, ".html"))
		mime = "text/html";//文本类型
	else if((filename, ",jpg"))
		mime = "image/html";//图片类型，这些类型都是规定的，可以在请求头看到
	
	//打开文件，读取内容，构建响应，发回给客户端
	char response[1024*1024] = {0};
	sprintf(response, "http/1.1 200 OK\r\nContent-Type: %s\r\n\r\n", mime);//往response填入整个字符串的内容,这里用\n\r会打不开图片
	int headlen = strlen(response);
	
	//上面是文件头，有了文件头，可以打开文件内容
	int filefd = open(filename, O_RDONLY);
	int filelen = read(filefd, response+headlen, sizeof(response)-headlen);
	
	//发送响应 头+内容
	write(fd, response, headlen+filelen);//fd是读的描述符，跟传入的形参fd不是同一个
	
	close(filefd);
	
	sleep(1);

int main(int argc, char *argv[])
{
	//1 创建监听套接字，返回套接字文件描述符
	int sockfd = create_listenfd();
		
	while(1)
	{	
		//2 接受客户端连接
		int fd = accept(sockfd, NULL, NULL);//这里会阻塞，什么是阻塞，有结果才会往下走
		printf("有客户端连接\n");
	
		//3 处理客户端发来的请求
		handle_request(fd);//需要返回fd参数，返回值void
		close(fd);
	}
	
	close(sockfd);//关闭文件描述符，养成良好代码风格
}