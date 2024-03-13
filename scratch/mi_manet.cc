
#include <fstream>
#include <iostream>
#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/internet-module.h"
#include "ns3/mobility-module.h"
#include "ns3/aodv-module.h"
#include "ns3/applications-module.h"
#include "ns3/yans-wifi-helper.h"
#include <ctime>
#include "ns3/header.h"
#include "ns3/netanim-module.h"
#include "packetService.cc"


using namespace ns3;
NS_LOG_COMPONENT_DEFINE("mi_manet");


int main(int argc, char **argv)
{
  double txp = 7.5;
  int nodeSpeed = 20; //in m/s
  int nodePause = 0;  //in s
  CommandLine cmd;
  cmd.Parse(argc, argv);
  // 创建20个节点
  NodeContainer nodes(20);
  std::string phyMode("DsssRate11Mbps");
  //Set Non-unicastMode rate to unicast mode
  Config::SetDefault("ns3::WifiRemoteStationManager::NonUnicastMode", StringValue(phyMode));
  // setting up wifi phy and channel using helpers
  WifiHelper wifi;
  wifi.SetStandard(WIFI_STANDARD_80211b);
  YansWifiPhyHelper wifiPhy;
  YansWifiChannelHelper wifiChannel;
  wifiChannel.SetPropagationDelay("ns3::ConstantSpeedPropagationDelayModel");
  wifiChannel.AddPropagationLoss("ns3::FriisPropagationLossModel");
  wifiPhy.SetChannel(wifiChannel.Create());
  // Add a mac and disable rate control
  WifiMacHelper wifiMac;
  wifi.SetRemoteStationManager("ns3::ConstantRateWifiManager",
                               "DataMode", StringValue(phyMode),
                               "ControlMode", StringValue(phyMode));
  wifiPhy.Set("TxPowerStart", DoubleValue(txp));
  wifiPhy.Set("TxPowerEnd", DoubleValue(txp));
  wifiMac.SetType("ns3::AdhocWifiMac");
  NetDeviceContainer adhocDevices = wifi.Install(wifiPhy, wifiMac, nodes);
  MobilityHelper mobilityAdhoc;
  int64_t streamIndex = 0; // used to get consistent mobility across scenarios
  ObjectFactory pos;
  pos.SetTypeId("ns3::RandomRectanglePositionAllocator");
  pos.Set("X", StringValue("ns3::UniformRandomVariable[Min=0.0|Max=300.0]"));
  pos.Set("Y", StringValue("ns3::UniformRandomVariable[Min=0.0|Max=1500.0]"));
  Ptr<PositionAllocator> taPositionAlloc = pos.Create()->GetObject<PositionAllocator>();
  streamIndex += taPositionAlloc->AssignStreams(streamIndex);
  std::stringstream ssSpeed;
  ssSpeed << "ns3::UniformRandomVariable[Min=0.0|Max=" << nodeSpeed << "]";
  std::stringstream ssPause;
  ssPause << "ns3::ConstantRandomVariable[Constant=" << nodePause << "]";
  mobilityAdhoc.SetMobilityModel("ns3::RandomWaypointMobilityModel",
                                 "Speed", StringValue(ssSpeed.str()),
                                 "Pause", StringValue(ssPause.str()),
                                 "PositionAllocator", PointerValue(taPositionAlloc));
  mobilityAdhoc.SetPositionAllocator(taPositionAlloc);
  mobilityAdhoc.Install(nodes);
  streamIndex += mobilityAdhoc.AssignStreams(nodes, streamIndex);
  NS_UNUSED(streamIndex); // From this point, streamIndex is unused
  AodvHelper aodv;
  Ipv4ListRoutingHelper list;
  InternetStackHelper internet;
  list.Add(aodv, 100);
  internet.SetRoutingHelper(list);
  internet.Install(nodes);
  NS_LOG_INFO("assigning ip address");
  Ipv4AddressHelper addressAdhoc;
  addressAdhoc.SetBase("10.1.1.0", "255.255.255.0");
  Ipv4InterfaceContainer adhocInterfaces;
  adhocInterfaces = addressAdhoc.Assign(adhocDevices);
  // 创建socket
  ns3::PacketMetadata::Enable();
  TypeId tid = UdpSocketFactory::GetTypeId();
  // 从20个节点中随机抽取1一个节点作为服务器
  srand(time(0));
  int indexOfServer = rand() % 19;
  Ptr<Socket> serverSink = Socket::CreateSocket(nodes.Get(indexOfServer), tid); // 创建了一个udpsocket
  InetSocketAddress serverAddress = InetSocketAddress(adhocInterfaces.GetAddress(indexOfServer), 80);
  serverSink->Bind(serverAddress);
  serverSink->SetRecvCallback(MakeCallback(&ReceivePacket)); // 设置监听收到包的函数
  // 剩余19个作为客户端
  for (int i = 0; i < 20; ++i)
  {
    if (i == indexOfServer)
      continue;
    Ptr<Node> clientNode = nodes.Get(i);
    Ptr<Socket> client = Socket::CreateSocket(clientNode, tid);
    InetSocketAddress clientAddress = InetSocketAddress(adhocInterfaces.GetAddress(i), 80);
    client->SetAllowBroadcast(false); // 不允许广播
    client->Bind(clientAddress);
    client->Connect(serverAddress); // 连接好服务器地址
    /*
        连接成功后。通过GenerateTraffic 方法发送packet。来模拟事件.
        client->GetNode ()->GetId ()是context。这里的context指的是包含的是事件对应的节点的信息。
        如果是产生要跨节点的调度事件，则需要给调度器传递context即节点的信息。
        跨节点的调度事件的这种情况通常发生在发送包被传递到信道，、
        要使信道中其它节点能接收到该数据包，则在调度接收事件时还要传递目标节点的context信息
        在Seconds(10)后调用GenerateTraffic
        后面为GenerateTraffic的参数
        */
    // imulator::ScheduleWithContext(client->GetNode()->GetId(), Seconds(10), &GenerateTraffic,
    //                                client, 100, 5, Seconds(2));S
  }
  // wifiPhy.EnablePcapAll("mi_manet"); //Packet Capture.

  // Simulator::Stop(Seconds(10.0));
  Simulator::Run();
  Simulator::Destroy();
  return 0;
}
