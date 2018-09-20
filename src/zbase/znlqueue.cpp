#include "znlqueue.h"

NLQueue::NLQueue()
{    
    m_iQueueHeader = 0;
    m_iQueueWriteTail = 0;
    m_iQueueSize = 0;

    for (int i = 0; i < NL_QUEUE_SIZE; ++i)
    {
        m_pQueueMessages[i] = NULL;
    }
}

NLQueue::~NLQueue()
{
    
}

bool NLQueue::put_message(NLMessage* msg)
{
    // 这个函数可能有多个线程同时调用
    if (NULL == msg)
    {
        // 需要放入队列的消息为空
        return false;
    }
    
    // 针对多线程同时访问的处理，按幼儿园原则：多个孩子抢一个玩具，老师没收玩具
    // 先预判一下， 两个原子操作组合起来就不一定是原子操作了
    if (m_iQueueSize < NL_QUEUE_FULL_SIZE)
    {
        // 锁定一个名额
        m_iQueueSize++;
    }
    
    // 防止多个线程 同时进行了锁定, 再次进行判断 
    if (m_iQueueSize > NL_QUEUE_FULL_SIZE)
    {
        // 大家全部都放回去
        m_iQueueSize--;
        return false;
    }

    // 获取槽位
    int slot_id = m_iQueueWriteTail++;

    // 将消息放入槽位
    if (slot_id > (NL_QUEUE_SIZE - 1))
    {
        slot_id -= NL_QUEUE_SIZE;
    }
    m_pQueueMessages[slot_id] = msg;

    return true;
    
}

bool NLQueue::pop_message(NLMessage*& msg)
{
    // 针对m_iQueueWriteTail的操作放到这里， 因为这个函数只有一个线程使用
    if (m_iQueueWriteTail > NL_QUEUE_LAST_SLOT)
	{
		m_iQueueWriteTail -= NL_QUEUE_SIZE;
	}

    // 是否为空
    if (m_iQueueSize == 0)
    {
        // 没有消息返回false
        return false;
    }    

    // 只有读取线程使用 header
    msg = m_pQueueMessages[m_iQueueHeader];
    if (NULL == msg)
    {
        // 可能是其他线程该消息还没有放入
        // 此时直接返回NULL, 外部读取线程遇到该情况
        // 需要等待下次循环在取出
        // 此时不需要调整队列的大小 
        return true;
    }
    
    m_pQueueMessages[m_iQueueHeader] = NULL;
    m_iQueueHeader++;
    if (m_iQueueHeader > (NL_QUEUE_SIZE - 1))
    {
        m_iQueueHeader -= NL_QUEUE_SIZE;
    }
    
    m_iQueueSize--;

    return true;    
}



