#include <iostream>
#include <string>
#include "user.pb.h"
#include "mrpcapplication.h"
#include "rpcprovider.h"
/*
UserService原来是一个本地服务，提供了两个进程内的本地方法，Login和GetFriendLists
*/
class UserService : public fixbug::UserServiceRpc   //使用在rpc服务发布端(rpc服务提供者)
{
public:
    bool Login(std::string name,std::string pwd)
    {
        std::cout << "doing local service: Login"  << std::endl;
        std::cout << "name:" << name << " pwd:" << pwd << std::endl;
        return true;
    }

    bool Register(uint32_t id, std::string name,std::string pwd)
    {
        std::cout << "doing local service: Register" << std::endl;
        std::cout << "id:" << id << "name:" << name << " pwd:" << pwd << std::endl;
        return true;
    }

    /*
    重写基类UserServiceRpc的虚函数，下面这些方法都是框架直接调用的
    1.caller ===> Login(LoginRequest)  ===> muduo ===> callee
    2.callee ===> Login(LoginRequesr)  ===> 交到下面重写的Login方法上
    */
    void Login(::google::protobuf::RpcController* controller,
                       const ::fixbug::LoginRequest* request,
                       ::fixbug::LoginResponse* response,
                       ::google::protobuf::Closure* done)
    {
        //框架给业务上报了请求参数LoginRequest，应用获取相应数据做本地业务
        std::string name = request->name();
        std::string pwd = request->pwd();

        bool login_result = Login(name,pwd);   //做本地业务


        //把响应写入
        fixbug::ResultCode *code = response->mutable_result();
        code->set_errcode(0);
        code->set_errmsg("");
        response->set_sucess(login_result);

        //执行回调操作
        done->Run();
    }

    void Register(::google::protobuf::RpcController* controller,
                       const ::fixbug::RegisterRequest* request,
                       ::fixbug::RegisterResponse* response,
                       ::google::protobuf::Closure* done)
    {
        uint32_t id = request->id();
        std::string name = request->name();
        std::string pwd = request->pwd();

        bool register_result = Register(id,name,pwd);


        fixbug::ResultCode *code = response->mutable_result();
        code->set_errcode(0);
        code->set_errmsg("");
        response->set_success(register_result);

        done->Run();
    }
};


int main(int argc ,char **argv)
{
    //调用框架的初始化操作 provider -i config.conf
    MrpcApplication::Init(argc,argv);

    //把UserService对象发布到rpc节点上
    RpcProvider provider;
    provider.NotifyService(new UserService());

    //启动一个rpc服务发布节点  run以后 ， 进程进入阻塞状态，等待远程的rpc调用请求
    provider.Run();

    return 0;
}