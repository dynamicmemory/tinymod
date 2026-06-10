#pragma once 
#include <string>
#include <queue>
#include <memory>
#include "iprotocol.hpp"
#include "tcpsocket.hpp"
#include "message.hpp"
#include "connection.hpp"
#include "itransport.hpp"
#include "imultiplexer.hpp"

/*
 * Client 
 * 
 * A network client abstraction build on top of:
 * - a transport layer (TCP /TLS)
 * - a message protocol (framing/decoding strategy)
 * - a multiplexing strategy (I/O readiness handling)
 * 
 * The Client is event-driven and must be progressed via `tick()`
 * inside an external event loop.
 *
 * Received messages are decoded into discrete Message objects
 * and stored internally in a queue (inbox_).
 */
class Client {
private:
    TCPSocket socket; 
    std::string host;
    std::string port;
    std::string protocol_;
    std::string transport_;
    std::string multistrategy_;

    std::unique_ptr<IMultiplexer> multiplexer_;

    fd_set master_;

    Connection connection_; 
    bool connected_;
    std::queue<Message> inbox_;
public:
    /**
     * Constructs a client and immediately connects to the remote endpoint.
     *
     * @param host Remote host address
     * @param port Remote port
     * @param protocol Protocol type identifier (e.g. "default", "newline")
     * @param transport Transport type identifier (e.g. "tcp", "tls")
     * @param multiplexer Multiplexing strategy identifier (e.g. "select")
     */
    Client(const std::string &, const std::string &, 
           const std::string &, const std::string &, const std::string &);
    void init_();

    /**
     * Advances the client state machine.
     *
     * - Waits for socket readiness using the multiplexer
     * - Reads raw bytes from transport
     * - Decodes protocol frames into messages
     * - Pushes decoded messages into inbox_
     *
     * @param timeout
     *  < 0 : blocking wait
     *    0 : non-blocking poll
     *  > 0 : timed wait (milliseconds)
     */
    void tick(int timeout=0);

    /**
     * Returns true if at least one decoded message is available.
     */
    bool has_message();

    /**
     * Retrieves and removes the next decoded message from the inbox.
     */
    Message next();

    /**
     * Returns true if the underlying connection is still active.
     */
    bool is_connected();

    /**
     * Sends a message to the connected peer.
     *
     * The message is encoded using the configured protocol
     * and transmitted via the configured transport.
     */
    void send(const std::string &); 

    void broadcast();
    
    std::unique_ptr<IProtocol> set_protocol_();
    std::unique_ptr<ITransport> set_transport_(TCPSocket &&);
    void set_multiplexer_();

    /**
     * Returns true if the transport layer is ready for I/O.
     */
    bool is_ready();

    /**
     * Verifies the server TLS certificate against a provided reference.
     *
     * Only valid when using a TLS transport.
     *
     * @throws std::runtime_error if transport is not TLS or validation fails
     */
    void verify_certificate(std::string &);
};
