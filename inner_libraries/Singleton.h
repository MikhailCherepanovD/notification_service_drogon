#pragma once
#include <iostream>
#include "mutex"
#include <iostream>
using std::cout;
using std::endl;
using std::mutex;
using std::lock_guard;
template <typename T>
class Singleton{
private:
    static int inheritorCounter;
    static T* inheritorInstance;
    static mutex mtxConstructor;
    static mutex mtxGetInstance;
    class MemGuard{
    public:
        MemGuard(){
        }
        ~MemGuard(){
            delete inheritorInstance;
            inheritorInstance=nullptr;
        }
    };
protected:
    Singleton(){
        lock_guard<mutex> lock(mtxConstructor);
        inheritorCounter++;
        if(inheritorCounter > 1){
            throw std::runtime_error("Could not create more than once object of Singleton class");
        }
        cout<<"Create singleton object Singleton"<<endl;
    }
    ~Singleton(){
        cout<<"Delete singleton object Singleton"<<endl;
    }
public:
    Singleton(const Singleton&) = delete;
    Singleton& operator=(const Singleton&) = delete;
    static T& getInstance(){
        lock_guard<mutex> lock(mtxGetInstance);
        static MemGuard g;
        if (!inheritorInstance) {
            inheritorInstance = new T();
        }
        return *inheritorInstance;
    }
};
template <typename T>
T* Singleton<T>::inheritorInstance=nullptr;
template <typename T>
int Singleton<T>::inheritorCounter=0;
template <typename T>
std::mutex Singleton<T>::mtxConstructor;
template <typename T>
std::mutex Singleton<T>::mtxGetInstance;