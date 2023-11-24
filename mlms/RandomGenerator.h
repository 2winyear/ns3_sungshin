#include "ns3/application.h"

class RandomGenerator : public Application
{
public:
    RandomGenerator ();
    void SetDelay (RandomVariable delay);
    void SetSize (RandomVariable size);
    void SetRemote (std::string socketType, 
                    Address remote);
private:
    virtual void StartApplication (void);
    virtual void StopApplication (void);
    void DoGenerate (void);

    RandomVariable m_delay;
    RandomVariable m_size;
    Ptr<Socket> m_socket;
};