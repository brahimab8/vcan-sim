#include <csignal>
#include <iostream>
#include <optional>

#include "monitor_core.h"
#include "monitor_session.h"
#include "socketcan/driver.h"

static MonitorSession* g_session = nullptr;

void stop_handler(int)
{
    if (g_session) g_session->stop();
}

int main(int argc, char** argv)
{
    std::string interface = "vcan0";
    std::optional<std::string> dbc_path = std::nullopt;
    std::optional<std::string> csv_dir = std::optional<std::string>("data/csv");

    if (argc > 1) interface = argv[1];
    if (argc > 2) dbc_path = std::string(argv[2]);

    SocketCanDriver driver(interface);
    MonitorCore core(dbc_path, csv_dir);
    MonitorSession session(driver, core, std::cout);
    g_session = &session;

    std::signal(SIGINT, stop_handler);
    std::signal(SIGTERM, stop_handler);

    std::cout << "can_monitor: listening on " << interface << "\n";
    session.run();

    std::cout << "can_monitor: exiting\n";
    return 0;
}
