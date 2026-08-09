#include <QApplication>
#include "gcs_ui/gcs_window.hpp"
#include <rclcpp/rclcpp.hpp>

int main(int argc, char * argv[])
{
    // Initialize ROS2 Context
    rclcpp::init(argc, argv);

    // Initialize Qt Application context
    QApplication app(argc, argv);

    // Construct and display your main workspace
    GCSWindow window;
    window.show();

    // Run Qt main execution thread loop
    int result = app.exec();

    // Teardown ROS2 context smoothly on window exit
    rclcpp::shutdown();
    return result;
}
