
/* implementation for the non-template base, for the connection<> template class */

#include "abstract_tcp_server2-connection_core.precompile.hpp"
// TODO precompile is not working well yet


#include "../../src/epee/network_throttle-advanced.h"


// ################################################################################################
// ################################################################################################
// the "header part". Not separeted out for .hpp because point of this modification is 
// to rebuild just 1 translation unit while working on this code.
// (But maybe common parts will be separated out later though - if needed)
// ################################################################################################
// ################################################################################################

namespace epee
{
namespace net_utils
{


/* ============================================================================ */

class connection_basic_pimpl {
	public:
		connection_basic_pimpl(const std::string &name);

		static int m_default_tos;

		network_throttle_bw m_throttle; // per-perr
    critical_section m_throttle_lock;

		void sleep_before_packet(size_t packet_size, int phase); // execute a sleep ; phase is not really used now(?) could be used for different kinds of sleep e.g. direct/queue write
};


} // namespace
} // namespace






// ################################################################################################
// ################################################################################################
// The implementation part
// ################################################################################################
// ################################################################################################

namespace epee
{
namespace net_utils
{

// ================================================================================================
// connection_basic_pimpl
// ================================================================================================
	
connection_basic_pimpl::connection_basic_pimpl(const std::string &name) : m_throttle(name) { }

// ================================================================================================
// network_throttle
// ================================================================================================


network_throttle::~network_throttle() { }

network_throttle::packet_info::packet_info() 
	: m_size(0)
{ 
}

network_throttle::network_throttle(const std::string &name, int window_size)
	: m_window_size( (window_size==-1) ? 10 : window_size  ),
	  m_history( m_window_size )
{
	set_name(name);
	m_network_add_cost = 128;
	m_network_minimal_segment = 256;
	m_network_max_segment = 1024*1024;
	m_any_packet_yet = false;
	m_slot_size = 1.0; // hard coded in few places
	m_target_speed = 16 * 1024; // other defaults are probably defined in the command-line parsing code when this class is used e.g. as main global throttle
}

void network_throttle::set_name(const std::string &name) 
{
	m_name = name;
}

void network_throttle::set_target_speed( network_speed_kbps target ) 
{
	m_target_speed = target;
}
			
void network_throttle::tick()
{
	double time_now = get_time_seconds();
	if (!m_any_packet_yet) m_start_time = time_now; // starting now

	network_time_seconds current_sample_time_slot = time_to_slot( time_now ); // T=13.7 --> 13  (for 1-second smallwindow)
	network_time_seconds last_sample_time_slot = time_to_slot( m_last_sample_time );

	// moving to next position, and filling gaps
	// !! during this loop the m_last_sample_time and last_sample_time_slot mean the variable moved in +1
	// TODO optimize when moving few slots at once
	while ( (!m_any_packet_yet) || (last_sample_time_slot < current_sample_time_slot))
	{
		LOG_PRINT_L4("Moving counter buffer by 1 second " << last_sample_time_slot << " < " << current_sample_time_slot << " (last time " << m_last_sample_time<<")");
		// rotate buffer 
		for (size_t i=m_history.size()-1; i>=1; --i) m_history[i] = m_history[i-1];
		m_history[0] = packet_info();
		if (! m_any_packet_yet) 
		{
			m_last_sample_time = time_now;	
		}
		m_last_sample_time += 1;	last_sample_time_slot = time_to_slot( m_last_sample_time ); // increase and recalculate time, time slot
		m_any_packet_yet=true;
	}
	m_last_sample_time = time_now; // the real exact last time
}

void network_throttle::handle_trafic_exact(size_t packet_size) 
{
	_handle_trafic_exact(packet_size, packet_size);
}

void network_throttle::_handle_trafic_exact(size_t packet_size, size_t orginal_size)
{
	double A=0, W=0, D=0, R=0;
	tick();
	calculate_times(packet_size, A,W,D,R, false,-1);
	m_history[0].m_size += packet_size;
	LOG_PRINT_L0("Throttle " << m_name << ": packet of ~"<<packet_size<<"b " << " (from "<<orginal_size<<" b)" << " Speed AVG="<<A<<" / " << " Limit="<<m_target_speed<<" bit/sec " );
	//  (window="<<W<<" s)"); // XXX
}

void network_throttle::handle_trafic_tcp(size_t packet_size)
{
	size_t all_size = packet_size + m_network_add_cost;
	all_size = std::max( m_network_minimal_segment , all_size);
	_handle_trafic_exact( all_size , packet_size );
}

void network_throttle::handle_congestion(double overheat) {
	// TODO
}

network_time_seconds network_throttle::get_sleep_time_after_tick(size_t packet_size) {
	tick();
	return get_sleep_time(packet_size);
}

network_time_seconds network_throttle::get_sleep_time(size_t packet_size) const 
{
	double A=0, W=0, D=0, R=0;
	double Dmax;
	calculate_times(packet_size, A,W,D,R, true, -1);              Dmax=D;
	calculate_times(packet_size, A,W,D,R, true, m_window_size/2); Dmax=std::max(D,Dmax);
	calculate_times(packet_size, A,W,D,R, true, 5);               Dmax=std::max(D,Dmax);
	return D;
}

void network_throttle::update_overheat() {
	auto now = get_time_seconds();
	auto diff = now - m_overheat_time;
	m_overheat -= diff;
	m_overheat = std::max(m_overheat, 0.);
	m_overheat_time = now;

	LOG_PRINT_L0("No lag, actual overheat: " << m_overheat << ", diff: " << diff);
}

void network_throttle::calculate_times(size_t packet_size, double &A, double &W, double &D, double &R, bool dbg, double force_window) const 
{
	const double the_window_size = std::min( (double)m_window_size , 
		((force_window>0) ? force_window : m_window_size) 
	);

	if (!m_any_packet_yet) {
		W=0; A=0; D=0; 
		R=0; // should be overrided by caller
		return ; // no packet yet, I can not decide about sleep time
	}

	network_time_seconds window_len = (the_window_size-1) * m_slot_size ; // -1 since current slot is not finished
	window_len += (m_last_sample_time - time_to_slot(m_last_sample_time));  // add the time for current slot e.g. 13.7-13 = 0.7

	auto time_passed = get_time_seconds() - m_start_time;
	W = std::max( std::min( window_len , time_passed ) , m_slot_size )  ; // window length resulting from size of history but limited by how long ago history was started,
	// also at least slot size (e.g. 1 second) to not be ridiculous
	// window_len e.g. 5.7 because takes into account current slot time

	size_t Epast = 0; // summ of traffic till now
	for (auto sample : m_history) Epast += sample.m_size; 
	const size_t E = Epast;
	const size_t Enow = Epast + packet_size ; // including the data we're about to send now

	const double M = m_target_speed; // max
	const double D1 = (Epast - M*W) / M; // delay - how long to sleep to get back to target speed
	const double D2 = (Enow  - M*W) / M; // delay - how long to sleep to get back to target speed (including current packet)
	update_overheat();
	D = (D1*0.75 + D2*0.25) + m_overheat; // finall sleep depends on both with/without current packet

	A = Epast/W; // current avg. speed (for info)

	double Wgood=-1;
	{	// how much data we recommend now to download
		Wgood = the_window_size/2 + 1;
		Wgood = std::min(3., std::max(1., std::min( the_window_size/2., Wgood)));
		if (W > Wgood) { // we have a good enough window to estimate
			R = M*W - E; 
		} else { // now window to estimate, so let's request in this way:
			R = M*1 - E;
		}
	}

	if (dbg) {
		std::ostringstream oss; 
		oss << "["; 
		for (auto sample: m_history) oss << sample.m_size << " ";
		oss << "]" << std::ends;
		std::string history_str = oss.str();
		LOG_PRINT_L0(
			"dbg " << m_name << ": " 
			<< "speed is A=" << std::setw(8) <<A<<" vs "
			<< "Max=" << std::setw(8) <<M<<" "
			<< " so sleep: "
			<< "D=" << std::setw(8) <<D<<" sec "
//			<< "D1=" << std::setw(8) <<D1<<" "
//			<< "D2=" << std::setw(8) <<D2<<" "
			<< "Overheat=" << std::setw(8) <<m_overheat<<" sec "
			<< "Overheat time=" << std::setw(8) <<m_overheat_time<<" sec "
			<< "E="<< std::setw(8) << E << " "
			<< "M=" << std::setw(8) << M <<" W="<< std::setw(8) << W << " "
			<< "R=" << std::setw(8) << R << " Wgood" << std::setw(8) << Wgood << " "
			<< "History: " << std::setw(8) << history_str << " "
			<< "m_last_sample_time=" << std::setw(8) << m_last_sample_time

		);

	}
}

double network_throttle::get_time_seconds() const {
	using namespace boost::chrono;
	auto point = steady_clock::now();
	auto time_from_epoh = point.time_since_epoch();
	auto ms = duration_cast< milliseconds >( time_from_epoh ).count();
	double ms_f = ms;
	return ms_f / 1000.;
}

size_t network_throttle::get_recommended_size_of_planned_transport_window(double force_window) const {
	double A=0,W=0,D=0,R=0;
	network_throttle::calculate_times(0, A, W, D, R, true, force_window);
	R += m_network_add_cost;
	if (R<0) R=0;
	if (R>m_network_max_segment) R=m_network_max_segment;
	size_t RI = (long int)R;
	return RI;
}

size_t network_throttle::get_recommended_size_of_planned_transport() const {
	size_t Rmin;
	Rmin =           get_recommended_size_of_planned_transport_window( -1 );
	Rmin = std::min( get_recommended_size_of_planned_transport_window( m_window_size/2) , Rmin );
	Rmin = std::min( get_recommended_size_of_planned_transport_window( 8              ) , Rmin );
	return Rmin;
}

void network_throttle::set_overheat(double lag) {
	m_overheat += lag;
	m_overheat_time = get_time_seconds();
	LOG_PRINT_L0("Lag: " << lag << ", overheat: " << m_overheat );
}


// ================================================================================================
// connection_basic
// ================================================================================================

// static variables:
int connection_basic_pimpl::m_default_tos;

// methods:
connection_basic::connection_basic(boost::asio::io_service& io_service, i_connection_filter* &pfilter)
	: 
	mI(new connection_basic_pimpl("peer") ),
	strand_(io_service),
	socket_(io_service),
	m_want_close_connection(0), 
	m_was_shutdown(0), 
	m_pfilter(pfilter)
{ 
	/*boost::asio::SettableSocketOption option;// = new boost::asio::SettableSocketOption();
	option.level(IPPROTO_IP);
	option.name(IP_TOS);
	option.value(&tos);
	option.size = sizeof(tos);
	socket_.set_option(option);*/
	// TODO socket options
}

connection_basic::~connection_basic() {
}


void connection_basic::set_rate_up_limit(uint64_t limit) {
	{
	  CRITICAL_REGION_LOCAL(	network_throttle_manager::m_lock_get_global_throttle_out );
		network_throttle_manager::get_global_throttle_out().set_target_speed(limit);
	}
	//	connection_basic_pimpl::m_throttle_global.m_out.set_target_speed(limit);
}

void connection_basic::set_rate_down_limit(uint64_t limit) {
	{
	  CRITICAL_REGION_LOCAL(	network_throttle_manager::m_lock_get_global_throttle_in );
		network_throttle_manager::get_global_throttle_in().set_target_speed(limit);
	}

	{
	  CRITICAL_REGION_LOCAL(	network_throttle_manager::m_lock_get_global_throttle_inreq );
		network_throttle_manager::get_global_throttle_inreq().set_target_speed(limit);
	}
}

void connection_basic::set_rate_limit(uint64_t limit) {
	// TODO
}

void connection_basic::set_rate_autodetect(uint64_t limit) {
	// TODO
	LOG_PRINT_L0("inside connection_basic we set autodetect (this is additional notification)..");
}
 
void connection_basic::set_tos_flag(int tos) {
	connection_basic_pimpl::m_default_tos = tos;
}

void connection_basic_pimpl::sleep_before_packet(size_t packet_size, int phase) {
// XXX LATER XXX
	{
	  CRITICAL_REGION_LOCAL(	network_throttle_manager::m_lock_get_global_throttle_out );
		network_throttle_manager::get_global_throttle_out().handle_trafic_tcp( packet_size ); // increase counter - global

		//epee::critical_region_t<decltype(m_throttle_global_lock)> guard(m_throttle_global_lock); // *** critical *** 
		//m_throttle_global.m_out.handle_trafic_tcp( packet_size ); // increase counter - global
	}

	double delay=0; // will be calculated
	do
	{ // rate limiting
		{ 
	  	CRITICAL_REGION_LOCAL(	network_throttle_manager::m_lock_get_global_throttle_out );
			delay = network_throttle_manager::get_global_throttle_out().get_sleep_time_after_tick( 0 ); // decission from global
			// epee::critical_region_t<decltype(m_throttle_global_lock)> guard(m_throttle_global_lock); // *** critical ***
			// delay = m_throttle_global.m_out.get_sleep_time_after_tick( 0 ); // decission from global
		}

		if (delay > 0) { boost::this_thread::sleep(boost::posix_time::milliseconds( (int)(delay * 1000) ));	}
	} while(delay > 0);
}

void connection_basic::set_start_time() {
	CRITICAL_REGION_LOCAL(	network_throttle_manager::m_lock_get_global_throttle_out );
	m_start_time = network_throttle_manager::get_global_throttle_out().get_time_seconds();
}

void connection_basic::do_send_handler_start(const void* ptr , size_t cb ) {
	mI->sleep_before_packet(cb,1);
	set_start_time();
}

void connection_basic::do_send_handler_delayed(const void* ptr , size_t cb ) {
}

void connection_basic::do_send_handler_write(const void* ptr , size_t cb ) {
}

void connection_basic::do_send_handler_stop(const void* ptr , size_t cb ) {
}

void connection_basic::do_send_handler_after_write( const boost::system::error_code& e, size_t cb ) {
	CRITICAL_REGION_LOCAL(	network_throttle_manager::m_lock_get_global_throttle_out );
	auto sending_time = network_throttle_manager::get_global_throttle_out().get_time_seconds() - m_start_time;
	LOG_PRINT_L0("====== TIME ===== " << sending_time );
	// lag: if current sending time > max sending time
	if(sending_time > 0.1)
		network_throttle_manager::get_global_throttle_out().set_overheat(sending_time);
	//else network_throttle_manager::get_global_throttle_out().setOverheat() ;

}

void connection_basic::do_send_handler_write_from_queue( const boost::system::error_code& e, size_t cb ) {
	mI->sleep_before_packet(cb,2);
	set_start_time();
}

void connection_basic::do_read_handler_start(const boost::system::error_code& e, std::size_t bytes_transferred) { // from read, after read completion
	const size_t packet_size = bytes_transferred;
	{
	  CRITICAL_REGION_LOCAL(	network_throttle_manager::m_lock_get_global_throttle_in );
		network_throttle_manager::get_global_throttle_in().handle_trafic_tcp( packet_size ); // increase counter - global
		// epee::critical_region_t<decltype(mI->m_throttle_global_lock)> guard(mI->m_throttle_global_lock); // *** critical *** 
		// mI->m_t1hrottle_global.m_in.handle_trafic_tcp( packet_size ); // increase counter - global
	}
}

} // namespace
} // namespace
