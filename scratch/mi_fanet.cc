#include "ns3/point-to-point-module.h"
#include "ns3/ipv4-global-routing-helper.h"
#include <fstream>
#include <string>
#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/applications-module.h"
#include "ns3/mobility-module.h"
#include "ns3/config-store-module.h"
#include "ns3/wifi-module.h"

#include "ns3/aodv-helper.h"
#include "ns3/internet-module.h"
#include "ns3/netanim-module.h"

using namespace ns3;
NS_LOG_COMPONENT_DEFINE("Mob");

void
ServerConnectionEstablished (Ptr<const ThreeGppHttpServer>, Ptr<Socket>)
{
  NS_LOG_INFO ("Client has established a connection to the server.");
}

void
MainObjectGenerated (uint32_t size)
{
  NS_LOG_INFO ("Server generated a main object of " << size << " bytes.");
}

void
EmbeddedObjectGenerated (uint32_t size)
{
  NS_LOG_INFO ("Server generated an embedded object of " << size << " bytes.");
}

void
ServerTx (Ptr<const Packet> packet)
{
  NS_LOG_INFO ("Server sent a packet of " << packet->GetSize () << " bytes.");
}

void
ClientRx (Ptr<const Packet> packet, const Address &address)
{
  NS_LOG_INFO ("Client received a packet of " << packet->GetSize () << " bytes from " << address);
}

void
ClientMainObjectReceived (Ptr<const ThreeGppHttpClient>, Ptr<const Packet> packet)
{
  Ptr<Packet> p = packet->Copy ();
  ThreeGppHttpHeader header;
  p->RemoveHeader (header);
  if (header.GetContentLength () == p->GetSize ()
      && header.GetContentType () == ThreeGppHttpHeader::MAIN_OBJECT)
    {
      NS_LOG_INFO ("Client has successfully received a main object of "
                   << p->GetSize () << " bytes.");
    }
  else
    {
      NS_LOG_INFO ("Client failed to parse a main object. ");
    }
}

void
ClientEmbeddedObjectReceived (Ptr<const ThreeGppHttpClient>, Ptr<const Packet> packet)
{
  Ptr<Packet> p = packet->Copy ();
  ThreeGppHttpHeader header;
  p->RemoveHeader (header);
  if (header.GetContentLength () == p->GetSize ()
      && header.GetContentType () == ThreeGppHttpHeader::EMBEDDED_OBJECT)
    {
      NS_LOG_INFO ("Client has successfully received an embedded object of "
                   << p->GetSize () << " bytes.");
    }
  else
    {
      NS_LOG_INFO ("Client failed to parse an embedded object. ");
    }
}

