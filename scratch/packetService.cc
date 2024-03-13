#include <fstream>
#include <iostream>
#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/internet-module.h"

#include <ctime>
#include "ns3/header.h"

using namespace ns3;
NS_LOG_COMPONENT_DEFINE("mi_manet");

void ReceivePacket(Ptr<Socket> socket)
{
    std::cout << "接受包" << std::endl;
    Ptr<Packet> packet;
    Address from; //设置起始地址
    Ipv6Header header1;

    while ((packet = socket->RecvFrom(from))) //循环接收
    {
        std::cout << "1大小" << packet->GetSize() << std::endl;
        packet->RemoveHeader(header1);
        // UdpHeader header2;
        // packet->RemoveHeader(header2);

        // std::cout<<"收到的头：" <<packet->RemoveHeader(header1)<<std::endl;

        // std::cout << from << std::endl;
        if (packet->GetSize() > 0) //不为空
        {
            packet->EnablePrinting();

            //以string类型输出packet内容
            std::ostringstream os;
            packet->CopyData(&os, 10);
            // NS_LOG_INFO(os.str());
            std::cout << os.str().substr() << std::endl;
            // std::cout<<os.str()<<std::endl;

            // std::ofstream ofile;               //定义输出文件
            // ofile.open("test1.txt");     //作为输出文件打开
            // ofile<<os.str().substr()<<std::endl;   //标题写入文件

            std::cout << "大小" << packet->GetSize() << std::endl;
            //以int类型输出packet内容
            uint8_t buffer[10];
            packet->CopyData(buffer, 10);
            for (int i = 0; i < 10; i++)
                std::cout << "packet content:" << int(buffer[i]) << std::endl; //这里需要int强转一下

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

void SendHELPacket(Ptr<Socket> socket, Ipv4InterfaceContainer adhocInterfaces, int client, Address &address)
{

    // 在HEL包中要输入 MessageType, MessageSize,SendBufferSize, ReceiveBufferSize, ServerAddress等字段

    uint8_t m_type = 0;                                // 1字节
    uint32_t s_bufferSize = pow(2, 16);                // 32位，4字节
    uint32_t r_bufferSize = pow(2, 16);                // 32位字段， 4 字节
    uint32_t addressLen = address.GetSerializedSize(); // 4字节
    int packetSizeOfHEL = 1 + addressLen;              // 4字节
    uint8_t buffer[255];                               // 255 改为packetSizeOfHEL会引发报错
    buffer[0] = m_type;
    address.CopyAllTo(&buffer[1], addressLen);
    Ptr<Packet> packet = Create<Packet>(buffer, packetSizeOfHEL);
    socket->Send(packet);
    Time startTime = Simulator::Now();
    InetSocketAddress clientAddress = adhocInterfaces.GetAddress(client);
    NS_LOG_INFO("Client:" << clientAddress << " send a HEL Message to " << address);

    // 在发送HEL包后应该等待ACK包，在等待ACK包后，发送浏览请求包,通过使用ns3的模拟事件实现
}

// 在ReceivePacket中收到HEL包后，向客户端发送ACK包
void SendACKPacket(Ptr<Socket> socket, Address &clientAddress, Address &serviceAddress)
{
    uint8_t m_type = 1; // 1字节
    uint32_t s_bufferSize = pow(2, 16); // 32位，4字节
    uint32_t r_bufferSize = pow(2, 16); // 32位字段， 4 字节

    uint8_t buffer[9];

    buffer[0] = m_type;
    for(int i = 0; i < 4; ++i){
        buffer[i + 1] = s_bufferSize >> i * 8;
        buffer[i + 5] = r_bufferSize >> i * 8;
    }

    Ptr<Packet> packet = Create<Packet>(buffer, 9);
    socket->Send(packet);
    Time sendTime = Simulator::Now();
    NS_LOG_INFO("Service:" << serviceAddress << " send a ACK Message to " << clientAddress);
}

// 客户端发送MES_Browser数据包

void SendMES_BrowserRequestPacket(Ptr<Socket> socket, Ipv4InterfaceContainer adhocInterfaces, int client, Address &address){
    uint8_t m_type = 2; // 1字节, 代表MES字段
    uint8_t s_type = 0; // 0:browser request, 1:browse response, 2:read request, 3: read response
    // 设置 数据包的payload
    uint8_t name_space = 0;
    uint8_t browseNodeId = 1; 

    
}

void SendMES_BrowserResponsePacket(Ptr<Socket> socket, Ipv4InterfaceContainer adhocInterfaces, int client, Address &address){
    uint8_t m_type = 2; // 1字节, 代表MES字段
    uint8_t s_type = 1; // 0:browser request, 1:browse response, 2:read request, 3: read response
    // 设置 数据包的payload，通过namespace和nodeId两个字段，访问当在该namespace中，和nodeid底下所有的有关系的其余节点
    


}




