#include "gcs_ui/gcs_window.hpp"

GCSWindow::GCSWindow(QWidget * parent)
: QMainWindow(parent)
{
    // 1. Setup UI Layout elements
    QWidget * central_widget = new QWidget(this);
    QVBoxLayout * layout = new QVBoxLayout(central_widget);
    
    status_label_ = new QLabel("Waiting for Drone Heartbeat...", this);
    status_label_->setAlignment(Qt::AlignCenter);
    
    layout->addWidget(status_label_);
    setCentralWidget(central_widget);
    resize(800, 600);
    setWindowTitle("Next-Gen Drone Ground Control Station");

    // 2. Instantiate the isolated GCS ROS2 Node
    ros_node_ = rclcpp::Node::make_shared("gcs_telemetry_node");

    // 3. Create a listener callback mapping directly into our Qt layout logic
    status_sub_ = ros_node_->create_subscription<std_msgs::msg::String>(
        "/drone_01/baseline_status", 10,
        [this](const std_msgs::msg::String::SharedPtr msg) {
            // Marshall data across ROS to Qt formatting structures safely
            QString message_data = QString::fromStdString(msg->data);
            update_status_display(message_data);
        }
    );

    // 4. Setup execution timers so Qt and ROS2 share runtime safely without blocking the UI thread
    ros_timer_ = new QTimer(this);
    connect(ros_timer_, &QTimer::timeout, this, &GCSWindow::spin_ros2_loop);
    ros_timer_->start(10); // Check for incoming ROS messages every 10ms
}

GCSWindow::~GCSWindow()
{
}

void GCSWindow::spin_ros2_loop()
{
    // Non-blocking spin check of the active executor queue
    if (rclcpp::ok()) {
        rclcpp::spin_some(ros_node_);
    }
}

void GCSWindow::update_status_display(const QString & text)
{
    status_label_->setText("Latest Status: " + text);
}
