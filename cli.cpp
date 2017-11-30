
#include"client.h"
#include<stdlib.h>
#include<unistd.h>
using namespace std;

pthread_t tid;//创建新的线程的id
extern int clientfd;

//接收服务器发来的消息
void* threadproc(void *lparg)
{
    //该线程启动循环接收模式，不断接收服务器发送过来的消息
    while(1)
	{
		//获取服务器发送过来的消息
		char buff[1024]  ={0};
		recv(clientfd,buff,1023,0);

printf("recv:%s\n",buff);

		Json::Reader reader;
		Json::Value root;
    

		//利用json解析服务器发来的消息
		if(0 == reader.parse(buff, root))
		{
			cerr<<"json format invalid:"<<buff<<endl;
			return NULL;
		}
		
		//root[1]["name"];
		//cout<<root[1]["reason"]<<endl;(dumped)
printf("reader ok\n");
		string name = root["name"].asString();
		string reason = root["reason"].asString();
		cout<<name<<" : "<<reason<<endl;
	}
}

//登录
void doLogin()
{
    char name[20]={0};//登录名
    char pwd[20]={0};//密码
    
    cout<<"please login!"<<endl;
    cout<<"name:";
    fgets(name,19,stdin);//获取登录名
    name[strlen(name)-1] = 0;

    cout<<"pwd:";
    fgets(pwd,19,stdin);//获取密码
    pwd[strlen(pwd)-1] = 0;


//将要发送到服务器的内容放到json里面
    Json::Value json;
    json["msgtype"] = MSG_TYPE_LOGIN;//消协类型
    json["name"] = name;
    json["pwd"] = pwd;
    
//jison.toStyledString().c_str()返回的是将需要发送的内容整合在一起的字符串的指针
    int ret = send(clientfd, json.toStyledString().c_str(),
        strlen(json.toStyledString().c_str()), 0);
        
    //recv等待服务器返回的结果
	char buff[128]  ={0};
	recv(clientfd,buff,127,0);

	Json::Reader reader;
	Json::Value root;

	//将服务器返回的信息用json进行解析
	if(0 == reader.parse(buff, root))
	{
		cerr<<"json format invalid:"<<buff<<endl;
		return;
	}

	//判断登录是否完成
	const char *reason = root["reason"].asString().c_str();
	if(strncmp(reason,"ok",2) == 0)
	{
		cout<<"login success!"<<endl;    
    //如果登录成功，马上启动接收线程
    //新线程专门接受服务器发来的消息，
   		 pthread_create(&tid, NULL, threadproc, NULL);

    //主线程专门给服务器发送消息
	         doTalk();
	}
	else
	{
		//登录失败就返回
		cout<<reason<<endl;
		return;
	}
 
}

//聊天（给服务器发送消息）
void doTalk(void)
{
	while(1)
	{
		//让用户选择需要进行的选择
		int choice;
		cout<<"please choice :"<<endl;
		cout<<"1.talk one"<<"2.talk all"<<"3.show online"<<endl;
		cin>>choice;
		getchar();//\n
cout<<choice<<endl;
		//根据用户的选择，选择不同的服务
		switch(choice)
		{
			//单对单聊天
			case 1:
			{
				Json::Value json;
				json["msgtype"] = MSG_TYPE_TALK; 

				char name[20] = {0};
				cout<<"name: ";
				fgets(name,19,stdin);
				name[strlen(name)-1] = 0;

				char reason[1024] = {0};
				cout<<"reason: ";
				fgets(reason,1023,stdin);
				reason[strlen(reason)-1] = 0;

				json["name"] = name;
				json["reason"] = reason;
				
				send(clientfd,json.toStyledString().c_str(),
					strlen(json.toStyledString().c_str()),0);
			};
			break;
			//发送消息给全体人员
			case 2:
			{
				Json::Value json;
				json["msgtype"] = MSG_TYPE_GROUP; 

				char reason[1024] = {0};
				cout<<"reason: ";
				fgets(reason,1023,stdin);
				reason[strlen(reason)-1] = 0;

				json["reason"] = reason;
				send(clientfd,json.toStyledString().c_str(),
					strlen(json.toStyledString().c_str()),0);
			};
			break;
			//获取当前在线的用户
			case 3:
			{
				Json::Value json;
				json["msgtype"] = MSG_TYPE_QUERY; 
				send(clientfd,json.toStyledString().c_str(),
					strlen(json.toStyledString().c_str()),0);
			};
			break;
			default:
			cout<<"!!!!!!!error!!!!!!!"<<endl;
				break;
		}
	}
}

//注册
void doRegister(void)
{
	//获取注册信息
	char name[20] = {0};
	char pwd[20] = {0};
	cout<<"please cin name && pwssword:"<<endl;
	cout<<"name: ";
	fgets(name,19,stdin);
	name[strlen(name)-1] = 0;

	cout<<"password: ";
	fgets(pwd,19,stdin);
	pwd[strlen(pwd)-1]  = 0;

	//将注册信息放入json
	Json::Value json;
	json["msgtype"] = MSG_TYPE_REGISTER;
	json["name"] = name;
	json["pwd"] = pwd;

	//将json发送到服务器
	send(clientfd,json.toStyledString().c_str(),
		strlen(json.toStyledString().c_str()),0);
	
	char buff[128] = {0};
	recv(clientfd,buff,127,0);
	//获取服务器端回应
	Json::Reader reader;
	Json::Value root;

	//用json解析服务器发来的消息
	if(0 == reader.parse(buff, root))
	{
		cerr<<"json format invalid:"<<json<<endl;
		return;
	}

	//判断注册是否完成
	const char *reason = root["reason"].asString().c_str();
	if(strcmp(reason,"ok") == 0)
	{
		cout<<"register success! please login"<<endl; 
		//注册完成就进行登录
		doLogin();
	}
	else
	{
		cout<<reason<<endl;
		return;
	}
}

//退出
void doExit(int sig)
{
	//发送下线消息
	Json::Value json;
	json["msgtype"] = MSG_TYPE_OFFLINE;

	send(clientfd,json.toStyledString().c_str(),strlen(json.toStyledString().c_str()),0);

	sleep(0.1);
	//关闭文件描述符
	close(clientfd);
	
	
	exit(0);
	return;
}



