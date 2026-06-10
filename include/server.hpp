#pragma once 
#include <string>
#include <unordered_map>
#include <queue>
#include <memory>
#include "tcpsocket.hpp"
#include "message.hpp"
#include "iprotocol.hpp"
#include "itransport.hpp"
#include "imultiplexer.hpp"
#include "connection.hpp"

/**
 * Server
 *
 * A multi-client TCP server abstraction supporting:
 *  - pluggable transport layer (TCP / TLS)
 *  - pluggable message protocol (framing / decoding)
 *  - pluggable I/O multiplexing strategy
 *
 * The server is event-driven and must be progressed via `tick()`
 * inside an external loop.
 *
 * Each accepted client is represented internally as a Connection
 * mapped by its file descriptor.
 *
 * Incoming data is:
 *  raw bytes → transport → protocol → decoded messages → inbox_
 *
 * The inbox stores Message objects representing:
 *  - client data
 *  - connection events (connect / disconnect)
 */
class Server {
private:
    std::string host;
    std::string port;
    std::string protocol_;
    std::string transport_;
    std::string multistrategy_;
    
    std::unique_ptr<IMultiplexer> multiplexer_;

    int server_socket;
    TCPSocket socket;

    // Active client connections mapped by file descriptor
    std::unordered_map<int, Connection> connections;
    // Deferred removal list (processed after tick iteration)
    std::vector<int> disconnected_fds;

    fd_set master_;
    int max_fd_;

    std::queue<Message> inbox_;
public:
    /**
     * Constructs a server socket bound to (host, port)
     * and prepares it for accepting connections.
     */
    Server(const std::string &, const std::string &, 
           const std::string &, const std::string &, const std::string &);

    /*
     * Initializes multiplexer and registers listening socket.
     */
    void init_();

    /**
     * Advances the server event loop.
     *
     * - waits for I/O readiness via multiplexer
     * - accepts new client connections
     * - reads data from ready clients
     * - decodes messages via protocol
     * - queues messages into inbox_
     * - handles client disconnections
     *
     * @param timeout
     *  < 0 : blocking wait
     *    0 : non-blocking poll
     *  > 0 : timed wait (milliseconds)
     */
    void tick(int timeout=0);

    /**
     * Returns true if at least one message or event is available.
     */
    bool has_message();

    /**
     * Retrieves and removes the next message/event from the inbox.
     */
    Message next();

    /**
     * Sends a message to a specific client.
     *
     * @param fd Target client file descriptor
     * @param message Payload to send
     */
    void send(int, const std::string &); 

    /**
     * Removes a client from the server.
     *
     * The connection will be closed and removed from the multiplexer.
     */
    void kick(int);

    // TODO: void broadcast();

    void accept_client_(int);

    /*
     * Processes incoming data for a specific client.
     */
    void handle_client_(int);

    /*
     * Factory: creates protocol implementation.
     */
    std::unique_ptr<IProtocol> set_protocol_();

    /*
     * Factory: creates transport implementation.
     */
    std::unique_ptr<ITransport> set_transport_(TCPSocket &&);

    /*
     * Factory: creates multiplexer implementation.
     */
    void set_multiplexer_();
};
