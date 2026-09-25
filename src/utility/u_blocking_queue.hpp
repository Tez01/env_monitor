#include <queue>
#include <condition_variable>

template <typename T>
class BlockingQueue{
    public:
        void add(const T &item){
            std::lock_guard<std::mutex> lock{mutex_};
            queue_.push(item);

            cv_.notify_one();

        }

        T get(){
            std::unique_lock<std::mutex> lock{mutex_};

            while(queue_.empty()){
                cv_.wait(lock);
            }

            T item = queue_.front();
            queue_.pop();

            return item;
        }


    private:
        std::queue<T> queue_;    // MUST: Bound max length of queue
        std::mutex mutex_;
        std::condition_variable cv_;
};