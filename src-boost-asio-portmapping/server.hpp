/**
 * @file   server.hpp
 * @author Alexey Bochkovskiy <alexeyab84@gmail.com>
 *
 * @brief PortMapping Server
 *
 *
 */
// ----------------------------------------------------------------------------
#ifndef SERVER_HPP
#define SERVER_HPP
// ----------------------------------------------------------------------------
#include "connection.hpp"

// ----------------------------------------------------------------------------

#include <boost/noncopyable.hpp>
#include <boost/aligned_storage.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/thread/thread.hpp>
#include <boost/asio.hpp>
namespace ba = boost::asio;
// ----------------------------------------------------------------------------

#include <vector>

// ----------------------------------------------------------------------------

///
/// Port mapping server class
/// 
///
class T_server : private boost::noncopyable {
	enum { connections_in_memory_pool = 10 };   ///< maximum connections in memory pool
public:
	T_server(ba::io_service& io_service_acceptors, ba::io_service& io_service_executors, 
			   unsigned int thread_num_acceptors, unsigned int thread_num_executors, 
			   unsigned int remote_port, std::string remote_address,
			   unsigned int local_port = 10001, std::string local_interface_address = "");
	~T_server();
	
	// constexpr and the types for memory pool of objects of connections
	typedef boost::aligned_storage<sizeof(T_connection) * connections_in_memory_pool> T_memory_pool; ///< type of memory pool	
	typedef boost::shared_ptr<T_memory_pool> T_memory_pool_ptr;   ///< type of pointer of memory pool 

	/// call to destructors of each of connections in memory pool (uses as shared pointer deleter)
	struct T_memory_pool_deleter {
		void operator()(T_memory_pool *const ptr) { 
			//std::cout << "DELETER!!! \n";
			for(size_t i = 0; i < connections_in_memory_pool; ++i) 
				reinterpret_cast<T_connection *>(ptr)[i].~T_connection(); // only call to destructor
			delete ptr;
		}
	};

private:
	/// Run when new connection is accepted
	void handle_accept(T_memory_pool_ptr memory_pool_ptr, size_t i_connect, const boost::system::error_code& e);
	
	ba::io_service& io_service_acceptors_;  ///< reference to io_service
	ba::io_service& io_service_executors_;  ///< reference to io_service
	ba::io_service::work work_acceptors_;   ///< object to inform the io_service_acceptors_ when it has work to do
	ba::io_service::work work_executors_;   ///< object to inform the io_service_executors_ when it has work to do
	std::vector<boost::thread> thr_grp_acceptors_;  ///< thread pool object for acceptors
	std::vector<boost::thread> thr_grp_executors_;  ///< thread pool object for executors
	const ba::ip::tcp::endpoint local_endpoint_;    ///< object, that points to the connection endpoint of local interface
	ba::ip::tcp::acceptor acceptor_;                ///< object, that accepts new connections
	ba::ip::tcp::resolver::iterator remote_endpoint_it_;   ///< object, that points to the connection endpoint of remote server
};
// ----------------------------------------------------------------------------


// ----------------------------------------------------------------------------
#endif // SERVER_HPP