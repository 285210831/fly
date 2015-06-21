/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *                    _______    _                                     *
 *                   (  ____ \  ( \     |\     /|                      * 
 *                   | (    \/  | (     ( \   / )                      *
 *                   | (__      | |      \ (_) /                       *
 *                   |  __)     | |       \   /                        *
 *                   | (        | |        ) (                         *
 *                   | )        | (____/\  | |                         *
 *                   |/         (_______/  \_/                         *
 *                                                                     *
 *                                                                     *
 *     fly is an awesome c++ network library.                          *
 *                                                                     *
 *   @author: lichuan                                                  *
 *   @qq: 308831759                                                    *
 *   @email: 308831759@qq.com                                          *
 *   @github: https://github.com/lichuan/fly                           *
 *   @date: 2015-06-21 18:45:20                                        *
 *                                                                     *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#ifndef FLY__TASK__TASK
#define FLY__TASK__TASK

#include "fly/base/common.hpp"

namespace fly {
namespace task {

class Task
{
public:
    Task(uint64 seq);
    virtual ~Task() = default;
    virtual void run() = 0;
    uint64 seq();
    void set_executor_id(uint32 id);
    
protected:
    uint32 m_executor_id;
    
private:
    uint64 m_seq;
};

class Loop_Task : public Task
{
public:
    Loop_Task(uint64 seq);
    virtual void run() override;
    virtual void run_in_loop() = 0;
    void stop();

private:
    bool m_running = true;
};

}
}

#endif