int main(int argc, char *argv[])
{
    double simTimeSec = 30000;
    CommandLine cmd;
    cmd.Parse(argc, argv);
    Time::SetResolution (Time::NS); // ns指的是纳秒
    LogComponentEnableAll (LOG_PREFIX_TIME);
    //LogComponentEnableAll (LOG_PREFIX_FUNC);
    //LogComponentEnable ("ThreeGppHttpClient", LOG_INFO);
    ///LogComponentEnable ("ThreeGppHttpServer", LOG_INFO);
    LogComponentEnable ("Mob", LOG_INFO);

    NodeContainer c;
    c.Create(20); //20 wireless nodes

    WifiHelper wifi;
    wifi.SetStandard(WIFI_STANDARD_80211b);
    //80211a, 80211b, 80211n, 2.4g and 5G, 80211ac, 80211ax is also supported.80211p (VANETs, WAVE)

    WifiMacHelper mac;
    mac.SetType("ns3::AdhocWifiMac");
    //AdhocWifiMac, StaWifiMac, ApWifiMac
    wifi.SetRemoteStationManager("ns3::ConstantRateWifiManager",
                                 "DataMode", StringValue("OfdmRate54Mbps"));
    YansWifiPhyHelper wifiPhy = YansWifiPhyHelper();
    YansWifiChannelHelper wifiChannel = YansWifiChannelHelper::Default();
    wifiChannel.SetPropagationDelay("ns3::ConstantSpeedPropagationDelayModel");
    wifiChannel.AddPropagationLoss("ns3::FriisPropagationLossModel");
    wifiPhy.SetChannel(wifiChannel.Create());
    NetDeviceContainer cDevices = wifi.Install(wifiPhy, mac, c);
    //
    NS_LOG_INFO("Enabling AODV routing on all backbone nodes");

    // 路由选择
    AodvHelper aodv;
    //AODV protocol is being using FANETs.
    InternetStackHelper internet;
    internet.SetRoutingHelper(aodv); // has effect on the next Install ()
    internet.Install(c);

    //
    // Assign IPv4 addresses to the device drivers (actually to the associated
    // IPv4 interfaces) we just created.
    //
    Ipv4AddressHelper ipAddrs;
    ipAddrs.SetBase("192.168.0.0", "255.255.255.0");
    Ipv4InterfaceContainer cInterfaces;
    cInterfaces = ipAddrs.Assign(cDevices);

    /*
    //Mobility Model - 2D
    MobilityHelper mobility;

    mobility.SetPositionAllocator ("ns3::GridPositionAllocator",
      "MinX", DoubleValue (0.0),
      "MinY", DoubleValue (0.0),
      "DeltaX", DoubleValue (5.0),
      "DeltaY", DoubleValue (10.0),
      "GridWidth", UintegerValue (3),
      "LayoutType", StringValue ("RowFirst"));

    mobility.SetMobilityModel ("ns3::RandomWalk2dMobilityModel",  "Bounds", RectangleValue (Rectangle (-100, 100, -100, 100)));
    mobility.Install (c);
    */

    //Mobility Model -3D
    MobilityHelper mobility;
    mobility.SetMobilityModel("ns3::GaussMarkovMobilityModel",
                              "Bounds", BoxValue(Box(0, 100, 0, 100, 0, 100)),
                              "TimeStep", TimeValue(Seconds(0.5)),
                              "Alpha", DoubleValue(0.85),
                              "MeanVelocity", StringValue("ns3::UniformRandomVariable[Min=800|Max=1200]"),
                              "MeanDirection", StringValue("ns3::UniformRandomVariable[Min=0|Max=6.283185307]"),
                              "MeanPitch", StringValue("ns3::UniformRandomVariable[Min=0.05|Max=0.05]"),
                              "NormalVelocity", StringValue("ns3::NormalRandomVariable[Mean=0.0|Variance=0.0|Bound=0.0]"),
                              "NormalDirection", StringValue("ns3::NormalRandomVariable[Mean=0.0|Variance=0.2|Bound=0.4]"),
                              "NormalPitch", StringValue("ns3::NormalRandomVariable[Mean=0.0|Variance=0.02|Bound=0.04]"));
    mobility.SetPositionAllocator("ns3::RandomBoxPositionAllocator",
                                  "X", StringValue("ns3::UniformRandomVariable[Min=0|Max=100]"),
                                  "Y", StringValue("ns3::UniformRandomVariable[Min=0|Max=100]"),
                                  "Z", StringValue("ns3::UniformRandomVariable[Min=0|Max=100]"));
    mobility.Install(c);

    // 应用层协议设置
    // UdpEchoServerHelper echoServer(9); // 9为端口

    // ApplicationContainer serverApps = echoServer.Install(c.Get(0));
    // serverApps.Start(Seconds(1.0));
    // serverApps.Stop(Seconds(10.0));

    // UdpEchoClientHelper echoClient(cInterfaces.GetAddress(0), 9);
    // echoClient.SetAttribute("MaxPackets", UintegerValue(1));
    // echoClient.SetAttribute("Interval", TimeValue(Seconds(1.0)));
    // echoClient.SetAttribute("PacketSize", UintegerValue(1024));

    // ApplicationContainer clientApps = echoClient.Install(c.Get(1));
    // clientApps.Start(Seconds(2.0));
    // clientApps.Stop(Seconds(10.0));

    // Create HTTP server helper
    Ipv4Address serverAddress = cInterfaces.GetAddress (1);
  ThreeGppHttpServerHelper serverHelper (serverAddress);

  // Install HTTP server
  ApplicationContainer serverApps = serverHelper.Install (c.Get (1));
  Ptr<ThreeGppHttpServer> httpServer = serverApps.Get (0)->GetObject<ThreeGppHttpServer> ();

  // Example of connecting to the trace sources
  httpServer->TraceConnectWithoutContext ("ConnectionEstablished",
                                          MakeCallback (&ServerConnectionEstablished));
  httpServer->TraceConnectWithoutContext ("MainObject", MakeCallback (&MainObjectGenerated));
  httpServer->TraceConnectWithoutContext ("EmbeddedObject", MakeCallback (&EmbeddedObjectGenerated));
  httpServer->TraceConnectWithoutContext ("Tx", MakeCallback (&ServerTx));

  // Setup HTTP variables for the server
  PointerValue varPtr;
  httpServer->GetAttribute ("Variables", varPtr);
  Ptr<ThreeGppHttpVariables> httpVariables = varPtr.Get<ThreeGppHttpVariables> ();
  httpVariables->SetMainObjectSizeMean (102400); // 100kB
  httpVariables->SetMainObjectSizeStdDev (40960); // 40kB


  // Create HTTP client helper
  ThreeGppHttpClientHelper clientHelper (serverAddress);

  // Install HTTP client
  ApplicationContainer clientApps = clientHelper.Install (c.Get (0));
  Ptr<ThreeGppHttpClient> httpClient = clientApps.Get (0)->GetObject<ThreeGppHttpClient> ();

  // Example of connecting to the trace sources
  httpClient->TraceConnectWithoutContext ("RxMainObject", MakeCallback (&ClientMainObjectReceived));
  httpClient->TraceConnectWithoutContext ("RxEmbeddedObject", MakeCallback (&ClientEmbeddedObjectReceived));
  httpClient->TraceConnectWithoutContext ("Rx", MakeCallback (&ClientRx));

  // Stop browsing after 30 minutes
  clientApps.Stop (Seconds (simTimeSec));
    wifiPhy.EnablePcapAll("Fanet3D"); //Packet Capture.
    //Network Animation using NetAnim.
    AnimationInterface anim("Fanet3D.xml");
    //Ascii Trace Metrics can be processed using Tracemetrics Software.
    AsciiTraceHelper ascii;
    wifiPhy.EnableAsciiAll(ascii.CreateFileStream("Fanet3D.tr"));

    Simulator::Stop(Seconds(10.0));
    Simulator::Run();
    Simulator::Destroy();
    return 0;
}