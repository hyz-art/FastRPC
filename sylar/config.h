#ifndef __SYLAR_CONFIG_H__
#define __SYLAR_CONFIG_H__

#include <memory>
#include <string>
#include <sstream>
#include <boost/lexical_cast.hpp>
#include <unordered_map>
#include <functional>
#include <yaml-cpp/yaml.h>

#include <vector>
#include <map>
#include <set>
#include <list>
#include <unordered_map>
#include <unordered_set>

#include "log.h"
#include "util.h"
#include "mutex.h"
namespace sylar
{
    class ConfigVarBase
    {
    public:
        typedef std::shared_ptr<ConfigVarBase> ptr;
        ConfigVarBase(const std::string &name, const std::string &decription = "")
            : m_name(name), m_description(decription) {
            std::transform(m_name.begin(),m_name.end(),m_name.begin(),::tolower);
        }
        virtual ~ConfigVarBase() {}
        const std::string &getName() const { return m_name; }
        const std::string &getDescription() const { return m_description; }
        virtual std::string toString()=0;
        virtual bool fromString(const std::string& val)=0;
        virtual std::string getTypeName() const=0;
    protected:
        std::string m_name;
        std::string m_description;
    };

    template<class F,class T>
    class LexicalCast{
        public:
        T operator() (const F &v) {
            return boost::lexical_cast<T>(v);
        }
    };
    /**
     * @brief 把字符串反序列化成容器
     */
    template<class T>
    class LexicalCast<std::string, std::vector<T>>{
        public:
        std::vector<T> operator() (const std::string& v){
            YAML::Node node=YAML::Load(v);
            typename std::vector<T> vec;
            std::stringstream ss;
            for(size_t i=0;i<node.size();++i){
                ss.str("");
                ss<<node[i];
                vec.push_back( LexicalCast<std::string,T>()(ss.str()));
            }
            return vec;
        }
    };

    template<class T>
    class LexicalCast< std::vector<T>,std::string>{
        public:
        std::string operator()(const std::vector<T>& v) {
            YAML::Node node(YAML::NodeType::Sequence);
            for (auto& i : v) {
                node.push_back(YAML::Load(LexicalCast<T, std::string>()(i)));
            }
            std::stringstream ss;
            ss << node;
            return ss.str();
        }
    };

    template <class T>
    class LexicalCast<std::string, std::list<T>> {
    public:
        std::list<T> operator()(const std::string& v) {
            YAML::Node node = YAML::Load(v);
            typename std::list<T> vec;
            std::stringstream ss;
            for (size_t i = 0; i < node.size(); ++i) {
                ss.str("");
                ss << node[i];
                vec.push_back(LexicalCast<std::string, T>()(ss.str()));
            }
            return vec;
        }
    };
    template<class T>
    class LexicalCast< std::list<T>,std::string>{
        public:
        std::string operator()(const std::list<T>& v) {
            YAML::Node node(YAML::NodeType::Sequence);
            for (auto& i : v) {
                node.push_back(YAML::Load(LexicalCast<T, std::string>()(i)));
            }
            std::stringstream ss;
            ss << node;
            return ss.str();
        }
    };

    template <class T>
    class LexicalCast<std::string, std::set<T>> {
    public:
        std::set<T> operator()(const std::string& v) {
            YAML::Node node = YAML::Load(v);
            typename std::set<T> vec;
            std::stringstream ss;
            for (size_t i = 0; i < node.size(); ++i) {
                ss.str("");
                ss << node[i];
                vec.insert(LexicalCast<std::string, T>()(ss.str()));
            }
            return vec;
        }
    };
    template<class T>
    class LexicalCast<std::set<T>, std::string> {
    public:
        std::string operator()(const std::set<T>& v) {
            YAML::Node node(YAML::NodeType::Sequence); // 创建 YAML 序列
            for (auto& i : v) {
                node.push_back(YAML::Load(LexicalCast<T, std::string>()(i))); // 每个元素转换为字符串
            }
            std::stringstream ss;
            ss << node; // 将整个节点序列化为字符串
            return ss.str();
        }
    };

