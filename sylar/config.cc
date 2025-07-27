#include "sylar/config.h"
#include "sylar/log.h"
namespace sylar{

ConfigVarBase::ptr Config::LookupBase(const std::string& name){
    auto it=GetDatas().find(name);
    return it==GetDatas().end()? nullptr:it->second;
}

//Config::ConfigVarMap Config::s_datas;
static void ListAllMember(const std::string& prefix, const YAML::Node &node,
    std::list<std::pair<std::string, const YAML::Node>>& output){
    if(prefix.find_first_not_of("abcdefghijklmnopqrstuvwxyz._0123456789")!=std::string::npos){
        //不是最大无符号值，表示有非法值，返回索引
        SYLAR_LOG_ERROR(SYLAR_LOG_ROOT())<<"Config invalid name: " << prefix << " : " << node;
        return;
    }
    output.push_back(std::make_pair(prefix,node));
    if(node.IsMap()){
        for(auto it=node.begin();it!=node.end();++it){
            ListAllMember(prefix.empty()?it->first.Scalar():prefix+"."+it->first.Scalar(),it->second,output);
        }
    }
}

void Config::LoadFromYaml(const YAML::Node& root) {
     // 将root中的所有节点信息保存到all_nodes中
    std::list<std::pair<std::string, const YAML::Node>> all_nodes;
    ListAllMember("", root, all_nodes);
    // 遍历list
    for (auto& i : all_nodes) {
        std::string key = i.first;
        if (key.empty()) {
            continue;
        }
        std::transform(key.begin(), key.end(), key.begin(), ::tolower);
        // 查找名为key的配置参数
        ConfigVarBase::ptr var = LookupBase(key);
        if (var) {
             // 若为纯量，则调用fromString;为数组，将其转换为字符串
            if (i.second.IsScalar()) {
                var->fromString(i.second.Scalar());
            } else {
                std::stringstream ss;
                ss << i.second;
                var->fromString(ss.str());
            }
        }
    }
}


void Config::Visit(std::function<void(ConfigVarBase::ptr)> cb){
    RWMutexType::ReadLock lock(GetMutex());
    ConfigVarMap& m_datas=GetDatas();
    for(auto it:m_datas){
        cb(it.second);
    }
}
}