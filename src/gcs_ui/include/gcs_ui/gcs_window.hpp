#ifndef GCS_UI__GCS_WINDOW_HPP_
#define GCS_UI__GCS_WINDOW_HPP_

#include <QMainWindow>
#include <QTimer>
#include <QLabel>
#include <QVBoxLayout>
#include <rclcpp/rclcpp.hpp>
#include <std_msgs/msg/string.hpp>

class GCSWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit GCSWindow(QWidget * parent = nullptr);
    virtual ~GCSWindow();

private slots:
    // Periodic processing loop to keep ROS2 background queues moving
    void spin_ros2_loop();
    
    // Qt slot to update UI text safely when telemetry drops in
    void update_status_display(const QString & text);

private:
    // ROS2 Infrastructure Handles
    rclcpp::Node::SharedPtr ros_node_;
    rclcpp::Subscription<std_msgs::msg::String>::SharedPtr status_sub_;

    // Qt Window Elements
    QTimer * ros_timer_;
    QLabel * status_label_;
};

#endif  // GCS_UI__GCS_WINDOW_HPP_
