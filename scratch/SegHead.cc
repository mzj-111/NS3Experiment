#include "ns3/assert.h"
#include "ns3/log.h"
#include "ns3/header.h"
#include "ns3/simulator.h"
#include "seq-ts-header.h"
#include "ns3/header.h"
#include "ns3/nstime.h"

using namespace ns3;

// 自定义包头
class SegHeader : public Header
{
public:
    SegHeader()
        : m_seq(0),
          m_ts(Simulator::Now().GetTimeStep())
    {
        NS_LOG_FUNCTION(this);
    }

    void SetSeq(uint32_t seq)
    {
        NS_LOG_FUNCTION(this << seq);
        m_seq = seq;
    }

    uint32_t GetSeq(void) const
    {
        NS_LOG_FUNCTION(this);
        return m_seq;
    }

    Time GetTs(void) const
    {
        NS_LOG_FUNCTION(this);
        return TimeStep(m_ts);
    }

    static TypeId GetTypeId(void)
    {
        static TypeId tid = TypeId("ns3::SegHeader")
                                .SetParent<Header>()
                                .SetGroupName("Applications")
                                .AddConstructor<SegHeader>();
        return tid;
    }

    TypeId GetInstanceTypeId(void) const
    {
        return GetTypeId();
    }

    void Print(std::ostream &os) const
    {
        NS_LOG_FUNCTION(this << &os);
        os << "(seq=" << m_seq << " time=" << TimeStep(m_ts).As(Time::S) << ")";
    }

    uint32_t GetSerializedSize() const
    {
        return 4 + 8 + 255;
    }

    void Serialize(Buffer::Iterator start) const
    {
        NS_LOG_FUNCTION(this << &start);
        Buffer::Iterator i = start;
        i.WriteU8(u_type);

        switch (u_type)
        {
        case 1:
            SerializeHEL(i);
            break;
        case 2:
            SerializeACK(i);
            break;
        case 3:
            SerializeMES(i);
            break;
        case 4:
            SerializeERR(i);
            break;

        default:
            break;
        }
    }

    void SerializeHEL(Buffer::Iterator i) const
    {
        i.WriteHtolsbU32(2147483648); // SendBufferSize
        i.WriteHtolsbU32(2147483648); // ReceiveBufferSize
    }

    void SerializeACK(Buffer::Iterator i) const
    {
        i.WriteHtolsbU32(2147483648); // SendBufferSize
        i.WriteHtolsbU32(2147483648); // ReceiveBufferSize
    }

    void SerializeMES(Buffer::Iterator i) const
    {
        i.WriteHtolsbU32(2147483648); // SendBufferSize
        i.WriteHtolsbU32(2147483648); // ReceiveBufferSize
    }

    void SerializeERR(Buffer::Iterator i) const
    {
        i.WriteHtolsbU32(2147483648); // SendBufferSize
        i.WriteHtolsbU32(2147483648); // ReceiveBufferSize
    }

    uint32_t Deserialize(Buffer::Iterator start)
    {
        NS_LOG_FUNCTION(this << &start);
        Buffer::Iterator i = start;
        m_seq = i.ReadNtohU32();
        m_ts = i.ReadNtohU64();
        return GetSerializedSize();
    }

    void setUType(int type)
    {
        u_type = type;
    }

private:
    uint32_t m_seq; //!< Sequence number
    uint64_t m_ts;  //!< Timestamp
    uint8_t u_type; // 通用字段类型。分为HEL，ACK，MES，ERR
    uint8_t m_size; // 消息长度
};