#ifndef _I_DISPATCH_H__
#define _I_DISPATCH_H__

class NLMessage;
class IDispatch
{
public:
    virtual int post_message(NLMessage* msg) = 0;
};
#endif
