#include <ns3/simulator.h>
#include <ns3/seq-ts-header.h>
#include <ns3/udp-header.h>
#include <ns3/ipv4-header.h>
#include "ns3/ppp-header.h"
#include "ns3/boolean.h"
#include "ns3/uinteger.h"
#include "ns3/double.h"
#include "ns3/data-rate.h"
#include "ns3/pointer.h"
#include "rdma-hw.h"
#include "ppp-header.h"
#include "qbb-header.h"
#include "cn-header.h"
#include "ns3/unsched-tag.h"
#include <algorithm> 

namespace ns3 {

TypeId RdmaHw::GetTypeId (void)
{
	static TypeId tid = TypeId ("ns3::RdmaHw")
	                    .SetParent<Object> ()
	                    .AddAttribute("MinRate",
	                                  "Minimum rate of a throttled flow",
	                                  DataRateValue(DataRate("100Mb/s")),
	                                  MakeDataRateAccessor(&RdmaHw::m_minRate),
	                                  MakeDataRateChecker())
	                    .AddAttribute("Mtu",
	                                  "Mtu.",
	                                  UintegerValue(1000),
	                                  MakeUintegerAccessor(&RdmaHw::m_mtu),
	                                  MakeUintegerChecker<uint32_t>())
	                    .AddAttribute ("CcMode",
	                                   "which mode of DCQCN is running",
	                                   UintegerValue(0),
	                                   MakeUintegerAccessor(&RdmaHw::m_cc_mode),
	                                   MakeUintegerChecker<uint32_t>())
	                    .AddAttribute("NACKGenerationInterval",
	                                  "The NACK Generation interval",
	                                  DoubleValue(500.0),
	                                  MakeDoubleAccessor(&RdmaHw::m_nack_interval),
	                                  MakeDoubleChecker<double>())
	                    .AddAttribute("L2ChunkSize",
	                                  "Layer 2 chunk size. Disable chunk mode if equals to 0.",
	                                  UintegerValue(0),
	                                  MakeUintegerAccessor(&RdmaHw::m_chunk),
	                                  MakeUintegerChecker<uint32_t>())
	                    .AddAttribute("L2AckInterval",
	                                  "Layer 2 Ack intervals. Disable ack if equals to 0.",
	                                  UintegerValue(0),
	                                  MakeUintegerAccessor(&RdmaHw::m_ack_interval),
	                                  MakeUintegerChecker<uint32_t>())
	                    .AddAttribute("L2BackToZero",
	                                  "Layer 2 go back to zero transmission.",
	                                  BooleanValue(false),
	                                  MakeBooleanAccessor(&RdmaHw::m_backto0),
	                                  MakeBooleanChecker())
	                    .AddAttribute("EwmaGain",
	                                  "Control gain parameter which determines the level of rate decrease",
	                                  DoubleValue(1.0 / 16),
	                                  MakeDoubleAccessor(&RdmaHw::m_g),
	                                  MakeDoubleChecker<double>())
	                    .AddAttribute ("RateOnFirstCnp",
	                                   "the fraction of rate on first CNP",
	                                   DoubleValue(1.0),
	                                   MakeDoubleAccessor(&RdmaHw::m_rateOnFirstCNP),
	                                   MakeDoubleChecker<double> ())
	                    .AddAttribute("ClampTargetRate",
	                                  "Clamp target rate.",
	                                  BooleanValue(false),
	                                  MakeBooleanAccessor(&RdmaHw::m_EcnClampTgtRate),
	                                  MakeBooleanChecker())
	                    .AddAttribute("RPTimer",
	                                  "The rate increase timer at RP in microseconds",
	                                  DoubleValue(1500.0),
	                                  MakeDoubleAccessor(&RdmaHw::m_rpgTimeReset),
	                                  MakeDoubleChecker<double>())
	                    .AddAttribute("RateDecreaseInterval",
	                                  "The interval of rate decrease check",
	                                  DoubleValue(4.0),
	                                  MakeDoubleAccessor(&RdmaHw::m_rateDecreaseInterval),
	                                  MakeDoubleChecker<double>())
	                    .AddAttribute("FastRecoveryTimes",
	                                  "The rate increase timer at RP",
	                                  UintegerValue(5),
	                                  MakeUintegerAccessor(&RdmaHw::m_rpgThreshold),
	                                  MakeUintegerChecker<uint32_t>())
	                    .AddAttribute("AlphaResumInterval",
	                                  "The interval of resuming alpha",
	                                  DoubleValue(55.0),
	                                  MakeDoubleAccessor(&RdmaHw::m_alpha_resume_interval),
	                                  MakeDoubleChecker<double>())
	                    .AddAttribute("RateAI",
	                                  "Rate increment unit in AI period",
	                                  DataRateValue(DataRate("5Mb/s")),
	                                  MakeDataRateAccessor(&RdmaHw::m_rai),
	                                  MakeDataRateChecker())
	                    .AddAttribute("RateHAI",
	                                  "Rate increment unit in hyperactive AI period",
	                                  DataRateValue(DataRate("50Mb/s")),
	                                  MakeDataRateAccessor(&RdmaHw::m_rhai),
	                                  MakeDataRateChecker())
	                    .AddAttribute("VarWin",
	                                  "Use variable window size or not",
	                                  BooleanValue(false),
	                                  MakeBooleanAccessor(&RdmaHw::m_var_win),
	                                  MakeBooleanChecker())
	                    .AddAttribute("FastReact",
	                                  "Fast React to congestion feedback",
	                                  BooleanValue(true),
	                                  MakeBooleanAccessor(&RdmaHw::m_fast_react),
	                                  MakeBooleanChecker())
	                    .AddAttribute("MiThresh",
	                                  "Threshold of number of consecutive AI before MI",
	                                  UintegerValue(5),
	                                  MakeUintegerAccessor(&RdmaHw::m_miThresh),
	                                  MakeUintegerChecker<uint32_t>())
	                    .AddAttribute("TargetUtil",
	                                  "The Target Utilization of the bottleneck bandwidth, by default 95%",
	                                  DoubleValue(0.95),
	                                  MakeDoubleAccessor(&RdmaHw::m_targetUtil),
	                                  MakeDoubleChecker<double>())
	                    .AddAttribute("UtilHigh",
	                                  "The upper bound of Target Utilization of the bottleneck bandwidth, by default 98%",
	                                  DoubleValue(0.98),
	                                  MakeDoubleAccessor(&RdmaHw::m_utilHigh),
	                                  MakeDoubleChecker<double>())
	                    .AddAttribute("RateBound",
	                                  "Bound packet sending by rate, for test only",
	                                  BooleanValue(true),
	                                  MakeBooleanAccessor(&RdmaHw::m_rateBound),
	                                  MakeBooleanChecker())
	                    .AddAttribute("MultiRate",
	                                  "Maintain multiple rates in HPCC",
	                                  BooleanValue(true),
	                                  MakeBooleanAccessor(&RdmaHw::m_multipleRate),
	                                  MakeBooleanChecker())
	                    .AddAttribute("SampleFeedback",
	                                  "Whether sample feedback or not",
	                                  BooleanValue(false),
	                                  MakeBooleanAccessor(&RdmaHw::m_sampleFeedback),
	                                  MakeBooleanChecker())
	                    .AddAttribute("TimelyAlpha",
	                                  "Alpha of TIMELY",
	                                  DoubleValue(0.875),
	                                  MakeDoubleAccessor(&RdmaHw::m_tmly_alpha),
	                                  MakeDoubleChecker<double>())
	                    .AddAttribute("TimelyBeta",
	                                  "Beta of TIMELY",
	                                  DoubleValue(0.8),
	                                  MakeDoubleAccessor(&RdmaHw::m_tmly_beta),
	                                  MakeDoubleChecker<double>())
	                    .AddAttribute("TimelyTLow",
	                                  "TLow of TIMELY (ns)",
	                                  UintegerValue(50000),
	                                  MakeUintegerAccessor(&RdmaHw::m_tmly_TLow),
	                                  MakeUintegerChecker<uint64_t>())
	                    .AddAttribute("TimelyTHigh",
	                                  "THigh of TIMELY (ns)",
	                                  UintegerValue(500000),
	                                  MakeUintegerAccessor(&RdmaHw::m_tmly_THigh),
	                                  MakeUintegerChecker<uint64_t>())
	                    .AddAttribute("TimelyMinRtt",
	                                  "MinRtt of TIMELY (ns)",
	                                  UintegerValue(20000),
	                                  MakeUintegerAccessor(&RdmaHw::m_tmly_minRtt),
	                                  MakeUintegerChecker<uint64_t>())
	                    .AddAttribute("DctcpRateAI",
	                                  "DCTCP's Rate increment unit in AI period",
	                                  DataRateValue(DataRate("1000Mb/s")),
	                                  MakeDataRateAccessor(&RdmaHw::m_dctcp_rai),
	                                  MakeDataRateChecker())
	                    .AddAttribute("PintSmplThresh",
	                                  "PINT's sampling threshold in rand()%65536",
	                                  UintegerValue(65536),
	                                  MakeUintegerAccessor(&RdmaHw::pint_smpl_thresh),
	                                  MakeUintegerChecker<uint32_t>())
	                    .AddAttribute("PowerTCPEnabled", "to enable PowerTCP", BooleanValue(false), MakeBooleanAccessor(&RdmaHw::PowerTCPEnabled), MakeBooleanChecker())
	                    .AddAttribute("PowerTCPdelay", "to enable PowerTCP in delaymode", BooleanValue(false), MakeBooleanAccessor(&RdmaHw::PowerTCPdelay), MakeBooleanChecker())
	                    .AddAttribute("enableMultiPath","this enables handling out-of-order packets",BooleanValue(false), MakeBooleanAccessor(&RdmaHw::enableMultiPath), MakeBooleanChecker())
	                    .AddAttribute("rto","retransmission timeout",DoubleValue(UINT64_MAX), MakeDoubleAccessor(&RdmaHw::rto), MakeDoubleChecker<double>())
	                    .AddAttribute("IntialCwnd","Initial congestion window in Bytes",UintegerValue(UINT64_MAX), MakeUintegerAccessor(&RdmaHw::initCwnd), MakeUintegerChecker<uint64_t>())
	                    .AddAttribute("nSpines", "number of paths to choose from (assuming Leaf/Spine topology)", UintegerValue(UINT32_MAX), MakeUintegerAccessor(&RdmaHw::nSpines), MakeUintegerChecker<uint32_t>())
	                    .AddAttribute("sourceRouting", "specify the path within the packets", BooleanValue(false), MakeBooleanAccessor(&RdmaHw::sourceRouting), MakeBooleanChecker())
	                    ;
	return tid;
}

RdmaHw::RdmaHw() {
}

void RdmaHw::SetNode(Ptr<Node> node) {
	m_node = node;
}
void RdmaHw::Setup(QpCompleteCallback cb) {
	for (uint32_t i = 0; i < m_nic.size(); i++) {
		Ptr<QbbNetDevice> dev = m_nic[i].dev;
		if (dev == NULL)
			continue;
		// share data with NIC
		dev->m_rdmaEQ->m_qpGrp = m_nic[i].qpGrp;
		// setup callback
		dev->m_rdmaReceiveCb = MakeCallback(&RdmaHw::Receive, this);
		dev->m_rdmaLinkDownCb = MakeCallback(&RdmaHw::SetLinkDown, this);
		dev->m_rdmaPktSent = MakeCallback(&RdmaHw::PktSent, this);
		// config NIC
		dev->m_rdmaEQ->m_rdmaGetNxtPkt = MakeCallback(&RdmaHw::GetNxtPacket, this);
	}
	// setup qp complete callback
	m_qpCompleteCallback = cb;

	m_rand = CreateObject<UniformRandomVariable>();
}

uint32_t RdmaHw::GetNicIdxOfQp(Ptr<RdmaQueuePair> qp) {
	auto &v = m_rtTable[qp->dip.Get()];
	if (v.size() > 0) {
		return v[qp->GetHash() % v.size()];
	} else {
		NS_ASSERT_MSG(false, "We assume at least one NIC is alive");
	}
}
uint64_t RdmaHw::GetQpKey(uint32_t dip, uint16_t sport, uint16_t pg) {
	return ((uint64_t)dip << 32) | ((uint64_t)sport << 16) | (uint64_t)pg;
}
Ptr<RdmaQueuePair> RdmaHw::GetQp(uint32_t dip, uint16_t sport, uint16_t pg) {
	uint64_t key = GetQpKey(dip, sport, pg);
	auto it = m_qpMap.find(key);
	if (it != m_qpMap.end())
		return it->second;
	return NULL;
}
void RdmaHw::AddQueuePair(uint64_t size, uint16_t pg, Ipv4Address sip, Ipv4Address dip, uint16_t sport, uint16_t dport, uint32_t win, uint64_t baseRtt, Callback<void> notifyAppFinish, Time stopTime) {
	// create qp
	Ptr<RdmaQueuePair> qp = CreateObject<RdmaQueuePair>(pg, sip, dip, sport, dport);
	qp->SetSize(size);
	qp->SetWin(win);
	qp->SetBaseRtt(baseRtt);
	qp->SetVarWin(m_var_win);
	qp->SetAppNotifyCallback(notifyAppFinish);
	qp->stopTime = stopTime;

	if (stopTime == Simulator::GetMaximumSimulationTime()-MicroSeconds(1)){
		qp->incastFlow = 1;
	}
	else{
		qp->incastFlow = 0;
	}

	uint32_t nic_idx = GetNicIdxOfQp(qp);

	if (sourceRouting){
		std::map<uint32_t, uint32_t> numBytesPerPath; // path, numBytes
		int minBytesPath = -1;
		uint64_t minBytes = UINT64_MAX;
		for (uint32_t i = 0; i < m_nic[nic_idx].dev->m_rdmaEQ->m_qpGrp->GetN(); i++){
			Ptr<RdmaQueuePair> qp = m_nic[nic_idx].dev->m_rdmaEQ->m_qpGrp->Get(i);
			if (numBytesPerPath.find(i) != numBytesPerPath.end())
				numBytesPerPath[i] += qp->GetBytesLeft();
			else
				numBytesPerPath[i] = qp->GetBytesLeft();
		}
		for (auto it = numBytesPerPath.begin(); it != numBytesPerPath.end(); ++it ){
			if (it->second < minBytes){
				minBytes = it->second;
				minBytesPath = it->first;
			}
		}
		NS_ASSERT_MSG(minBytesPath >=0, "could not find a path for QP in source routing");
		qp->pathId = minBytesPath;
	}

	// add qp
	m_nic[nic_idx].qpGrp->AddQp(qp);
	uint64_t key = GetQpKey(dip.Get(), sport, pg);
	m_qpMap[key] = qp;


	qp->powerEnabled = PowerTCPEnabled;

	// set init variables
	if (m_nic[nic_idx].dev == NULL) {
		std::cout << "sip " << sip << " dip " << dip << " sport " << sport  << " dport " << dport << std::endl;
	}
	DataRate m_bps = m_nic[nic_idx].dev->GetDataRate();
	if(win){
		if (initCwnd==UINT64_MAX)
			qp->SetWin(m_bps.GetBitRate() * 1 * baseRtt * 1e-9 / 8);
		else
			qp->SetWin(initCwnd);

	}
	qp->m_rate = m_bps;
	qp->m_max_rate = m_bps;
	if (m_cc_mode == 1) {
		qp->mlx.m_targetRate = m_bps;
	} else if (m_cc_mode == 3) {
		qp->hp.m_curRate = m_bps;
		if (m_multipleRate) {
			for (uint32_t i = 0; i < IntHeader::maxHop; i++)
				qp->hp.hopState[i].Rc = m_bps;
		}
	} else if (m_cc_mode == 7) {
		qp->tmly.m_curRate = m_bps;
	} else if (m_cc_mode == 10) {
		qp->hpccPint.m_curRate = m_bps;
	}

	// Notify Nic
	m_nic[nic_idx].dev->NewQp(qp);
	m_nic[nic_idx].dev->m_rdmaEQ->sourceRouting = sourceRouting;
}

void RdmaHw::DeleteQueuePair(Ptr<RdmaQueuePair> qp) {
	// remove qp from the m_qpMap
	uint64_t key = GetQpKey(qp->dip.Get(), qp->sport, qp->m_pg);
	m_qpMap.erase(key);
}

Ptr<RdmaRxQueuePair> RdmaHw::GetRxQp(uint32_t sip, uint32_t dip, uint16_t sport, uint16_t dport, uint16_t pg, bool create) {
	uint64_t key = ((uint64_t)dip << 32) | ((uint64_t)pg << 16) | (uint64_t)dport;
	auto it = m_rxQpMap.find(key);
	if (it != m_rxQpMap.end())
		return it->second;
	if (create) {
		// create new rx qp
		Ptr<RdmaRxQueuePair> q = CreateObject<RdmaRxQueuePair>();
		// init the qp
		q->sip = sip;
		q->dip = dip;
		q->sport = sport;
		q->dport = dport;
		q->m_ecn_source.qIndex = pg;
		// store in map
		m_rxQpMap[key] = q;
		if (sourceRouting)
			q->pathId = m_rand->GetInteger (0, nSpines-1); // choose one path uniformly at random
		return q;
	}
	return NULL;
}
uint32_t RdmaHw::GetNicIdxOfRxQp(Ptr<RdmaRxQueuePair> q) {
	auto &v = m_rtTable[q->dip];
	if (v.size() > 0) {
		return v[q->GetHash() % v.size()];
	} else {
		NS_ASSERT_MSG(false, "We assume at least one NIC is alive");
	}
}
void RdmaHw::DeleteRxQp(uint32_t dip, uint16_t pg, uint16_t dport) {
	uint64_t key = ((uint64_t)dip << 32) | ((uint64_t)pg << 16) | (uint64_t)dport;
	m_rxQpMap.erase(key);
}

int RdmaHw::ReceiveUdp(Ptr<Packet> p, CustomHeader &ch) {
	uint8_t ecnbits = ch.GetIpv4EcnBits();

	uint32_t payload_size = p->GetSize() - ch.GetSerializedSize();

	// TODO find corresponding rx queue pair
	Ptr<RdmaRxQueuePair> rxQp = GetRxQp(ch.dip, ch.sip, ch.udp.dport, ch.udp.sport, ch.udp.pg, true);
	if (ecnbits != 0) {
		rxQp->m_ecn_source.ecnbits |= ecnbits;
		rxQp->m_ecn_source.qfb++;
	}
	rxQp->m_ecn_source.total++;
	rxQp->m_milestone_rx = m_ack_interval;

	int x = ReceiverCheckSeq(ch.udp.seq, rxQp, payload_size);
	if (x == 1 || x == 2 || x==8) { //generate ACK or NACK
		if (x==2) std::cout << "Nack triggered" << std::endl;
		qbbHeader seqh;
		seqh.SetPG(ch.udp.pg);
		seqh.SetSport(ch.udp.dport);
		seqh.SetDport(ch.udp.sport);
		seqh.SetIntHeader(ch.udp.ih);
		if (ecnbits)
			seqh.SetCnp();

	    if (x == 8)
			seqh.SetSeq(ch.udp.seq + payload_size);
		else
			seqh.SetSeq(rxQp->ReceiverNextExpectedSeq);
	    
		// for (auto it = rxQp->reOrderBuffer.begin(); it != myMap.end();) {
		// 	if (it.first == rxQp->ReceiverNextExpectedSeq){
		// 		it = rxQp->reOrderBuffer.erase(it);
		// 	}
		// 	else{
		// 		break;
		// 	}
		// }

	    // std::cout << "reordersize " << rxQp->reOrderBuffer.size() << std::endl;
	    // std::cout << "GenAck " << x << " seq " << seqh.GetSeq() << std::endl;
	    // uint32_t reOrderPkt = 0;
	    // for(uint32_t i = 0; i < rxQp->reOrderBuffer.size(); i++){
	    //   if (std::get<0>(rxQp->reOrderBuffer[i]) != rxQp->ReceiverNextExpectedSeq){
	    //     break;
	    //   }
	    //   else{
	    //   	reOrderPkt += 1;
	    //   	rxQp->ReceiverNextExpectedSeq += std::get<1>(rxQp->reOrderBuffer[i]);
	    //   }
	    // }
	    // while (reOrderPkt){
	    // 	rxQp->reOrderBuffer.erase(rxQp->reOrderBuffer.begin());
	    // 	reOrderPkt--;
	    // }

		Ptr<Packet> newp = Create<Packet>(std::max(60 - 14 - 20 - (int)seqh.GetSerializedSize(), 0));
		newp->AddHeader(seqh);

		Ipv4Header head;	// Prepare IPv4 header
		head.SetDestination(Ipv4Address(ch.sip));
		head.SetSource(Ipv4Address(ch.dip));
		head.SetProtocol(x != 2 ? 0xFC : 0xFD); //ack=0xFC nack=0xFD
		head.SetTtl(64);
		head.SetPayloadSize(newp->GetSize());
		if (sourceRouting)
			head.SetIdentification(rxQp->pathId); // We repurpose this header field for path ID in the case of source routing.
		else
			head.SetIdentification(rxQp->m_ipid++);

		newp->AddHeader(head);
		AddHeader(newp, 0x800);	// Attach PPP header

	    // send
		uint32_t nic_idx = GetNicIdxOfRxQp(rxQp);
		m_nic[nic_idx].dev->RdmaEnqueueHighPrioQ(newp);
		m_nic[nic_idx].dev->TriggerTransmit();
	}
	return 0;
}

int RdmaHw::ReceiveCnp(Ptr<Packet> p, CustomHeader &ch) {
	// QCN on NIC
	// This is a Congestion signal
	// Then, extract data from the congestion packet.
	// We assume, without verify, the packet is destinated to me
	uint32_t qIndex = ch.cnp.qIndex;
	if (qIndex == 1) {		//DCTCP
		std::cout << "TCP--ignore\n";
		return 0;
	}
	uint16_t udpport = ch.cnp.fid; // corresponds to the sport
	uint8_t ecnbits = ch.cnp.ecnBits;
	uint16_t qfb = ch.cnp.qfb;
	uint16_t total = ch.cnp.total;

	uint32_t i;
	// get qp
	Ptr<RdmaQueuePair> qp = GetQp(ch.sip, udpport, qIndex);
	if (qp == NULL)
		std::cout << "ERROR: QCN NIC cannot find the flow\n";
	// get nic
	uint32_t nic_idx = GetNicIdxOfQp(qp);
	Ptr<QbbNetDevice> dev = m_nic[nic_idx].dev;

	if (qp->m_rate == 0)			//lazy initialization
	{
		qp->m_rate = dev->GetDataRate();
		if (m_cc_mode == 1) {
			qp->mlx.m_targetRate = dev->GetDataRate();
		} else if (m_cc_mode == 3) {
			qp->hp.m_curRate = dev->GetDataRate();
			if (m_multipleRate) {
				for (uint32_t i = 0; i < IntHeader::maxHop; i++)
					qp->hp.hopState[i].Rc = dev->GetDataRate();
			}
		} else if (m_cc_mode == 7) {
			qp->tmly.m_curRate = dev->GetDataRate();
		} else if (m_cc_mode == 10) {
			qp->hpccPint.m_curRate = dev->GetDataRate();
		}
	}
	return 0;
}

int RdmaHw::ReceiveAck(Ptr<Packet> p, CustomHeader &ch) {
	uint16_t qIndex = ch.ack.pg;
	uint16_t port = ch.ack.dport;
	uint32_t seq = ch.ack.seq;
	uint8_t cnp = (ch.ack.flags >> qbbHeader::FLAG_CNP) & 1;
	// int i;
	Ptr<RdmaQueuePair> qp = GetQp(ch.sip, port, qIndex);
	if (qp == NULL) {
		std::cout << "ERROR: " << "node:" << m_node->GetId() << ' ' << (ch.l3Prot == 0xFC ? "ACK" : "NACK") << " NIC cannot find the flow\n";
		return 0;
	}

	uint32_t nic_idx = GetNicIdxOfQp(qp);
	Ptr<QbbNetDevice> dev = m_nic[nic_idx].dev;
	if (m_ack_interval == 0)
		std::cout << "ERROR: shouldn't receive ack\n";
	else {
		if (m_backto0) {
			uint32_t goback_seq = seq / m_chunk * m_chunk;
			qp->Acknowledge(goback_seq);
		} else {
			if (!enableMultiPath)
				qp->Acknowledge(seq);
			else{
				// if (!qp->pktsInflight.empty()){
					// Possible case: Ack arrived after a recover state
					// std::cout << "AckSeq " << seq << " expectedAck " << std::get<0>(qp->pktsInflight[0]) << std::endl;
				// }
				auto x = qp->pktsInflight.begin();
				if (x != qp->pktsInflight.end() && x->first == seq){
					std::get<1>(x->second) = true;
					for (auto it = qp->pktsInflight.begin(); it != qp->pktsInflight.end();){
						if (std::get<1>(it->second) == true){
							qp->Acknowledge(it->first);
							std::get<2>(it->second).Cancel(); // cancel the timeout;
							it = qp->pktsInflight.erase(it);
						}
						else{
							break;
						}
					}
				}
				else{
					auto it = qp->pktsInflight.find(seq);
					if (it != qp->pktsInflight.end()){
						std::get<1>(it->second) = true; // indicate that the packet has been ACKed
					}
					else{
						NS_LOG_INFO("Received duplicate ACK");
					}
				}
				// Timeout should be schedule only when a packet is being transmitted.
				// if (!qp->timeout.IsRunning()){
				// 	qp->timeout = Simulator::Schedule(NanoSeconds(rto),&RdmaHw::RecoverQueue,this,qp);
				// }
			}
		}
		if (qp->IsFinished()) {
			if (qp->timeout.IsRunning()){
				qp->timeout.Cancel();
			}
			QpComplete(qp);
		}
	}
	if (ch.l3Prot == 0xFD) // NACK
		RecoverQueue(qp);

	// handle cnp
	if (cnp) {
		// std::cout << "cnp received" <<  std::endl;
		if (m_cc_mode == 1) { // mlx version
			cnp_received_mlx(qp);
		}
	}

	if (m_cc_mode == 3) {
		HandleAckHp(qp, p, ch);
	} else if (m_cc_mode == 7) {
		HandleAckTimely(qp, p, ch);
	} else if (m_cc_mode == 8) {
		HandleAckDctcp(qp, p, ch);
	} else if (m_cc_mode == 10) {
		HandleAckHpPint(qp, p, ch);
	}
	// ACK may advance the on-the-fly window, allowing more packets to send
	dev->TriggerTransmit();
	return 0;
}

int RdmaHw::Receive(Ptr<Packet> p, CustomHeader &ch) {
	if (ch.l3Prot == 0x11) { // UDP
		ReceiveUdp(p, ch);
	} else if (ch.l3Prot == 0xFF) { // CNP
		ReceiveCnp(p, ch);
	} else if (ch.l3Prot == 0xFD) { // NACK
		ReceiveAck(p, ch);
	} else if (ch.l3Prot == 0xFC) { // ACK
		ReceiveAck(p, ch);
	}
	return 0;
}

int RdmaHw::ReceiverCheckSeq(uint32_t seq, Ptr<RdmaRxQueuePair> q, uint32_t size) {
	uint32_t expected = q->ReceiverNextExpectedSeq;
	// if (q->reOrderBuffer.size())
		// std::cout << "expected " << expected << " seq " << seq << " size " << size << " reOrderBuffer " << q->reOrderBuffer.size() << " firstSeq " << q->reOrderBuffer.begin()->first << std::endl;
	
	auto x = q->reOrderBuffer.begin();
	if (x != q->reOrderBuffer.end() && (x->first == expected || seq == expected)) {
		for (auto it = q->reOrderBuffer.begin(); it != q->reOrderBuffer.end();){
			if (it->first == q->ReceiverNextExpectedSeq){
				q->ReceiverNextExpectedSeq += it->second;
				it = q->reOrderBuffer.erase(it);
				q->m_lastNACK = 0; // reset NACK interval
				// std::cout << "restting NACK interval" << std::endl;
			}
			else{
				break;
			}
		}
	}
	else if (x != q->reOrderBuffer.end()){
		NS_ASSERT_MSG(x->first > seq, "Malfunctioning reorder buffer");
	}

	if (seq == expected) {
		if (q->ReceiverNextExpectedSeq == expected) // reorder buffer did not change anything. So, we are ok to increment as usual.
			q->ReceiverNextExpectedSeq = expected + size;
		if (q->ReceiverNextExpectedSeq >= q->m_milestone_rx) {
			q->m_milestone_rx += m_ack_interval;
			return 1; //Generate ACK
		} else if (q->ReceiverNextExpectedSeq % m_chunk == 0) {
			return 1;
		} else {
			return 5;
		}
	} else if (seq > expected) {
		// Generate NACK
		if (enableMultiPath){

			q->reOrderBuffer[seq] = size;

			if (Simulator::Now() >= q->m_nackTimer && (q->m_lastNACK!=0 && q->m_lastNACK!= expected)){
				q->m_nackTimer = Simulator::Now() + MicroSeconds(m_nack_interval);
				q->m_lastNACK = expected;
				std::cout << "NACK Triggered" << std::endl;
				return 2; // Send NACK
			}
			else if (q->m_lastNACK==0){
				q->m_nackTimer = Simulator::Now() + MicroSeconds(m_nack_interval);
				q->m_lastNACK = expected;
			}
			return 8; //Ack just for corresponding seq
		}
		else if (Simulator::Now() >= q->m_nackTimer || q->m_lastNACK != expected) {
			q->m_nackTimer = Simulator::Now() + MicroSeconds(m_nack_interval);
			q->m_lastNACK = expected;
			if (m_backto0) {
				q->ReceiverNextExpectedSeq = q->ReceiverNextExpectedSeq / m_chunk * m_chunk;
			}
			return 2;
		} else
			return 4;
	} else {
		// Duplicate.
		return 3;
	}
}
void RdmaHw::AddHeader (Ptr<Packet> p, uint16_t protocolNumber) {
	PppHeader ppp;
	ppp.SetProtocol (EtherToPpp (protocolNumber));
	p->AddHeader (ppp);
}
uint16_t RdmaHw::EtherToPpp (uint16_t proto) {
	switch (proto) {
	case 0x0800: return 0x0021;   //IPv4
	case 0x86DD: return 0x0057;   //IPv6
	default: NS_ASSERT_MSG (false, "PPP Protocol number not defined!");
	}
	return 0;
}

void RdmaHw::RecoverQueue(Ptr<RdmaQueuePair> qp) {
	qp->snd_nxt = qp->snd_una;
	std::cout << "Recover triggered" << std::endl;
	// Remove sender's inflight buffer and all the timers
	for (auto it = qp->pktsInflight.begin(); it != qp->pktsInflight.end();){
		std::get<2>(it->second).Cancel(); // cancel the timeout;
		it = qp->pktsInflight.erase(it);
	}
}

void RdmaHw::QpComplete(Ptr<RdmaQueuePair> qp) {
	NS_ASSERT(!m_qpCompleteCallback.IsNull());
	if (m_cc_mode == 1) {
		Simulator::Cancel(qp->mlx.m_eventUpdateAlpha);
		Simulator::Cancel(qp->mlx.m_eventDecreaseRate);
		Simulator::Cancel(qp->mlx.m_rpTimer);
	}
	Simulator::Cancel(qp->timeout);

	// This callback will log info
	// It may also delete the rxQp on the receiver
	m_qpCompleteCallback(qp);

	qp->m_notifyAppFinish();

	// delete the qp
	DeleteQueuePair(qp);
}

void RdmaHw::SetLinkDown(Ptr<QbbNetDevice> dev) {
	printf("RdmaHw: node:%u a link down\n", m_node->GetId());
}

void RdmaHw::AddTableEntry(Ipv4Address &dstAddr, uint32_t intf_idx) {
	uint32_t dip = dstAddr.Get();
	m_rtTable[dip].push_back(intf_idx);
}

void RdmaHw::ClearTable() {
	m_rtTable.clear();
}

void RdmaHw::RedistributeQp() {
	// clear old qpGrp
	for (uint32_t i = 0; i < m_nic.size(); i++) {
		if (m_nic[i].dev == NULL)
			continue;
		m_nic[i].qpGrp->Clear();
	}

	// redistribute qp
	for (auto &it : m_qpMap) {
		Ptr<RdmaQueuePair> qp = it.second;
		uint32_t nic_idx = GetNicIdxOfQp(qp);
		m_nic[nic_idx].qpGrp->AddQp(qp);
		// Notify Nic
		m_nic[nic_idx].dev->ReassignedQp(qp);
	}
}

Ptr<Packet> RdmaHw::GetNxtPacket(Ptr<RdmaQueuePair> qp) {

	uint32_t seqNum = 0;
	uint32_t payload_size = 0;
	bool retransmit = false;
	if (!qp->retransmitQueue.empty()){
		seqNum = qp->retransmitQueue[0].first;
		payload_size = qp->retransmitQueue[0].second;
		retransmit = true;
	}
	else{
		seqNum = qp->snd_nxt;
		payload_size = qp->GetBytesLeft();
		if (m_mtu < payload_size)
			payload_size = m_mtu;
		retransmit = false; 
	}
	Ptr<Packet> p;
	uint32_t sentBytes = qp->m_size - qp->GetBytesLeft();
	uint32_t nic_idx = GetNicIdxOfQp(qp);
	DataRate m_bps = m_nic[nic_idx].dev->GetDataRate();
	double bdp = m_bps.GetBitRate() * 1 * qp->m_baseRtt * 1e-9 / 8;
	UnSchedTag unschedtag;
	if (sentBytes <= bdp){
		unschedtag.SetValue(1);
	}
	else{
		unschedtag.SetValue(0);
	}
	p = Create<Packet> (payload_size);
	p->AddPacketTag(unschedtag);
	// add SeqTsHeader
	SeqTsHeader seqTs;
	seqTs.SetSeq (seqNum);
	seqTs.SetPG (qp->m_pg);
	p->AddHeader (seqTs);
	// add udp header
	UdpHeader udpHeader;
	udpHeader.SetDestinationPort (qp->dport);
	udpHeader.SetSourcePort (qp->sport);
	p->AddHeader (udpHeader);
	// add ipv4 header
	Ipv4Header ipHeader;
	ipHeader.SetSource (qp->sip);
	ipHeader.SetDestination (qp->dip);
	ipHeader.SetProtocol (0x11);
	ipHeader.SetPayloadSize (p->GetSize());
	ipHeader.SetTtl (64);
	ipHeader.SetTos (0);
	if (sourceRouting)
		ipHeader.SetIdentification(qp->pathId);
	else
		ipHeader.SetIdentification (qp->m_ipid);
	p->AddHeader(ipHeader);
	// add ppp header
	PppHeader ppp;
	ppp.SetProtocol (0x0021); // EtherToPpp(0x800), see point-to-point-net-device.cc
	p->AddHeader (ppp);


	// update state
	if(!retransmit) // do not increment snd_nxt for retransmissions
		qp->snd_nxt += payload_size;
	qp->m_ipid++;

	// std::cout << "snd_nxt " << qp->snd_nxt << " size " << payload_size << std::endl;
	if (enableMultiPath){
		if (!retransmit){
			EventId timeoutEvent = Simulator::Schedule(NanoSeconds(rto), &RdmaHw::RetransmitPacket, this, qp, seqNum + payload_size);
			qp->pktsInflight[seqNum + payload_size] = std::make_tuple(payload_size,false, timeoutEvent, 1);
		}
		else{
			uint32_t backoff_exp = std::get<3>(qp->pktsInflight[seqNum + payload_size]);
			std::get<3>(qp->pktsInflight[seqNum + payload_size]) = backoff_exp << 1; // exponential backoff
			EventId timeoutEvent = Simulator::Schedule(NanoSeconds(backoff_exp*rto), &RdmaHw::RetransmitPacket, this, qp, seqNum + payload_size);
			std::get<2>(qp->pktsInflight[seqNum + payload_size]) = timeoutEvent; // update the timeout event
		}
		// qp->pktsInflight.push_back(std::make_tuple(qp->snd_nxt,payload_size,false, timeoutEvent, 1));
		// if (qp->timeout.IsExpired()){
		// 	qp->timeout = Simulator::Schedule(NanoSeconds(rto),&RdmaHw::RecoverQueue,this,qp);
		// }
	}
	// return
	return p;
}

void RdmaHw::RetransmitPacket(Ptr<RdmaQueuePair> qp, uint32_t expectedAckSeq){// expectedAckSeq - payloadsize gives the sequence number from the sender perspective.
	uint32_t payload_size = std::get<0>(qp->pktsInflight[expectedAckSeq]);
	uint32_t seqNum = expectedAckSeq - payload_size;
	qp->retransmitQueue.push_back(std::make_pair(seqNum, payload_size)); // we don't care if the same packet exists in the retransmit queue already
	std::cout << "Retransmit " << " node " << m_node->GetId() << " srcIp " << qp->sip.Get()
				<< " dstIp " << qp->dip.Get() << " srcPort " << qp->sport << " dstPort " <<  qp->dport << std::endl;
	return;
}

void RdmaHw::PktSent(Ptr<RdmaQueuePair> qp, Ptr<Packet> pkt, Time interframeGap) {
	qp->lastPktSize = pkt->GetSize();
//	SeqTsHeader seqTs;
//	pkt->PeekHeader(seqTs);
	uint32_t seq = qp->snd_nxt;
	qp->rates[qp->snd_nxt] = Simulator::Now().GetNanoSeconds();
	UpdateNextAvail(qp, interframeGap, pkt->GetSize());

}

void RdmaHw::UpdateNextAvail(Ptr<RdmaQueuePair> qp, Time interframeGap, uint32_t pkt_size) {
	Time sendingTime;
	if (m_rateBound)
		sendingTime = interframeGap + qp->m_rate.CalculateBytesTxTime(pkt_size);
	else
		sendingTime = interframeGap + qp->m_max_rate.CalculateBytesTxTime(pkt_size);
	qp->m_nextAvail = Simulator::Now() + sendingTime;
}

void RdmaHw::ChangeRate(Ptr<RdmaQueuePair> qp, DataRate new_rate) {
#if 1
	Time sendingTime = qp->m_rate.CalculateBytesTxTime(qp->lastPktSize);
	Time new_sendintTime = new_rate.CalculateBytesTxTime(qp->lastPktSize);
	qp->m_nextAvail = qp->m_nextAvail + new_sendintTime - sendingTime;
	// update nic's next avail event
	uint32_t nic_idx = GetNicIdxOfQp(qp);
	m_nic[nic_idx].dev->UpdateNextAvail(qp->m_nextAvail);
#endif
	// change to new rate
	qp->m_rate = new_rate;
}

#define PRINT_LOG 0
/******************************
 * Mellanox's version of DCQCN
 *****************************/
void RdmaHw::UpdateAlphaMlx(Ptr<RdmaQueuePair> q) {
#if PRINT_LOG
	//std::cout << Simulator::Now() << " alpha update:" << m_node->GetId() << ' ' << q->mlx.m_alpha << ' ' << (int)q->mlx.m_alpha_cnp_arrived << '\n';
	//printf("%lu alpha update: %08x %08x %u %u %.6lf->", Simulator::Now().GetTimeStep(), q->sip.Get(), q->dip.Get(), q->sport, q->dport, q->mlx.m_alpha);
#endif
	if (q->mlx.m_alpha_cnp_arrived) {
		q->mlx.m_alpha = (1 - m_g) * q->mlx.m_alpha + m_g; 	//binary feedback
	} else {
		q->mlx.m_alpha = (1 - m_g) * q->mlx.m_alpha; 	//binary feedback
	}
#if PRINT_LOG
	//printf("%.6lf\n", q->mlx.m_alpha);
#endif
	q->mlx.m_alpha_cnp_arrived = false; // clear the CNP_arrived bit
	ScheduleUpdateAlphaMlx(q);
}
void RdmaHw::ScheduleUpdateAlphaMlx(Ptr<RdmaQueuePair> q) {
	q->mlx.m_eventUpdateAlpha = Simulator::Schedule(MicroSeconds(m_alpha_resume_interval), &RdmaHw::UpdateAlphaMlx, this, q);
}

void RdmaHw::cnp_received_mlx(Ptr<RdmaQueuePair> q) {
	q->mlx.m_alpha_cnp_arrived = true; // set CNP_arrived bit for alpha update
	q->mlx.m_decrease_cnp_arrived = true; // set CNP_arrived bit for rate decrease
	if (q->mlx.m_first_cnp) {
		// init alpha
		q->mlx.m_alpha = 1;
		q->mlx.m_alpha_cnp_arrived = false;
		// schedule alpha update
		ScheduleUpdateAlphaMlx(q);
		// schedule rate decrease
		ScheduleDecreaseRateMlx(q, 1); // add 1 ns to make sure rate decrease is after alpha update
		// set rate on first CNP
		q->mlx.m_targetRate = q->m_rate = m_rateOnFirstCNP * q->m_rate;
		q->mlx.m_first_cnp = false;
	}
}

void RdmaHw::CheckRateDecreaseMlx(Ptr<RdmaQueuePair> q) {
	ScheduleDecreaseRateMlx(q, 0);
	if (q->mlx.m_decrease_cnp_arrived) {
#if PRINT_LOG
		printf("%lu rate dec: %08x %08x %u %u (%0.3lf %.3lf)->", Simulator::Now().GetTimeStep(), q->sip.Get(), q->dip.Get(), q->sport, q->dport, q->mlx.m_targetRate.GetBitRate() * 1e-9, q->m_rate.GetBitRate() * 1e-9);
#endif
		bool clamp = true;
		if (!m_EcnClampTgtRate) {
			if (q->mlx.m_rpTimeStage == 0)
				clamp = false;
		}
		if (clamp)
			q->mlx.m_targetRate = q->m_rate;
		q->m_rate = std::max(m_minRate, q->m_rate * (1 - q->mlx.m_alpha / 2));
		// reset rate increase related things
		q->mlx.m_rpTimeStage = 0;
		q->mlx.m_decrease_cnp_arrived = false;
		Simulator::Cancel(q->mlx.m_rpTimer);
		q->mlx.m_rpTimer = Simulator::Schedule(MicroSeconds(m_rpgTimeReset), &RdmaHw::RateIncEventTimerMlx, this, q);
#if PRINT_LOG
		printf("(%.3lf %.3lf)\n", q->mlx.m_targetRate.GetBitRate() * 1e-9, q->m_rate.GetBitRate() * 1e-9);
#endif
	}
}
void RdmaHw::ScheduleDecreaseRateMlx(Ptr<RdmaQueuePair> q, uint32_t delta) {
	q->mlx.m_eventDecreaseRate = Simulator::Schedule(MicroSeconds(m_rateDecreaseInterval) + NanoSeconds(delta), &RdmaHw::CheckRateDecreaseMlx, this, q);
}

void RdmaHw::RateIncEventTimerMlx(Ptr<RdmaQueuePair> q) {
	q->mlx.m_rpTimer = Simulator::Schedule(MicroSeconds(m_rpgTimeReset), &RdmaHw::RateIncEventTimerMlx, this, q);
	RateIncEventMlx(q);
	q->mlx.m_rpTimeStage++;
}
void RdmaHw::RateIncEventMlx(Ptr<RdmaQueuePair> q) {
	// check which increase phase: fast recovery, active increase, hyper increase
	if (q->mlx.m_rpTimeStage < m_rpgThreshold) { // fast recovery
		FastRecoveryMlx(q);
	} else if (q->mlx.m_rpTimeStage == m_rpgThreshold) { // active increase
		ActiveIncreaseMlx(q);
	} else { // hyper increase
		HyperIncreaseMlx(q);
	}
}

void RdmaHw::FastRecoveryMlx(Ptr<RdmaQueuePair> q) {
#if PRINT_LOG
	printf("%lu fast recovery: %08x %08x %u %u (%0.3lf %.3lf)->", Simulator::Now().GetTimeStep(), q->sip.Get(), q->dip.Get(), q->sport, q->dport, q->mlx.m_targetRate.GetBitRate() * 1e-9, q->m_rate.GetBitRate() * 1e-9);
#endif
	q->m_rate = (q->m_rate / 2) + (q->mlx.m_targetRate / 2);
#if PRINT_LOG
	printf("(%.3lf %.3lf)\n", q->mlx.m_targetRate.GetBitRate() * 1e-9, q->m_rate.GetBitRate() * 1e-9);
#endif
}
void RdmaHw::ActiveIncreaseMlx(Ptr<RdmaQueuePair> q) {
#if PRINT_LOG
	printf("%lu active inc: %08x %08x %u %u (%0.3lf %.3lf)->", Simulator::Now().GetTimeStep(), q->sip.Get(), q->dip.Get(), q->sport, q->dport, q->mlx.m_targetRate.GetBitRate() * 1e-9, q->m_rate.GetBitRate() * 1e-9);
#endif
	// get NIC
	uint32_t nic_idx = GetNicIdxOfQp(q);
	Ptr<QbbNetDevice> dev = m_nic[nic_idx].dev;
	// increate rate
	q->mlx.m_targetRate += m_rai;
	if (q->mlx.m_targetRate > dev->GetDataRate())
		q->mlx.m_targetRate = dev->GetDataRate();
	q->m_rate = (q->m_rate / 2) + (q->mlx.m_targetRate / 2);
#if PRINT_LOG
	printf("(%.3lf %.3lf)\n", q->mlx.m_targetRate.GetBitRate() * 1e-9, q->m_rate.GetBitRate() * 1e-9);
#endif
}
void RdmaHw::HyperIncreaseMlx(Ptr<RdmaQueuePair> q) {
#if PRINT_LOG
	printf("%lu hyper inc: %08x %08x %u %u (%0.3lf %.3lf)->", Simulator::Now().GetTimeStep(), q->sip.Get(), q->dip.Get(), q->sport, q->dport, q->mlx.m_targetRate.GetBitRate() * 1e-9, q->m_rate.GetBitRate() * 1e-9);
#endif
	// get NIC
	uint32_t nic_idx = GetNicIdxOfQp(q);
	Ptr<QbbNetDevice> dev = m_nic[nic_idx].dev;
	// increate rate
	q->mlx.m_targetRate += m_rhai;
	if (q->mlx.m_targetRate > dev->GetDataRate())
		q->mlx.m_targetRate = dev->GetDataRate();
	q->m_rate = (q->m_rate / 2) + (q->mlx.m_targetRate / 2);
#if PRINT_LOG
	printf("(%.3lf %.3lf)\n", q->mlx.m_targetRate.GetBitRate() * 1e-9, q->m_rate.GetBitRate() * 1e-9);
#endif
}

/***********************
 * High Precision CC
 ***********************/
void RdmaHw::HandleAckHp(Ptr<RdmaQueuePair> qp, Ptr<Packet> p, CustomHeader &ch) {
	uint32_t ack_seq = ch.ack.seq;
	// update rate
	if (ack_seq > qp->hp.m_lastUpdateSeq) { // if full RTT feedback is ready, do full update
		if (PowerTCPEnabled) {
			UpdateRatePower(qp, p, ch, false);
		}
		else
			UpdateRateHp(qp, p, ch, false);
	} else { // do fast react
		if (PowerTCPEnabled)
			FastReactPower(qp, p, ch);
		else
			FastReactHp(qp, p, ch);
	}
}

void RdmaHw::UpdateRateHp(Ptr<RdmaQueuePair> qp, Ptr<Packet> p, CustomHeader &ch, bool fast_react) {
	uint32_t next_seq = qp->snd_nxt;
	bool print = !fast_react || true;


	if (qp->hp.m_lastUpdateSeq == 0) { // first RTT

		qp->hp.m_lastUpdateSeq = next_seq;
		// store INT
		IntHeader &ih = ch.ack.ih;
		NS_ASSERT(ih.nhop <= IntHeader::maxHop);
		for (uint32_t i = 0; i < ih.nhop; i++)
			qp->hp.hop[i] = ih.hop[i];
#if PRINT_LOG
		if (print) {
			printf("%lu %s %08x %08x %u %u [%u,%u,%u]", Simulator::Now().GetTimeStep(), fast_react ? "fast" : "update", qp->sip.Get(), qp->dip.Get(), qp->sport, qp->dport, qp->hp.m_lastUpdateSeq, ch.ack.seq, next_seq);
			for (uint32_t i = 0; i < ih.nhop; i++)
				printf(" %u %lu %lu", ih.hop[i].GetQlen(), ih.hop[i].GetBytes(), ih.hop[i].GetTime());
			printf("\n");
		}
#endif
	} else {
		// check packet INT
		IntHeader &ih = ch.ack.ih;
		if (ih.nhop <= IntHeader::maxHop) {
			double max_c = 0;
			bool inStable = false;
#if PRINT_LOG
			if (print)
				printf("%lu %s %08x %08x %u %u [%u,%u,%u]", Simulator::Now().GetTimeStep(), fast_react ? "fast" : "update", qp->sip.Get(), qp->dip.Get(), qp->sport, qp->dport, qp->hp.m_lastUpdateSeq, ch.ack.seq, next_seq);
#endif
			// check each hop
			double U = 0;
			uint64_t dt = 0;
			bool updated[IntHeader::maxHop] = {false}, updated_any = false;
			NS_ASSERT(ih.nhop <= IntHeader::maxHop);
			for (uint32_t i = 0; i < ih.nhop; i++) {
				if (m_sampleFeedback) {
					if (ih.hop[i].GetQlen() == 0 and fast_react)
						continue;
				}
				updated[i] = updated_any = true;
#if PRINT_LOG
				if (print)
					printf(" %u(%u) %lu(%lu) %lu(%lu)", ih.hop[i].GetQlen(), qp->hp.hop[i].GetQlen(), ih.hop[i].GetBytes(), qp->hp.hop[i].GetBytes(), ih.hop[i].GetTime(), qp->hp.hop[i].GetTime());
#endif
				uint64_t tau = ih.hop[i].GetTimeDelta(qp->hp.hop[i]);
				double duration = tau * 1e-9;
				double txRate = (ih.hop[i].GetBytesDelta(qp->hp.hop[i])) * 8 / duration;

				double u;
				u = txRate / ih.hop[i].GetLineRate() + (double)std::min(ih.hop[i].GetQlen(), qp->hp.hop[i].GetQlen()) * qp->m_max_rate.GetBitRate() / ih.hop[i].GetLineRate() / qp->m_win;


#if PRINT_LOG
				if (print)
					printf(" %.3lf %.3lf", txRate, u);
#endif
				if (!m_multipleRate) {
					// for aggregate (single R)
					if (u > U) {
						U = u;
						dt = tau;
					}
				} else {
					// for per hop (per hop R)
					if (tau > qp->m_baseRtt)
						tau = qp->m_baseRtt;
					qp->hp.hopState[i].u = (qp->hp.hopState[i].u * (qp->m_baseRtt - tau) + u * tau) / double(qp->m_baseRtt);
				}
				qp->hp.hop[i] = ih.hop[i];
			}

			DataRate new_rate;
			int32_t new_incStage;
			DataRate new_rate_per_hop[IntHeader::maxHop];
			int32_t new_incStage_per_hop[IntHeader::maxHop];
			if (!m_multipleRate) {
				// for aggregate (single R)
				if (updated_any) {
					if (dt > 1.0 * qp->m_baseRtt)
						dt = 1.0 * qp->m_baseRtt;


					qp->hp.u = (qp->hp.u * (qp->m_baseRtt - dt) + U * dt) / double(qp->m_baseRtt);
					max_c = qp->hp.u / m_targetUtil;

					if (max_c >= 1 || qp->hp.m_incStage >= m_miThresh) {
						new_rate = qp->hp.m_curRate / max_c + m_rai;
						new_incStage = 0;
					}
					else {
						new_rate = qp->hp.m_curRate + m_rai;
						new_incStage = qp->hp.m_incStage + 1;
					}

					if (new_rate < m_minRate)
						new_rate = m_minRate;
					if (new_rate > qp->m_max_rate)
						new_rate = qp->m_max_rate;
#if PRINT_LOG
					if (print)
						printf(" u=%.6lf U=%.3lf dt=%u max_c=%.3lf", qp->hp.u, U, dt, max_c);
#endif
#if PRINT_LOG
					if (print)
						printf(" rate:%.3lf->%.3lf\n", qp->hp.m_curRate.GetBitRate() * 1e-9, new_rate.GetBitRate() * 1e-9);
#endif
				}
			} else {
				// for per hop (per hop R)
				new_rate = qp->m_max_rate;
				for (uint32_t i = 0; i < ih.nhop; i++) {
					if (updated[i]) {
						double c = qp->hp.hopState[i].u / m_targetUtil;
						if (c >= 1 || qp->hp.hopState[i].incStage >= m_miThresh) {
							new_rate_per_hop[i] = qp->hp.hopState[i].Rc / c + m_rai;
							new_incStage_per_hop[i] = 0;
						} else {
							new_rate_per_hop[i] = qp->hp.hopState[i].Rc + m_rai;
							new_incStage_per_hop[i] = qp->hp.hopState[i].incStage + 1;
						}
						// bound rate
						if (new_rate_per_hop[i] < m_minRate)
							new_rate_per_hop[i] = m_minRate;
						if (new_rate_per_hop[i] > qp->m_max_rate)
							new_rate_per_hop[i] = qp->m_max_rate;
						// find min new_rate
						if (new_rate_per_hop[i] < new_rate)
							new_rate = new_rate_per_hop[i];
#if PRINT_LOG
						if (print)
							printf(" [%u]u=%.6lf c=%.3lf", i, qp->hp.hopState[i].u, c);
#endif
#if PRINT_LOG
						if (print)
							printf(" %.3lf->%.3lf", qp->hp.hopState[i].Rc.GetBitRate() * 1e-9, new_rate.GetBitRate() * 1e-9);
#endif
					} else {
						if (qp->hp.hopState[i].Rc < new_rate)
							new_rate = qp->hp.hopState[i].Rc;
					}
				}
#if PRINT_LOG
				printf("\n");
#endif
			}

			if (updated_any) {
				ChangeRate(qp, new_rate);
			}
			if (!fast_react) {
				if (updated_any) {
					qp->hp.m_curRate = new_rate;
					qp->hp.m_incStage = new_incStage;
				}
				if (m_multipleRate) {
					// for per hop (per hop R)
					for (uint32_t i = 0; i < ih.nhop; i++) {
						if (updated[i]) {
							qp->hp.hopState[i].Rc = new_rate_per_hop[i];
							qp->hp.hopState[i].incStage = new_incStage_per_hop[i];
						}
					}
				}
			}
		}
		if (!fast_react) {
			if (next_seq > qp->hp.m_lastUpdateSeq)
				qp->hp.m_lastUpdateSeq = next_seq; //+ rand() % 2 * m_mtu;
		}
	}
}

void RdmaHw::FastReactHp(Ptr<RdmaQueuePair> qp, Ptr<Packet> p, CustomHeader &ch) {
	if (m_fast_react)
		UpdateRateHp(qp, p, ch, true);
}


/**********************
 * PowerTCP (Int/Delay versions) called from HandleAckHp function at the moment
 *********************/

void RdmaHw::UpdateRatePower(Ptr<RdmaQueuePair> qp, Ptr<Packet> p, CustomHeader &ch, bool fast_react) {
	uint32_t next_seq = qp->snd_nxt;
	bool print = !fast_react || true;
	double prevRtt = qp->m_baseRtt;
	double prevCompletion = Simulator::Now().GetNanoSeconds();
	std::map<uint32_t, double>::iterator it = qp->rates.find(ch.ack.seq);
	DataRate old ;
	double rtt;

	if (it != qp->rates.end()) {
		prevRtt = Simulator::Now().GetNanoSeconds() - it->second;
		if (PowerTCPdelay) {
			qp->m_baseRtt = std::min(uint64_t(Simulator::Now().GetNanoSeconds() - it->second), qp->m_baseRtt);
		}
		prevCompletion = Simulator::Now().GetNanoSeconds();
        qp->rates.erase(it);
	}
	if (qp->hp.m_lastUpdateSeq == 0 && !PowerTCPdelay) {
		qp->prevRtt = prevRtt;
		qp->prevCompletion = Simulator::Now().GetNanoSeconds();
		qp->hp.m_lastUpdateSeq = next_seq;
		// store INT
		IntHeader &ih = ch.ack.ih;
		NS_ASSERT(ih.nhop <= IntHeader::maxHop);
		for (uint32_t i = 0; i < ih.nhop; i++)
			qp->hp.hop[i] = ih.hop[i];
	}else {
		// check packet INT
		IntHeader &ih = ch.ack.ih;
		if (ih.nhop <= IntHeader::maxHop) {
			double max_c = 0;
			bool inStable = false;
			// check each hop
			double U = 0;
			uint64_t dt = 0;
			bool updated[IntHeader::maxHop] = {false}, updated_any = false;
			NS_ASSERT(ih.nhop <= IntHeader::maxHop);
			for (uint32_t i = 0; i < ih.nhop; i++) {
				if (m_sampleFeedback) {
					if (ih.hop[i].GetQlen() == 0 and fast_react)
						continue;
				}
				updated[i] = updated_any = true;

				uint64_t tau = ih.hop[i].GetTimeDelta(qp->hp.hop[i]);
				double duration = tau * 1e-9;
				double rxRate = (ih.hop[i].GetBytesDelta(qp->hp.hop[i])) * 8.0 / duration;

				double u;

				if (!PowerTCPdelay) {
					double A = rxRate;
					// double A = txRate + (double(ih.hop[i].GetQlen() * 8.0) - double(qp->hp.hop[i].GetQlen() * 8.0)) / duration;
					double power = ( A ) * (double(ih.hop[i].GetQlen() * 8.0) + ih.hop[i].GetLineRate() * (qp->m_baseRtt * 1e-9));
					double powerx = (power) / (ih.hop[i].GetLineRate() * (ih.hop[i].GetLineRate() * qp->m_baseRtt * 1e-9) );
					u = powerx; // PowerTCP
				}
				else {
					// delay approach
					double A = ( double(prevRtt - qp->prevRtt) / (prevCompletion - qp->prevCompletion) + 1  );
					if (A < 0.5)
						A = 0.5;
					double power = ( A ) * (prevRtt);
					double powerx = (power) / (1.05 * qp->m_baseRtt);
					u = powerx; // theta-PowerTCP
				}
				if (u > U) {
					U = u;
					if (PowerTCPdelay) {
						dt = prevCompletion - qp->prevCompletion;
					}
					else {
						dt = tau;
					}
				}
				qp->hp.hop[i] = ih.hop[i];
			}

			DataRate new_rate;
			int32_t new_incStage;
			DataRate new_rate_per_hop[IntHeader::maxHop];
			int32_t new_incStage_per_hop[IntHeader::maxHop];

			if (updated_any) {
				if (dt > 1.0 * qp->m_baseRtt)
					dt = 1.0 * qp->m_baseRtt;

				if (U < 0) {
					U = qp->hp.u;
				}
				qp->hp.u = (qp->hp.u * (1.0 * qp->m_baseRtt - dt) + U * dt) / double(1.0 * qp->m_baseRtt);
				if (!PowerTCPdelay) {
					max_c = qp->hp.u / m_targetUtil;
					new_rate = (0.9 * ( qp->hp.m_curRate / max_c + DataRate("150Mbps") ) + 0.1 * qp->hp.m_curRate);

				}
				else {
					max_c = qp->hp.u;
					new_rate = (0.7 * ( qp->hp.m_curRate / max_c + DataRate("150Mbps") ) + 0.3 * qp->hp.m_curRate);
				}
				if (new_rate < m_minRate)
					new_rate = m_minRate;
				if (new_rate > qp->m_max_rate)
					new_rate = qp->m_max_rate;
			}
			qp->prevRtt = prevRtt;
			qp->prevCompletion = Simulator::Now().GetNanoSeconds();
			if (updated_any) {
				ChangeRate(qp, new_rate);
			}
			if (!fast_react) {
				if (updated_any) {
					qp->hp.m_curRate = new_rate;
					qp->hp.m_incStage = new_incStage;
				}
			}
		}
		if (!fast_react) {
			if (next_seq > qp->hp.m_lastUpdateSeq)
				qp->hp.m_lastUpdateSeq = next_seq;
		}
	}
}

void RdmaHw::FastReactPower(Ptr<RdmaQueuePair> qp, Ptr<Packet> p, CustomHeader &ch) {
	if (m_fast_react)
		UpdateRatePower(qp, p, ch, true);
}

/**********************
 * TIMELY
 *********************/
void RdmaHw::HandleAckTimely(Ptr<RdmaQueuePair> qp, Ptr<Packet> p, CustomHeader &ch) {
	uint32_t ack_seq = ch.ack.seq;
	// update rate
	if (ack_seq > qp->tmly.m_lastUpdateSeq) { // if full RTT feedback is ready, do full update
		UpdateRateTimely(qp, p, ch, false);
	} else { // do fast react
		FastReactTimely(qp, p, ch);
	}
}
void RdmaHw::UpdateRateTimely(Ptr<RdmaQueuePair> qp, Ptr<Packet> p, CustomHeader &ch, bool us) {
	uint32_t next_seq = qp->snd_nxt;
	uint64_t rtt = Simulator::Now().GetTimeStep() - ch.ack.ih.ts;
	bool print = !us;
	if (qp->tmly.m_lastUpdateSeq != 0) { // not first RTT
		int64_t new_rtt_diff = (int64_t)rtt - (int64_t)qp->tmly.lastRtt;
		double rtt_diff = (1 - m_tmly_alpha) * qp->tmly.rttDiff + m_tmly_alpha * new_rtt_diff;
		double gradient = rtt_diff / m_tmly_minRtt;
		bool inc = false;
		double c = 0;
#if PRINT_LOG
		if (print)
			printf("%lu node:%u rtt:%lu rttDiff:%.0lf gradient:%.3lf rate:%.3lf", Simulator::Now().GetTimeStep(), m_node->GetId(), rtt, rtt_diff, gradient, qp->tmly.m_curRate.GetBitRate() * 1e-9);
#endif
		if (rtt < m_tmly_TLow) {
			inc = true;
		} else if (rtt > m_tmly_THigh) {
			c = 1 - m_tmly_beta * (1 - (double)m_tmly_THigh / rtt);
			inc = false;
		} else if (gradient <= 0) {
			inc = true;
		} else {
			c = 1 - m_tmly_beta * gradient;
			if (c < 0)
				c = 0;
			inc = false;
		}
		if (inc) {
			if (qp->tmly.m_incStage < 5) {
				qp->m_rate = qp->tmly.m_curRate + m_rai;
			} else {
				qp->m_rate = qp->tmly.m_curRate + m_rhai;
			}
			if (qp->m_rate > qp->m_max_rate)
				qp->m_rate = qp->m_max_rate;
			if (!us) {
				qp->tmly.m_curRate = qp->m_rate;
				qp->tmly.m_incStage++;
				qp->tmly.rttDiff = rtt_diff;
			}
		} else {
			qp->m_rate = std::max(m_minRate, qp->tmly.m_curRate * c);
			if (!us) {
				qp->tmly.m_curRate = qp->m_rate;
				qp->tmly.m_incStage = 0;
				qp->tmly.rttDiff = rtt_diff;
			}
		}
#if PRINT_LOG
		if (print) {
			printf(" %c %.3lf\n", inc ? '^' : 'v', qp->m_rate.GetBitRate() * 1e-9);
		}
#endif
	}
	if (!us && next_seq > qp->tmly.m_lastUpdateSeq) {
		qp->tmly.m_lastUpdateSeq = next_seq;
		// update
		qp->tmly.lastRtt = rtt;
	}
}
void RdmaHw::FastReactTimely(Ptr<RdmaQueuePair> qp, Ptr<Packet> p, CustomHeader &ch) {
}


/**********************
 * DCTCP
 *********************/
void RdmaHw::HandleAckDctcp(Ptr<RdmaQueuePair> qp, Ptr<Packet> p, CustomHeader &ch) {
	uint32_t ack_seq = ch.ack.seq;
	uint8_t cnp = (ch.ack.flags >> qbbHeader::FLAG_CNP) & 1;
	bool new_batch = false;

	// update alpha
	qp->dctcp.m_ecnCnt += (cnp > 0);
	if (ack_seq > qp->dctcp.m_lastUpdateSeq) { // if full RTT feedback is ready, do alpha update
#if PRINT_LOG
		printf("%lu %s %08x %08x %u %u [%u,%u,%u] %.3lf->", Simulator::Now().GetTimeStep(), "alpha", qp->sip.Get(), qp->dip.Get(), qp->sport, qp->dport, qp->dctcp.m_lastUpdateSeq, ch.ack.seq, qp->snd_nxt, qp->dctcp.m_alpha);
#endif
		new_batch = true;
		if (qp->dctcp.m_lastUpdateSeq == 0) { // first RTT
			qp->dctcp.m_lastUpdateSeq = qp->snd_nxt;
			qp->dctcp.m_batchSizeOfAlpha = qp->snd_nxt / m_mtu + 1;
		} else {
			double frac = std::min(1.0, double(qp->dctcp.m_ecnCnt) / qp->dctcp.m_batchSizeOfAlpha);
			qp->dctcp.m_alpha = (1 - m_g) * qp->dctcp.m_alpha + m_g * frac;
			qp->dctcp.m_lastUpdateSeq = qp->snd_nxt;
			qp->dctcp.m_ecnCnt = 0;
			qp->dctcp.m_batchSizeOfAlpha = (qp->snd_nxt - ack_seq) / m_mtu + 1;
#if PRINT_LOG
			printf("%.3lf F:%.3lf", qp->dctcp.m_alpha, frac);
#endif
		}
#if PRINT_LOG
		printf("\n");
#endif
	}

	// check cwr exit
	if (qp->dctcp.m_caState == 1) {
		if (ack_seq > qp->dctcp.m_highSeq)
			qp->dctcp.m_caState = 0;
	}

	// check if need to reduce rate: ECN and not in CWR
	if (cnp && qp->dctcp.m_caState == 0) {
#if PRINT_LOG
		printf("%lu %s %08x %08x %u %u %.3lf->", Simulator::Now().GetTimeStep(), "rate", qp->sip.Get(), qp->dip.Get(), qp->sport, qp->dport, qp->m_rate.GetBitRate() * 1e-9);
#endif
		qp->m_rate = std::max(m_minRate, qp->m_rate * (1 - qp->dctcp.m_alpha / 2));
#if PRINT_LOG
		printf("%.3lf\n", qp->m_rate.GetBitRate() * 1e-9);
#endif
		qp->dctcp.m_caState = 1;
		qp->dctcp.m_highSeq = qp->snd_nxt;
	}

	// additive inc
	if (qp->dctcp.m_caState == 0 && new_batch)
		qp->m_rate = std::min(qp->m_max_rate, qp->m_rate + m_dctcp_rai);
}

/*********************
 * HPCC-PINT
 ********************/
void RdmaHw::SetPintSmplThresh(double p) {
	pint_smpl_thresh = (uint32_t)(65536 * p);
}
void RdmaHw::HandleAckHpPint(Ptr<RdmaQueuePair> qp, Ptr<Packet> p, CustomHeader &ch) {
	uint32_t ack_seq = ch.ack.seq;
	if (rand() % 65536 >= pint_smpl_thresh)
		return;
	// update rate
	if (ack_seq > qp->hpccPint.m_lastUpdateSeq) { // if full RTT feedback is ready, do full update
		UpdateRateHpPint(qp, p, ch, false);
	} else { // do fast react
		UpdateRateHpPint(qp, p, ch, true);
	}
}

void RdmaHw::UpdateRateHpPint(Ptr<RdmaQueuePair> qp, Ptr<Packet> p, CustomHeader &ch, bool fast_react) {
	uint32_t next_seq = qp->snd_nxt;
	if (qp->hpccPint.m_lastUpdateSeq == 0) { // first RTT
		qp->hpccPint.m_lastUpdateSeq = next_seq;
	} else {
		// check packet INT
		IntHeader &ih = ch.ack.ih;
		double U = Pint::decode_u(ih.GetPower());

		DataRate new_rate;
		int32_t new_incStage;
		double max_c = U / m_targetUtil;

		if (max_c >= 1 || qp->hpccPint.m_incStage >= m_miThresh) {
			new_rate = qp->hpccPint.m_curRate / max_c + m_rai;
			new_incStage = 0;
		} else {
			new_rate = qp->hpccPint.m_curRate + m_rai;
			new_incStage = qp->hpccPint.m_incStage + 1;
		}
		if (new_rate < m_minRate)
			new_rate = m_minRate;
		if (new_rate > qp->m_max_rate)
			new_rate = qp->m_max_rate;
		ChangeRate(qp, new_rate);
		if (!fast_react) {
			qp->hpccPint.m_curRate = new_rate;
			qp->hpccPint.m_incStage = new_incStage;
		}
		if (!fast_react) {
			if (next_seq > qp->hpccPint.m_lastUpdateSeq)
				qp->hpccPint.m_lastUpdateSeq = next_seq; //+ rand() % 2 * m_mtu;
		}
	}
}

}
