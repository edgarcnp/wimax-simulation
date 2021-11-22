#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/applications-module.h"
#include "ns3/mobility-module.h"
#include "ns3/config-store-module.h"
#include "ns3/wimax-module.h"
#include "ns3/internet-module.h"
#include "ns3/global-route-manager.h"
#include "ns3/ipcs-classifier-record.h"
#include "ns3/service-flow.h"
#include "ns3/netanim-module.h"
#include "ns3/net-device-container.h"
#include <iostream>

using namespace ns3;

int main(int argc, char *argv[]){
	WimaxHelper::SchedulerType scheduler = WimaxHelper::SCHED_TYPE_SIMPLE;
   	LogComponentEnable("UdpClient", LOG_LEVEL_INFO);	
  	LogComponentEnable("UdpServer", LOG_LEVEL_INFO);
   	NodeContainer ssNodes; //Subscriber Station Node
   	NodeContainer bsNodes; //Base Station Node
   	ssNodes.Create(2); //Create 2 Subscriber Stations
   	bsNodes.Create(1); //Create 1 Base Station
   	WimaxHelper wimax;
   
   	NetDeviceContainer ssDevs;
   	NetDeviceContainer bsDevs;
   
   	ssDevs = wimax.Install(ssNodes, WimaxHelper::DEVICE_TYPE_SUBSCRIBER_STATION, WimaxHelper::SIMPLE_PHY_TYPE_OFDM, scheduler);
   	bsDevs = wimax.Install(ssNodes, WimaxHelper::DEVICE_TYPE_BASE_STATION, WimaxHelper::SIMPLE_PHY_TYPE_OFDM, scheduler);
   
   	Ptr<SubscriberStationNetDevice>ss[2];
  	for(int i = 0; i < 2; i++){
      	ss[i] = ssDevs.Get(i) -> GetObject<SubscriberStationNetDevice>();
      	ss[i] -> SetModulationType(WimaxPhy::MODULATION_TYPE_QAM64_34);
   	}
   
  	Ptr<BaseStationNetDevice>bs;
   	bs = bsDevs.Get(0) -> GetObject<BaseStationNetDevice>();
   	InternetStackHelper stack;
   	stack.Install(bsNodes);
  	stack.Install(ssNodes);
   	Ipv4AddressHelper address;
   	address.SetBase("10.1.7.0", "255.255.255.0"); //IP : 10.1.7.0/24
   	Ipv4InterfaceContainer SSinterfaces = address.Assign(ssDevs);
   	Ipv4InterfaceContainer BSinterface = address.Assign(bsDevs);
   
   	Ptr<ListPositionAllocator>
   	positionAlloc = CreateObject<ListPositionAllocator>();
   	positionAlloc -> Add(Vector(15, 25, 0));
   	positionAlloc -> Add(Vector(25, 15, 0));
   	positionAlloc -> Add(Vector(5, 15, 0));
   	MobilityHelper bs_mobility;
   	bs_mobility.SetMobilityModel("ns3::ConstantPositionMobilityModel");
   	bs_mobility.SetPositionAllocator(positionAlloc);
   	bs_mobility.Install(bsNodes);
   	bs_mobility.Install(ssNodes);
  	 
   	UdpServerHelper udpServer;
   	ApplicationContainer serverApps;
   	UdpClientHelper udpClient;
   	ApplicationContainer clientApps;
   	udpServer = UdpServerHelper (22000);
   	serverApps = udpServer.Install(ssNodes.Get(0));
   	serverApps.Start(Seconds(4.0));
   	serverApps.Stop(Seconds(10.0));
   	udpClient = UdpClientHelper(SSinterfaces.GetAddress(0), 22000);
   	udpClient.SetAttribute("MaxPackets", UintegerValue(150000));
   	udpClient.SetAttribute("Interval", TimeValue(Seconds(0.1)));
   	udpClient.SetAttribute("PacketSize", UintegerValue(512));
   	clientApps = udpClient.Install(ssNodes.Get(1));
   	clientApps.Start(Seconds(5.0));
   	clientApps.Stop(Seconds(9.5));
   	clientApps.Stop(Seconds(15.0));
  	 
   	IpcsClassifierRecord DlClassifierUgs(Ipv4Address("0.0.0.0"), Ipv4Mask("0.0.0.0"), SSinterfaces.GetAddress(0), Ipv4Mask("255.255.255.255"), 0, 65000, 22000, 22000, 17, 1);
   	ServiceFlow DlServiceFlowUgs = wimax.CreateServiceFlow(ServiceFlow::SF_DIRECTION_DOWN, ServiceFlow::SF_TYPE_RTPS, DlClassifierUgs);
   	ss[0] -> AddServiceFlow(DlServiceFlowUgs);
  	IpcsClassifierRecord UlClassifierUgs(SSinterfaces.GetAddress(1), Ipv4Mask("255.255.255.255"), Ipv4Address("0.0.0.0"), Ipv4Mask("0.0.0.0"), 0, 65000, 22000, 22000, 17, 1);
   	ServiceFlow UlServiceFlowUgs = wimax.CreateServiceFlow(ServiceFlow::SF_DIRECTION_UP, ServiceFlow::SF_TYPE_RTPS, UlClassifierUgs);
   	ss[1] -> AddServiceFlow(UlServiceFlowUgs);
   
   	AnimationInterface anim("animationWiMAX.xml");
   	anim.SetConstantPosition(ssNodes.Get(0), 1.0, 3.0);
   	anim.SetConstantPosition(ssNodes.Get(1), 4.0, 6.0);
   	anim.SetConstantPosition(bsNodes.Get(0), 7.0, 8.0);
   	
   	Simulator::Run(); // Run Simulation
   	Simulator::Destroy();
   	return 0;
}