    template <class T>
    class LexicalCast<std::string, std::unordered_set<T>> {
    public:
        std::unordered_set<T> operator()(const std::string& v) {
            YAML::Node node = YAML::Load(v);
            typename std::unordered_set<T> vec;
            std::stringstream ss;
            for (size_t i = 0; i < node.size(); ++i) {
                ss.str("");
                ss << node[i];
                vec.insert(LexicalCast<std::string, T>()(ss.str()));
            }
            return vec;
        }
    };
    template<class T>
    class LexicalCast< std::unordered_set<T>,std::string>{
        public:
        std::string operator()(const std::unordered_set<T>& v) {
            YAML::Node node(YAML::NodeType::Sequence);
            for (auto& i : v) {
                node.push_back(YAML::Load(LexicalCast<T, std::string>()(i)));
            }
            std::stringstream ss;
            ss << node;
            return ss.str();
        }
    };

    template <class T>
    class LexicalCast<std::string, std::map<std::string,T>> {
    public:
        std::map<std::string,T> operator()(const std::string& v) {
            YAML::Node node = YAML::Load(v);
            typename std::map<std::string,T> vec;
            std::stringstream ss;
            for (auto it = node.begin(); it != node.end(); ++it) {
                ss.str("");
                ss << it->second;
                vec.insert(std::make_pair(it->first.Scalar(), LexicalCast<std::string, T>()(ss.str())));
            }
            return vec;
        }
    };
    template<class T>
    class LexicalCast< std::map<std::string,T>,std::string>{
        public:
        std::string operator()(const std::map<std::string,T>& v) {
            YAML::Node node(YAML::NodeType::Map);
            for (auto& i : v) {
                node[i.first] =YAML::Load(LexicalCast<T, std::string>()(i.second));
            }
            std::stringstream ss;
            ss << node;
            return ss.str();
        }
    };

    template <class T>
    class LexicalCast<std::string, std::unordered_map<std::string,T>> {
    public:
        std::unordered_map<std::string,T> operator()(const std::string& v) {
            YAML::Node node = YAML::Load(v);
            typename std::unordered_map<std::string,T> vec;
            std::stringstream ss;
            for (auto it = node.begin(); it != node.end(); ++it) {
                ss.str("");
                ss << it->second;
                vec.insert(std::make_pair(it->first.Scalar(), LexicalCast<std::string, T>()(ss.str())));
            }
            return vec;
        }
    };
    template<class T>
    class LexicalCast< std::unordered_map<std::string,T>,std::string>{
        public:
        std::string operator()(const std::unordered_map<std::string,T>& v) {
            YAML::Node node(YAML::NodeType::Sequence);
            for (auto& i : v) {
                node.push_back(YAML::Load(LexicalCast<T, std::string>()(i.second)));
            }
            std::stringstream ss;
            ss << node;
            return ss.str();
        }
    };


    template<class T, class FromStr=LexicalCast<std::string,T>, class ToStr=LexicalCast<T,std::string>>
    class ConfigVar:public ConfigVarBase{
        public:
        typedef RWMutex RWMutexType;
        typedef std::shared_ptr<ConfigVar> ptr;
        typedef std::function<void(const T& old_value, const T& new_value)>  on_change_cb;
        ConfigVar(const std::string& name, const T& default_value, const std::string& description="")
            :ConfigVarBase(name,description), m_val(default_value){}
        
        std::string toString()override{
            try{
                //return boost::lexical_cast<std::string>(m_val);
                RWMutexType::ReadLock lock(m_mutex);
                return ToStr()(m_val);
            }catch(std::exception& e){
                SYLAR_LOG_ERROR(SYLAR_LOG_ROOT())
                    <<"ConfigVar::toString exception "<<e.what()<<" convert: "<<TypeToName<T>()
                    <<" to string"<<" name="<<m_name;
            }
            return "";
        }

        bool fromString(const std::string& val) override{
            try{                
                //setValue(val);
                 setValue(FromStr()(val));
            }catch (std::exception& e){
                SYLAR_LOG_ERROR(SYLAR_LOG_ROOT())
                    <<"ConfiVar::fromString exception "<<e.what()<<" convert: string to "
                    <<TypeToName<T>()<<" name= "<<m_name<<" - "<<val;
            }
            return false;
        }

