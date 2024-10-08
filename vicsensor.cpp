#include <iostream>

#include <sdbusplus/asio/connection.hpp>
#include <boost/asio/io_service.hpp>
#include <sdbusplus/asio/object_server.hpp>
#include <boost/asio/deadline_timer.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>

using namespace std;

void PollAdd(boost::asio::deadline_timer &t,
                shared_ptr<sdbusplus::asio::dbus_interface> &iface, double &temp, bool &created)
{
    t.expires_from_now(boost::posix_time::seconds(1));
    t.async_wait([&](const boost::system::error_code& ec) {
        if (!ec)
        {
            if (created) {
                if (temp < 60){
                    cerr << "temp: " << temp << endl;
                    temp = temp + 1.0;
                }
                else {
                    cerr << "reset temp to 0" << endl;
                    temp = 0;
                }
                iface->set_property("Value", temp);
            }
            PollAdd(t, iface, temp, created);
        }
    });
}

int main(){
    static bool created = false;
    static double temp = 0;

    boost::asio::io_service io;
    auto systemBus = std::make_shared<sdbusplus::asio::connection>(io);
    systemBus->request_name("xyz.openbmc_project.SampleSensor");
    auto server = sdbusplus::asio::object_server(systemBus);

    std::shared_ptr<sdbusplus::asio::dbus_interface> nameIface;
    std::shared_ptr<sdbusplus::asio::dbus_interface> valueIface;
    
    boost::asio::deadline_timer t(io);
        
    std::function<void(sdbusplus::message::message&)>genSensor = 
        [&](sdbusplus::message::message&) {
            if (!created) {
                created = true;

                nameIface =
                server.add_interface("/xyz/openbmc_project/sensors/temperature/SampleSensor", "xyz.openbmc_project.Sensor.Name");

                valueIface =
                server.add_interface("/xyz/openbmc_project/sensors/temperature/SampleSensor", "xyz.openbmc_project.Sensor.Value");

                nameIface->register_property("Name", std::string("SampleSensor"));
                valueIface->register_property("Value", temp);
                nameIface->initialize();
                valueIface->initialize();
                cerr << "createSensor in match \n";
            }
        };

    static sdbusplus::bus::match::match sensorAdded(
        *systemBus,
        "type='signal',member='InterfacesAdded',arg0path='/xyz/openbmc_project/inventory/system/chassis/chassis/SampleSensor'",
        genSensor);
        
    t.expires_from_now(boost::posix_time::seconds(1));
    t.async_wait([&](const boost::system::error_code& ec) {
        if (!ec) { 
            PollAdd(t, valueIface, temp, created);
        }
    });
    io.run();
    return 0;
}