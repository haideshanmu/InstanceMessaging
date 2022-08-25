#ifndef user_service_h
#define user_service_h

#include <gflags/gflags.h>
#include <butil/logging.h>
#include <brpc/server.h>
#include "message.pb.h"
#include "../server/MySqlHandler.h"

class UserServiceIml : public chatserver::UserService
{
public:
    UserServiceIml(){};
    ~UserServiceIml(){};

    virtual void UserRigster(google::protobuf::RpcController *cntl_base,
                             const chatserver::UserRegisterMessage *request,
                             chatserver::UserRegisterReplyMessage *response,
                             google::protobuf::Closure *done)
    {
        brpc::ClosureGuard done_guard(done);

        brpc::Controller *cntl =
            static_cast<brpc::Controller *>(cntl_base);

        LOG(INFO) << "Received request from " << cntl->remote_side() << " : Register Service : " << request->user();

        MysqlHanlder handler;

        std::string select_user = "select user from user";

        if (!handler.get()->isValid() || !handler.get()->MysqlQueryWrap(select_user))
        {
            LOG(INFO) << "Erro From Internal Mysql";
            response->set_is_valid(false);
            response->set_errorstr("Erro From Internal Mysql");
            return;
        }

        int rows = handler.get()->MysqlNumRowsWrap();

        response->set_is_valid(true);

        for (int i = 0; i < rows; ++i)
        {
            if (request->user() == (*handler.get())[i][0])
            {
                response->set_is_valid(false);
                response->set_errorstr("User has existed , chose another userid");
                break;
            }
        }
        handler.get()->MysqlFreeWrap();
        if (response->is_valid())
        {
            char buf[128];

            sprintf(buf, "insert into user(user,passwd) values('%s','%s')", request->user().c_str(), request->passwd().c_str());

            int ret = handler.get()->MysqlInsertWrap(buf);

            if (ret != 0)
            {
                response->set_is_valid(false);
                response->set_errorstr("Could Not Register user for you");
            }
        }
    }

private:
    std::unordered_set<std::string> users;
};

#endif