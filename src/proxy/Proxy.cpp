/* XMRig
 * Copyright 2010      Jeff Garzik <jgarzik@pobox.com>
 * Copyright 2012-2014 pooler      <pooler@litecoinpool.org>
 * Copyright 2014      Lucas Jones <https://github.com/lucasjones>
 * Copyright 2014-2016 Wolf9466    <https://github.com/OhGodAPet>
 * Copyright 2016      Jay D Dee   <jayddee246@gmail.com>
 * Copyright 2016-2017 XMRig       <support@xmrig.com>
 *
 *
 *   This program is free software: you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation, either version 3 of the License, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program. If not, see <http://www.gnu.org/licenses/>.
 */


#include <ctime>
#include <inttypes.h>
#include <memory>


#include "Counters.h"
#include "log/Log.h"
#include "Options.h"
#include "proxy/Miner.h"
#include "proxy/Miners.h"
#include "proxy/Proxy.h"
#include "proxy/Server.h"
#include "proxy/splitters/NonceSplitter.h"


Proxy::Proxy(const Options *options) :
    m_options(options)
{
    std::srand(std::time(0) ^ (uintptr_t) this);

    m_agent = userAgent();
    m_miners = new Miners();
    m_splitter = new NonceSplitter(options, m_agent);

    m_timer.data = this;
    uv_timer_init(uv_default_loop(), &m_timer);
}


Proxy::~Proxy()
{
    free(m_agent);
}


void Proxy::connect()
{
    m_splitter->connect();

    const std::vector<Addr*> &addrs = Options::i()->addrs();
    for (const Addr *addr : addrs) {
        bind(addr->host(), addr->port());
    }

    Counters::start();
    uv_timer_start(&m_timer, Proxy::onTimer, kTickInterval, kTickInterval);
}


void Proxy::printConnections()
{
    m_splitter->printConnections();
}


void Proxy::printHashrate()
{
    LOG_INFO("\x1B[01;32m* \x1B[01;37mspeed\x1B[0m \x1B[01;30m(1m) \x1B[01;36m%03.1f\x1B[0m, \x1B[01;30m(10m) \x1B[01;36m%03.1f\x1B[0m, \x1B[01;30m(1h) \x1B[01;36m%03.1f\x1B[0m, \x1B[01;30m(12h) \x1B[01;36m%03.1f\x1B[0m, \x1B[01;30m(24h) \x1B[01;36m%03.1f KH/s",
             Counters::hashrate(60), Counters::hashrate(600), Counters::hashrate(3600), Counters::hashrate(3600 * 12), Counters::hashrate(3600 * 24));
}


#ifdef APP_DEVEL
void Proxy::printState()
{
    LOG_NOTICE("---------------------------------");
    m_splitter->printState();
    LOG_NOTICE("---------------------------------");
}
#endif


void Proxy::onMinerClose(Miner *miner)
{
    m_miners->remove(miner);

    if (miner->mapperId() >= 0) {
        m_splitter->remove(miner);
    }
}


void Proxy::onMinerLogin(Miner *miner, const LoginRequest &request)
{
    m_splitter->login(miner, request);
}


void Proxy::onMinerSubmit(Miner *miner, const JobResult &request)
{
    m_splitter->submit(miner, request);
}


void Proxy::onNewMinerAccepted(Miner *miner)
{
    miner->setListener(this);
    m_miners->add(miner);
}


void Proxy::bind(const char *ip, uint16_t port)
{
    auto server = new Server(ip, port, this);

    if (server->bind()) {
        m_servers.push_back(server);
    }
    else {
        delete server;
    }
}


void Proxy::gc()
{
    m_splitter->gc();
}


void Proxy::onTimer(uv_timer_t *handle)
{
    static_cast<Proxy*>(handle->data)->gc();

    LOG_INFO("\x1B[01;36m%03.1f KH/s\x1B[0m, shares: \x1B[01;37m%" PRIu64 "\x1B[0m/%s%" PRIu64 "\x1B[0m +%" PRIu64 ", upstreams: \x1B[01;37m%" PRIu64 "\x1B[0m, miners: \x1B[01;37m%" PRIu64 "\x1B[0m (max \x1B[01;37m%" PRIu64 "\x1B[0m) +%u/-%u",
             Counters::hashrate(60), Counters::accepted(), Counters::rejected() ? "\x1B[31m" : "\x1B[01;37m", Counters::rejected(),
             Counters::tick.accepted, Counters::upstreams(), Counters::miners(), Counters::minersMax(), Counters::tick.added, Counters::tick.removed, Counters::minersMax());

    Counters::reset();
}
