#include <fstream>
#include "ns3/core-module.h"
#include "ns3/header.h"
#include "ns3/ipv6-address.h"
#include "ns3/packet.h"
#include "ns3/ipv6-header.h"
#include "ns3/mac48-address.h"
#include "ns3/ethernet-header.h"
#include "ns3/ipv6-routing-protocol.h"
#include "ns3/internet-module.h"
#include "ns3/csma-module.h"
#include "ns3/ipv6-static-routing-helper.h"
#include "ns3/udp-client-server-helper.h"
// #include "ns3/mpi-interface.h"
#include "ns3/socket.h"
#include "ns3/callback.h"
#include "ns3/applications-module.h"
#include "ns3/ping6-helper.h"
#include "ns3/ripng.h"
#include "ns3/udp-header.h"
#include "ns3/netanim-module.h"

using namespace ns3;
NS_LOG_COMPONENT_DEFINE ("Ipv6_P2P_Example");

void
ReceivePacket (Ptr<Socket> socket)
{
	std::cout<<"接受包"<<std::endl;
  Ptr<Packet> packet;
 	Address from;//设置起始地址
	Ipv6Header header1;

  while ((packet = socket->RecvFrom (from)))//循环接收
  {
    std::cout<<"1大小"<<packet->GetSize()<<std::endl;
	  packet->RemoveHeader(header1);
	  // UdpHeader header2;
	  // packet->RemoveHeader(header2);
    
    // std::cout<<"收到的头：" <<packet->RemoveHeader(header1)<<std::endl;

	  // std::cout << from << std::endl;
      if (packet->GetSize () > 0)//不为空
        {
    	  packet->EnablePrinting();

        //以string类型输出packet内容
        std::ostringstream os;
        packet->CopyData(&os,10);
        // NS_LOG_INFO(os.str());
        std::cout<<os.str().substr()<<std::endl;
        // std::cout<<os.str()<<std::endl;

        // std::ofstream ofile;               //定义输出文件
        // ofile.open("test1.txt");     //作为输出文件打开
        // ofile<<os.str().substr()<<std::endl;   //标题写入文件

        std::cout<<"大小"<<packet->GetSize()<<std::endl;
        //以int类型输出packet内容
        uint8_t buffer[10];
        packet->CopyData(buffer,10);
        for(int i=0;i<10;i++)
          std::cout<<"packet content:"<<int(buffer[i])<<std::endl;//这里需要int强转一下


        /*
    	  uint8_t buffer[16];
    	 //	     	  s=packet->Serialize(buf,32);
    	 	     	  packet->CopyData(buffer,16);
    	 	    	  std::cout<<"溯源路由器地址段：";
    	 	    	  for(int i=0;i<8;i++){
    	 	    		  printf("%02X",buffer[i]);
    	 	    	  }
    	   printf("\n");
    	  Inet6SocketAddress iaddr = Inet6SocketAddress::ConvertFrom (from);
    	    std::ostringstream oss;
    	    oss << "Received one tracesource packet! Socket: " << iaddr.GetIpv6 ()
    	        << " port: " << iaddr.GetPort ()
    	        << " at time = " << Simulator::Now ().GetSeconds ()
    	        << "";

    	  std::string x = oss.str();
    	  std::cout << x << std::endl;
          */
         // NS_LOG_UNCOND (x);
        }
    }
}

/**
 * 发送分组回调方法
 */
static void GenerateTraffic (Ptr<Socket> socket, uint32_t pktSize,
                             uint32_t pktCount, Time pktInterval )
{
  NS_LOG_FUNCTION(socket<<pktSize<<pktCount<<pktInterval);
  if (pktCount > 0)
    {
      Time startTime=Simulator::Now();
      NS_LOG_LOGIC("start time:"<<startTime);


      //修改packet
      // uint8_t fill[10]={1,2,3,4,5,6,7,8,9,0};
      uint8_t fill[10]={'1','2','3','4','5','6','7','8','9','0'}; //能放字符，也能放数字，但是，必须是ASCII码的前256位
	    // for (int i=0;i<2;i++) fill[i]=i;//因为是8位整形，最大只能到256
      Ptr<Packet> packet=Create<Packet>(fill,10);
/*
      Ipv6Header header;
      std::cout<<"1我的 next header:"<<header.GetNextHeader()<<std::endl;
      header.SetNextHeader(2);//使用拓展报头，
      std::cout<<"2我的 next header:"<<header.GetNextHeader()<<std::endl;
      packet->AddHeader(header);
*/      
      socket->Send(packet);
      // socket->Send (Create<Packet> (pktSize));
      //这是个回调函数，根据pktCount的大小进行重复的发送
      Simulator::Schedule (pktInterval, &GenerateTraffic,

                           socket, pktSize,pktCount-1, pktInterval);
    }
  else
    {
      socket->Close ();
    }
}

int main(int argc,char **argv){
    CommandLine cmd;
    cmd.Parse(argc,argv);

    Ptr<Node> node0=CreateObject<Node>();
    Ptr<Node> node1=CreateObject<Node>();
    NodeContainer node(node0,node1);

    InternetStackHelper internetv6;
    internetv6.SetIpv4StackInstall(false);
    internetv6.Install(node);

    CsmaHelper csma;
    csma.SetChannelAttribute("DataRate",StringValue("10Mbps"));
    csma.SetChannelAttribute("Delay",StringValue("2ms"));
    NetDeviceContainer device=csma.Install(node);

    Ipv6AddressHelper ipv6;
    ipv6.SetBase(Ipv6Address("2001:1::"),Ipv6Prefix(64));
    Ipv6InterfaceContainer i=ipv6.Assign(device);

    ns3::PacketMetadata::Enable();

    TypeId tid=UdpSocketFactory::GetTypeId();
    //socket的创建,作为服务端
    Ptr<Socket> recvSink = Socket::CreateSocket (node0, tid);
    Inet6SocketAddress local = Inet6SocketAddress (i.GetAddress(0,1),80);
    recvSink->Bind (local);//绑定地址，等待连接
    recvSink->SetRecvCallback (MakeCallback (&ReceivePacket));//设置回调函数地址


    //创建socket，作为客户端
    Ptr<Socket> source = Socket::CreateSocket (node1, tid);
    Inet6SocketAddress remote = Inet6SocketAddress (i.GetAddress(1,1), 80);
    source->SetAllowBroadcast (false);
    source->Bind(remote);//绑定地址
    source->Connect (local);//发起连接
    // 连接成功，通过GenerateTraffic方法发packet,,,下面的四个参数是GenerateTraffic的参数
    Simulator::ScheduleWithContext (source->GetNode ()->GetId (),Seconds (10), &GenerateTraffic,
                                    source, 100, 5, Seconds(2));

    csma.EnablePcapAll("csma_v6");
    AnimationInterface anim ("csma_v6.xml");
    Simulator::Run();
    Simulator::Destroy();
  
    return 0;
}