        std::string getTypeName() const override { return TypeToName<T>(); }

        const T getValue() {
            RWMutexType::ReadLock lock(m_mutex);
            return m_val;
        }
        
        void setValue(const T& v) { 
            {
                RWMutexType::ReadLock lock(m_mutex);
                //值不变就返回
                if (v == m_val) {
                    return;
                }
                for (auto& i : m_cbs) {
                    //唤醒val， 回调函数：旧值 → 新值
                    i.second(m_val, v);
                }
            }
            RWMutexType::WriteLock lock(m_mutex);
            m_val = v;//设置值
        }
        uint64_t addListener(on_change_cb cb){
            static uint64_t s_fun_id=0;
            RWMutexType::WriteLock lock(m_mutex);
            ++s_fun_id;
            m_cbs[s_fun_id]=cb;
            return s_fun_id;
        }
        void delListener(uint64_t key){
            RWMutexType::WriteLock lock(m_mutex);
            m_cbs.erase(key);
        }
        on_change_cb getListener(uint64_t key){
            RWMutexType::ReadLock lock(m_mutex);
            auto it=m_cbs.find(key);
            return it==m_cbs.end()?nullptr:it->second;
        }
        void clearListener(){
            RWMutexType::WriteLock lock(m_mutex);
            m_cbs.clear();
        }
    private:
        RWMutexType m_mutex;
        T m_val;
        std::map<uint64_t,on_change_cb> m_cbs;
    };

    class Config{
        public:
        typedef RWMutex RWMutexType;
        typedef std::unordered_map<std::string, ConfigVarBase::ptr> ConfigVarMap;
        //定义的时候模板赋值
        template<class T>
        static typename ConfigVar<T>::ptr Lookup(const std::string& name, const T& default_value,
            const std::string& description=""){
            RWMutexType::WriteLock lock(GetMutex());
            auto it=GetDatas().find(name);
            if(it!=GetDatas().end()){
                auto tmp=std::dynamic_pointer_cast<ConfigVar<T>>(it->second);
                if(tmp){//ConfigVarBase::ptr
                    SYLAR_LOG_INFO(SYLAR_LOG_ROOT())<<" Lookup name= "<<name<<" exists";
                    return tmp;
                }else{
                    SYLAR_LOG_INFO(SYLAR_LOG_ROOT())<<name<<" exists but  type not "<<TypeToName<T>()
                        <<" real_type= "<< it->second->getTypeName() << " " <<it->second->toString();
                    return nullptr; 
                }
            }
            if(name.find_first_not_of("abcdefghijklmnopqrstuvwxyz._0123456789")!=std::string::npos){
                SYLAR_LOG_ERROR(SYLAR_LOG_ROOT()) << "Lookup name invalid " << name;
                throw std::invalid_argument(name);
            }
            typename ConfigVar<T>::ptr v(new ConfigVar<T>(name,default_value,description));
            GetDatas()[name]=v;
            return v;
        }

        template<class T>
        static typename ConfigVar<T>::ptr Lookup(const std::string& name){
            RWMutexType::ReadLock lock(GetMutex());
            auto it=GetDatas().find(name);
            if(it==GetDatas().end())
                return nullptr;
            return std::dynamic_pointer_cast<ConfigVar<T>>(it->second);
        }
        //node初始化配置文件
        static void LoadFromYaml(const YAML::Node& root);
        //加载path的配置文件
        static void LoadFromConfDir(const std::string& path, bool force=false);
        //配置基类
        static ConfigVarBase::ptr LookupBase(const std::string& name);
        //遍历配置项
        static void Visit(std::function<void(ConfigVarBase::ptr)> cb);
        private:
        //static ConfigVarMap m_datas;
        static ConfigVarMap& GetDatas(){
            static ConfigVarMap s_datas;
            return s_datas;
        }
        static RWMutexType& GetMutex(){
            static RWMutexType s_mutex;
            return s_mutex;
        }
    };
    
}
#endif
