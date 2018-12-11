#include "threadpool.h"

ThreadPool *ThreadPool::getInstance()
{
    if (pThreadPool == nullptr){
        pThreadPool = new ThreadPool();
	pThreadPool->bStop = false;
    }

    return pThreadPool;
}

void ThreadPool::destoryPool() throw()
{
    if (pThreadPool != nullptr)
	delete pThreadPool;
    pThreadPool = nullptr;
}

void ThreadPool::addThread() throw()
{
    vWork.emplace_back([this]{
        std::mutex localMutex;
        std::unique_lock<std::mutex> localLock(localMutex);
        for (;;){
            if (bStop)
                break;

            this->condition.wait(localLock, [this]{
                return bStop || !this->tasks.empty();});

            this->list_mutex.lock();
            if (this->tasks.size() <= 0){
                this->list_mutex.unlock();
                continue;
            }

            auto tmpTask = this->tasks.front();
            this->tasks.erase(this->tasks.begin());
            this->list_mutex.unlock();
            tmpTask();
        }
    });
}

ThreadPool::~ThreadPool()
{
    bStop = true;

    list_mutex.lock();
    tasks.clear();
    list_mutex.unlock();

    for (auto &tmpWork : vWork){
        for (unsigned int i=0; i<vWork.size(); i++){
            condition.notify_one();
        }
        tmpWork.join();
    }

    vWork.clear();

    std::cout<<"ThreadPool destory successful"<<std::endl;
}

ThreadPool *ThreadPool::pThreadPool = nullptr;


